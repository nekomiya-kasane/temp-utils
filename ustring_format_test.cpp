#include "ustring.h"
#include <format>
#include <gtest/gtest.h>
#include <string>

TEST(UstringFormatTest, BasicFormatting)
{
  ustring str = u8"Hello";
  EXPECT_EQ(std::format("{}", str), "Hello");
  EXPECT_EQ(std::format("{:s}", str), "Hello");
}

TEST(UstringFormatTest, WidthAndAlignment)
{
  ustring str = u8"Test";
  // Right alignment (default)
  EXPECT_EQ(std::format("{:6}", str), "  Test");
  // Left alignment
  EXPECT_EQ(std::format("{:<6}", str), "Test  ");
  // Center alignment
  EXPECT_EQ(std::format("{:^6}", str), " Test ");
}

TEST(UstringFormatTest, FillAndAlign)
{
  ustring str = u8"Test";
  // Custom fill character with right alignment
  EXPECT_EQ(std::format("{:*>6}", str), "**Test");
  // Custom fill character with left alignment
  EXPECT_EQ(std::format("{:*<6}", str), "Test**");
  // Custom fill character with center alignment
  EXPECT_EQ(std::format("{:*^6}", str), "*Test*");
}

TEST(UstringFormatTest, UnicodeContent)
{
  ustring str = u8"你好世界";
  EXPECT_EQ(std::format("{}", str), "你好世界");
  EXPECT_EQ(std::format("{:^8}", str), " 你好世界 ");
}

TEST(UstringFormatTest, EmojiContent)
{
  ustring str = u8"Hello 👋 World 🌍";
  EXPECT_EQ(std::format("{}", str), "Hello 👋 World 🌍");
  EXPECT_EQ(std::format("{:^20}", str), " Hello 👋 World 🌍  ");
}

TEST(UstringFormatTest, FormatToString)
{
  ustring name = u8"Alice";
  ustring greeting = u8"Hello";
  std::string result;
  std::format_to(std::back_inserter(result), "{}, {}!", greeting, name);
  EXPECT_EQ(result, "Hello, Alice!");
}

TEST(UstringFormatTest, FormatToUstring)
{
  ustring name = u8"お兄ちゃんの";
  ustring greeting = u8"バカ！へんたい！うるさい！💢";
  ustring result;
  auto back = std::back_inserter(result);
  std::format_to(back, "{} {}", name, greeting);
  EXPECT_EQ(result, u8"お兄ちゃんの バカ！へんたい！うるさい！💢");

  // todo: this won't compile
  //std::format_to(back, TEXT(u8"{}　{}"), name, greeting);
  //EXPECT_EQ(result, u8"お兄ちゃんの　バカ！へんたい！うるさい！💢");
}

TEST(UstringFormatTest, MixedTypeFormatting)
{
  ustring str = u8"Count";
  int number = 42;
  double value = 3.14;
  EXPECT_EQ(std::format("{}: {}, pi ≈ {:.2f}", str, number, value), "Count: 42, pi ≈ 3.14");
}

TEST(UstringFormatTest, EscapeSequences)
{
  ustring str = u8"Line1\nLine2\tTabbed";
  EXPECT_EQ(std::format("{}", str), "Line1\nLine2\tTabbed");
}

TEST(UstringFormatTest, EmptyString)
{
  ustring str;
  EXPECT_EQ(std::format("{}", str), "");
  EXPECT_EQ(std::format("{:10}", str), "          ");
}

TEST(UstringFormatTest, NestedFormatting)
{
  ustring outer = u8"Outer";
  ustring inner = u8"Inner";
  std::string result = std::format("{}({})", outer, std::format("[{}]", inner));
  EXPECT_EQ(result, "Outer([Inner])");
}

TEST(UstringFormatTest, CompileTimeFormatString)
{
  ustring str = u8"Test";
  static constexpr auto fmt = "{:*^10}";
  EXPECT_EQ(std::format(fmt, str), "***Test***");
}

TEST(UstringFormatTest, FormatToWithIterator)
{
  ustring str = u8"Test";
  std::vector<char> buffer;
  std::format_to(std::back_inserter(buffer), "{:>5}", str);
  std::string result(buffer.begin(), buffer.end());
  EXPECT_EQ(result, " Test");
}

TEST(UstringFormatTest, MultipleArgumentsWithDifferentTypes)
{
  ustring str1 = u8"Hello";
  ustring str2 = u8"World";
  int num = 2023;
  std::string result = std::format("{} {} - {}", str1, str2, num);
  EXPECT_EQ(result, "Hello World - 2023");
}
