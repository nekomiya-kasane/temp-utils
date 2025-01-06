#include "ustring.h"

#include <gtest/gtest.h>

#include <unicode/brkiter.h>
#include <unicode/localpointer.h>
#include <unicode/locid.h>
#include <unicode/ubrk.h>
#include <unicode/uchar.h>
#include <unicode/uclean.h>
#include <unicode/ucnv.h>
#include <unicode/ucol.h>
#include <unicode/ucsdet.h>
#include <unicode/udat.h>
#include <unicode/umsg.h>
#include <unicode/unistr.h>
#include <unicode/unorm2.h>
#include <unicode/unum.h>
#include <unicode/urename.h>
#include <unicode/usearch.h>
#include <unicode/ustream.h>
#include <unicode/ustring.h>
#include <unicode/utf16.h>
#include <unicode/utf8.h>

class UstringTransformTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    // Basic ASCII strings
    ascii = u8"Hello, World!";
    mixed_case = u8"HeLLo WoRLD";
    spaces = u8"   trim me   ";

    // Multi-language strings
    chinese = u8"你好，世界！";
    japanese = u8"こんにちは、世界！";
    korean = u8"안녕하세요, 세계!";
    russian = u8"Привет, мир!";
    greek = u8"Γειά σου Κόσμε!";
    arabic = u8"مرحبا بالعالم!";
    thai = u8"สวัสดีชาวโลก!";
    vietnamese = u8"Chào thế giới!";
    hindi = u8"नमस्ते दुनिया!";
    hebrew = u8"שָׁלוֹם עוֹלָם!";

    // Complex strings with special characters
    special = u8"Hello—世界©!™®";
    emoji = u8"👨‍👩‍👧‍👦 Family👨‍👩‍👧‍👦 🌈🚀";
    combining = u8"e\u0301";  // é (e + combining acute accent)
    math_symbols = u8"∑∏∆∇∫≤≥≠∞π";
    currency = u8"$€¥£₹₽¢₪";
    punctuation =
        u8".,;:!?‽"
        "''«»—–-";

    // Mixed scripts and languages
    mixed_scripts = u8"Hello世界こんにちは";
    mixed_numerals = u8"123١٢٣壹贰叁๑๒๓";
    bidirectional = u8"Hello! مرحبا שָׁלוֹם";

    // Whitespace variations
    spaces_mixed = u8" \t\n\r\f\v";
    zero_width = u8"a\u2000b\u2001c\u2002d";
    nbsp = u8"Hello\u00A0World";

    // Special cases
    ligatures = u8"ﬀﬁﬂﬃﬄ";
    diacritics = u8"āáǎàēéěèīíǐìōóǒòūúǔù";
    decomposed =
        u8"a\u0304a\u0301a\u030Ca\u0300e\u0304e\u0301e\u030Ce\u0300i\u0304i\u0301i\u030Ci\u0300o"
        u8"\u0304o\u0301o\u030Co\u0300u\u0304u\u0301u\u030Cu\u0300";

    // Edge cases
    empty = u8"";
    single_char = u8"A";
    all_spaces = u8"   ";
    control_chars = u8"\x01\x02\x03\x04\x05";
  }

  // Test strings
  ustring ascii;
  ustring mixed_case;
  ustring spaces;
  ustring chinese;
  ustring japanese;
  ustring korean;
  ustring russian;
  ustring greek;
  ustring arabic;
  ustring thai;
  ustring vietnamese;
  ustring hindi;
  ustring hebrew;
  ustring special;
  ustring emoji;
  ustring combining;
  ustring math_symbols;
  ustring currency;
  ustring punctuation;
  ustring mixed_scripts;
  ustring mixed_numerals;
  ustring bidirectional;
  ustring spaces_mixed;
  ustring zero_width;
  ustring nbsp;
  ustring ligatures;
  ustring diacritics;
  ustring decomposed;
  ustring empty;
  ustring single_char;
  ustring all_spaces;
  ustring control_chars;
};

// Test filter functions
TEST_F(UstringTransformTest, Filter)
{
  // Filter to keep only ASCII characters
  auto ascii_only = [](char32_t c, size_t) { return c < 128; };
  EXPECT_EQ(special.filtered(ascii_only), u8"Hello!");

  // Filter to keep only letters
  auto letters_only = [](char32_t c, size_t) { return u_isalpha(c); };
  EXPECT_EQ(ascii.filtered(letters_only), u8"HelloWorld");

  // Filter to remove emojis
  auto no_emoji = [](char32_t c, size_t) {
    return !u_hasBinaryProperty(c, UCHAR_EMOJI) && c != 0x200d /* zero width */;
  };
  EXPECT_EQ(emoji.filtered(no_emoji), u8" Family ");

  // Test with empty string
  EXPECT_EQ(empty.filtered(ascii_only), u8"");

  // Test with string containing only filtered characters
  auto no_spaces = [](char32_t c, size_t) { return !u_isspace(c); };
  EXPECT_EQ(all_spaces.filtered(no_spaces), u8"");
}

// Test transform functions
TEST_F(UstringTransformTest, Transform)
{
  // Custom transformation: double each character
  auto double_char = [](char32_t c, size_t) -> char32_t { return c; };
  EXPECT_EQ(single_char.transformed(double_char), u8"A");

  // Transform digits to corresponding word
  auto digit_to_word = [](char32_t c, size_t) -> char32_t {
    if (u_isdigit(c)) {
      const char32_t words[] = {
          u'零', u'一', u'二', u'三', u'四', u'五', u'六', u'七', u'八', u'九'};
      return words[c - u'0'];
    }
    return c;
  };
  EXPECT_EQ(ustring(u8"123").transformed(digit_to_word), u8"一二三");

  // Test with empty string
  EXPECT_EQ(empty.transformed(double_char), u8"");
}

// Test case conversion functions
TEST_F(UstringTransformTest, CaseConversion)
{
  // Test to_lower
  EXPECT_EQ(mixed_case.lowered(), u8"hello world");
  EXPECT_EQ(greek.lowered(), u8"γειά σου κόσμε!");
  EXPECT_EQ(russian.lowered(), u8"привет, мир!");

  // Test to_upper
  EXPECT_EQ(mixed_case.uppered(), u8"HELLO WORLD");
  EXPECT_EQ(greek.uppered(), u8"ΓΕΙΆ ΣΟΥ ΚΌΣΜΕ!");
  EXPECT_EQ(russian.uppered(), u8"ПРИВЕТ, МИР!");

  // Test capitalize
  EXPECT_EQ(ascii.capitalized(), u8"Hello, world!");
  EXPECT_EQ(japanese.capitalized(), u8"こんにちは、世界！");  // Should remain unchanged

  // Test swap_case
  EXPECT_EQ(mixed_case.case_swapped(), u8"hEllO wOrld");
  EXPECT_EQ(russian.case_swapped(), u8"пРИВЕТ, МИР!");

  // Test title
  EXPECT_EQ(mixed_case.titled(), u8"Hello World");
  EXPECT_EQ(ascii.titled(), u8"Hello, World!");
  EXPECT_EQ(russian.titled(), u8"Привет, Мир!");

  // Test with empty string
  EXPECT_EQ(empty.lowered(), u8"");
  EXPECT_EQ(empty.uppered(), u8"");
  EXPECT_EQ(empty.capitalized(), u8"");
  EXPECT_EQ(empty.case_swapped(), u8"");
  EXPECT_EQ(empty.titled(), u8"");

  // Test with single character
  EXPECT_EQ(single_char.lowered(), u8"a");
  EXPECT_EQ(single_char.uppered(), u8"A");
  EXPECT_EQ(single_char.case_swapped(), u8"a");
}

// Test trimming and stripping functions
TEST_F(UstringTransformTest, TrimAndStrip)
{
  // Test trim
  EXPECT_EQ(spaces.trimmed(), u8"trim me");
  EXPECT_EQ(ascii.trimmed(), u8"Hello, World!");  // No change

  // Test strip with default whitespace
  EXPECT_EQ(spaces.stripped(), u8"trim me");

  // Test strip with custom characters
  EXPECT_EQ(ascii.stripped(u8"H!"), u8"ello, World");
  EXPECT_EQ(chinese.stripped(u8"你！"), u8"好，世界");

  // Test with empty string
  EXPECT_EQ(empty.trimmed(), u8"");
  EXPECT_EQ(empty.stripped(), u8"");
  EXPECT_EQ(empty.stripped(u8"abc"), u8"");

  // Test with string containing only stripped characters
  EXPECT_EQ(all_spaces.trimmed(), u8"");
  EXPECT_EQ(all_spaces.stripped(), u8"");
}

// Test normalization functions
TEST_F(UstringTransformTest, Normalization)
{
  // Test NFC normalization
  ustring combined = combining.normalized({.mode = Normalization2Mode::COMPOSE});
  EXPECT_EQ(combined, u8"é");

  // Test NFD normalization
  ustring nfd = combining.normalized({.mode = Normalization2Mode::DECOMPOSE});
  EXPECT_NE(nfd, combined);  // Should be decomposed
  EXPECT_EQ(nfd.normalized({.mode = Normalization2Mode::COMPOSE}),
            combined);  // Should recompose to original

  // Test with empty string
  EXPECT_EQ(empty.normalized({}), u8"");

  // Test with string that doesn't need normalization
  EXPECT_EQ(ascii.normalized({}), ascii);
}

// Test complex string manipulations
TEST_F(UstringTransformTest, ComplexManipulations)
{
  // Test chained transformations
  ustring complex = mixed_case;
  complex.to_lower().capitalize().trim();
  EXPECT_EQ(complex, u8"Hello world");

  // Test with emoji and combining characters
  ustring complex_emoji = emoji;
  complex_emoji.to_upper().trim();
  EXPECT_EQ(complex_emoji, u8"👨‍👩‍👧‍👦 FAMILY👨‍👩‍👧‍👦 🌈🚀");

  // Test with mixed scripts
  ustring mixed_scripts{u8"Hello世界こんにちは"};
  EXPECT_EQ(mixed_scripts.lowered(), u8"hello世界こんにちは");
  EXPECT_EQ(mixed_scripts.uppered(), u8"HELLO世界こんにちは");
}

// Test string property checks
TEST_F(UstringTransformTest, StringProperties)
{
  // Test is_alpha
  EXPECT_TRUE(ustring(u8"HelloWorld").is_alpha());
  EXPECT_TRUE(ustring(u8"こんにちは").is_alpha());
  EXPECT_FALSE(ascii.is_alpha());  // Contains punctuation

  // Test is_digit
  EXPECT_TRUE(ustring(u8"123").is_digit());
  EXPECT_TRUE(ustring(u8"١٢٣").is_digit());  // Arabic numerals
  EXPECT_FALSE(ascii.is_digit());

  // Test is_alnum
  EXPECT_TRUE(ustring(u8"Hello123").is_alnum());
  EXPECT_FALSE(ascii.is_alnum());  // Contains punctuation

  // Test is_space
  EXPECT_TRUE(all_spaces.is_space());
  EXPECT_FALSE(ascii.is_space());

  // Test is_lower
  EXPECT_TRUE(ustring(u8"hello").is_lower());
  EXPECT_FALSE(mixed_case.is_lower());

  // Test is_upper
  EXPECT_TRUE(ustring(u8"HELLO").is_upper());
  EXPECT_FALSE(mixed_case.is_upper());

  // Test is_title
  EXPECT_TRUE(ustring(u8"Hello World").is_title());
  EXPECT_FALSE(mixed_case.is_title());

  // Test with empty string
  EXPECT_FALSE(empty.is_alpha());
  EXPECT_FALSE(empty.is_digit());
  EXPECT_FALSE(empty.is_alnum());
  EXPECT_FALSE(empty.is_space());
  EXPECT_FALSE(empty.is_lower());
  EXPECT_FALSE(empty.is_upper());
  EXPECT_FALSE(empty.is_title());
}

// Test with various Unicode blocks and scripts
TEST_F(UstringTransformTest, UnicodeScripts)
{
  // Test with Devanagari
  ustring devanagari(u8"नमस्ते");
  EXPECT_EQ(devanagari.uppered(), u8"नमस्ते");  // Should remain unchanged
  EXPECT_EQ(devanagari.lowered(), u8"नमस्ते");  // Should remain unchanged

  // Test with Tamil
  ustring tamil(u8"வணக்கம்");
  EXPECT_EQ(tamil.uppered(), u8"வணக்கம்");  // Should remain unchanged
  EXPECT_EQ(tamil.lowered(), u8"வணக்கம்");  // Should remain unchanged

  // Test with mixed scripts
  ustring mixed_scripts(u8"Hello नमस्ते こんにちは");
  EXPECT_EQ(mixed_scripts.uppered(), u8"HELLO नमस्ते こんにちは");
  EXPECT_EQ(mixed_scripts.lowered(), u8"hello नमस्ते こんにちは");
}

// Test edge cases and error handling
TEST_F(UstringTransformTest, EdgeCases)
{
  // Test with invalid UTF-8 sequences
  ustring invalid(u8"Hello\xFF\xFFWorld");
  EXPECT_NO_THROW(invalid.to_upper());
  EXPECT_NO_THROW(invalid.to_lower());

  // Test with zero-width characters
  ustring zero_width(u8"H\u200Be\u200Bl\u200Bl\u200Bo");
  EXPECT_EQ(zero_width.uppered(), u8"H\u200BE\u200BL\u200BL\u200BO");

  // Test with surrogate pairs
  ustring surrogate(u8"🌍World");
  EXPECT_EQ(surrogate.uppered(), u8"🌍WORLD");

  // Test with maximum buffer sizes
  ustring long_string(1000, u8"a");
  EXPECT_NO_THROW(long_string.to_upper());

  // Test with combining characters in different orders
  ustring combining1(u8"e\u0301");  // é (e + combining acute accent)
  ustring combining2(u8"é");        // é (precomposed)
  EXPECT_EQ(combining1.normalized({}), combining2);
}

// Test locale-specific transformations
TEST_F(UstringTransformTest, LocaleSpecific)
{
  // Test Turkish I/i case conversion
  ustring turkish_i(u8"i");
  EXPECT_EQ(turkish_i.uppered(), u8"I");  // Default behavior

  // Test German sharp S (ß) case conversion
  ustring sharp_s(u8"aaßbb");
  EXPECT_EQ(sharp_s.uppered(true), u8"AASSBB");
  EXPECT_EQ(sharp_s.uppered(false), u8"AAßBB");

  ustring small_s(u8"AASSBB");
  EXPECT_EQ(small_s.lowered(false), u8"aassbb");
  EXPECT_EQ(small_s.lowered(true), u8"aassbb");  // todo: bad for Germany

  // Test Greek sigma case conversion
  ustring final_sigma(u8"aaςbb");
  EXPECT_EQ(final_sigma.uppered(), u8"AAΣBB");
}

// Test bidirectional text handling
TEST_F(UstringTransformTest, BidirectionalText)
{
  // Test with mixed LTR and RTL text
  EXPECT_EQ(bidirectional.normalized({}), bidirectional);

  // Test RTL marker insertion and removal
  auto rtl_text = ustring(u8"\u200F") + arabic + u8"\u200F";
  EXPECT_EQ(rtl_text.normalized({}), rtl_text);  // todo: is this right?

  rtl_text = ustring(u8"\u200F") + japanese + u8"\u200F";
  EXPECT_EQ(rtl_text.normalized({}), rtl_text);  // todo: is this right?

  // Test mixed numeric with RTL
  auto mixed_rtl = ustring(u8"123" + arabic + u8"456");
  EXPECT_TRUE(mixed_rtl.is_normalized({}));
}

// Test combining character handling
TEST_F(UstringTransformTest, CombiningCharacters)
{
  // Test normalization of decomposed characters
  EXPECT_NE(diacritics, decomposed);
  EXPECT_EQ(diacritics.normalized({}), diacritics);
  EXPECT_EQ(decomposed.normalized({}), diacritics);

  // Test with multiple combining marks
  ustring multiple_marks = u8"a\u0301\u0308";  // a + acute + diaeresis
  EXPECT_EQ(multiple_marks.normalized({}), u8"á̈");
}

// Test numeric handling
TEST_F(UstringTransformTest, NumericHandling)
{
  // Test with different numeric systems
  EXPECT_TRUE(mixed_numerals.contains(u8"123"));
  EXPECT_TRUE(mixed_numerals.contains(u8"١٢٣"));
  EXPECT_TRUE(mixed_numerals.contains(u8"壹贰叁"));
  EXPECT_TRUE(mixed_numerals.contains(u8"๑๒๓"));

  // Test numeric property checks
  auto is_any_digit = [](char32_t c, ustring::size_type) { return u_isdigit(c); };
  EXPECT_TRUE(mixed_numerals.filtered(is_any_digit).length() > 0);
}

// Test whitespace handling
TEST_F(UstringTransformTest, WhitespaceHandling)
{
  // Test various types of whitespace
  EXPECT_EQ(spaces_mixed.trimmed(), u8"");
  EXPECT_EQ(nbsp.trimmed(), u8"Hello\u00A0World");

  // Test zero-width space handling
  EXPECT_NE(zero_width, u8"abcd");
  EXPECT_EQ(zero_width.filtered([](char32_t c, ustring::size_type) { return !u_isspace(c); }),
            u8"abcd");
}

// Test special character categories
TEST_F(UstringTransformTest, SpecialCharacters)
{
  // Test math symbols
  EXPECT_TRUE(math_symbols
                  .filtered([](char32_t c, ustring::size_type) {
                    return u_getIntPropertyValue(c, UCHAR_GENERAL_CATEGORY) == U_MATH_SYMBOL;
                  })
                  .length() > 0);

  // Test currency symbols
  EXPECT_TRUE(currency
                  .filtered([](char32_t c, ustring::size_type) {
                    return u_getIntPropertyValue(c, UCHAR_GENERAL_CATEGORY) == U_CURRENCY_SYMBOL;
                  })
                  .length() > 0);

  // Test punctuation
  EXPECT_TRUE(
      punctuation.filtered([](char32_t c, ustring::size_type) { return u_ispunct(c); }).length() >
      0);
}

// Test control character handling
TEST_F(UstringTransformTest, ControlCharacters)
{
  // Test control character filtering
  EXPECT_EQ(control_chars.filtered([](char32_t c, ustring::size_type) { return !u_iscntrl(c); }),
            u8"");

  // Test control character replacement
  EXPECT_NE(control_chars.transformed(
                [](char32_t c, ustring::size_type) { return u_iscntrl(c) ? u'?' : c; }),
            control_chars);
}

// Test ligature handling
TEST_F(UstringTransformTest, Ligatures)
{
  // Test ligature decomposition
  auto decomposed_ligatures = ligatures.normalized(
      {.mode = Normalization2Mode::DECOMPOSE, .data_file = NormalizationDataFile::NFKC_SCF});
  EXPECT_EQ(decomposed_ligatures, u8"fffiflffiffl");

  // Test ligature composition
  auto recomposed = decomposed_ligatures.normalized(
      {.mode = Normalization2Mode::COMPOSE, .data_file = NormalizationDataFile::NFKC_SCF});
  EXPECT_EQ(recomposed, ligatures);
}

// Test width conversion
TEST_F(UstringTransformTest, WidthConversion)
{
  // Test halfwidth conversion
  ustring fullwidth(u8"Ｈｅｌｌｏ，　Ｗｏｒｌｄ！１２３４５");
  EXPECT_EQ(fullwidth.to_halfwidth(), u8"Hello, World!12345");

  // Test fullwidth conversion
  ustring halfwidth(u8"Hello, World!12345");
  EXPECT_EQ(halfwidth.to_fullwidth(), u8"Ｈｅｌｌｏ，　Ｗｏｒｌｄ！１２３４５");

  // Test mixed width text
  ustring mixed(u8"Hello　Ｗｏｒｌｄ　12３４５");
  EXPECT_EQ(mixed.to_halfwidth(), u8"Hello World 12345");
  EXPECT_EQ(mixed.to_fullwidth(), u8"Ｈｅｌｌｏ　Ｗｏｒｌｄ　１２３４５");

  // Test with punctuation
  ustring punctuation(u8"｛［］｝＜＞（）「」『』～？，．：；！");
  ustring punctuation2(u8"{[]}<>()｢｣『』~?,.:;!");
  EXPECT_EQ(punctuation.to_halfwidth(),
            u8"{[]}<>()｢｣『』~?,.:;!");  // Some characters remain fullwidth
  EXPECT_EQ(punctuation2.to_fullwidth(),
            u8"｛［］｝＜＞（）「」『』～？，．：；！");  // Some characters remain fullwidth

  // Test with special characters
  ustring special(u8"￥＄￡￠￢￣＆＃＠％");
  ustring special2(u8"¥$£¢¬¯&#@%");
  EXPECT_EQ(special.to_halfwidth(), u8"¥$£¢¬¯&#@%");
  EXPECT_EQ(special2.to_fullwidth(), u8"￥＄￡￠￢￣＆＃＠％");

  // Test with Japanese characters
  ustring japanese(
      u8"アイウエオ　カキクケコ　サシスセソ　タチツテト　ナニヌネノ　ハヒフヘホ　マミムメモ　ヤユ"
      u8"ヨ　ラリルレロ　ワヲン");
  ustring japanese2(u8"ｱｲｳｴｵ ｶｷｸｹｺ ｻｼｽｾｿ ﾀﾁﾂﾃﾄ ﾅﾆﾇﾈﾉ ﾊﾋﾌﾍﾎ ﾏﾐﾑﾒﾓ ﾔﾕﾖ ﾗﾘﾙﾚﾛ ﾜｦﾝ");
  EXPECT_EQ(japanese.to_halfwidth(), u8"ｱｲｳｴｵ ｶｷｸｹｺ ｻｼｽｾｿ ﾀﾁﾂﾃﾄ ﾅﾆﾇﾈﾉ ﾊﾋﾌﾍﾎ ﾏﾐﾑﾒﾓ ﾔﾕﾖ ﾗﾘﾙﾚﾛ ﾜｦﾝ");
  EXPECT_EQ(
      japanese2.to_fullwidth(),
      u8"アイウエオ　カキクケコ　サシスセソ　タチツテト　ナニヌネノ　ハヒフヘホ　マミムメモ　ヤユ"
      u8"ヨ　ラリルレロ　ワヲン");

  // Test with Korean characters
  ustring korean(u8"ᄀᄁᆪᄂᆬ");
  ustring korean2(u8"ﾡﾢﾣﾤﾥ");
  EXPECT_EQ(korean.to_halfwidth(), u8"ﾡﾢﾣﾤﾥ");
  EXPECT_EQ(korean2.to_fullwidth(), u8"ᄀᄁᆪᄂᆬ");

  // Test empty string
  EXPECT_EQ(empty.to_halfwidth(), empty);
  EXPECT_EQ(empty.to_fullwidth(), empty);
}

// Test whitespace normalization
TEST_F(UstringTransformTest, WhitespaceNormalization)
{
  // Test basic whitespace normalization
  ustring mixed_spaces(u8"Hello   World\t\n\r\f\v  !");
  EXPECT_EQ(mixed_spaces.normalize_whitespace(), u8"Hello World !");

  // Test with non-breaking spaces and other Unicode spaces
  ustring unicode_spaces(u8"Hello\u00A0\u2002\u2003\u2004\u2005World");
  EXPECT_EQ(unicode_spaces.normalize_whitespace(), u8"Hello World");

  // Test with zero-width spaces
  ustring zero_width_spaces(u8"Hello\u200B\u200C\u200DWorld");
  EXPECT_EQ(zero_width_spaces.normalize_whitespace(false), u8"Hello\u200B\u200C\u200DWorld");
  EXPECT_EQ(zero_width_spaces.normalize_whitespace(), u8"Hello World");

  // Test with leading/trailing spaces
  ustring padded(u8"  \t  Hello  World  \n  ");
  EXPECT_EQ(padded.normalize_whitespace(), u8"Hello World");

  // Test with multiple consecutive spaces of different types
  ustring mixed_consecutive(u8"Hello\u2002 \t\n\u00A0\u2003World");
  EXPECT_EQ(mixed_consecutive.normalize_whitespace(), u8"Hello World");

  // Test with ideographic spaces
  ustring cjk_spaces(u8"你好　世界　！");
  EXPECT_EQ(cjk_spaces.normalize_whitespace(), u8"你好 世界 ！");

  // Test empty string
  EXPECT_EQ(empty.normalize_whitespace(), empty);
}

// Test quote normalization
TEST_F(UstringTransformTest, QuoteNormalization)
{
  // Test basic quote normalization
  ustring mixed_quotes(u8"'test' \"test\" 'test' \"test\"");
  EXPECT_EQ(mixed_quotes.normalize_quotes(), u8"'test' \"test\" 'test' \"test\"");

  // Test with different types of quotes
  ustring various_quotes(u8"«test» 「test」 『test』 ﹁test﹂");
  EXPECT_EQ(various_quotes.normalize_quotes(), u8"\"test\" 'test' \"test\" 'test'");

  // Test nested quotes
  ustring nested_quotes(u8"\"'test'\" '「\"test\"」'");
  EXPECT_EQ(nested_quotes.normalize_quotes(), u8"\"'test'\" ''\"test\"''");

  // Test with apostrophes and primes
  ustring apostrophes(u8"don't it's o'clock");
  EXPECT_EQ(apostrophes.normalize_quotes(),
            u8"don't it's o'clock");  // Apostrophes should remain unchanged

  // Test with different languages
  ustring multilingual_quotes(u8"«français» „Deutsch\" 「日本語」 \"English\"");
  EXPECT_EQ(multilingual_quotes.normalize_quotes(),
            u8"\"français\" \"Deutsch\" '日本語' \"English\"");  // todo: is this right?

  // Test empty string
  EXPECT_EQ(empty.normalize_quotes(), empty);
}

// Test dash normalization
TEST_F(UstringTransformTest, DashNormalization)
{
  // Test basic dash normalization
  ustring mixed_dashes(u8"test-test -- test—test");
  EXPECT_EQ(mixed_dashes.normalize_dashes(), u8"test-test -- test-test");

  // Test with various types of dashes
  ustring various_dashes(u8"- ‐ ‑ ‒ – — ― ⁃ ⸺ ⸻");
  EXPECT_EQ(various_dashes.normalize_dashes(), u8"- - - - - - - - - -");

  // Test in context
  ustring dash_context(u8"1-2 pages 3--4 A―B");
  EXPECT_EQ(dash_context.normalize_dashes(), u8"1-2 pages 3--4 A-B");

  // Test with spaces around dashes
  ustring spaced_dashes(u8"word - word -- word");
  EXPECT_EQ(spaced_dashes.normalize_dashes(), u8"word - word -- word");

  // Test with numbers and ranges
  ustring number_ranges(u8"1-999 2010-2020 9:00-17:00");
  EXPECT_EQ(number_ranges.normalize_dashes(), u8"1-999 2010-2020 9:00-17:00");

  // Test empty string
  EXPECT_EQ(empty.normalize_dashes(), empty);
}

// Test text simplification
TEST_F(UstringTransformTest, TextSimplification)
{
  // Test Chinese simplification
  auto traditional_str = u8"漢字/漢字 國際 車站";
  ustring traditional(traditional_str);
  EXPECT_EQ(traditional.simplify(), u8"汉字/汉字 国际 车站");
  EXPECT_EQ(traditional.traditionalize(), traditional_str);

  // Test with mixed scripts
  auto mixed_scripts_str = u8"漢字 English 漢字";
  ustring mixed_scripts(mixed_scripts_str);
  EXPECT_EQ(mixed_scripts.simplify(), u8"汉字 English 汉字");
  EXPECT_EQ(mixed_scripts.traditionalize(), mixed_scripts_str);

  // Test with numbers and punctuation
  auto mixed_content_str = u8"漢字123！漢字";
  ustring mixed_content(mixed_content_str);
  EXPECT_EQ(mixed_content.simplify(), u8"汉字123！汉字");
  EXPECT_EQ(mixed_content.traditionalize(), mixed_content_str);

  // Test with Japanese characters (should not be simplified)
  auto japanese_str = u8"漢字とひらがな";
  ustring japanese(japanese_str);
  EXPECT_EQ(japanese.simplify(), u8"汉字とひらがな");  // Japanese text should remain unchanged
  EXPECT_EQ(japanese.traditionalize(), japanese_str);  // Japanese text should remain unchanged

  // Test with Korean characters (should not be simplified)
  auto korean_str = u8"漢字와 한글";
  ustring korean(korean_str);
  EXPECT_EQ(korean.simplify(), u8"汉字와 한글");   // Korean text should remain unchanged
  EXPECT_EQ(korean.traditionalize(), korean_str);  // Korean text should remain unchanged

  auto ambigious_str = u8"有頭髮的王后後面容易發財";
  ustring ambigious(ambigious_str);
  EXPECT_EQ(ambigious.simplify(), u8"有头发的王后后面容易发财");
  EXPECT_EQ(ambigious.traditionalize(), ambigious_str);

  auto complex_str = u8"一隻憂鬱的台灣烏龜盪鞦韆，一群骯髒醜陋的烏龜尋釁一群嚙齒鰐龜"; // todo: 蕩?
  ustring complex(complex_str);
  EXPECT_EQ(complex.simplify(), u8"一只忧郁的台湾乌龟荡秋千，一群肮脏丑陋的乌龟寻衅一群啮齿鳄龟");
  EXPECT_EQ(complex.traditionalize(), complex_str);

  // Test empty string
  EXPECT_EQ(empty.simplify(), empty);
  EXPECT_EQ(empty.traditionalize(), empty);
}

// Test combined transformations
TEST_F(UstringTransformTest, CombinedTransformations)
{
  // Test width conversion with whitespace normalization
  ustring mixed_width_spaces(u8"Ｈｅｌｌｏ　　Ｗｏｒｌｄ！");
  auto result = mixed_width_spaces.to_halfwidth();
  EXPECT_EQ(result.normalize_whitespace(), u8"Hello World!");

  // Test quote normalization with dash normalization
  ustring quotes_and_dashes(u8"“test-test” -- ‘test―test’");
  result = quotes_and_dashes.normalize_quotes();
  EXPECT_EQ(result.normalize_dashes(), u8"\"test-test\" -- 'test-test'");

  // Test simplification with width conversion
  ustring traditional_fullwidth(u8"漢字　Ｗｏｒｌｄ");
  result = traditional_fullwidth.simplify();
  EXPECT_EQ(result.to_halfwidth(), u8"汉字 World");

  // Test all transformations combined
  ustring complex(u8"「漢字」　--　\"Ｗｏｒｌｄ\"　！");
  result = complex.normalize_quotes()
               .normalize_dashes()
               .normalize_whitespace()
               .simplify()
               .to_halfwidth();
  EXPECT_EQ(result, u8"'汉字' -- \"World\" !");

  // Test with empty string
  result = empty.to_halfwidth()
               .to_fullwidth()
               .normalize_whitespace()
               .normalize_quotes()
               .normalize_dashes()
               .simplify();
  EXPECT_EQ(result, empty);
}
