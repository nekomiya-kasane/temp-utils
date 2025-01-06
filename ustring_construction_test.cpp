#include "ustring.h"

#include <gtest/gtest.h>
#include <string>

TEST(UstringConstructionTest, DefaultConstructor) {
  ustring str;
  EXPECT_EQ(str.length(), 0);
  EXPECT_TRUE(str.empty());
}

TEST(UstringConstructionTest, CharPointerConstructor) {
  const char *text = "Hello, World!";
  ustring str(text);
  EXPECT_EQ(str.length(), 13);
  EXPECT_FALSE(str.empty());
  EXPECT_EQ(str.to_string(), text);
}

TEST(UstringConstructionTest, StringConstructor) {
  std::string text = "Hello, World!";
  ustring str(text);
  EXPECT_EQ(str.length(), 13);
  EXPECT_FALSE(str.empty());
  EXPECT_EQ(str.to_string(), text);
}

TEST(UstringConstructionTest, UTF8Constructor) {
  std::u8string text = u8"你好，世界！";
  ustring str(text);
  EXPECT_EQ(str.length(), 6); 
  EXPECT_FALSE(str.empty());
  EXPECT_TRUE(str.to_u8string() == text);
}

TEST(UstringConstructionTest, CopyConstructor) {
  ustring original(u8"Hello, 世界！");
  ustring copy(original);
  EXPECT_EQ(copy.length(), original.length());
  EXPECT_TRUE(copy.to_u8string() == original.to_u8string());
}

TEST(UstringConstructionTest, MoveConstructor) {
  std::u8string text = u8"Hello, 世界！";
  ustring original(text);
  size_t originalLength = original.length();
  ustring moved(std::move(original));
  EXPECT_EQ(moved.length(), originalLength);
  EXPECT_TRUE(moved.to_u8string() == text);
  EXPECT_TRUE(original.empty());
}

TEST(UstringConstructionTest, AssignmentOperator) {
  ustring str1(u8"Hello");
  ustring str2;
  str2 = str1;
  EXPECT_EQ(str2.length(), str1.length());
  EXPECT_TRUE(str2.to_u8string() == str1.to_u8string());
}

TEST(UstringConstructionTest, MoveAssignmentOperator) {
  ustring str1(u8"Hello");
  size_t str1Length = str1.length();
  std::string str1Content = str1.to_string();
  ustring str2;
  str2 = std::move(str1);
  EXPECT_EQ(str2.length(), str1Length);
  EXPECT_EQ(str2.to_string(), str1Content);
  EXPECT_TRUE(str1.empty());
}

TEST(UstringConstructionTest, DifferentEncodings) {
  // UTF-8
  ustring utf8_str(u8"你好");
  EXPECT_EQ(utf8_str.length(), 2);

  // UTF-16
  std::u16string utf16_text = u"你好";
  ustring utf16_str(utf16_text);
  EXPECT_EQ(utf16_str.length(), 2);
  EXPECT_EQ(utf16_str.to_string(), utf8_str.to_string());

  // UTF-32
  std::u32string utf32_text = U"你好";
  ustring utf32_str(utf32_text);
  EXPECT_EQ(utf32_str.length(), 2);
  EXPECT_EQ(utf32_str.to_string(), utf8_str.to_string());
}

TEST(UstringConstructionTest, SpecialCharacters) {
  ustring null_str("\0", size_t(1));
  EXPECT_EQ(null_str.length(), 1);

  ustring newline_str("Hello\nWorld");
  EXPECT_EQ(newline_str.length(), 11);

  ustring emoji_str(u8"😀🌍🎉");
  EXPECT_EQ(emoji_str.length(), 3); 
}

TEST(UstringConstructionTest, InvalidUTF8) {
  const char invalid_utf8[] = {char(0xFF), char(0xFF), 'a', 'b', 'c', 0};
  EXPECT_NO_THROW({
    ustring str(invalid_utf8);
    EXPECT_GT(str.length(), 0); 
  });
}

TEST(UstringConstructionTest, LargeString) {
  std::string large_text(1000000, 'a'); 
  EXPECT_NO_THROW({
    ustring large_str(large_text);
    EXPECT_EQ(large_str.length(), 1000000);
  });
}

TEST(UstringConstructionTest, StringLength) {
  ustring ascii("Hello");
  EXPECT_EQ(ascii.length(), 5);

  ustring emoji(u8"😀🌍🎉");  // 3个emoji，每个都是1个code point
  EXPECT_EQ(emoji.length(), 3);

  // 中文字符串
  ustring chinese(u8"你好世界");  // 4个汉字，每个都是1个code point
  EXPECT_EQ(chinese.length(), 4);

  // 组合字符
  ustring combined(u8"é");  // 1个code point (é 可以是一个预组合字符)
  EXPECT_EQ(combined.length(), 1);

  // 空字符串
  ustring empty;
  EXPECT_EQ(empty.length(), 0);

  // 混合字符串
  ustring mixed(u8"Hello你好😀");  // 5(ASCII) + 2(中文) + 1(emoji) = 8个code points
  EXPECT_EQ(mixed.length(), 8);
}
