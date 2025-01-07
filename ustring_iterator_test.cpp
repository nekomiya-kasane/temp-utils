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

TEST_F(UStringWordIteratorTest, BasicIteration)
{
  // Empty string
  auto empty_begin = empty_str.words_begin();
  auto empty_end = empty_str.words_end();
  EXPECT_EQ(empty_begin, empty_end);

  // Simple string
  std::vector<std::string> words;
  for (auto it = simple_str.words_begin(); it != simple_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }
  EXPECT_EQ(words.size(), 3);
  EXPECT_EQ(words[0], "Hello");
  EXPECT_EQ(words[1], " ");
  EXPECT_EQ(words[2], "World");
}

TEST_F(UStringWordIteratorTest, ComplexIteration)
{
  // "Hello,  World! How are you?\nI'm fine."
  std::vector<std::string> words;
  for (auto it = complex_str.words_begin(); it != complex_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }

  EXPECT_EQ(words.size(), 17);
  EXPECT_EQ(words[0], "Hello");
  EXPECT_EQ(words[1], ",");
  EXPECT_EQ(words[2], "  ");
  EXPECT_EQ(words[3], "World");
  EXPECT_EQ(words[4], "!");
  EXPECT_EQ(words[5], " ");
  EXPECT_EQ(words[6], "How");
  EXPECT_EQ(words[7], " ");
  EXPECT_EQ(words[8], "are");
  EXPECT_EQ(words[9], " ");
  EXPECT_EQ(words[10], "you");
  EXPECT_EQ(words[11], "?");
  EXPECT_EQ(words[12], "\n");
  EXPECT_EQ(words[13], "I'm");
  EXPECT_EQ(words[14], " ");
  EXPECT_EQ(words[15], "fine");
  EXPECT_EQ(words[16], ".");
}

TEST_F(UStringWordIteratorTest, UTF8Words)
{
  // "Hello 世界! 你好 🌍"
  std::vector<std::string> words;
  for (auto it = utf8_str.words_begin(); it != utf8_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }

  EXPECT_EQ(words.size(), 8);
  EXPECT_EQ(words[0], "Hello");
  EXPECT_EQ(words[1], " ");
  EXPECT_EQ(words[2], "世界");
  EXPECT_EQ(words[3], "!");
  EXPECT_EQ(words[4], " ");
  EXPECT_EQ(words[5], "你好");
  EXPECT_EQ(words[6], " ");
  EXPECT_EQ(words[7], "🌍");
}

TEST_F(UStringWordIteratorTest, WhitespaceHandling)
{
  // "   Multiple   Spaces   "
  // todo: is this right?
  std::vector<std::string> words;
  for (auto it = whitespace_str.words_begin(); it != whitespace_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }

  EXPECT_EQ(words.size(), 5);
  EXPECT_EQ(words[0], "   ");
  EXPECT_EQ(words[1], "Multiple");
  EXPECT_EQ(words[2], "   ");
  EXPECT_EQ(words[3], "Spaces");
  EXPECT_EQ(words[4], "   ");
}

TEST_F(UStringWordIteratorTest, PunctuationHandling)
{
  ustring punct_str("Hello, world! This... is-a_test; (with) [some] {punctuation}.");
  std::vector<std::string> words;
  for (auto it = punct_str.words_begin(); it != punct_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }

  EXPECT_EQ(words.size(), 28);
  EXPECT_EQ(words[0], "Hello");
  EXPECT_EQ(words[1], ",");
  EXPECT_EQ(words[2], " ");
  EXPECT_EQ(words[3], "world");
  EXPECT_EQ(words[4], "!");
  EXPECT_EQ(words[5], " ");
  EXPECT_EQ(words[6], "This");
  EXPECT_EQ(words[7], ".");
  EXPECT_EQ(words[8], ".");
  EXPECT_EQ(words[9], ".");
  EXPECT_EQ(words[10], " ");
  EXPECT_EQ(words[11], "is");
  EXPECT_EQ(words[12], "-");
  EXPECT_EQ(words[13], "a_test");
  EXPECT_EQ(words[14], ";");
  EXPECT_EQ(words[15], " ");
  EXPECT_EQ(words[16], "(");
  EXPECT_EQ(words[17], "with");
  EXPECT_EQ(words[18], ")");
  EXPECT_EQ(words[19], " ");
  EXPECT_EQ(words[20], "[");
  EXPECT_EQ(words[21], "some");
  EXPECT_EQ(words[22], "]");
  EXPECT_EQ(words[23], " ");
  EXPECT_EQ(words[24], "{");
  EXPECT_EQ(words[25], "punctuation");
  EXPECT_EQ(words[26], "}");
  EXPECT_EQ(words[27], ".");
}

TEST_F(UStringWordIteratorTest, NumberHandling)
{
  ustring num_str("Test123 456.789 1,000 0xFF 1.2e-3");
  std::vector<std::string> words;
  for (auto it = num_str.words_begin(); it != num_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }

  EXPECT_EQ(words.size(), 11);
  EXPECT_EQ(words[0], "Test123");
  EXPECT_EQ(words[1], " ");
  EXPECT_EQ(words[2], "456.789");
  EXPECT_EQ(words[3], " ");
  EXPECT_EQ(words[4], "1,000");
  EXPECT_EQ(words[5], " ");
  EXPECT_EQ(words[6], "0xFF");
  EXPECT_EQ(words[7], " ");
  EXPECT_EQ(words[8], "1.2e"); // todo: this is strange, why not 1.2e-3
  EXPECT_EQ(words[9], "-");
  EXPECT_EQ(words[10], "3");
}

TEST_F(UStringWordIteratorTest, MixedScriptWords)
{
  ustring mixed_str("English中文Mix混合Words词语");
  std::vector<std::string> words;
  for (auto it = mixed_str.words_begin(); it != mixed_str.words_end(); ++it) {
    words.push_back(it->to_string());
  }

  EXPECT_EQ(words.size(), 6);
  EXPECT_EQ(words[0], "English");
  EXPECT_EQ(words[1], "中文");
  EXPECT_EQ(words[2], "Mix");
  EXPECT_EQ(words[3], "混合");
  EXPECT_EQ(words[4], "Words");
  EXPECT_EQ(words[5], "词语");
}

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

TEST(IteratorTest, GraphemeIterator)
{
  // Basic ASCII string
  ustring ascii("Hello World");
  std::vector<ustring_view> graphemes;
  for (auto it = ascii.graphemes_begin(); it != ascii.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 11);  // Each ASCII char is one grapheme

  // String with combining characters
  ustring combining("e\u0301");  // é (e + acute accent)
  auto it = combining.graphemes_begin();
  EXPECT_EQ((*it).size(), 3);  // One grapheme cluster of 3 bytes
  ++it;
  EXPECT_EQ(it, combining.graphemes_end());

  // String with emoji
  ustring emoji("👨‍👩‍👧‍👦");  // Family emoji (multiple code points)
  it = emoji.graphemes_begin();
  EXPECT_NE(it, emoji.graphemes_end());
  EXPECT_EQ((*it).size(), emoji.size());  // Entire emoji is one grapheme
  ++it;
  EXPECT_EQ(it, emoji.graphemes_end());

  // Bidirectional iteration
  ustring text("ABC");
  it = text.graphemes_end();
  --it;
  EXPECT_EQ((*it)[0], 'C');
  --it;
  EXPECT_EQ((*it)[0], 'B');
  --it;
  EXPECT_EQ((*it)[0], 'A');

  // Empty string
  ustring empty("");
  EXPECT_EQ(empty.graphemes_begin(), empty.graphemes_end());
}

TEST(IteratorTest, SentenceIterator)
{
  // Basic sentences
  ustring text("Hello world. How are you? I'm fine!");
  std::vector<ustring_view> sentences;
  for (auto it = text.sentences_begin(); it != text.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);
  EXPECT_EQ(sentences[0], "Hello world. ");
  EXPECT_EQ(sentences[1], "How are you? ");
  EXPECT_EQ(sentences[2], "I'm fine!");

  // Single sentence
  ustring single("Just one sentence.");
  auto it = single.sentences_begin();
  EXPECT_EQ(*it, "Just one sentence.");
  ++it;
  EXPECT_EQ(it, single.sentences_end());

  // Multiple sentence endings
  ustring multiple("First!! Second?? Third...");
  sentences.clear();
  for (auto it = multiple.sentences_begin(); it != multiple.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Bidirectional iteration
  it = text.sentences_end();
  --it;
  EXPECT_EQ(*it, "I'm fine!");
  --it;
  EXPECT_EQ(*it, "How are you? ");
  --it;
  EXPECT_EQ(*it, "Hello world. ");

  // Empty string
  ustring empty("");
  EXPECT_EQ(empty.sentences_begin(), empty.sentences_end());

  // Unicode sentences
  ustring unicode("¡Hola! ¿Cómo estás? Bien.");
  sentences.clear();
  for (auto it = unicode.sentences_begin(); it != unicode.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);
}

// Test both iterators with various edge cases
TEST(IteratorTest, EdgeCases)
{
  // Mixed text with emojis and sentences
  ustring mixed("Hi 👋! How are you 😊? I'm good 👍.");

  // Test grapheme iteration
  int grapheme_count = 0;
  for (auto it = mixed.graphemes_begin(); it != mixed.graphemes_end(); ++it) {
    grapheme_count++;
  }
  EXPECT_GT(grapheme_count, 3);  // Should be more than just the emojis

  // Test sentence iteration
  std::vector<ustring_view> sentences;
  for (auto it = mixed.sentences_begin(); it != mixed.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Test with various whitespace
  ustring whitespace("\n\nFirst.\n\nSecond.\n\n");
  int sentence_count = 0;
  for (auto it = whitespace.sentences_begin(); it != whitespace.sentences_end(); ++it) {
    sentence_count++;
  }
  EXPECT_EQ(sentence_count, 6);

  // Test with non-breaking spaces and other special characters
  ustring special("First\u00A0sentence.\u2003Second\u2003sentence.");
  sentence_count = 0;
  for (auto it = special.sentences_begin(); it != special.sentences_end(); ++it) {
    sentence_count++;
  }
  EXPECT_EQ(sentence_count, 2);
}

// Test iterator invalidation
TEST(IteratorTest, IteratorInvalidation)
{
  ustring text("Hello. World.");
  auto sentence_it = text.sentences_begin();
  auto grapheme_it = text.graphemes_begin();

  // Store the current values
  auto sentence_view = *sentence_it;
  auto grapheme_view = *grapheme_it;

  // Create a new string with the same content
  ustring text2("Hello. World.");

  // The iterators should still be valid for the original string
  EXPECT_EQ(*sentence_it, sentence_view);
  EXPECT_EQ(*grapheme_it, grapheme_view);
}

// Test copy construction and assignment
TEST(IteratorTest, CopyAndAssignment)
{
  ustring text("Hello. World.");

  // Test sentence iterator
  auto sent_it1 = text.sentences_begin();
  auto sent_it2(sent_it1);  // Copy construction
  EXPECT_EQ(*sent_it1, *sent_it2);

  auto sent_it3 = text.sentences_end();
  sent_it3 = sent_it1;  // Assignment
  EXPECT_EQ(*sent_it1, *sent_it3);

  // Test grapheme iterator
  auto graph_it1 = text.graphemes_begin();
  auto graph_it2(graph_it1);  // Copy construction
  EXPECT_EQ(*graph_it1, *graph_it2);

  auto graph_it3 = text.graphemes_end();
  graph_it3 = graph_it1;  // Assignment
  EXPECT_EQ(*graph_it1, *graph_it3);
}

// Test grapheme iterator with various scripts and languages
TEST(IteratorTest, MultilingualGraphemes)
{
  // Chinese characters and punctuation
  ustring chinese("你好，世界！");
  std::vector<ustring_view> graphemes;
  for (auto it = chinese.graphemes_begin(); it != chinese.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 6);  // 5 characters + 1 punctuation

  // Korean with combining characters
  ustring korean("안녕하세요");  // Hello in Korean
  graphemes.clear();
  for (auto it = korean.graphemes_begin(); it != korean.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 5);  // 5 syllable blocks

  // Thai with combining marks
  ustring thai("สวัสดี");  // Hello in Thai
  graphemes.clear();
  for (auto it = thai.graphemes_begin(); it != thai.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 4);  // Thai clusters

  // Devanagari with combining marks
  ustring devanagari("नमस्ते");  // Namaste in Hindi
  auto it = devanagari.graphemes_begin();
  EXPECT_NE(it, devanagari.graphemes_end());
  EXPECT_GT((*it).size(), 1);  // Should be more than 1 byte due to combining marks

  // Zero-width joiner sequences
  ustring zwj("👨‍👩‍👧‍👦👨‍💻👩‍🔬");
  graphemes.clear();
  for (auto it = zwj.graphemes_begin(); it != zwj.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 3);  // 3 emoji sequences

  // Regional indicators (flags)
  ustring flags("🇯🇵🇰🇷🇨🇳");  // Japan, Korea, China flags
  graphemes.clear();
  for (auto it = flags.graphemes_begin(); it != flags.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 3);  // 3 flags
}

// Test sentence iterator with various scripts and languages
TEST(IteratorTest, MultilingualSentences)
{
  // Chinese with mixed punctuation
  ustring chinese("你好！这是一个测试。你觉得怎么样？");
  std::vector<ustring_view> sentences;
  for (auto it = chinese.sentences_begin(); it != chinese.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Japanese with mixed punctuation
  ustring japanese("こんにちは。元気ですか？はい、元気です！");
  sentences.clear();
  for (auto it = japanese.sentences_begin(); it != japanese.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Korean with various endings
  ustring korean("안녕하세요? 잘 지내요... 네, 감사합니다!");
  sentences.clear();
  for (auto it = korean.sentences_begin(); it != korean.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Mixed scripts
  ustring mixed("Hello! 你好。안녕하세요? Bonjour!");
  sentences.clear();
  for (auto it = mixed.sentences_begin(); it != mixed.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 4);

  // Arabic with different sentence endings
  ustring arabic("مرحبا! كيف حالك؟ أنا بخير.");
  sentences.clear();
  for (auto it = arabic.sentences_begin(); it != arabic.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Complex punctuation cases
  ustring complex("Test... test...? Test!.. Next.");
  sentences.clear();
  for (auto it = complex.sentences_begin(); it != complex.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);
}

// Test edge cases with special Unicode characters
TEST(IteratorTest, SpecialUnicodeEdgeCases)
{
  // Surrogate pairs
  ustring surrogate("𝄞𝄢𝄪");  // Musical symbols
  int count = 0;
  for (auto it = surrogate.graphemes_begin(); it != surrogate.graphemes_end(); ++it) {
    count++;
  }
  EXPECT_EQ(count, 3);

  // Variation selectors
  ustring variation("️☹️☺️");  // Emoji variation sequences
  count = 0;
  for (auto it = variation.graphemes_begin(); it != variation.graphemes_end(); ++it) {
    count++;
  }
  EXPECT_EQ(count, 3);

  // Bidirectional text
  ustring bidi("Hello! مرحبا! שָׁלוֹם!");
  std::vector<ustring_view> sentences;
  for (auto it = bidi.sentences_begin(); it != bidi.sentences_end(); ++it) {
    sentences.push_back(*it);
  }
  EXPECT_EQ(sentences.size(), 3);

  // Text with various Unicode spaces and separators
  ustring spaces("First\u2028Second\u2029Third");  // Line/paragraph separators
  count = 0;
  for (auto it = spaces.sentences_begin(); it != spaces.sentences_end(); ++it) {
    count++;
  }
  EXPECT_EQ(count, 3);

  // Combining characters with multiple combining marks
  ustring combining("e\u0301\u0308");  // e + acute + diaeresis
  auto it = combining.graphemes_begin();
  EXPECT_EQ((*it).size(), 5);  // One grapheme cluster of 4 bytes
  ++it;
  EXPECT_EQ(it, combining.graphemes_end());
}

// Test boundary conditions and error cases
TEST(IteratorTest, BoundaryAndErrorCases)
{
  // Empty string iterations
  ustring empty;
  EXPECT_EQ(empty.graphemes_begin(), empty.graphemes_end());
  EXPECT_EQ(empty.sentences_begin(), empty.sentences_end());

  // Single character strings
  ustring single(".");
  auto git = single.graphemes_begin();
  EXPECT_NE(git, single.graphemes_end());
  ++git;
  EXPECT_EQ(git, single.graphemes_end());

  // String with only spaces
  ustring spaces("   \t\n   ");
  std::vector<ustring_view> graphemes;
  for (auto it = spaces.graphemes_begin(); it != spaces.graphemes_end(); ++it) {
    graphemes.push_back(*it);
  }
  EXPECT_EQ(graphemes.size(), 8);

  // Invalid UTF-8 sequences should be handled gracefully
  // Note: This test depends on how ICU handles invalid sequences
  ustring invalid("\xFF\xFE\xFD");
  int count = 0;
  for (auto it = invalid.graphemes_begin(); it != invalid.graphemes_end(); ++it) {
    count++;
  }
  EXPECT_GT(count, 0);  // Should handle invalid sequences somehow

  // Test iterator movement at boundaries
  ustring text("A.B.C.");
  auto sit = text.sentences_begin();
  auto end = text.sentences_end();

  // Move forward to end
  while (sit != end) {
    ++sit;
  }
  // Try moving past end
  ++sit;
  EXPECT_EQ(sit, end);

  // Move backward from end
  while (sit != text.sentences_begin()) {
    --sit;
  }
  // Try moving before beginning
  --sit;
  EXPECT_EQ(sit, text.sentences_begin());
}
