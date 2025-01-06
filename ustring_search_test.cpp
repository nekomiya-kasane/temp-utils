#include "ustring.h"
#include <gtest/gtest.h>

class UstringSearchTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    // Common test strings
    hello = u8"Hello, World!";
    repeated = u8"hello hello hello";
    mixed = u8"Hello, 世界! Hello, World!";
    empty = u8"";
  }

  ustring hello;
  ustring repeated;
  ustring mixed;
  ustring empty;

  std::u8string hello_u8 = u8"Hello, World!";
  std::u8string repeated_u8 = u8"hello hello hello";
  std::u8string mixed_u8 = u8"Hello, 世界! Hello, World!";
};

// Test copy and substr functions
TEST_F(UstringSearchTest, CopyAndSubstr)
{
  // Test copy
  char8_t buffer[20];
  EXPECT_EQ(hello.copy(buffer, 5, 0), 5);
  EXPECT_EQ(ustring(buffer, 5), u8"Hello");

  // Test substr
  EXPECT_EQ(hello.substr(0, 5), u8"Hello");
  EXPECT_EQ(hello.substr(7, 5), u8"World");
  EXPECT_EQ(hello.substr(7), u8"World!");
  EXPECT_EQ(hello.substr(hello.size()), u8"");
  EXPECT_EQ(hello.substr(hello.size() + 1).size(), 0);
}

// Test find functions
TEST_F(UstringSearchTest, Find)
{
  // Test find with ustring
  EXPECT_EQ(hello.find(ustring(u8"World")), hello_u8.find(u8"World"));
  EXPECT_EQ(hello.find(ustring(u8"NotFound")), ustring::npos);
  EXPECT_EQ(repeated.find(ustring(u8"hello"), 1), repeated.find(u8"hello", 1));

  // Test find with C-string
  EXPECT_EQ(hello.find(u8"World"), hello_u8.find(u8"World"));
  EXPECT_EQ(hello.find(u8"World", 0, 5), hello_u8.find(u8"World", 0, 5));
  EXPECT_EQ(hello.find(u8"NotFound"), ustring::npos);

  // Test find with character
  EXPECT_EQ(hello.find(u8'W'), hello_u8.find(u8'W'));
  EXPECT_EQ(hello.find(u8'Z'), ustring::npos);
  EXPECT_EQ(hello.find(u8'o', 8), hello_u8.find(u8'o', 8));
}

// Test rfind functions
TEST_F(UstringSearchTest, ReverseFind)
{
  // Test rfind with ustring
  EXPECT_EQ(mixed.rfind(ustring(u8"Hello")), mixed_u8.rfind(u8"Hello"));
  EXPECT_EQ(mixed.rfind(ustring(u8"NotFound")), ustring::npos);

  // Test rfind with C-string
  EXPECT_EQ(mixed.rfind(u8"Hello"), mixed_u8.rfind(u8"Hello"));
  EXPECT_EQ(mixed.rfind(u8"Hello", 13), mixed_u8.rfind(u8"Hello", 13));
  EXPECT_EQ(mixed.rfind(u8"Hello", 13, 5), mixed_u8.rfind(u8"Hello", 13, 5));

  // Test rfind with character
  EXPECT_EQ(mixed.rfind(u8'o'), mixed_u8.rfind(u8'o'));
  EXPECT_EQ(mixed.rfind(u8'o', 18), mixed_u8.rfind(u8'o', 18));
}

// Test find_first_of functions
TEST_F(UstringSearchTest, FindFirstOf)
{
  // Test find_first_of with ustring
  EXPECT_EQ(hello.find_first_of(ustring(u8"World")), hello_u8.find_first_of(u8"World"));  // 'l' is first match
  EXPECT_EQ(hello.find_first_of(ustring(u8"xyz")), ustring::npos);

  // Test find_first_of with C-string
  EXPECT_EQ(hello.find_first_of(u8"World"), hello_u8.find_first_of(u8"World"));
  EXPECT_EQ(hello.find_first_of(u8"World", 0, 5), hello_u8.find_first_of(u8"World", 0, 5));
  EXPECT_EQ(hello.find_first_of(u8"xyz"), ustring::npos);

  // Test find_first_of with character
  EXPECT_EQ(hello.find_first_of(u8'l'), hello_u8.find_first_of(u8'l'));
  EXPECT_EQ(hello.find_first_of(u8'z'), ustring::npos);
}

// Test find_last_of functions
TEST_F(UstringSearchTest, FindLastOf)
{
  // Test find_last_of with ustring
  EXPECT_EQ(hello.find_last_of(ustring(u8"World")),
            hello_u8.find_last_of(u8"World"));  // 'd' is last match
  EXPECT_EQ(hello.find_last_of(ustring(u8"xyz")), ustring::npos);

  // Test find_last_of with C-string
  EXPECT_EQ(hello.find_last_of(u8"World"), hello_u8.find_last_of(u8"World"));
  EXPECT_EQ(hello.find_last_of(u8"World", hello.size() - 1, 5),
            hello_u8.find_last_of(u8"World", hello_u8.size() - 1, 5));
  EXPECT_EQ(hello.find_last_of(u8"xyz"), ustring::npos);

  // Test find_last_of with character
  EXPECT_EQ(hello.find_last_of(u8'l'), hello_u8.find_last_of(u8'l'));
  EXPECT_EQ(hello.find_last_of(u8'z'), ustring::npos);
}

// Test find_first_not_of functions
TEST_F(UstringSearchTest, FindFirstNotOf)
{
  ustring spaces(u8"   Hello");
  std::u8string spaces_u8(u8"   Hello");

  // Test find_first_not_of with ustring
  EXPECT_EQ(spaces.find_first_not_of(ustring(u8" ")), spaces_u8.find_first_not_of(u8" "));
  EXPECT_EQ(hello.find_first_not_of(ustring(u8"Hel")), hello_u8.find_first_not_of(u8"Hel"));

  // Test find_first_not_of with C-string
  EXPECT_EQ(spaces.find_first_not_of(u8" "), spaces_u8.find_first_not_of(u8" "));
  EXPECT_EQ(spaces.find_first_not_of(u8" ", 0, 1), spaces_u8.find_first_not_of(u8" ", 0, 1));

  // Test find_first_not_of with character
  EXPECT_EQ(spaces.find_first_not_of(u8' '), spaces_u8.find_first_not_of(u8' '));
  EXPECT_EQ(hello.find_first_not_of(u8'H'), hello_u8.find_first_not_of(u8'H'));
}

// Test find_last_not_of functions
TEST_F(UstringSearchTest, FindLastNotOf)
{
  ustring spaces(u8"Hello   ");
  std::u8string spaces_u8(u8"Hello   ");

  // Test find_last_not_of with ustring
  EXPECT_EQ(spaces.find_last_not_of(ustring(u8" ")), spaces_u8.find_last_not_of(u8" "));
  EXPECT_EQ(hello.find_last_not_of(ustring(u8"!")), hello_u8.find_last_not_of(u8"!"));

  // Test find_last_not_of with C-string
  EXPECT_EQ(spaces.find_last_not_of(u8" "), spaces_u8.find_last_not_of(u8" "));
  EXPECT_EQ(spaces.find_last_not_of(u8" ", spaces.size() - 1, 1),
            spaces_u8.find_last_not_of(u8" ", spaces_u8.size() - 1, 1));

  // Test find_last_not_of with character
  EXPECT_EQ(spaces.find_last_not_of(u8' '), spaces_u8.find_last_not_of(u8' '));
  EXPECT_EQ(hello.find_last_not_of(u8'!'), hello_u8.find_last_not_of(u8'!'));
}

// Test edge cases
TEST_F(UstringSearchTest, EdgeCases)
{
  // Empty string tests
  EXPECT_EQ(empty.find(u8"anything"), ustring::npos);
  EXPECT_EQ(empty.rfind(u8"anything"), ustring::npos);
  EXPECT_EQ(empty.find_first_of(u8"anything"), ustring::npos);
  EXPECT_EQ(empty.find_last_of(u8"anything"), ustring::npos);
  EXPECT_EQ(empty.find_first_not_of(u8"anything"), ustring::npos);
  EXPECT_EQ(empty.find_last_not_of(u8"anything"), ustring::npos);

  // Null pointer tests
  EXPECT_EQ(hello.find(nullptr), ustring::npos);
  EXPECT_EQ(hello.rfind(nullptr), ustring::npos);
  EXPECT_EQ(hello.find_first_of(nullptr), ustring::npos);
  EXPECT_EQ(hello.find_last_of(nullptr), ustring::npos);
  EXPECT_EQ(hello.find_first_not_of(nullptr), 0);
  EXPECT_EQ(hello.find_last_not_of(nullptr), ustring::npos);
  EXPECT_EQ(hello.find_last_not_of(nullptr, 10), 10);

  // Position out of range tests
  EXPECT_EQ(hello.find(u8"World", hello.size() + 1), ustring::npos);
  EXPECT_EQ(hello.rfind(u8"World", hello.size() + 1), hello.rfind(u8"World"));
  EXPECT_EQ(hello.find_first_of(u8"World", hello.size() + 1), ustring::npos);
  EXPECT_EQ(hello.find_last_of(u8"World", hello.size() + 1), hello.find_last_of(u8"World"));
}

// Test with Unicode strings
TEST_F(UstringSearchTest, UnicodeStrings)
{
  ustring unicode(u8"Hello, 世界! こんにちは");
  std::u8string unicode_u8(u8"Hello, 世界! こんにちは");

  // Test find with Unicode characters
  EXPECT_EQ(unicode.find(u8"世界"), unicode_u8.find(u8"世界"));
  EXPECT_EQ(unicode.find(u8"こんにちは"), unicode_u8.find(u8"こんにちは"));

  // Test rfind with Unicode characters
  EXPECT_EQ(unicode.rfind(u8"世界"), unicode_u8.rfind(u8"世界"));

  // Test find_first_of with Unicode characters
  EXPECT_EQ(unicode.find_first_of(u8"世界こ"), unicode_u8.find_first_of(u8"世界こ"));

  // Test find_last_of with Unicode characters
  EXPECT_EQ(unicode.find_last_of(u8"世界こ"), unicode_u8.find_last_of(u8"世界こ"));

  // Test substring with Unicode characters
  auto sub = unicode.substr(7, sizeof(u8"世界!") / sizeof(char8_t) - 1);
  EXPECT_EQ(sub, u8"世界!");
}

// Test with more complex Unicode strings
TEST_F(UstringSearchTest, ComplexUnicodeStrings)
{
  ustring complex(u8"🌈Unicode🚀is🎭awesome🎨and😊fun🌟Unicode");
  std::u8string complex_u8(u8"🌈Unicode🚀is🎭awesome🎨and😊fun🌟Unicode");

  // Test find with emoji
  EXPECT_EQ(complex.find(u8"🚀"), complex_u8.find(u8"🚀"));
  EXPECT_EQ(complex.find(u8"😊fun🌟"), complex_u8.find(u8"😊fun🌟"));

  // Test rfind with mixed ASCII and emoji
  EXPECT_EQ(complex.rfind(u8"awesome🎨"), complex_u8.rfind(u8"awesome🎨"));

  // Test find_first_of with multiple emojis
  EXPECT_EQ(complex.find_first_of(u8"🎭🎨🌟"), complex_u8.find_first_of(u8"🎭🎨🌟"));

  // Test find_last_of with ASCII and emoji
  EXPECT_EQ(complex.find_last_of(u8"e🚀🌈"), complex_u8.find_last_of(u8"e🚀🌈"));

  // Test find_first_not_of with emoji
  EXPECT_EQ(complex.find_first_not_of(u8"🌈"), complex_u8.find_first_not_of(u8"🌈"));

  // Test find_last_not_of with ASCII
  EXPECT_EQ(complex.find_last_not_of(u8"🌟"), complex_u8.find_last_not_of(u8"🌟"));

  // Test substring with emoji boundaries
  auto sub = complex.substr(sizeof(u8"🌈") - 1, 7);  // Should be "Unicode"
  EXPECT_EQ(sub, u8"Unicode");

  // Test with strings containing zero-width joiners
  ustring zwj(u8"👨‍👩‍👧‍👦 Family👨‍👩‍👧‍👦 Family");
  EXPECT_EQ(zwj.find(u8"👨‍👩‍👧‍👦"), 0u);
  EXPECT_EQ(zwj.find(u8"Family"), sizeof("👨‍👩‍👧‍👦 ") - 1);
}
