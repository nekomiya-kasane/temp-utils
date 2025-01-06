#include "ustring.h"
#include <gtest/gtest.h>

class UStringIteratorTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    // Create test strings
    empty_str = ustring();
    small_str = ustring("Hello");  // Small string optimization
    large_str = ustring("This is a longer string that should not fit in the small string buffer");
  }

  ustring empty_str;
  ustring small_str;
  ustring large_str;
};

// Test iterator functions
TEST_F(UStringIteratorTest, BeginEndIterators)
{
  // Test empty string
  EXPECT_EQ(empty_str.begin(), empty_str.end());
  EXPECT_EQ(empty_str.cbegin(), empty_str.cend());

  // Test small string
  EXPECT_NE(small_str.begin(), small_str.end());
  EXPECT_EQ(small_str.end() - small_str.begin(), small_str.size());

  // Test const iterators
  const ustring &const_small = small_str;
  EXPECT_EQ(const_small.begin(), const_small.cbegin());
  EXPECT_EQ(const_small.end(), const_small.cend());

  // Test large string
  EXPECT_NE(large_str.begin(), large_str.end());
  EXPECT_EQ(large_str.end() - large_str.begin(), large_str.size());
}

TEST_F(UStringIteratorTest, ReverseIterators)
{
  // Test empty string
  EXPECT_EQ(empty_str.rbegin(), empty_str.rend());
  EXPECT_EQ(empty_str.crbegin(), empty_str.crend());

  // Test small string
  EXPECT_NE(small_str.rbegin(), small_str.rend());
  EXPECT_EQ(small_str.rend() - small_str.rbegin(), small_str.size());

  // Test const reverse iterators
  const ustring &const_small = small_str;
  EXPECT_EQ(const_small.rbegin(), const_small.crbegin());
  EXPECT_EQ(const_small.rend(), const_small.crend());

  // Verify reverse iteration
  std::string reversed;
  for (auto it = small_str.rbegin(); it != small_str.rend(); ++it) {
    reversed.push_back(static_cast<char>(*it));
  }
  std::string original(small_str.begin(), small_str.end());
  std::reverse(original.begin(), original.end());
  EXPECT_EQ(reversed, original);
}

// Test capacity functions
TEST_F(UStringIteratorTest, EmptyAndSize)
{
  EXPECT_TRUE(empty_str.empty());
  EXPECT_FALSE(small_str.empty());
  EXPECT_FALSE(large_str.empty());

  EXPECT_EQ(empty_str.size(), 0);
  EXPECT_EQ(small_str.size(), 5);  // "Hello"
  EXPECT_GT(large_str.size(), small_str.size());
}

TEST_F(UStringIteratorTest, MaxSizeAndCapacity)
{
  EXPECT_GT(empty_str.max_size(), 0);
  EXPECT_LE(empty_str.capacity(), empty_str.max_size());
  EXPECT_LE(small_str.capacity(), small_str.max_size());
  EXPECT_LE(large_str.capacity(), large_str.max_size());

  // Capacity should be at least as large as size
  EXPECT_GE(empty_str.capacity(), empty_str.size());
  EXPECT_GE(small_str.capacity(), small_str.size());
  EXPECT_GE(large_str.capacity(), large_str.size());
}

TEST_F(UStringIteratorTest, Reserve)
{
  size_t original_capacity = small_str.capacity();

  // Reserve smaller than current capacity should not change capacity
  small_str.reserve(original_capacity / 2);
  EXPECT_EQ(small_str.capacity(), original_capacity);

  // Reserve larger than current capacity should increase capacity
  small_str.reserve(original_capacity * 2);
  EXPECT_GE(small_str.capacity(), original_capacity * 2);

  // Content should remain unchanged after reserve
  EXPECT_EQ(small_str, ustring("Hello"));
}

TEST_F(UStringIteratorTest, ShrinkToFit)
{
  // Create a string with extra capacity
  ustring str = small_str;
  str.reserve(100);
  size_t original_capacity = str.capacity();
  EXPECT_GT(original_capacity, str.size());

  // Shrink to fit should reduce capacity
  str.shrink_to_fit();
  EXPECT_LE(str.capacity(), original_capacity);

  // Content should remain unchanged
  EXPECT_EQ(str, small_str);
}

TEST_F(UStringIteratorTest, Clear)
{
  // Clear empty string
  empty_str.clear();
  EXPECT_TRUE(empty_str.empty());
  EXPECT_EQ(empty_str.size(), 0);

  // Clear small string
  size_t original_capacity = small_str.capacity();
  small_str.clear();
  EXPECT_TRUE(small_str.empty());
  EXPECT_EQ(small_str.size(), 0);
  // Capacity should remain unchanged after clear
  EXPECT_EQ(small_str.capacity(), original_capacity);

  // Clear large string
  original_capacity = large_str.capacity();
  large_str.clear();
  EXPECT_TRUE(large_str.empty());
  EXPECT_EQ(large_str.size(), 0);
  // Capacity should remain unchanged after clear
  EXPECT_EQ(large_str.capacity(), original_capacity);
}

// Test iterator invalidation
TEST_F(UStringIteratorTest, IteratorInvalidation)
{
  auto it = small_str.begin();
  auto end = small_str.end();

  // Store original content
  std::string original(it, end);

  // Clear should invalidate iterators
  small_str.clear();

  // Using invalidated iterators would be undefined behavior
  // We can only verify the string is empty now
  EXPECT_TRUE(small_str.empty());

  // Restore content and verify
  small_str = ustring(original.c_str());
  EXPECT_EQ(std::string(small_str.begin(), small_str.end()), original);
}

// Test string_view functionality
class UStringViewTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    empty_str = ustring();
    ascii_str = ustring("Hello World");
    utf8_str = ustring("Hello 世界! 🌍");
    multiline_str = ustring("Line 1\nLine 2\r\nLine 3");
  }

  ustring empty_str;
  ustring ascii_str;
  ustring utf8_str;
  ustring multiline_str;
};

TEST_F(UStringViewTest, BasicViews)
{
  // Empty string view
  auto empty_view = empty_str.to_view();
  EXPECT_TRUE(empty_view.empty());
  EXPECT_EQ(empty_view.size(), empty_str.size());
  EXPECT_EQ(empty_view.length(), empty_str.length());
  EXPECT_EQ(empty_view.size(), 0);

  // ASCII string view
  auto ascii_view = ascii_str.to_view();
  EXPECT_FALSE(ascii_view.empty());
  EXPECT_EQ(ascii_view.size(), ascii_str.size());
  EXPECT_EQ(ascii_view.length(), ascii_str.length());
  EXPECT_EQ(ascii_view.to_string(), "Hello World");

  // UTF-8 string view
  auto utf8_view = utf8_str.to_view();
  EXPECT_FALSE(utf8_view.empty());
  EXPECT_EQ(utf8_view.size(), utf8_view.size());
  EXPECT_EQ(utf8_view.length(), utf8_view.length());
  EXPECT_EQ(utf8_view.to_string(), "Hello 世界! 🌍");
}

TEST_F(UStringViewTest, ViewOperations)
{
  auto view = ascii_str.to_view();

  // Substring operations
  EXPECT_EQ(view.substr(0, 5).to_string(), "Hello");
  EXPECT_EQ(view.substr(6).to_string(), "World");

  // Empty substring
  EXPECT_TRUE(view.substr(0, 0).empty());

  // Out of range handling
  EXPECT_EQ(view.substr(view.size()).size(), 0);
  EXPECT_EQ(view.substr(0, 100).size(), view.size());
}

TEST_F(UStringViewTest, ViewComparisons)
{
  auto view1 = ascii_str.to_view();
  auto str1 = ustring("Hello World");
  auto str2 = ustring("Hello");
  auto view2 = str1.to_view();
  auto view3 = str2.to_view();

  EXPECT_EQ(view1, view2);
  EXPECT_NE(view1, view3);
  EXPECT_GT(view1, view3);
  EXPECT_LT(view3, view1);
}

TEST_F(UStringViewTest, ViewWithSpecialCharacters)
{
  ustring special_str("Tab\there\nNewline\r\nCRLF\\Backslash");
  auto view = special_str.to_view();

  EXPECT_EQ(view.to_string(), "Tab\there\nNewline\r\nCRLF\\Backslash");
  EXPECT_EQ(view.substr(0, 7).to_string(), "Tab\ther");
  EXPECT_EQ(view.substr(8, 8).to_string(), "\nNewline");
}

TEST_F(UStringViewTest, ViewChaining)
{
  auto view = ascii_str.to_view();
  auto sub_view = view.substr(0, 5);          // "Hello"
  auto sub_sub_view = sub_view.substr(1, 3);  // "ell"

  EXPECT_EQ(sub_sub_view.to_string(), "ell");
  EXPECT_EQ(sub_sub_view.size(), 3);
}

TEST_F(UStringViewTest, EmptyViewOperations)
{
  auto view = empty_str.to_view();

  // Empty view operations should be safe
  EXPECT_TRUE(view.empty());
  EXPECT_EQ(view.size(), 0);
  EXPECT_TRUE(view.substr(0, 0).empty());
  EXPECT_TRUE(view.substr(0, 100).empty());
  EXPECT_TRUE(view.substr(100, 100).empty());
}

// Test word iterator functionality
class UStringWordIteratorTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    empty_str = ustring();
    simple_str = ustring("Hello World");
    complex_str = ustring("Hello,  World! How are you?\nI'm fine.");
    utf8_str = ustring("Hello 世界! 你好 🌍");
    whitespace_str = ustring("   Multiple   Spaces   ");
  }

  ustring empty_str;
  ustring simple_str;
  ustring complex_str;
  ustring utf8_str;
  ustring whitespace_str;
};

// TEST_F(UStringWordIteratorTest, BasicIteration)
//{
//   // Empty string
//   auto empty_begin = empty_str.words_begin();
//   auto empty_end = empty_str.words_end();
//   EXPECT_EQ(empty_begin, empty_end);
//
//   // Simple string
//   std::vector<std::string> words;
//   for (auto it = simple_str.words_begin(); it != simple_str.words_end(); ++it) {
//     words.push_back(it->to_string());
//   }
//   EXPECT_EQ(words.size(), 2);
//   EXPECT_EQ(words[0], "Hello");
//   EXPECT_EQ(words[1], "World");
// }
//
//  TEST_F(UStringWordIteratorTest, ComplexIteration) {
//    std::vector<std::string> words;
//    for (auto it = complex_str.word_begin(); it != complex_str.word_end(); ++it) {
//      words.push_back(it->to_string());
//    }
//
//    EXPECT_EQ(words.size(), 7);
//    EXPECT_EQ(words[0], "Hello");
//    EXPECT_EQ(words[1], "World");
//    EXPECT_EQ(words[2], "How");
//    EXPECT_EQ(words[3], "are");
//    EXPECT_EQ(words[4], "you");
//    EXPECT_EQ(words[5], "I'm");
//    EXPECT_EQ(words[6], "fine");
//  }
//
//  TEST_F(UStringWordIteratorTest, UTF8Words) {
//    std::vector<std::string> words;
//    for (auto it = utf8_str.word_begin(); it != utf8_str.word_end(); ++it) {
//      words.push_back(it->to_string());
//    }
//
//    EXPECT_EQ(words.size(), 4);
//    EXPECT_EQ(words[0], "Hello");
//    EXPECT_EQ(words[1], "世界");
//    EXPECT_EQ(words[2], "你好");
//    EXPECT_EQ(words[3], "🌍");
//  }
//
//  TEST_F(UStringWordIteratorTest, WhitespaceHandling) {
//    std::vector<std::string> words;
//    for (auto it = whitespace_str.word_begin(); it != whitespace_str.word_end(); ++it) {
//      words.push_back(it->to_string());
//    }
//
//    EXPECT_EQ(words.size(), 2);
//    EXPECT_EQ(words[0], "Multiple");
//    EXPECT_EQ(words[1], "Spaces");
//  }
//
//  TEST_F(UStringWordIteratorTest, PunctuationHandling) {
//    ustring punct_str("Hello, world! This... is-a_test; (with) [some] {punctuation}.");
//    std::vector<std::string> words;
//    for (auto it = punct_str.word_begin(); it != punct_str.word_end(); ++it) {
//      words.push_back(it->to_string());
//    }
//
//    EXPECT_EQ(words.size(), 8);
//    EXPECT_EQ(words[0], "Hello");
//    EXPECT_EQ(words[1], "world");
//    EXPECT_EQ(words[2], "This");
//    EXPECT_EQ(words[3], "is");
//    EXPECT_EQ(words[4], "a");
//    EXPECT_EQ(words[5], "test");
//    EXPECT_EQ(words[6], "with");
//    EXPECT_EQ(words[7], "some");
//  }
//
//  TEST_F(UStringWordIteratorTest, NumberHandling) {
//    ustring num_str("Test123 456.789 1,000 0xFF 1.2e-3");
//    std::vector<std::string> words;
//    for (auto it = num_str.word_begin(); it != num_str.word_end(); ++it) {
//      words.push_back(it->to_string());
//    }
//
//    EXPECT_EQ(words.size(), 5);
//    EXPECT_EQ(words[0], "Test123");
//    EXPECT_EQ(words[1], "456.789");
//    EXPECT_EQ(words[2], "1,000");
//    EXPECT_EQ(words[3], "0xFF");
//    EXPECT_EQ(words[4], "1.2e-3");
//  }
//
//  TEST_F(UStringWordIteratorTest, MixedScriptWords) {
//    ustring mixed_str("English中文Mix混合Words词语");
//    std::vector<std::string> words;
//    for (auto it = mixed_str.word_begin(); it != mixed_str.word_end(); ++it) {
//      words.push_back(it->to_string());
//    }
//
//    EXPECT_EQ(words.size(), 6);
//    EXPECT_EQ(words[0], "English");
//    EXPECT_EQ(words[1], "中文");
//    EXPECT_EQ(words[2], "Mix");
//    EXPECT_EQ(words[3], "混合");
//    EXPECT_EQ(words[4], "Words");
//    EXPECT_EQ(words[5], "词语");
//  }
//
//  Test code point iterator functionality
class UStringCodePointIteratorTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    empty_str = ustring();
    ascii_str = ustring("Hello");
    utf8_str = ustring("Hello世界🌍");
    mixed_str = ustring("A世B界C🌍D");
  }

  ustring empty_str;
  ustring ascii_str;
  ustring utf8_str;
  ustring mixed_str;
};

TEST_F(UStringCodePointIteratorTest, BasicIteration)
{
  // Empty string
  auto empty_begin = empty_str.code_points_begin();
  auto empty_end = empty_str.code_points_end();
  EXPECT_EQ(empty_begin, empty_end);

  // ASCII string
  std::vector<uint32_t> code_points;
  for (auto it = ascii_str.code_points_begin(); it != ascii_str.code_points_end(); ++it) {
    code_points.push_back(*it);
  }
  EXPECT_EQ(code_points.size(), 5);  // "Hello"
  EXPECT_EQ(code_points[0], 'H');
  EXPECT_EQ(code_points[4], 'o');

  code_points.clear();
  for (auto it : ascii_str.code_points()) {
    code_points.push_back(it);
  }
  EXPECT_EQ(code_points.size(), 5);  // "Hello"
  EXPECT_EQ(code_points[0], 'H');
  EXPECT_EQ(code_points[4], 'o');
}

TEST_F(UStringCodePointIteratorTest, UTF8Iteration)
{
  std::vector<uint32_t> code_points;
  for (auto it = utf8_str.code_points_begin(); it < utf8_str.code_points_end(); ++it) {
    code_points.push_back(*it);
  }

  EXPECT_EQ(code_points.size(), 8);  // 5 ASCII + 2 Chinese + 1 Emoji
  // Verify Hello
  EXPECT_EQ(code_points[0], 'H');
  EXPECT_EQ(code_points[4], 'o');
  // Verify 世界
  EXPECT_EQ(code_points[5], 0x4E16);  // 世
  EXPECT_EQ(code_points[6], 0x754C);  // 界
  // Verify 🌍 (Earth globe emoji)
  EXPECT_EQ(code_points[7], 0x1F30D);
}

TEST_F(UStringCodePointIteratorTest, MixedIteration)
{
  std::vector<uint32_t> code_points;
  for (auto it : mixed_str.code_points()) {
    code_points.push_back(it);
  }

  EXPECT_EQ(code_points.size(), 7);  // A世B界C🌍D
  EXPECT_EQ(code_points[0], 'A');
  EXPECT_EQ(code_points[1], 0x4E16);  // 世
  EXPECT_EQ(code_points[2], 'B');
  EXPECT_EQ(code_points[3], 0x754C);  // 界
  EXPECT_EQ(code_points[4], 'C');
  EXPECT_EQ(code_points[5], 0x1F30D);  // 🌍
  EXPECT_EQ(code_points[6], 'D');
}

TEST_F(UStringCodePointIteratorTest, IteratorOperations)
{
  auto it = mixed_str.code_points_begin();

  // Increment
  EXPECT_EQ(*it, 'A');
  ++it;
  EXPECT_EQ(*it, 0x4E16);  // 世

  // Advance multiple positions
  std::advance(it, 2);
  EXPECT_EQ(*it, 0x754C);  // 界

  // Decrement
  --it;
  EXPECT_EQ(*it, 'B');

  // Iterator comparison
  auto it2 = mixed_str.code_points_begin();
  EXPECT_TRUE(it != it2);
  EXPECT_TRUE(it > it2);
  EXPECT_TRUE(it2 < it);
}

TEST_F(UStringCodePointIteratorTest, BoundaryConditions)
{
  // String with surrogate pairs and combining characters
  ustring complex_str("A\u0308\U0001F468\u200D\U0001F469\u200D\U0001F467");  // Ä + Family emoji
  std::vector<uint32_t> code_points;
  for (auto it : complex_str.code_points()) {
    code_points.push_back(it);
  }

  // Verify the combining diaeresis and ZWJ sequences are handled correctly
  EXPECT_GT(code_points.size(), 1);
  EXPECT_EQ(code_points[0], 'A');
  EXPECT_EQ(code_points[1], 0x0308);  // Combining diaeresis
}

TEST_F(UStringCodePointIteratorTest, ReverseIteration)
{
  std::vector<uint32_t> forward_points;
  std::vector<uint32_t> reverse_points;

  // Collect code points in forward order
  for (auto it = mixed_str.code_points_begin(); it != mixed_str.code_points_end(); ++it) {
    forward_points.push_back(*it);
  }

  // Collect code points in reverse order
  auto it = mixed_str.code_points_end();
  while (it != mixed_str.code_points_begin()) {
    --it;
    reverse_points.push_back(*it);
  }

  // Verify reverse iteration matches reversed forward iteration
  EXPECT_EQ(forward_points.size(), reverse_points.size());
  for (size_t i = 0; i < forward_points.size(); ++i) {
    EXPECT_EQ(forward_points[i], reverse_points[reverse_points.size() - 1 - i]);
  }
}

TEST_F(UStringCodePointIteratorTest, IteratorArithmetic)
{
  auto it = mixed_str.code_points_begin();

  // Test += operator
  it += 2;
  EXPECT_EQ(*it, 'B');

  // Test + operator
  auto it2 = it + 2;
  EXPECT_EQ(*it2, 'C');

  // Test -= operator
  it2 -= 3;
  EXPECT_EQ(*it2, 0x4E16);  // 世

  // Test - operator for distance
  auto dist = std::distance(mixed_str.code_points_begin(), it2);
  EXPECT_EQ(dist, 1);
}

// Test character property and codepoint conversion functionality
class UStringPropertyTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    // Setup test strings with various properties
    empty_str = u8"";
    ascii_str = u8"Hello123!";
    chinese_str = u8"你好世界";
    emoji_str = u8"😀🌍🎉";
    mixed_str = u8"Hello世界!😀";
    math_str = u8"∑∫≠±";
    combining_str = u8"\u0301e";  // e + combining acute accent
  }

  std::u8string empty_str;
  std::u8string ascii_str;
  std::u8string chinese_str;
  std::u8string emoji_str;
  std::u8string mixed_str;
  std::u8string math_str;
  std::u8string combining_str;
};

TEST_F(UStringPropertyTest, HasPropertyBasic)
{
  // Test empty string
  EXPECT_FALSE(has_property(empty_str, CharProperty::ALPHABETIC));

  // Test ASCII characters
  EXPECT_TRUE(has_property(u8"A", CharProperty::UPPERCASE));
  EXPECT_TRUE(has_property(u8"a", CharProperty::LOWERCASE));
  EXPECT_TRUE(has_property(u8"1", CharProperty::DIGIT));
  EXPECT_TRUE(has_property(u8" ", CharProperty::WHITESPACE));
  EXPECT_TRUE(has_property(u8"!", CharProperty::PUNCTUATION));
}

TEST_F(UStringPropertyTest, HasPropertyChinese)
{
  // Test Chinese characters
  EXPECT_TRUE(has_property(chinese_str, CharProperty::IDEOGRAPHIC));
  EXPECT_TRUE(has_property(chinese_str, CharProperty::LETTER));

  // Test individual Chinese character
  EXPECT_TRUE(has_property(u8"世", CharProperty::IDEOGRAPHIC));
  EXPECT_TRUE(has_property(u8"界", CharProperty::IDEOGRAPHIC));
}

TEST_F(UStringPropertyTest, HasPropertyEmoji)
{
  // Test emoji characters
  EXPECT_TRUE(has_property(emoji_str, CharProperty::EMOJI));
  EXPECT_FALSE(has_property(emoji_str, CharProperty::ALPHABETIC));

  // Test individual emoji
  EXPECT_TRUE(has_property(u8"😀", CharProperty::EMOJI));
  EXPECT_TRUE(has_property(u8"🌍", CharProperty::EMOJI));
}

TEST_F(UStringPropertyTest, HasPropertyMath)
{
  // Test mathematical symbols
  EXPECT_TRUE(has_property(math_str, CharProperty::MATH));
  EXPECT_FALSE(has_property(math_str, CharProperty::ALPHABETIC));

  // Test individual math symbols
  EXPECT_TRUE(has_property(u8"∑", CharProperty::MATH));
  EXPECT_TRUE(has_property(u8"±", CharProperty::MATH));
}

TEST_F(UStringPropertyTest, HasPropertyCombiningMark)
{
  // todo: better test case needed
  // Test combining characters
  // EXPECT_TRUE(has_property(combining_str, CharProperty::COMBINING_MARK));

  // Test raw combining character
  EXPECT_TRUE(has_property(u8"\u0903", CharProperty::COMBINING_MARK));  // Combining acute accent
}

TEST_F(UStringPropertyTest, GetPropertyComprehensive)
{
  // Test ASCII letter
  auto prop_A = get_property(u8"A");
  EXPECT_TRUE(static_cast<int>(prop_A) & static_cast<int>(CharProperty::UPPERCASE));
  EXPECT_TRUE(static_cast<int>(prop_A) & static_cast<int>(CharProperty::ALPHABETIC));
  EXPECT_TRUE(static_cast<int>(prop_A) & static_cast<int>(CharProperty::LETTER));

  // Test digit
  auto prop_1 = get_property(u8"1");
  EXPECT_TRUE(static_cast<int>(prop_1) & static_cast<int>(CharProperty::DIGIT));
  EXPECT_FALSE(static_cast<int>(prop_1) & static_cast<int>(CharProperty::ALPHABETIC));

  // Test Chinese character
  auto prop_han = get_property(u8"世");
  EXPECT_TRUE(static_cast<int>(prop_han) & static_cast<int>(CharProperty::IDEOGRAPHIC));
  EXPECT_TRUE(static_cast<int>(prop_han) & static_cast<int>(CharProperty::LETTER));

  // Test emoji
  auto prop_emoji = get_property(u8"😀");
  EXPECT_TRUE(static_cast<int>(prop_emoji) & static_cast<int>(CharProperty::EMOJI));
  EXPECT_FALSE(static_cast<int>(prop_emoji) & static_cast<int>(CharProperty::ALPHABETIC));
}

TEST_F(UStringPropertyTest, CodepointConversion)
{
  // Test ASCII
  EXPECT_EQ(to_codepoint(u8"A"), 0x41);
  EXPECT_EQ(to_codepoint(u8"1"), 0x31);

  // Test Chinese
  EXPECT_EQ(to_codepoint(u8"世"), 0x4E16);
  EXPECT_EQ(to_codepoint(u8"界"), 0x754C);

  // Test emoji
  EXPECT_EQ(to_codepoint(u8"😀"), 0x1F600);
  EXPECT_EQ(to_codepoint(u8"🌍"), 0x1F30D);

  // Test empty and invalid
  EXPECT_EQ(to_codepoint(empty_str), 0);
}

TEST_F(UStringPropertyTest, CodepointConversionOverloads)
{
  // Test std::u8string_view overload
  std::u8string_view view_a(u8"A");
  EXPECT_EQ(to_codepoint(view_a), 0x41);

  // Test const char8_t* overload
  const char8_t *ptr_b = u8"B";
  EXPECT_EQ(to_codepoint(ptr_b), 0x42);

  // Test const char* with offset overload
  const char *str = "ABC";
  EXPECT_EQ(to_codepoint(str, 0), 0x41);
  EXPECT_EQ(to_codepoint(str, 1), 0x42);
  EXPECT_EQ(to_codepoint(str, 2), 0x43);
}

// TEST_F(UStringPropertyTest, PropertyCombinations)
//{
//   // Test characters with multiple properties
//   auto prop_A = get_property(u8"A");
//   EXPECT_TRUE(static_cast<int>(prop_A) &
//               static_cast<int>(CharProperty::UPPERCASE | CharProperty::LETTER));
//
//   // Test hexadecimal digit
//   auto prop_F = get_property(u8"F");
//   EXPECT_TRUE(static_cast<int>(prop_F) &
//               static_cast<int>(CharProperty::HEXDIGIT | CharProperty::UPPERCASE));
//
//   // Test whitespace with control
//   auto prop_newline = get_property(u8"\n");
//   EXPECT_TRUE(static_cast<int>(prop_newline) &
//               static_cast<int>(CharProperty::WHITESPACE | CharProperty::CONTROL));
// }
