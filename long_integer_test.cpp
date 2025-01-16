#include "long_integer.h"

#include <gtest/gtest.h>

#include <limits>
#include <random>

class LongIntegerTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}

  static uint64_t random_uint64()
  {
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;
    return dis(gen);
  }
};

// uint128_t tests
TEST_F(LongIntegerTest, Uint128Construction)
{
  uint128_t a;
  EXPECT_EQ(a.high, 0);
  EXPECT_EQ(a.low, 0);

  uint128_t b(42);
  EXPECT_EQ(b.high, 0);
  EXPECT_EQ(b.low, 42);

  uint128_t c(1, 2);
  EXPECT_EQ(c.high, 1);
  EXPECT_EQ(c.low, 2);
}

TEST_F(LongIntegerTest, Uint128Conversion)
{
  uint128_t a(42);
  EXPECT_EQ(static_cast<uint64_t>(a), 42);
  EXPECT_EQ(static_cast<uint32_t>(a), 42);
  EXPECT_TRUE(static_cast<bool>(a));

  uint128_t b;
  EXPECT_FALSE(static_cast<bool>(b));
}

TEST_F(LongIntegerTest, Uint128Arithmetic)
{
  uint128_t a(0, 0xFFFFFFFFFFFFFFFF);
  uint128_t b(0, 1);
  uint128_t c = a + b;
  EXPECT_EQ(c.high, 1);
  EXPECT_EQ(c.low, 0);

  uint128_t d(1, 0);
  uint128_t e(0, 1);
  uint128_t f = d - e;
  EXPECT_EQ(f.high, 0);
  EXPECT_EQ(f.low, 0xFFFFFFFFFFFFFFFF);

  uint128_t g(0, 0xFFFFFFFFFFFFFFFF);
  uint128_t h(0, 2);
  uint128_t i = g * h;
  EXPECT_EQ(i.high, 1);
  EXPECT_EQ(i.low, 0xFFFFFFFFFFFFFFFE);

  uint128_t j(2, 0);
  uint128_t k(0, 2);
  uint128_t l = j / k;
  EXPECT_EQ(l.high, 1);
  EXPECT_EQ(l.low, 0);
}

TEST_F(LongIntegerTest, Uint128BitwiseOperations)
{
  uint128_t a(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
  uint128_t b(0x7FFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF);

  // AND
  uint128_t c = a & b;
  EXPECT_EQ(c.high, 0x7FFFFFFFFFFFFFFF);
  EXPECT_EQ(c.low, 0x7FFFFFFFFFFFFFFF);

  // OR
  uint128_t d = a | b;
  EXPECT_EQ(d.high, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(d.low, 0xFFFFFFFFFFFFFFFF);

  // XOR
  uint128_t e = a ^ b;
  EXPECT_EQ(e.high, 0x8000000000000000);
  EXPECT_EQ(e.low, 0x8000000000000000);

  // NOT
  uint128_t f = ~b;
  EXPECT_EQ(f.high, 0x8000000000000000);
  EXPECT_EQ(f.low, 0x8000000000000000);
}

TEST_F(LongIntegerTest, Uint128Shifts)
{
  uint128_t a(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);

  // Left shift
  uint128_t b = a << 64;
  EXPECT_EQ(b.high, 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(b.low, 0);

  // Right shift
  uint128_t c = a >> 64;
  EXPECT_EQ(c.high, 0);
  EXPECT_EQ(c.low, 0xFFFFFFFFFFFFFFFF);

  // Small shifts
  uint128_t d(0x1);
  uint128_t e = d << 1;
  EXPECT_EQ(e.high, 0);
  EXPECT_EQ(e.low, 0x2);

  uint128_t f(0, 0x2);
  uint128_t g = f >> 1;
  EXPECT_EQ(g.high, 0);
  EXPECT_EQ(g.low, 0x1);
}

TEST_F(LongIntegerTest, Uint128EdgeCases) {
  uint128_t zero;
  uint128_t one(1);
  uint128_t max = std::numeric_limits<uint128_t>::max();
  
  // Addition edge cases
  EXPECT_EQ(zero + zero, zero);
  EXPECT_EQ(max + zero, max);
  EXPECT_EQ(max + one, zero);  // Overflow wraps around
  
  // Subtraction edge cases
  EXPECT_EQ(zero - zero, zero);
  EXPECT_EQ(one - one, zero);
  EXPECT_EQ(zero - one, max);  // Underflow wraps around
  
  // Multiplication edge cases
  EXPECT_EQ(zero * one, zero);
  EXPECT_EQ(max * zero, zero);
  EXPECT_EQ(max * one, max);
  
  // Division edge cases
  EXPECT_EQ(zero / one, zero);
  EXPECT_EQ(max / one, max);
  EXPECT_EQ(max / max, one);
  EXPECT_THROW(one / zero, std::domain_error);
}

TEST_F(LongIntegerTest, Uint128Conversions) {
  // Test various integer conversions
  uint128_t a(0xFFFFFFFF);
  EXPECT_EQ(static_cast<uint32_t>(a), 0xFFFFFFFF);
  EXPECT_EQ(static_cast<uint16_t>(a), 0xFFFF);
  EXPECT_EQ(static_cast<uint8_t>(a), 0xFF);
  
  // Test bool conversion
  uint128_t zero;
  uint128_t nonzero(1);
  EXPECT_FALSE(static_cast<bool>(zero));
  EXPECT_TRUE(static_cast<bool>(nonzero));
}

// int128_t tests
TEST_F(LongIntegerTest, Int128Construction)
{
  int128_t a;
  EXPECT_EQ(static_cast<int64_t>(a), 0);

  int128_t b(-42);
  EXPECT_EQ(static_cast<int64_t>(b), -42);

  int128_t c(42);
  EXPECT_EQ(static_cast<int64_t>(c), 42);
}

TEST_F(LongIntegerTest, Int128Arithmetic)
{
  int128_t a(std::numeric_limits<int64_t>::max());
  int128_t b(1);
  int128_t c = a + b;
  EXPECT_GT(static_cast<uint64_t>(c.value), std::numeric_limits<int64_t>::max());

  int128_t d(-42);
  int128_t e = -d;
  EXPECT_EQ(static_cast<int64_t>(e), 42);

  int128_t f(std::numeric_limits<int64_t>::min());
  int128_t g = -f;
  EXPECT_GT(static_cast<uint64_t>(g.value), std::numeric_limits<int64_t>::max());
}

TEST_F(LongIntegerTest, Int128EdgeCases) {
  int128_t zero;
  int128_t one(1);
  int128_t minus_one(-1);
  int128_t max = std::numeric_limits<int128_t>::max();
  int128_t min = std::numeric_limits<int128_t>::min();
  
  // Addition edge cases
  EXPECT_EQ(zero + zero, zero);
  EXPECT_EQ(max + zero, max);
  EXPECT_EQ(min + zero, min);
  EXPECT_EQ(one + minus_one, zero);
  
  // Subtraction edge cases
  EXPECT_EQ(zero - zero, zero);
  EXPECT_EQ(minus_one - minus_one, zero);
  EXPECT_EQ(zero - one, minus_one);
  
  // Negation
  EXPECT_EQ(-zero, zero);
  EXPECT_EQ(-minus_one, one);
  EXPECT_NE(-min, max);  // -min overflows for two's complement
  
  // Sign tests
  EXPECT_GT(one, zero);
  EXPECT_LT(minus_one, zero);
  EXPECT_GT(max, zero);
  EXPECT_LT(min, zero);
}

// uint256_t tests
TEST_F(LongIntegerTest, Uint256Construction)
{
  uint256_t a;
  EXPECT_EQ(static_cast<uint64_t>(a), 0);

  uint256_t b(uint128_t(42));
  EXPECT_EQ(static_cast<uint64_t>(b), 42);

  uint256_t c(uint128_t(1), uint128_t(2));
  EXPECT_EQ(static_cast<uint64_t>(c.high), 1);
  EXPECT_EQ(static_cast<uint64_t>(c.low), 2);
}

TEST_F(LongIntegerTest, Uint256BitwiseOperations)
{
  uint256_t a(uint128_t(0xFFFFFFFFFFFFFFFF), uint128_t(0xFFFFFFFFFFFFFFFF));
  uint256_t b(uint128_t(0x7FFFFFFFFFFFFFFF), uint128_t(0x7FFFFFFFFFFFFFFF));

  // AND
  uint256_t c = a & b;
  EXPECT_EQ(static_cast<uint64_t>(c.high), 0x7FFFFFFFFFFFFFFF);
  EXPECT_EQ(static_cast<uint64_t>(c.low), 0x7FFFFFFFFFFFFFFF);

  // OR
  uint256_t d = a | b;
  EXPECT_EQ(static_cast<uint64_t>(d.high), 0xFFFFFFFFFFFFFFFF);
  EXPECT_EQ(static_cast<uint64_t>(d.low), 0xFFFFFFFFFFFFFFFF);

  // XOR
  uint256_t e = a ^ b;
  EXPECT_EQ(static_cast<uint64_t>(e.high), 0x8000000000000000);
  EXPECT_EQ(static_cast<uint64_t>(e.low), 0x8000000000000000);
}

TEST_F(LongIntegerTest, Uint256EdgeCases) {
  uint256_t zero;
  uint256_t one(uint128_t(1));
  uint256_t max = std::numeric_limits<uint256_t>::max();
  
  // Addition edge cases
  EXPECT_EQ(zero + zero, zero);
  EXPECT_EQ(max + zero, max);
  EXPECT_EQ(max + one, zero);  // Overflow wraps around
  
  // Subtraction edge cases
  EXPECT_EQ(zero - zero, zero);
  EXPECT_EQ(one - one, zero);
  EXPECT_EQ(zero - one, max);  // Underflow wraps around
  
  // Bit operations
  uint256_t high_bit(uint128_t(uint64_t(1) << 63, 0), uint128_t(0));
  uint256_t low_bit(uint128_t(0), uint128_t(1));
  
  EXPECT_EQ(high_bit >> 255, low_bit);
  EXPECT_EQ(low_bit << 255, high_bit);
}

// int256_t tests
TEST_F(LongIntegerTest, Int256Construction)
{
  int256_t a;
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(a)), 0);

  int256_t b(int128_t(-42));
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(b)), -42);

  int256_t c(int128_t(42));
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(c)), 42);
}

TEST_F(LongIntegerTest, Int256Arithmetic)
{
  auto mx = std::numeric_limits<int64_t>::max();
  int256_t a{int128_t(mx)};
  int256_t b = int128_t(1);
  int256_t c = a + b;
  EXPECT_GT(static_cast<uint64_t>(static_cast<int128_t>(c)), std::numeric_limits<int64_t>::max());

  int256_t d(int128_t(-42));
  int256_t e = -d;
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(e)), 42);
}

TEST_F(LongIntegerTest, Int256EdgeCases) {
  int256_t zero;
  int256_t one(int128_t(1));
  int256_t minus_one(int128_t(-1));
  int256_t max = std::numeric_limits<int256_t>::max();
  int256_t min = std::numeric_limits<int256_t>::min();
  
  // Addition edge cases
  EXPECT_EQ(zero + zero, zero);
  EXPECT_EQ(max + zero, max);
  EXPECT_EQ(min + zero, min);
  EXPECT_EQ(one + minus_one, zero);
  
  // Subtraction edge cases
  EXPECT_EQ(zero - zero, zero);
  EXPECT_EQ(minus_one - minus_one, zero);
  EXPECT_EQ(zero - one, minus_one);
  
  // Negation
  EXPECT_EQ(-zero, zero);
  EXPECT_EQ(-minus_one, one);
  EXPECT_NE(-min, max);  // -min overflows for two's complement
  
  // Sign tests
  EXPECT_GT(one, zero);
  EXPECT_LT(minus_one, zero);
  EXPECT_GT(max, zero);
  EXPECT_LT(min, zero);
}

// Cross-type operations tests
TEST_F(LongIntegerTest, CrossTypeOperations)
{
  // uint128_t with built-in types
  uint128_t a(42);
  EXPECT_EQ((a & 0xFF).low, 42);
  EXPECT_EQ((a | 0xFF).low, 0xFF);
  EXPECT_EQ((a ^ 0xFF).low, 0xD5);

  // int128_t with built-in types
  int128_t b(-42);
  EXPECT_EQ(static_cast<int64_t>(b & 0xFF), -42 & 0xFF);
  EXPECT_EQ(static_cast<int64_t>(b | 0xFF), -42 | 0xFF);
  EXPECT_EQ(static_cast<int64_t>(b ^ 0xFF), -42 ^ 0xFF);

  // uint256_t with smaller types
  uint256_t c(uint128_t(42));
  EXPECT_EQ(static_cast<uint64_t>(c & uint128_t(0xFF)), 42);
  EXPECT_EQ(static_cast<uint64_t>(c | uint128_t(0xFF)), 0xFF);
  EXPECT_EQ(static_cast<uint64_t>(c ^ uint128_t(0xFF)), 0xD5);

  // int256_t with smaller types
  int256_t d(int128_t(-42));
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(d & int128_t(0xFF))), -42 & 0xFF);
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(d | int128_t(0xFF))), -42 | 0xFF);
  EXPECT_EQ(static_cast<int64_t>(static_cast<int128_t>(d ^ int128_t(0xFF))), -42 ^ 0xFF);
}

TEST_F(LongIntegerTest, CrossTypeComparisons) {
  // uint128_t comparisons
  uint128_t u128_max = std::numeric_limits<uint128_t>::max();
  uint64_t u64_max = std::numeric_limits<uint64_t>::max();
  
  EXPECT_GT(u128_max, u64_max);
  EXPECT_EQ(uint128_t(u64_max), u64_max);
  
  // int128_t comparisons
  int128_t i128_max = std::numeric_limits<int128_t>::max();
  int128_t i128_min = std::numeric_limits<int128_t>::min();
  int64_t i64_max = std::numeric_limits<int64_t>::max();
  int64_t i64_min = std::numeric_limits<int64_t>::min();
  
  EXPECT_GT(i128_max, i64_max);
  EXPECT_LT(i128_min, i64_min);
  EXPECT_EQ(int128_t(i64_max), i64_max);
  EXPECT_EQ(int128_t(i64_min), i64_min);
  
  // uint256_t comparisons
  uint256_t u256_max = std::numeric_limits<uint256_t>::max();
  uint256_t u256_u128(u128_max);
  
  EXPECT_GT(u256_max, u256_u128);
  EXPECT_EQ(uint256_t(uint128_t(u64_max)), u64_max);
  
  // int256_t comparisons
  int256_t i256_max = std::numeric_limits<int256_t>::max();
  int256_t i256_min = std::numeric_limits<int256_t>::min();
  int256_t i256_i128_max(i128_max);
  int256_t i256_i128_min(i128_min);
  
  EXPECT_GT(i256_max, i256_i128_max);
  EXPECT_LT(i256_min, i256_i128_min);
}

TEST_F(LongIntegerTest, BitManipulation) {
  // uint128_t bit operations
  uint128_t u128(0x0123456789ABCDEF, 0xFEDCBA9876543210);
  
  for (int i = 0; i < 128; ++i) {
    bool expected_bit = (i < 64) ? 
      ((0xFEDCBA9876543210ULL >> i) & 1) :
      ((0x0123456789ABCDEFULL >> (i - 64)) & 1);
    EXPECT_EQ(u128.bit(i), expected_bit);
  }
  
  // int128_t bit operations
  int128_t i128(-42);
  EXPECT_TRUE(i128.value.high & (uint64_t(1) << 63));  // Sign bit should be set
  
  // uint256_t bit operations
  uint256_t u256(uint128_t(0x0123456789ABCDEF, 0xFEDCBA9876543210),
                 uint128_t(0xFEDCBA9876543210, 0x0123456789ABCDEF));
  
  EXPECT_EQ(u256 >> 128, uint256_t(uint128_t(0x0123456789ABCDEF, 0xFEDCBA9876543210)));
  EXPECT_EQ(u256 << 128, uint256_t(uint128_t(0xFEDCBA9876543210, 0x0123456789ABCDEF), uint128_t(0)));
}

// Numeric limits tests
TEST_F(LongIntegerTest, Uint128NumericLimits)
{
  using limits = std::numeric_limits<uint128_t>;
  EXPECT_TRUE(limits::is_specialized);
  EXPECT_FALSE(limits::is_signed);
  EXPECT_TRUE(limits::is_integer);
  EXPECT_EQ(limits::digits, 128);
  EXPECT_EQ(limits::digits10, 38);

  uint128_t max_val = limits::max();
  EXPECT_EQ(max_val.high, ~uint64_t(0));
  EXPECT_EQ(max_val.low, ~uint64_t(0));

  uint128_t min_val = limits::min();
  EXPECT_EQ(min_val.high, 0);
  EXPECT_EQ(min_val.low, 0);
}

TEST_F(LongIntegerTest, Int128NumericLimits)
{
  using limits = std::numeric_limits<int128_t>;
  EXPECT_TRUE(limits::is_specialized);
  EXPECT_TRUE(limits::is_signed);
  EXPECT_TRUE(limits::is_integer);
  EXPECT_EQ(limits::digits, 127);
  EXPECT_EQ(limits::digits10, 38);

  int128_t max_val = limits::max();
  EXPECT_EQ(max_val.value.high, ~(uint64_t(1) << 63));
  EXPECT_EQ(max_val.value.low, ~uint64_t(0));

  int128_t min_val = limits::min();
  EXPECT_EQ(min_val.value.high, uint64_t(1) << 63);
  EXPECT_EQ(min_val.value.low, 0);
}

TEST_F(LongIntegerTest, Uint256NumericLimits)
{
  using limits = std::numeric_limits<uint256_t>;
  EXPECT_TRUE(limits::is_specialized);
  EXPECT_FALSE(limits::is_signed);
  EXPECT_TRUE(limits::is_integer);
  EXPECT_EQ(limits::digits, 256);
  EXPECT_EQ(limits::digits10, 77);

  uint256_t max_val = limits::max();
  EXPECT_EQ(max_val.high.high, ~uint64_t(0));
  EXPECT_EQ(max_val.high.low, ~uint64_t(0));
  EXPECT_EQ(max_val.low.high, ~uint64_t(0));
  EXPECT_EQ(max_val.low.low, ~uint64_t(0));

  uint256_t min_val = limits::min();
  EXPECT_EQ(min_val.high.high, 0);
  EXPECT_EQ(min_val.high.low, 0);
  EXPECT_EQ(min_val.low.high, 0);
  EXPECT_EQ(min_val.low.low, 0);
}

TEST_F(LongIntegerTest, Int256NumericLimits)
{
  using limits = std::numeric_limits<int256_t>;
  EXPECT_TRUE(limits::is_specialized);
  EXPECT_TRUE(limits::is_signed);
  EXPECT_TRUE(limits::is_integer);
  EXPECT_EQ(limits::digits, 255);
  EXPECT_EQ(limits::digits10, 76);

  int256_t max_val = limits::max();
  EXPECT_EQ(max_val.value.high.high, ~(static_cast<uint64_t>(1) << 63));
  EXPECT_EQ(max_val.value.high.low, ~static_cast<uint64_t>(0));
  EXPECT_EQ(max_val.value.low.high, ~static_cast<uint64_t>(0));
  EXPECT_EQ(max_val.value.low.low, ~static_cast<uint64_t>(0));

  int256_t min_val = limits::min();
  EXPECT_EQ(min_val.value.high.high, static_cast<uint64_t>(1) << 63);
  EXPECT_EQ(min_val.value.high.low, 0);
  EXPECT_EQ(min_val.value.low.high, 0);
  EXPECT_EQ(min_val.value.low.low, 0);
}
