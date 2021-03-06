/*
  Copyright (c) INRIA and Microsoft Corporation. All rights reserved.
  Licensed under the Apache 2.0 License.
*/

#include <inttypes.h>
#include <stdbool.h>
#include "kremlin/internal/types.h"
#include "kremlin/internal/target.h"

#ifndef __FStar_UInt128_Verified_H
#define __FStar_UInt128_Verified_H

#include "FStar_UInt_8_16_32_64.h"


inline static uint64_t
FStar_UInt128___proj__Mkuint128__item__low(FStar_UInt128_uint128 projectee)
{
  return projectee.low;
}

inline static uint64_t
FStar_UInt128___proj__Mkuint128__item__high(FStar_UInt128_uint128 projectee)
{
  return projectee.high;
}

inline static uint64_t FStar_UInt128_constant_time_carry(uint64_t a, uint64_t b)
{
  return (a ^ ((a ^ b) | ((a - b) ^ b))) >> (uint32_t)63U;
}

inline static uint64_t FStar_UInt128_carry(uint64_t a, uint64_t b)
{
  return FStar_UInt128_constant_time_carry(a, b);
}

inline static FStar_UInt128_uint128
FStar_UInt128_add(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low + b.low;
  lit.high = a.high + b.high + FStar_UInt128_carry(a.low + b.low, b.low);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_add_underspec(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low + b.low;
  lit.high = a.high + b.high + FStar_UInt128_carry(a.low + b.low, b.low);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_add_mod(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low + b.low;
  lit.high = a.high + b.high + FStar_UInt128_carry(a.low + b.low, b.low);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_sub(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low - b.low;
  lit.high = a.high - b.high - FStar_UInt128_carry(a.low, a.low - b.low);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_sub_underspec(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low - b.low;
  lit.high = a.high - b.high - FStar_UInt128_carry(a.low, a.low - b.low);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_sub_mod_impl(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low - b.low;
  lit.high = a.high - b.high - FStar_UInt128_carry(a.low, a.low - b.low);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_sub_mod(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  return FStar_UInt128_sub_mod_impl(a, b);
}

inline static FStar_UInt128_uint128
FStar_UInt128_logand(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low & b.low;
  lit.high = a.high & b.high;
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_logxor(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low ^ b.low;
  lit.high = a.high ^ b.high;
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_logor(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.low | b.low;
  lit.high = a.high | b.high;
  return lit;
}

inline static FStar_UInt128_uint128 FStar_UInt128_lognot(FStar_UInt128_uint128 a)
{
  FStar_UInt128_uint128 lit;
  lit.low = ~a.low;
  lit.high = ~a.high;
  return lit;
}

static uint32_t FStar_UInt128_u32_64 = (uint32_t)64U;

inline static uint64_t FStar_UInt128_add_u64_shift_left(uint64_t hi, uint64_t lo, uint32_t s)
{
  return (hi << s) + (lo >> (FStar_UInt128_u32_64 - s));
}

inline static uint64_t
FStar_UInt128_add_u64_shift_left_respec(uint64_t hi, uint64_t lo, uint32_t s)
{
  return FStar_UInt128_add_u64_shift_left(hi, lo, s);
}

inline static FStar_UInt128_uint128
FStar_UInt128_shift_left_small(FStar_UInt128_uint128 a, uint32_t s)
{
  if (s == (uint32_t)0U)
  {
    return a;
  }
  else
  {
    FStar_UInt128_uint128 lit;
    lit.low = a.low << s;
    lit.high = FStar_UInt128_add_u64_shift_left_respec(a.high, a.low, s);
    return lit;
  }
}

inline static FStar_UInt128_uint128
FStar_UInt128_shift_left_large(FStar_UInt128_uint128 a, uint32_t s)
{
  FStar_UInt128_uint128 lit;
  lit.low = (uint64_t)0U;
  lit.high = a.low << (s - FStar_UInt128_u32_64);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_shift_left(FStar_UInt128_uint128 a, uint32_t s)
{
  if (s < FStar_UInt128_u32_64)
  {
    return FStar_UInt128_shift_left_small(a, s);
  }
  else
  {
    return FStar_UInt128_shift_left_large(a, s);
  }
}

inline static uint64_t FStar_UInt128_add_u64_shift_right(uint64_t hi, uint64_t lo, uint32_t s)
{
  return (lo >> s) + (hi << (FStar_UInt128_u32_64 - s));
}

inline static uint64_t
FStar_UInt128_add_u64_shift_right_respec(uint64_t hi, uint64_t lo, uint32_t s)
{
  return FStar_UInt128_add_u64_shift_right(hi, lo, s);
}

inline static FStar_UInt128_uint128
FStar_UInt128_shift_right_small(FStar_UInt128_uint128 a, uint32_t s)
{
  if (s == (uint32_t)0U)
  {
    return a;
  }
  else
  {
    FStar_UInt128_uint128 lit;
    lit.low = FStar_UInt128_add_u64_shift_right_respec(a.high, a.low, s);
    lit.high = a.high >> s;
    return lit;
  }
}

inline static FStar_UInt128_uint128
FStar_UInt128_shift_right_large(FStar_UInt128_uint128 a, uint32_t s)
{
  FStar_UInt128_uint128 lit;
  lit.low = a.high >> (s - FStar_UInt128_u32_64);
  lit.high = (uint64_t)0U;
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_shift_right(FStar_UInt128_uint128 a, uint32_t s)
{
  if (s < FStar_UInt128_u32_64)
  {
    return FStar_UInt128_shift_right_small(a, s);
  }
  else
  {
    return FStar_UInt128_shift_right_large(a, s);
  }
}

inline static bool FStar_UInt128_eq(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  return a.low == b.low && a.high == b.high;
}

inline static bool FStar_UInt128_gt(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  return a.high > b.high || (a.high == b.high && a.low > b.low);
}

inline static bool FStar_UInt128_lt(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  return a.high < b.high || (a.high == b.high && a.low < b.low);
}

inline static bool FStar_UInt128_gte(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  return a.high > b.high || (a.high == b.high && a.low >= b.low);
}

inline static bool FStar_UInt128_lte(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  return a.high < b.high || (a.high == b.high && a.low <= b.low);
}

inline static FStar_UInt128_uint128
FStar_UInt128_eq_mask(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low = FStar_UInt64_eq_mask(a.low, b.low) & FStar_UInt64_eq_mask(a.high, b.high);
  lit.high = FStar_UInt64_eq_mask(a.low, b.low) & FStar_UInt64_eq_mask(a.high, b.high);
  return lit;
}

inline static FStar_UInt128_uint128
FStar_UInt128_gte_mask(FStar_UInt128_uint128 a, FStar_UInt128_uint128 b)
{
  FStar_UInt128_uint128 lit;
  lit.low =
    (FStar_UInt64_gte_mask(a.high, b.high) & ~FStar_UInt64_eq_mask(a.high, b.high))
    | (FStar_UInt64_eq_mask(a.high, b.high) & FStar_UInt64_gte_mask(a.low, b.low));
  lit.high =
    (FStar_UInt64_gte_mask(a.high, b.high) & ~FStar_UInt64_eq_mask(a.high, b.high))
    | (FStar_UInt64_eq_mask(a.high, b.high) & FStar_UInt64_gte_mask(a.low, b.low));
  return lit;
}

inline static FStar_UInt128_uint128 FStar_UInt128_uint64_to_uint128(uint64_t a)
{
  FStar_UInt128_uint128 lit;
  lit.low = a;
  lit.high = (uint64_t)0U;
  return lit;
}

inline static uint64_t FStar_UInt128_uint128_to_uint64(FStar_UInt128_uint128 a)
{
  return a.low;
}

inline static uint64_t FStar_UInt128_u64_mod_32(uint64_t a)
{
  return a & (uint64_t)0xffffffffU;
}

static uint32_t FStar_UInt128_u32_32 = (uint32_t)32U;

inline static uint64_t FStar_UInt128_u32_combine(uint64_t hi, uint64_t lo)
{
  return lo + (hi << FStar_UInt128_u32_32);
}

inline static FStar_UInt128_uint128 FStar_UInt128_mul32(uint64_t x, uint32_t y)
{
  FStar_UInt128_uint128 lit;
  lit.low =
    FStar_UInt128_u32_combine((x >> FStar_UInt128_u32_32)
      * (uint64_t)y
      + (FStar_UInt128_u64_mod_32(x) * (uint64_t)y >> FStar_UInt128_u32_32),
      FStar_UInt128_u64_mod_32(FStar_UInt128_u64_mod_32(x) * (uint64_t)y));
  lit.high =
    ((x >> FStar_UInt128_u32_32)
    * (uint64_t)y
    + (FStar_UInt128_u64_mod_32(x) * (uint64_t)y >> FStar_UInt128_u32_32))
    >> FStar_UInt128_u32_32;
  return lit;
}

inline static uint64_t FStar_UInt128_u32_combine_(uint64_t hi, uint64_t lo)
{
  return lo + (hi << FStar_UInt128_u32_32);
}

inline static FStar_UInt128_uint128 FStar_UInt128_mul_wide(uint64_t x, uint64_t y)
{
  FStar_UInt128_uint128 lit;
  lit.low =
    FStar_UInt128_u32_combine_(FStar_UInt128_u64_mod_32(x)
      * (y >> FStar_UInt128_u32_32)
      +
        FStar_UInt128_u64_mod_32((x >> FStar_UInt128_u32_32)
          * FStar_UInt128_u64_mod_32(y)
          + (FStar_UInt128_u64_mod_32(x) * FStar_UInt128_u64_mod_32(y) >> FStar_UInt128_u32_32)),
      FStar_UInt128_u64_mod_32(FStar_UInt128_u64_mod_32(x) * FStar_UInt128_u64_mod_32(y)));
  lit.high =
    (x >> FStar_UInt128_u32_32)
    * (y >> FStar_UInt128_u32_32)
    +
      (((x >> FStar_UInt128_u32_32)
      * FStar_UInt128_u64_mod_32(y)
      + (FStar_UInt128_u64_mod_32(x) * FStar_UInt128_u64_mod_32(y) >> FStar_UInt128_u32_32))
      >> FStar_UInt128_u32_32)
    +
      ((FStar_UInt128_u64_mod_32(x)
      * (y >> FStar_UInt128_u32_32)
      +
        FStar_UInt128_u64_mod_32((x >> FStar_UInt128_u32_32)
          * FStar_UInt128_u64_mod_32(y)
          + (FStar_UInt128_u64_mod_32(x) * FStar_UInt128_u64_mod_32(y) >> FStar_UInt128_u32_32)))
      >> FStar_UInt128_u32_32);
  return lit;
}

#define __FStar_UInt128_Verified_H_DEFINED
#endif
