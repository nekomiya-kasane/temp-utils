#include "ustring.h"
#include <gtest/gtest.h>
#include <string>

class UStringModificationTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Test append operations
TEST_F(UStringModificationTest, AppendString)
{
  ustring str1("Hello");
  ustring str2(" World");
  str1.append(str2);
  EXPECT_EQ(str1, ustring("Hello World"));
}

TEST_F(UStringModificationTest, AppendCString)
{
  ustring str("Hello");
  str.append(" World");
  EXPECT_EQ(str, ustring("Hello World"));

  // Test with nullptr
  str.append((char *)nullptr);
  EXPECT_EQ(str, ustring("Hello World"));  // Should remain unchanged
}

TEST_F(UStringModificationTest, AppendChar)
{
  ustring str("Hello");
  str.append('!');
  EXPECT_EQ(str, ustring("Hello!"));
}

TEST_F(UStringModificationTest, AppendSubstring)
{
  ustring str("Hello");
  const ustring::value_type *s = reinterpret_cast<const ustring::value_type *>(" World");
  str.append(s, 3);  // Append " Wo"
  EXPECT_EQ(str, ustring("Hello Wo"));
}

TEST_F(UStringModificationTest, AppendRepeatedChar)
{
  ustring str("Hello");
  str.append(3, '!');  // Append "!!!"
  EXPECT_EQ(str, ustring("Hello!!!"));
}

// Test operator+= operations
TEST_F(UStringModificationTest, OperatorPlusEqualsString)
{
  ustring str("Hello");
  str += ustring(" World");
  EXPECT_EQ(str, ustring("Hello World"));
}

TEST_F(UStringModificationTest, OperatorPlusEqualsCString)
{
  ustring str("Hello");
  str += " World";
  EXPECT_EQ(str, ustring("Hello World"));

  // Test with nullptr
  str += (char *)nullptr;
  EXPECT_EQ(str, ustring("Hello World"));  // Should remain unchanged
}

TEST_F(UStringModificationTest, OperatorPlusEqualsChar)
{
  ustring str("Hello");
  str += '!';
  EXPECT_EQ(str, ustring("Hello!"));
}

// Test push_back and pop_back
TEST_F(UStringModificationTest, PushBackPopBack)
{
  ustring str("Hello");
  str.push_back('!');
  EXPECT_EQ(str, ustring("Hello!"));

  str.pop_back();
  EXPECT_EQ(str, ustring("Hello"));
}

// Test insert operations
TEST_F(UStringModificationTest, InsertString)
{
  ustring str("Hello World");
  str.insert(5, ustring(" Beautiful"));
  EXPECT_EQ(str, ustring("Hello Beautiful World"));
}

TEST_F(UStringModificationTest, InsertCString)
{
  ustring str("Hello World");
  str.insert(5, " Beautiful");
  EXPECT_EQ(str, ustring("Hello Beautiful World"));
}

TEST_F(UStringModificationTest, InsertChar)
{
  ustring str("Hello World");
  auto it = str.begin() + 5;
  str.insert(it, '!');
  EXPECT_EQ(str, ustring("Hello! World"));
}

TEST_F(UStringModificationTest, InsertRepeatedChar)
{
  ustring str("Hello World");
  str.insert(5, 3, '!');  // Insert "!!!"
  EXPECT_EQ(str, ustring("Hello!!! World"));
}

// Test erase operations
TEST_F(UStringModificationTest, EraseSubstring)
{
  ustring str("Hello Beautiful World");
  str.erase(5, 10);  // Erase " Beautiful"
  EXPECT_EQ(str, ustring("Hello World"));
}

TEST_F(UStringModificationTest, ErasePosition)
{
  ustring str("Hello World");
  auto it = str.begin() + 5;  // Position of space
  str.erase(it);
  EXPECT_EQ(str, ustring("HelloWorld"));
}

TEST_F(UStringModificationTest, EraseRange)
{
  ustring str("Hello Beautiful World");
  auto first = str.begin() + 5;  // Start of " Beautiful"
  auto last = str.begin() + 15;  // End of " Beautiful"
  str.erase(first, last);
  EXPECT_EQ(str, ustring("Hello World"));
}

// Test edge cases and error conditions
TEST_F(UStringModificationTest, EdgeCases)
{
  ustring str;

  // Empty string operations
  str.append("");
  EXPECT_TRUE(str.empty());

  str.append("Hello");
  str.erase();
  EXPECT_TRUE(str.empty());

  // Out of range operations should throw
#ifdef _DEBUG
  EXPECT_THROW(str.insert(1, "Hello"), std::out_of_range);
  EXPECT_THROW(str.erase(1, 1), std::out_of_range);
#endif
}

// Test capacity management
TEST_F(UStringModificationTest, CapacityManagement)
{
  ustring str;
  size_t initial_capacity = str.capacity();

  // Add enough characters to force reallocation
  std::string long_string(initial_capacity + 1, 'a');
  str.append(reinterpret_cast<const ustring::value_type *>(long_string.c_str()));

  EXPECT_GT(str.capacity(), initial_capacity);
  EXPECT_EQ(str.size(), long_string.length());
}

// Test chaining operations
TEST_F(UStringModificationTest, ChainedOperations)
{
  ustring str("Hello");
  str.append(" ").append("World").append("!");
  EXPECT_EQ(str, ustring("Hello World!"));

  ((str += " ") += "Chain") += '!';
  EXPECT_EQ(str, ustring("Hello World! Chain!"));
}

// Test operator+ operations
TEST_F(UStringModificationTest, OperatorPlus)
{
  // Simple cases
  EXPECT_EQ(ustring("Hello") + ustring(" World"), ustring("Hello World"));
  EXPECT_EQ(ustring("Hello") + '!', ustring("Hello!"));
  EXPECT_EQ('!' + ustring("Hello"), ustring("!Hello"));
  EXPECT_EQ(u8"Hello " + ustring("World"), ustring("Hello World"));
  EXPECT_EQ(ustring("Hello") + u8" World", ustring("Hello World"));

  // More complex cases with various scripts
  ustring str1(u8"こんにちは");
  ustring str2(u8"世界");
  ustring str3(u8"مرحبا");
  ustring str4(u8"བོད་");
  EXPECT_EQ(str1 + u8" " + str2 + u8" " + str3 + u8" " + str4,
            ustring(u8"こんにちは 世界 مرحبا བོད་"));

  // Combining with empty strings
  EXPECT_EQ(ustring("") + ustring("Test"), ustring("Test"));
  EXPECT_EQ(ustring("Test") + ustring(""), ustring("Test"));

  // Combining with nullptr
  EXPECT_EQ(ustring("Test") + static_cast<const ustring::value_type *>(nullptr), ustring("Test"));
  EXPECT_EQ(static_cast<const ustring::value_type *>(nullptr) + ustring("Test"), ustring("Test"));

  // Unicode surrogate pairs and complex scripts
  ustring emoji(u8"😊");
  ustring arabic(u8"السلام عليكم");
  ustring tibetan(u8"བཀྲ་ཤིས་བདེ་ལེགས།");
  EXPECT_EQ(emoji + u8" " + arabic + u8" " + tibetan, ustring(u8"😊 السلام عليكم བཀྲ་ཤིས་བདེ་ལེགས།"));

  // Combining characters with complex scripts
  ustring base(u8"ا");
  ustring combining(u8"\u0651");  // Arabic shadda
  EXPECT_EQ(base + combining, ustring(u8"اّ"));

  // Mixed types and scripts in a single expression
  EXPECT_EQ(ustring("A") + 'B' + u8"C" + ustring(u8"د") + u8"ཨ", ustring(u8"ABCدཨ"));
}

// Test multilingual and special character operations
TEST_F(UStringModificationTest, MultilingualAppend)
{
  ustring str(u8"Hello");

  // Chinese
  str.append(u8" 你好");
  EXPECT_EQ(str, ustring(u8"Hello 你好"));

  // Japanese
  str.append(u8" こんにちは");
  EXPECT_EQ(str, ustring(u8"Hello 你好 こんにちは"));

  // Korean
  str.append(u8" 안녕하세요");
  EXPECT_EQ(str, ustring(u8"Hello 你好 こんにちは 안녕하세요"));

  // Russian
  str.append(u8" Привет");
  EXPECT_EQ(str, ustring(u8"Hello 你好 こんにちは 안녕하세요 Привет"));

  // Arabic (Right-to-left text)
  str.append(u8" مرحبا");
  EXPECT_EQ(str, ustring(u8"Hello 你好 こんにちは 안녕하세요 Привет مرحبا"));
}

TEST_F(UStringModificationTest, SpecialCharactersAppend)
{
  ustring str(u8"Test");

  // Emojis
  str.append(u8" 😀🌟🌍");
  EXPECT_EQ(str, ustring(u8"Test 😀🌟🌍"));

  // Mathematical symbols
  str.append(u8" ∑∏√∞≠");
  EXPECT_EQ(str, ustring(u8"Test 😀🌟🌍 ∑∏√∞≠"));

  // Currency symbols
  str.append(u8" $€¥£₹");
  EXPECT_EQ(str, ustring(u8"Test 😀🌟🌍 ∑∏√∞≠ $€¥£₹"));

  // Diacritical marks
  str.append(u8" áéíóúñ");
  EXPECT_EQ(str, ustring(u8"Test 😀🌟🌍 ∑∏√∞≠ $€¥£₹ áéíóúñ"));
}

TEST_F(UStringModificationTest, MultilingualInsert)
{
  ustring str(u8"Start End");

  // Insert Chinese between words
  str.insert(6, u8"中文");
  EXPECT_EQ(str, ustring(u8"Start 中文End"));

  // Insert emoji
  str.insert(5, u8"🌟");
  EXPECT_EQ(str, ustring(u8"Start🌟 中文End"));

  // Insert mixed scripts
  str.insert(str.size(), u8" あア");  // Hiragana and Katakana
  EXPECT_EQ(str, ustring(u8"Start🌟 中文End あア"));
}

TEST_F(UStringModificationTest, ComplexScriptModification)
{
  ustring str;

  // Test with Thai script (which has complex rendering rules)
  str.append(u8"สวัสดี");
  str.insert(3, u8"❤️");  // Insert emoji in middle of Thai word
  EXPECT_EQ(str.size(), ustring(u8"สวั❤️สดี").size());

  // Test with Devanagari script (which has conjuncts)
  str = ustring(u8"नमस्ते");
  str.insert(2, u8"🙏");  // Insert emoji in middle of conjunct
  EXPECT_EQ(str.size(), ustring(u8"नम🙏स्ते").size());
}

TEST_F(UStringModificationTest, BidirectionalTextHandling)
{
  // Mix of LTR and RTL text
  ustring str(u8"Hello");

  // Add Arabic (RTL) text
  str.append(u8" مرحبا");
  EXPECT_EQ(str, ustring(u8"Hello مرحبا"));

  // Insert Hebrew (RTL) in the middle
  str.insert(5, u8" שלום");
  EXPECT_EQ(str, ustring(u8"Hello שלום مرحبا"));

  // Test character counting with mixed directional text
  EXPECT_EQ(str.size(), ustring(u8"Hello שלום مرحبا").size());
}

TEST_F(UStringModificationTest, ZeroWidthCharacters)
{
  ustring str(u8"Test");

  // Zero-width joiner
  str.append(u8"\u200D");
  EXPECT_EQ(str.length(), 5);
  EXPECT_EQ(str.size(), 7);
  EXPECT_EQ(str.size(), strlen(reinterpret_cast<const char *>(u8"Test\u200D")));

  // Zero-width non-joiner
  str.append(u8"\u200C");
  EXPECT_EQ(str.length(), 6);
  EXPECT_EQ(str.size(), 10);

  // Variation selectors
  str.append(u8"️");  // Variation selector-16 (U+FE0F)
  EXPECT_EQ(str.length(), 7);
  EXPECT_EQ(str.size(), 13);
}

TEST_F(UStringModificationTest, CombiningCharacters)
{
  ustring str(u8"e");

  // Add combining acute accent
  str.append(u8"\u0301");  // Combining acute accent
  EXPECT_EQ(str.length(), 2);
  EXPECT_EQ(str.size(), 3);

  // Add multiple combining characters
  str = ustring(u8"a");
  str.append(u8"\u0308\u0301");  // Combining diaeresis and acute
  EXPECT_EQ(str.length(), 3);
  EXPECT_EQ(str.size(), 5);

  // Test insertion of combining characters
  // todo: should we support this?
  str = ustring(u8"cafe");
  str.insert(3, u8"\u0301");  // Insert acute accent over 'e'
  EXPECT_EQ(str.length(), 5);
  ustring str2(u8"café");
  EXPECT_EQ(str.size(), str2.size());
}

TEST_F(UStringModificationTest, SurrogatePairs)
{
  ustring str;

  // Emoji that requires surrogate pair in UTF-16
  str.append(u8"🌍");
  EXPECT_EQ(str.length(), 1);
  EXPECT_EQ(str.size(), ustring(u8"🌍").size());

  // Multiple surrogate pairs
  str.append(u8"🌟🎉");
  EXPECT_EQ(str.length(), 3);
  EXPECT_EQ(str.size(), ustring(u8"🌍🌟🎉").size());

  // Insert between surrogate pairs
  str.insert(0, u8"⭐");
  EXPECT_EQ(str.length(), 4);
  EXPECT_EQ(str.size(), ustring(u8"⭐🌍🌟🎉").size());
}
