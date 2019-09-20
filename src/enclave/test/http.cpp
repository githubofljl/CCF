// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../httpserver.h"

#include <doctest/doctest.h>
#include <queue>
#include <string>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

std::vector<uint8_t> post(const std::string& body)
{
  auto req = fmt::format(
    "POST / HTTP/1.1\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: {}\r\n\r\n{}",
    body.size(),
    body);
  return std::vector<uint8_t>(req.begin(), req.end());
}

class StubProc : public enclave::http::MsgProcessor
{
  std::queue<std::vector<uint8_t>> chunks;

public:
  virtual void msg(std::vector<uint8_t> m)
  {
    chunks.emplace(m);
  }

  void expect(std::vector<std::string> msgs)
  {
    for (auto s : msgs)
    {
      if (!chunks.empty())
      {
        CHECK(std::string(chunks.front().begin(), chunks.front().end()) == s);
        chunks.pop();
      }
      else
      {
        CHECK_MESSAGE(false, fmt::format("Did not contain: {}", s));
      }
    }
    CHECK(chunks.size() == 0);
  }
};

TEST_CASE("Complete request")
{
  std::string r("{}");

  StubProc sp;
  enclave::http::Parser p(sp);

  auto req = post(r);
  auto parsed = p.execute(req.data(), req.size());

  sp.expect({r});
}

TEST_CASE("Partial request")
{
  std::string r("{}");

  StubProc sp;
  enclave::http::Parser p(sp);

  auto req = post(r);
  size_t offset = 10;

  auto parsed = p.execute(req.data(), req.size() - offset);
  CHECK(parsed == req.size() - offset);
  parsed = p.execute(req.data() + req.size() - offset, offset);
  CHECK(parsed == offset);

  sp.expect({r});
}

TEST_CASE("Partial body")
{
  std::string r("{\"a_json_key\": \"a_json_value\"}");

  StubProc sp;
  enclave::http::Parser p(sp);

  auto req = post(r);
  size_t offset = 10;

  auto parsed = p.execute(req.data(), req.size() - offset);
  CHECK(parsed == req.size() - offset);

  parsed = p.execute(req.data() + req.size() - offset, offset);
  CHECK(parsed == offset);

  sp.expect({r});
}