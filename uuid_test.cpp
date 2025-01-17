#include "uuid.h"
#include <gtest/gtest.h>

class UUIDTest : public testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(UUIDTest, UUIDConversion)
{
  // Test constructing Uuid from smaller IDs
  UniqueId16 smallId(0x1234);
  UniqueId32 int32Id(0x12345678);

  Uuid uuidFromSmall(smallId);
  Uuid uuidFromInt32(int32Id);

  //EXPECT_EQ(uuidFromSmall & 0xFFFF, 0x1234);
  //EXPECT_EQ(uuidFromInt32 & 0xFFFFFFFF, 0x12345678);

  //// Test truncating Uuid to smaller IDs
  //Uuid uuid(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);
  //UniqueId16 truncatedSmall = static_cast<UniqueId16>(uuid);
  //UniqueId32 truncatedInt32 = static_cast<UniqueId32>(uuid);

  //EXPECT_EQ(truncatedSmall.value(), 0x3210);
  //EXPECT_EQ(truncatedInt32.value(), 0x76543210);

  //// Test splitting Uuid into smaller IDs
  //auto splitSmall = uuid.split<uint16_t, 2>();
  //static_assert(splitSmall.size() == 8, "Uuid should split into 8 SmallIds");

  //EXPECT_EQ(splitSmall[0].value(), 0x3210);
  //EXPECT_EQ(splitSmall[1].value(), 0x7654);
  //EXPECT_EQ(splitSmall[2].value(), 0xBA98);
  //EXPECT_EQ(splitSmall[3].value(), 0xFEDC);
  //EXPECT_EQ(splitSmall[4].value(), 0xDEF0);
  //EXPECT_EQ(splitSmall[5].value(), 0x9ABC);
  //EXPECT_EQ(splitSmall[6].value(), 0x5678);
  //EXPECT_EQ(splitSmall[7].value(), 0x1234);

  //auto splitInt32 = uuid.split<uint32_t, 4>();
  //static_assert(splitInt32.size() == 4, "Uuid should split into 4 Int32Ids");

  //EXPECT_EQ(splitInt32[0].value(), 0x76543210);
  //EXPECT_EQ(splitInt32[1].value(), 0xFEDCBA98);
  //EXPECT_EQ(splitInt32[2].value(), 0x9ABCDEF0);
  //EXPECT_EQ(splitInt32[3].value(), 0x12345678);
}

TEST_F(UUIDTest, BitwiseOperations)
{
  Uuid uuid1(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
  Uuid uuid2(0x0000000000000000, 0x0000000000000000);
  Uuid uuid3(0x1234567890ABCDEFULL, 0xFEDCBA9876543210ULL);

  // Test bitwise operations between UUIDs
  EXPECT_EQ((uuid1 & uuid3).high(), 0x1234567890ABCDEF);
  EXPECT_EQ((uuid1 & uuid3).low(), 0xFEDCBA9876543210);

  EXPECT_EQ((uuid2 | uuid3).high(), 0x1234567890ABCDEF);
  EXPECT_EQ((uuid2 | uuid3).low(), 0xFEDCBA9876543210);

  EXPECT_EQ((uuid1 ^ uuid3).high(), 0xEDCBA98776543210);
  EXPECT_EQ((uuid1 ^ uuid3).low(), 0x0123456789ABCDEF);

  EXPECT_EQ((~uuid3).high(), 0xEDCBA98776543210);
  EXPECT_EQ((~uuid3).low(), 0x0123456789ABCDEF);

  // Test compound assignment operators
  Uuid temp = uuid1;
  temp &= uuid3;
  EXPECT_EQ(temp.high(), 0x1234567890ABCDEF);
  EXPECT_EQ(temp.low(), 0xFEDCBA9876543210);

  temp = uuid2;
  temp |= uuid3;
  EXPECT_EQ(temp.high(), 0x1234567890ABCDEF);
  EXPECT_EQ(temp.low(), 0xFEDCBA9876543210);

  temp = uuid1;
  temp ^= uuid3;
  EXPECT_EQ(temp.high(), 0xEDCBA98776543210);
  EXPECT_EQ(temp.low(), 0x0123456789ABCDEF);

  // Test bitwise operations with integral types
  EXPECT_EQ((uuid3 & 0xFFFF).high(), 0x0000000000000000);
  EXPECT_EQ((uuid3 & 0xFFFF).low(), 0x0000000000003210);

  EXPECT_EQ((uuid3 | 0xFFFF).high(), 0x1234567890ABCDEF);
  EXPECT_EQ((uuid3 | 0xFFFF).low(), 0xFEDCBA9876543FFF);

  EXPECT_EQ((uuid3 ^ 0xFFFF).high(), 0x1234567890ABCDEF);
  EXPECT_EQ((uuid3 ^ 0xFFFF).low(), 0xFEDCBA9876543DEF);

  // Test compound assignment with integral types
  temp = uuid3;
  temp &= 0xFFFF;
  EXPECT_EQ(temp.high(), 0x0000000000000000);
  EXPECT_EQ(temp.low(), 0x0000000000003210);

  temp = uuid3;
  temp |= 0xFFFF;
  EXPECT_EQ(temp.high(), 0x1234567890ABCDEF);
  EXPECT_EQ(temp.low(), 0xFEDCBA9876543FFF);

  temp = uuid3;
  temp ^= 0xFFFF;
  EXPECT_EQ(temp.high(), 0x1234567890ABCDEF);
  EXPECT_EQ(temp.low(), 0xFEDCBA9876543DEF);
}

TEST_F(UUIDTest, Formatting)
{
  Uuid uuid(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);

  // Test default formatting
  EXPECT_EQ(uuid.toString(), "123456789abcdef0-fedc-ba98-7654-3210");

  // Test various format specifiers
  //EXPECT_EQ(std::format("{}", uuid), "123456789abcdef0-fedc-ba98-7654-3210");
  //EXPECT_EQ(std::format("{:X}", uuid), "123456789ABCDEF0-FEDC-BA98-7654-3210");
  //EXPECT_EQ(std::format("{:-}", uuid), "123456789abcdef0fedcba9876543210");
  //EXPECT_EQ(std::format("{:-X}", uuid), "123456789ABCDEF0FEDCBA9876543210");

  //// Test width and alignment
  //EXPECT_EQ(std::format("{:40}", uuid), "  123456789abcdef0-fedc-ba98-7654-3210");
  //EXPECT_EQ(std::format("{:<40}", uuid), "123456789abcdef0-fedc-ba98-7654-3210  ");
  //EXPECT_EQ(std::format("{:>40}", uuid), "  123456789abcdef0-fedc-ba98-7654-3210");

  //// Test fill character
  //EXPECT_EQ(std::format("{:0>40}", uuid), "00123456789abcdef0-fedc-ba98-7654-3210");
  //EXPECT_EQ(std::format("{:_<40}", uuid), "123456789abcdef0-fedc-ba98-7654-3210__");
}
