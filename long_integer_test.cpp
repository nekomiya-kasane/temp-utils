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

// Formatter tests
TEST_F(LongIntegerTest, Uint128Formatting) {
    uint128_t a(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);
    
    // Default decimal format
    EXPECT_EQ(std::format("{}", a), "24197857203266734864406886066");
    
    //// Hex format
    //EXPECT_EQ(std::format("{:x}", a), "123456789abcdef0fedcba9876543210");
    //EXPECT_EQ(std::format("{:X}", a), "123456789ABCDEF0FEDCBA9876543210");
    //EXPECT_EQ(std::format("{:#x}", a), "0x123456789abcdef0fedcba9876543210");
    //EXPECT_EQ(std::format("{:#X}", a), "0X123456789ABCDEF0FEDCBA9876543210");
    //
    //// Binary format
    //uint128_t b(0, 0b1010);
    //EXPECT_EQ(std::format("{:b}", b), "1010");
    //EXPECT_EQ(std::format("{:#b}", b), "0b1010");
    //EXPECT_EQ(std::format("{:#B}", b), "0B1010");
    //
    //// Octal format
    //EXPECT_EQ(std::format("{:o}", b), "12");
    //EXPECT_EQ(std::format("{:#o}", b), "012");
    //
    //// Width and alignment
    //EXPECT_EQ(std::format("{:10}", b), "10        ");  // Left align (default)
    //EXPECT_EQ(std::format("{:>10}", b), "        10");  // Right align
    //EXPECT_EQ(std::format("{:^10}", b), "    10    ");  // Center align
    //EXPECT_EQ(std::format("{:0>10}", b), "0000000010");  // Right align with zeros
}
//
//TEST_F(LongIntegerTest, Int128Formatting) {
//    int128_t a(-42);
//    int128_t b(0x123456789ABCDEF0ULL, 0);   
//    
//    // Basic formatting
//    EXPECT_EQ(std::format("{}", a), "-42");
//    EXPECT_EQ(std::format("{:x}", std::abs(a)), "2a");
//    
//    // Width and alignment with sign
//    EXPECT_EQ(std::format("{:10}", a), "-42       ");
//    EXPECT_EQ(std::format("{:>10}", a), "       -42");
//    EXPECT_EQ(std::format("{:^10}", a), "   -42    ");
//    
//    // Hex format with large numbers
//    EXPECT_EQ(std::format("{:#x}", b), "0x123456789abcdef0");
//}
//
//TEST_F(LongIntegerTest, Uint256Formatting) {
//    uint256_t a(uint128_t(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL), 
//                uint128_t(0xAAAAAAAABBBBBBBBULL, 0xCCCCCCCCDDDDDDDDULL));
//    
//    // Default format
//    EXPECT_EQ(std::format("{}", a), "123456789abcdef0fedcba9876543210aaaaaaaabbbbbbbbccccccccdddddddd");
//    
//    // Hex format with prefix
//    EXPECT_EQ(std::format("{:#X}", a), "0X123456789ABCDEF0FEDCBA9876543210AAAAAAAABBBBBBBBCCCCCCCCDDDDDDDD");
//    
//    // Small numbers should format like uint128
//    uint256_t b(uint128_t(0), uint128_t(42));
//    EXPECT_EQ(std::format("{}", b), "42");
//}
//
//TEST_F(LongIntegerTest, Int256Formatting) {
//    int256_t a = -int256_t(uint256_t(uint128_t(0), uint128_t(42)));
//    
//    // Negative numbers
//    EXPECT_EQ(std::format("{}", a), "-42");
//    EXPECT_EQ(std::format("{:x}", std::abs(a)), "2a");
//    
//    // Width and alignment
//    EXPECT_EQ(std::format("{:10}", a), "-42       ");
//    EXPECT_EQ(std::format("{:>10}", a), "       -42");
//    
//    // Large negative number
//    int256_t b = -int256_t(uint256_t(uint128_t(1), uint128_t(0)));
//    EXPECT_EQ(std::format("{:#x}", std::abs(b)), "0x100000000000000000000000000000000");
//}

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

TEST_F(LongIntegerTest, Uint256DivisionAndBitwise) {
  uint256_t zero;
  uint256_t one(1);
  uint256_t max(uint128_t(-1), uint128_t(-1));

  EXPECT_EQ(zero + zero, zero);
  EXPECT_EQ(max + one, zero);
  EXPECT_EQ(zero - zero, zero);
  EXPECT_EQ(zero - one, max);
  EXPECT_EQ(max - max, zero);

  // Division edge cases
  EXPECT_EQ(zero / one, zero);
  EXPECT_EQ(max / one, max);
  EXPECT_EQ(max / max, one);
  EXPECT_THROW(zero / zero, std::domain_error);
  
  // Large number division
  uint256_t large(uint128_t(0xFFFFFFFFFFFFFFFFULL), uint128_t(0));
  EXPECT_EQ(large / uint256_t(2), uint256_t(uint128_t(0x7FFFFFFFFFFFFFFFULL), uint128_t(0)));
  
  // Multiplication edge cases
  EXPECT_EQ(zero * zero, zero);
  EXPECT_EQ(one * one, one);
  EXPECT_EQ(max * zero, zero);
  EXPECT_EQ(max * one, max);
  
  // Bitwise operations
  EXPECT_EQ(zero & zero, zero);
  EXPECT_EQ(max & zero, zero);
  EXPECT_EQ(max & max, max);
  EXPECT_EQ(max & one, one);
  
  EXPECT_EQ(zero | zero, zero);
  EXPECT_EQ(max | zero, max);
  EXPECT_EQ(one | zero, one);
  
  EXPECT_EQ(zero ^ zero, zero);
  EXPECT_EQ(max ^ zero, max);
  EXPECT_EQ(max ^ max, zero);
  
  // Shift operations
  EXPECT_EQ(one << 0, one);
  EXPECT_EQ(one << 128, uint256_t(uint128_t(1), uint128_t(0)));
  EXPECT_EQ(one << 255, uint256_t(uint128_t(1) << 127, uint128_t(0)));
  EXPECT_EQ(one << 256, zero);
  
  EXPECT_EQ(max >> 0, max);
  EXPECT_EQ(max >> 128, uint256_t(uint128_t(0), uint128_t(-1)));
  EXPECT_EQ(max >> 255, uint256_t(uint128_t(0), uint128_t(1)));
  EXPECT_EQ(max >> 256, zero);
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

TEST_F(LongIntegerTest, Int256ArithmeticAndBitwise)
{
  int256_t zero;
  int256_t one(1);
  int256_t minus_one(-1);
  int256_t max(uint256_t(uint128_t(0x7FFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL), 
                         uint128_t(0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL)));
  int256_t min(-max - one);

  // Division tests
  EXPECT_EQ(zero / one, zero);
  EXPECT_EQ(max / one, max);
  EXPECT_EQ(min / minus_one, max);
  EXPECT_THROW(zero / zero, std::domain_error);

  // Large number division
  int256_t large(int128_t(0x7FFFFFFFFFFFFFFFLL));
  EXPECT_EQ(large / int256_t(2), int256_t(0x3FFFFFFFFFFFFFFFLL));
  EXPECT_EQ((-large) / int256_t(2), int256_t(-0x3FFFFFFFFFFFFFFFLL));

  // Multiplication tests
  EXPECT_EQ(zero * zero, zero);
  EXPECT_EQ(one * one, one);
  EXPECT_EQ(minus_one * minus_one, one);
  EXPECT_EQ(max * zero, zero);
  EXPECT_EQ(min * zero, zero);
  EXPECT_EQ(max * one, max);
  EXPECT_EQ(min * one, min);
  EXPECT_EQ(max * minus_one, -max);
  EXPECT_EQ(min * minus_one, -min);

  // Bitwise operations
  EXPECT_EQ(zero & zero, zero);
  EXPECT_EQ(max & zero, zero);
  EXPECT_EQ(max & max, max);
  EXPECT_EQ(max & one, one);
  EXPECT_EQ(min & minus_one, min);

  EXPECT_EQ(zero | zero, zero);
  EXPECT_EQ(max | zero, max);
  EXPECT_EQ(one | zero, one);
  EXPECT_EQ(min | minus_one, minus_one);

  EXPECT_EQ(zero ^ zero, zero);
  EXPECT_EQ(max ^ zero, max);
  EXPECT_EQ(max ^ max, zero);
  EXPECT_EQ(min ^ minus_one, ~min);

  // Shift operations
  EXPECT_EQ(one << 0, one);
  EXPECT_EQ(one << 128, int256_t(uint256_t(uint128_t(1), uint128_t(0))));
  EXPECT_EQ(one << 255, int256_t(uint256_t(uint128_t(1) << 127, uint128_t(0))));
  EXPECT_EQ(one << 256, zero);

  EXPECT_EQ(minus_one >> 0, minus_one);
  EXPECT_EQ(minus_one >> 128, minus_one);
  EXPECT_EQ(minus_one >> 255, minus_one);
  EXPECT_EQ(minus_one >> 256, minus_one);
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

TEST_F(LongIntegerTest, BitManipulation)
{
  uint128_t a(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL);
  
  // Test alternating bits pattern
  for (size_t i = 0; i < 64; ++i) {
    EXPECT_EQ(a.bit(i), (i % 2) == 1) << "at bit " << i;  // low part
    EXPECT_EQ(a.bit(i + 64), (i % 2) == 0) << "at bit " << (i + 64);  // high part
  }
  
  // Test out of range bits
  EXPECT_FALSE(a.bit(128));
  EXPECT_FALSE(a.bit(129));
  EXPECT_FALSE(a.bit(255));
  
  // Test int128_t bit access
  int128_t b(-1);  // All bits set
  for (size_t i = 0; i < 128; ++i) {
    EXPECT_TRUE(b.bit(i)) << "at bit " << i;
  }
  EXPECT_FALSE(b.bit(128));
  
  int128_t c(1);  // Only lowest bit set
  EXPECT_TRUE(c.bit(0));
  for (size_t i = 1; i < 128; ++i) {
    EXPECT_FALSE(c.bit(i)) << "at bit " << i;
  }
  
  // Test uint256_t bit access
  uint256_t d(uint128_t(0x5555555555555555ULL, 0xAAAAAAAAAAAAAAAAULL),
              uint128_t(0xF0F0F0F0F0F0F0F0ULL, 0x0F0F0F0F0F0F0F0FULL));
              
  // Test low part (first 128 bits)
  for (size_t i = 0; i < 64; ++i) {
    EXPECT_EQ(d.bit(i), (i % 2) == 1) << "at bit " << i;
    EXPECT_EQ(d.bit(i + 64), (i % 2) == 0) << "at bit " << (i + 64);
  }
  
  // Test high part (next 128 bits)
  for (size_t i = 0; i < 64; ++i) {
    EXPECT_EQ(d.bit(i + 128), (i % 4) >= 2) << "at bit " << (i + 128);
    EXPECT_EQ(d.bit(i + 192), (i % 4) < 2) << "at bit " << (i + 192);
  }
  
  // Test out of range bits
  EXPECT_FALSE(d.bit(256));
  EXPECT_FALSE(d.bit(257));
  EXPECT_FALSE(d.bit(512));
  
  // Test int256_t bit access
  int256_t e(-1);  // All bits set
  for (size_t i = 0; i < 256; ++i) {
    EXPECT_TRUE(e.bit(i)) << "at bit " << i;
  }
  EXPECT_FALSE(e.bit(256));
  
  int256_t f(1);  // Only lowest bit set
  EXPECT_TRUE(f.bit(0));
  for (size_t i = 1; i < 256; ++i) {
    EXPECT_FALSE(f.bit(i)) << "at bit " << i;
  }
  
  // Test specific bit patterns
  uint256_t g(uint128_t(1) << 127, uint128_t(1));  // Bits 0 and 127 set
  EXPECT_TRUE(g.bit(0));
  EXPECT_TRUE(g.bit(127));
  EXPECT_FALSE(g.bit(1));
  EXPECT_FALSE(g.bit(126));
  EXPECT_FALSE(g.bit(128));
  EXPECT_FALSE(g.bit(255));
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
