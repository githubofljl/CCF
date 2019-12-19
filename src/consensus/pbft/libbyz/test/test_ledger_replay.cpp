// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "Message.h"
#include "Node.h"
#include "Replica.h"
#include "Request.h"
#include "consensus/pbft/pbftinfo.h"
#include "consensus/pbft/pbfttables.h"
#include "consensus/pbft/pbfttypes.h"
#include "host/ledger.h"
#include "kv/test/stub_consensus.h"
#include "network_mock.h"

#include <cstdio>
#include <doctest/doctest.h>

// power of 2 since ringbuffer circuit size depends on total_requests
static constexpr size_t total_requests = 32;

class ExecutionMock
{
public:
  ExecutionMock(size_t init_counter_) : command_counter(init_counter_) {}
  size_t command_counter;
  struct fake_req
  {
    uint8_t rt;
    int64_t ctx;
  };

  ExecCommand exec_command = [](
                               Byz_req* inb,
                               Byz_rep* outb,
                               _Byz_buffer* non_det,
                               int client,
                               bool ro,
                               Seqno total_requests_executed,
                               ByzInfo& info) {
    auto request = reinterpret_cast<fake_req*>(inb->contents);
    info.ctx = request->ctx;
    info.full_state_merkle_root.fill(0);
    info.replicated_state_merkle_root.fill(0);
    info.full_state_merkle_root.data()[0] = request->rt;
    info.replicated_state_merkle_root.data()[0] = request->rt;
    return 0;
  };
};

NodeInfo get_node_info()
{
  std::vector<PrincipalInfo> principal_info;

  PrincipalInfo pi = {
    0,
    (short)(3000),
    "ip",
    "96031a6cbe405894f1c0295881bd3946f0215f95fc40b7f1f0cc89b821c58504",
    "8691c3438859c142a26b5f251b96f39a463799430315d34ce8a4db0d2638f751",
    "name-1",
    true};
  principal_info.emplace_back(pi);

  GeneralInfo gi = {
    2, 0, 0, "generic", 1800000, 5000, 100, 9999250000, 50, principal_info};

  NodeInfo node_info = {
    gi.principal_info[0],
    "0045c65ec31179652c57ae97f50de77e177a939dce74e39d7db51740663afb69",
    gi};

  return node_info;
}

void create_replica(
  std::vector<char>& service_mem, pbft::Store& store, pbft::PbftInfo& pbft_info)
{
  auto node_info = get_node_info();

  replica = new Replica(
    node_info,
    service_mem.data(),
    service_mem.size(),
    Create_Mock_Network(),
    pbft_info,
    store);
  replica->init_state();
  for (auto& pi : node_info.general_info.principal_info)
  {
    if (pi.id != node_info.own_info.id)
    {
      replica->add_principal(pi);
    }
  }
}

TEST_CASE("Test Ledger Replay")
{
  int mem_size = 400 * 8192;
  std::vector<char> service_mem(mem_size, 0);
  ExecutionMock exec_mock(0);

  auto store = std::make_shared<ccf::Store>(
    pbft::replicate_type_pbft, pbft::replicated_tables_pbft);
  auto consensus = std::make_shared<kv::StubConsensus>();
  store->set_consensus(consensus);
  auto& pbft_info = store->create<pbft::PbftInfo>(
    pbft::Tables::PBFT_INFO, kv::SecurityDomain::PUBLIC);
  auto& derived_map = store->create<std::string, std::string>(
    "derived_map", kv::SecurityDomain::PUBLIC);
  auto replica_store = std::make_unique<pbft::Adaptor<ccf::Store>>(store);

  create_replica(service_mem, *replica_store, pbft_info);
  replica->register_exec(exec_mock.exec_command);

  auto write_consensus = std::make_shared<kv::StubConsensus>();
  INFO("Create dummy pre-prepares and write them to ledger");
  {
    auto write_store = std::make_shared<ccf::Store>(
      pbft::replicate_type_pbft, pbft::replicated_tables_pbft);
    write_store->set_consensus(write_consensus);
    auto& write_pbft_info = write_store->create<pbft::PbftInfo>(
      pbft::Tables::PBFT_INFO, kv::SecurityDomain::PUBLIC);
    auto& write_derived_map = write_store->create<std::string, std::string>(
      "derived_map", kv::SecurityDomain::PUBLIC);

    auto write_pbft_store =
      std::make_unique<pbft::Adaptor<ccf::Store>>(write_store);

    LedgerWriter ledger_writer(*write_pbft_store, write_pbft_info);

    Req_queue rqueue;
    for (size_t i = 1; i < total_requests; i++)
    {
      Byz_req req;
      Byz_alloc_request(&req, sizeof(ExecutionMock::fake_req));

      auto fr = reinterpret_cast<ExecutionMock::fake_req*>(req.contents);
      fr->rt = i;
      fr->ctx = i;

      Request* request = (Request*)req.opaque;
      request->request_id() = i;
      request->authenticate(req.size, false);
      request->trim();

      rqueue.append(request);
      size_t num_requests = 1;
      auto pp = std::make_unique<Pre_prepare>(1, i, rqueue, num_requests);

      ccf::Store::Tx tx;
      auto req_view = tx.get_view(write_pbft_info);
      req_view->put(
        0,
        {pbft::InfoType::REQUEST,
         {},
         {0,
          0,
          {},
          {(const uint8_t*)request->contents(),
           (const uint8_t*)request->contents() + request->size()}}});

      auto der_view = tx.get_view(write_derived_map);
      der_view->put("key1", "value1");

      REQUIRE(tx.commit() == kv::CommitSuccess::OK);

      // imitate exec command
      ByzInfo info;
      info.ctx = fr->ctx;
      info.full_state_merkle_root.fill(0);
      info.replicated_state_merkle_root.fill(0);
      info.full_state_merkle_root.data()[0] = fr->rt;
      info.replicated_state_merkle_root.data()[0] = fr->rt;

      pp->set_merkle_roots_and_ctx(
        info.full_state_merkle_root,
        info.replicated_state_merkle_root,
        info.ctx);

      ledger_writer.write_pre_prepare(pp.get());
    }
    // remove the requests that were not processed, only written to the ledger
    replica->big_reqs()->clear();
  }

  INFO("Read the ledger entries and replay them out of order and in order");
  {
    replica->activate_pbft_local_hooks();
    // ledgerenclave work
    std::vector<std::vector<uint8_t>> entries;
    while (true)
    {
      auto ret = write_consensus->replay_data();
      if (!ret.second)
      {
        break;
      }
      // TODO: when deserialise will be called by pbft, in that place pbft will
      // have to also append the write set to the ledger
      entries.emplace_back(ret.first);
    }
    // apply out of order first
    REQUIRE(
      store->deserialise(entries.back()) == kv::DeserialiseSuccess::FAILED);

    ccf::Store::Tx tx;
    auto info_view = tx.get_view(pbft_info);
    auto info = info_view->get(0);
    REQUIRE(!info.has_value());

    REQUIRE(entries.size() > 0);

    Seqno seqno = 1;
    // apply all of the data in order
    for (const auto& entry : entries)
    {
      REQUIRE(store->deserialise(entry) == kv::DeserialiseSuccess::PASS);
      ccf::Store::Tx tx;
      auto info_view = tx.get_view(pbft_info);
      auto info = info_view->get(0);
      REQUIRE(info.has_value());
      if (info.value().type == pbft::InfoType::REQUEST)
      {
        REQUIRE(info.value().request.raw.size() > 0);
      }
      else if (info.value().type == pbft::InfoType::PRE_PREPARE)
      {
        REQUIRE(info.value().pre_prepare.seqno == seqno);
        seqno++;
      }
      // no derived data should have gotten deserialised
      auto der_view = tx.get_view(derived_map);
      auto derived_val = der_view->get("key1");
      REQUIRE(!derived_val.has_value());
    }

    auto last_executed = replica->get_last_executed();
    REQUIRE(last_executed == total_requests - 1);
  }
}