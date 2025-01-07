#pragma once

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <format>
#include <functional>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

enum class CharProperty : uint32_t {
  NONE = 0,
  // Basic categories
  ALPHABETIC = 1 << 0,
  LOWERCASE = 1 << 1,
  UPPERCASE = 1 << 2,
  WHITESPACE = 1 << 3,
  DIGIT = 1 << 4,
  PUNCTUATION = 1 << 5,
  CONTROL = 1 << 6,
  EMOJI = 1 << 7,
  IDEOGRAPHIC = 1 << 8,
  LETTER = 1 << 9,
  MATH = 1 << 10,
  HEXDIGIT = 1 << 11,
  COMBINING_MARK = 1 << 12,

  DASH = 1 << 13,
  DIACRITIC = 1 << 14,
  EXTENDER = 1 << 15,
  GRAPHEME_BASE = 1 << 16,
  GRAPHEME_EXTEND = 1 << 17,
  GRAPHEME_LINK = 1 << 18,
  IDS_BINARY_OPERATOR = 1 << 19,
  IDS_TRINARY_OPERATOR = 1 << 20,
  JOIN_CONTROL = 1 << 21,
  LOGICAL_ORDER_EXCEPTION = 1 << 22,
  NONCHARACTER_CODE_POINT = 1 << 23,
  QUOTATION_MARK = 1 << 24,
  RADICAL = 1 << 25,
  SOFT_DOTTED = 1 << 26,
  TERMINAL_PUNCTUATION = 1 << 27,
  UNIFIED_IDEOGRAPH = 1 << 28,
  VARIATION_SELECTOR = 1 << 29,
  SPACE = 1 << 30,
  // PRIVATE_USE = 1 << 31
};

bool has_property(std::u8string_view str, CharProperty property);
bool has_property(const char8_t *str, CharProperty property);
bool has_property(char32_t codepoint, CharProperty property);
CharProperty get_property(std::u8string_view str);
CharProperty get_property(const char8_t *str);
CharProperty get_property(char32_t codepoint);

char32_t to_codepoint(std::u8string_view str);
char32_t to_codepoint(const char8_t *str);
char32_t to_codepoint(const char *s, size_t offset);

enum class Normalization2Mode {
  /**
   * Decomposition followed by composition.
   * Same as standard NFC when using an "nfc" instance.
   * Same as standard NFKC when using an "nfkc" instance.
   * For details about standard Unicode normalization forms
   * see http://www.unicode.org/unicode/reports/tr15/
   */
  COMPOSE,
  /**
   * Map, and reorder canonically.
   * Same as standard NFD when using an "nfc" instance.
   * Same as standard NFKD when using an "nfkc" instance.
   * For details about standard Unicode normalization forms
   * see http://www.unicode.org/unicode/reports/tr15/
   */
  DECOMPOSE,
  /**
   * "Fast C or D" form.
   * If a string is in this form, then further decomposition <i>without reordering</i>
   * would yield the same form as DECOMPOSE.
   * Text in "Fast C or D" form can be processed efficiently with data tables
   * that are "canonically closed", that is, that provide equivalent data for
   * equivalent text, without having to be fully normalized.
   * Not a standard Unicode normalization form.
   * Not a unique form: Different FCD strings can be canonically equivalent.
   * For details see http://www.unicode.org/notes/tn5/#FCD
   */
  FCD,
  /**
   * Compose only contiguously.
   * Also known as "FCC" or "Fast C Contiguous".
   * The result will often but not always be in NFC.
   * The result will conform to FCD which is useful for processing.
   * Not a standard Unicode normalization form.
   * For details see http://www.unicode.org/notes/tn5/#FCC
   */
  COMPOSE_CONTIGUOUS
};

enum class NormalizationDataFile { NFC, NFKC, NFKC_CF, NFKC_SCF, CUSTOM };

struct NormalizationConfig {
  Normalization2Mode mode = Normalization2Mode::COMPOSE;
  NormalizationDataFile data_file = NormalizationDataFile::NFC;
  const char *custom_data_file = nullptr;
};

enum class WordBreak {
  /** Tag value for "words" that do not fit into any of other categories.
   *  Includes spaces and most punctuation. */
  UBRK_WORD_NONE = 0,
  /** Upper bound for tags for uncategorized words. */
  UBRK_WORD_NONE_LIMIT = 100,
  /** Tag value for words that appear to be numbers, lower limit.    */
  UBRK_WORD_NUMBER = 100,
  /** Tag value for words that appear to be numbers, upper limit.    */
  UBRK_WORD_NUMBER_LIMIT = 200,
  /** Tag value for words that contain letters, excluding
   *  hiragana, katakana or ideographic characters, lower limit.    */
  UBRK_WORD_LETTER = 200,
  /** Tag value for words containing letters, upper limit  */
  UBRK_WORD_LETTER_LIMIT = 300,
  /** Tag value for words containing kana characters, lower limit */
  UBRK_WORD_KANA = 300,
  /** Tag value for words containing kana characters, upper limit */
  UBRK_WORD_KANA_LIMIT = 400,
  /** Tag value for words containing ideographic characters, lower limit */
  UBRK_WORD_IDEO = 400,
  /** Tag value for words containing ideographic characters, upper limit */
  UBRK_WORD_IDEO_LIMIT = 500
};

std::string to_utf8(char32_t codepoint);
std::string to_utf16(char32_t codepoint);
std::string to_utf32(char32_t codepoint);

enum class ToTitleOptions : uint32_t {
  DEFAULT = 0x00,
  /**
   * Titlecase the string as a whole rather than each word.
   * (Titlecase only the character at index 0, possibly adjusted.)
   * Option bits value for titlecasing APIs that take an options bit set.
   *
   * It is an error to specify multiple titlecasing iterator options together,
   * including both an options bit and an explicit BreakIterator.
   */
  WHOLE_STRING = 0x20,

  /**
   * Titlecase sentences rather than words.
   * (Titlecase only the first character of each sentence, possibly adjusted.)
   * Option bits value for titlecasing APIs that take an options bit set.
   *
   * It is an error to specify multiple titlecasing iterator options together,
   * including both an options bit and an explicit BreakIterator.
   */
  SENTENCES = 0x40,

  /**
   * Do not lowercase non-initial parts of words when titlecasing.
   * Option bit for titlecasing APIs that take an options bit set.
   *
   * By default, titlecasing will titlecase the character at each
   * (possibly adjusted) BreakIterator index and
   * lowercase all other characters up to the next iterator index.
   * With this option, the other characters will not be modified.
   */
  NO_LOWERCASE = 0x100,

  /**
   * Do not adjust the titlecasing BreakIterator indexes;
   * titlecase exactly the characters at breaks from the iterator.
   * Option bit for titlecasing APIs that take an options bit set.
   *
   * By default, titlecasing will take each break iterator index,
   * adjust it to the next relevant character (see U_TITLECASE_ADJUST_TO_CASED),
   * and titlecase that one.
   *
   * Other characters are lowercased.
   *
   * It is an error to specify multiple titlecasing adjustment options together.
   */
  NO_BREAK_ADJUSTMENT = 0x200,

  /**
   * Adjust each titlecasing BreakIterator index to the next cased character.
   * (See the Unicode Standard, chapter 3, Default Case Conversion, R3 toTitlecase(X).)
   * Option bit for titlecasing APIs that take an options bit set.
   *
   * This used to be the default index adjustment in ICU.
   * Since ICU 60, the default index adjustment is to the next character that is
   * a letter, number, symbol, or private use code point.
   * (Uncased modifier letters are skipped.)
   * The difference in behavior is small for word titlecasing,
   * but the new adjustment is much better for whole-string and sentence titlecasing:
   * It yields "49ers" and "«丰(abc)»" instead of "49Ers" and "«丰(Abc)»".
   */
  ADJUST_TO_CASED = 0x400
};

class ustring {
 public:
  using value_type = char8_t;
  using size_type = int32_t;
  using difference_type = ptrdiff_t;
  using reference = value_type &;
  using const_reference = const value_type &;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using iterator = const value_type *;
  using const_iterator = const value_type *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  class code_point_iterator;
  class grapheme_iterator;
  class word_iterator;
  class sentence_iterator;

  class view {
   public:
    using value_type = ustring::value_type;
    using size_type = ustring::size_type;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using const_reference = const value_type &;
    using pointer = const value_type *;
    using const_pointer = const value_type *;
    using iterator = const value_type *;
    using const_iterator = const value_type *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    view() noexcept : _data(nullptr), _size(0) {}
    view(const ustring &str) noexcept : _data(str.data()), _size(str.size()) {}
    view(const value_type *str, size_type len) noexcept : _data(str), _size(len) {}

    bool operator==(const char *rhs) const noexcept;
    std::strong_ordering operator<=>(const char *rhs) const noexcept;
    bool operator==(const char8_t *rhs) const noexcept;
    std::strong_ordering operator<=>(const char8_t *rhs) const noexcept;
    bool operator==(const char16_t *rhs) const noexcept;
    std::strong_ordering operator<=>(const char16_t *rhs) const noexcept;
    bool operator==(const char32_t *rhs) const noexcept;
    std::strong_ordering operator<=>(const char32_t *rhs) const noexcept;
    friend bool operator==(const view &lhs, const view &rhs) noexcept;
    friend std::strong_ordering operator<=>(const view &lhs, const view &rhs) noexcept;
    template<typename T>
      requires std::convertible_to<T, view>
    friend bool operator==(const view &lhs, T &&rhs) noexcept
    {
      return lhs == view(std::forward<T>(rhs));
    }
    template<typename T>
      requires std::convertible_to<T, view>
    friend std::strong_ordering operator<=>(const view &lhs, const T *rhs) noexcept
    {
      return lhs <=> view(std::forward<T>(rhs));
    }

    friend ustring operator+(const ustring &lhs, const ustring &rhs);
    friend ustring operator+(const ustring &lhs, value_type rhs);
    friend ustring operator+(value_type lhs, const ustring &rhs);
    friend ustring operator+(const value_type *lhs, const ustring &rhs);
    friend ustring operator+(const ustring &lhs, const value_type *rhs);

    [[nodiscard]] const_iterator begin() const noexcept
    {
      return _data;
    }
    [[nodiscard]] const_iterator end() const noexcept
    {
      return _data + _size;
    }
    [[nodiscard]] const_iterator cbegin() const noexcept
    {
      return begin();
    }
    [[nodiscard]] const_iterator cend() const noexcept
    {
      return end();
    }

    [[nodiscard]] const_reverse_iterator rbegin() const noexcept
    {
      return const_reverse_iterator(end());
    }
    [[nodiscard]] const_reverse_iterator rend() const noexcept
    {
      return const_reverse_iterator(begin());
    }
    [[nodiscard]] const_reverse_iterator crbegin() const noexcept
    {
      return rbegin();
    }
    [[nodiscard]] const_reverse_iterator crend() const noexcept
    {
      return rend();
    }

    [[nodiscard]] size_type size() const noexcept
    {
      return _size;
    }
    [[nodiscard]] bool empty() const noexcept
    {
      return _size == 0;
    }
    [[nodiscard]] const_pointer data() const noexcept
    {
      return _data;
    }

    [[nodiscard]] const_reference operator[](size_type pos) const
    {
      return _data[pos];
    }
    [[nodiscard]] const_reference at(size_type pos) const
    {
      if (pos >= _size)
        throw std::out_of_range("ustring::view::at");
      return _data[pos];
    }

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::string_view to_string_view() const &;
    [[nodiscard]] std::u8string to_u8string() const;
    [[nodiscard]] std::u8string_view to_u8string_view() const &;
    [[nodiscard]] std::u16string to_u16string() const;
    [[nodiscard]] std::u32string to_u32string() const;
    [[nodiscard]] std::wstring to_wstring() const;

    [[nodiscard]] static constexpr size_type max_size() noexcept
    {
      return (static_cast<size_type>(1) << (sizeof(size_type) * 8 - 2)) - 1;
    }

    [[nodiscard]] size_type length() const noexcept;

    [[nodiscard]] size_t hash() const noexcept
    {
      return std::hash<std::u8string_view>{}(std::u8string_view(data(), size()));
    }

    [[nodiscard]] ustring copy() const;
    [[nodiscard]] size_type copy(value_type *dest, size_type n, size_type pos = 0) const;
    [[nodiscard]] ustring substr(size_type pos = 0, size_type n = npos) const;
    [[nodiscard]] view substr_view(size_type pos = 0, size_type n = npos) const;

    [[nodiscard]] size_type find(const ustring &str, size_type pos = 0) const noexcept;
    [[nodiscard]] size_type find(const value_type *s, size_type pos, size_type n) const;
    [[nodiscard]] size_type find(const value_type *s, size_type pos = 0) const;
    [[nodiscard]] size_type find(value_type c, size_type pos = 0) const noexcept;

    [[nodiscard]] size_type rfind(const ustring &str, size_type pos = npos) const noexcept;
    [[nodiscard]] size_type rfind(const value_type *s, size_type pos, size_type n) const;
    [[nodiscard]] size_type rfind(const value_type *s, size_type pos = npos) const;
    [[nodiscard]] size_type rfind(value_type c, size_type pos = npos) const noexcept;

    [[nodiscard]] size_type find_first_of(const ustring &str, size_type pos = 0) const noexcept;
    [[nodiscard]] size_type find_first_of(const value_type *s, size_type pos, size_type n) const;
    [[nodiscard]] size_type find_first_of(const value_type *s, size_type pos = 0) const;
    [[nodiscard]] size_type find_first_of(value_type c, size_type pos = 0) const noexcept;

    [[nodiscard]] size_type find_last_of(const ustring &str, size_type pos = npos) const noexcept;
    [[nodiscard]] size_type find_last_of(const value_type *s, size_type pos, size_type n) const;
    [[nodiscard]] size_type find_last_of(const value_type *s, size_type pos = npos) const;
    [[nodiscard]] size_type find_last_of(value_type c, size_type pos = npos) const noexcept;

    [[nodiscard]] size_type find_first_not_of(const ustring &str,
                                              size_type pos = 0) const noexcept;
    [[nodiscard]] size_type find_first_not_of(const value_type *s,
                                              size_type pos,
                                              size_type n) const;
    [[nodiscard]] size_type find_first_not_of(const value_type *s, size_type pos = 0) const;
    [[nodiscard]] size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept;

    [[nodiscard]] size_type find_last_not_of(const ustring &str,
                                             size_type pos = npos) const noexcept;
    [[nodiscard]] size_type find_last_not_of(const value_type *s,
                                             size_type pos,
                                             size_type n) const;
    [[nodiscard]] size_type find_last_not_of(const value_type *s, size_type pos = npos) const;
    [[nodiscard]] size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept;

    [[nodiscard]] size_t count(const ustring &str) const noexcept;
    [[nodiscard]] size_t count(const value_type *s) const;
    [[nodiscard]] size_t count(char32_t c) const;
    [[nodiscard]] size_t count(std::function<bool(char32_t)> f) const;

    [[nodiscard]] bool contains(const ustring &str) const noexcept;
    [[nodiscard]] bool contains(const value_type *s) const;
    [[nodiscard]] bool contains(char32_t c) const noexcept;
    [[nodiscard]] bool contains(value_type c) const noexcept;
    [[nodiscard]] bool contains(std::function<bool(char32_t)> f) const;

    [[nodiscard]] int compare(const ustring &str) const noexcept;
    [[nodiscard]] int compare(size_type pos1, size_type n1, const ustring &str) const;
    [[nodiscard]] int compare(size_type pos1,
                              size_type n1,
                              const ustring &str,
                              size_type pos2,
                              size_type n2 = npos) const;
    [[nodiscard]] int compare(const value_type *s) const;
    [[nodiscard]] int compare(size_type pos1, size_type n1, const value_type *s) const;
    [[nodiscard]] int compare(size_type pos1,
                              size_type n1,
                              const value_type *s,
                              size_type n2) const;

    // iterators
    [[nodiscard]] code_point_iterator code_points_begin() const noexcept
    {
      return code_point_iterator(*this);
    }
    [[nodiscard]] code_point_iterator code_points_end() const noexcept
    {
      return code_point_iterator(*this, size());
    }
    [[nodiscard]] auto code_points() const noexcept
    {
      return std::ranges::subrange(code_points_begin(), code_points_end());
    }
    [[nodiscard]] grapheme_iterator graphemes_begin() const noexcept
    {
      return grapheme_iterator(*this);
    }
    [[nodiscard]] grapheme_iterator graphemes_end() const noexcept
    {
      return grapheme_iterator(*this, size());
    }
    [[nodiscard]] auto graphemes() const noexcept
    {
      return std::ranges::subrange(graphemes_begin(), graphemes_end());
    }
    [[nodiscard]] word_iterator words_begin() const noexcept
    {
      return word_iterator(*this);
    }
    [[nodiscard]] word_iterator words_end() const noexcept
    {
      return word_iterator(*this, size());
    }
    [[nodiscard]] auto words() const noexcept
    {
      return std::ranges::subrange(words_begin(), words_end());
    }
    [[nodiscard]] sentence_iterator sentences_begin() const noexcept
    {
      return sentence_iterator(*this);
    }
    [[nodiscard]] sentence_iterator sentences_end() const noexcept
    {
      return sentence_iterator(*this, size());
    }
    [[nodiscard]] auto sentences() const noexcept
    {
      return std::ranges::subrange(sentences_begin(), sentences_end());
    }

    [[nodiscard]] std::vector<view> split(char32_t delimiter) const;
    [[nodiscard]] std::vector<view> split(std::unordered_set<char32_t> &&delimiter) const;
    [[nodiscard]] std::vector<view> split(const ustring &delimiter) const;
    [[nodiscard]] std::vector<view> split_words(const char *locale) const;

   private:
    const value_type *_data;
    size_type _size;
  };

  class code_point_iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = char32_t;
    using pointer = char32_t *;
    using reference = char32_t;
    using size_type = ustring::size_type;
    using difference_type = ustring::difference_type;

    code_point_iterator();
    code_point_iterator(const view &str, size_type pos = 0);
    code_point_iterator(const ustring &str, size_type pos = 0);
    code_point_iterator(const code_point_iterator &);
    code_point_iterator(code_point_iterator &&) noexcept;
    code_point_iterator &operator=(const code_point_iterator &);
    code_point_iterator &operator=(code_point_iterator &&) noexcept;

    code_point_iterator &operator++();
    code_point_iterator &operator++(int);
    code_point_iterator &operator+=(size_t step);
    code_point_iterator operator+(size_t step) const;
    code_point_iterator &operator--();
    code_point_iterator operator-=(size_t step);
    code_point_iterator operator-(size_t step) const;

    char32_t operator*() const
    {
      return _codepoint;
    }
    const char32_t *operator->() const
    {
      return &_codepoint;
    }

    difference_type operator-(const code_point_iterator &other) const
    {
      return static_cast<difference_type>(_data - other._data);
    }

    bool operator==(const code_point_iterator &other) const
    {
      return _data == other._data;
    }
    std::strong_ordering operator<=>(const code_point_iterator &other) const
    {
      return _data <=> other._data;
    }

    size_type size() const
    {
      return _size;
    }

    char32_t codepoint()
    {
      return _codepoint;
    }

    bool is_valid() const
    {
      return _size > 0 && _size < 4;
    }
    bool is_unknown() const
    {
      // assert(is_valid());
      return _codepoint;
    }
    bool same_as(char32_t c) const
    {
      return is_valid() && _codepoint == c;
    }
    bool same_as(const code_point_iterator &c) const
    {
      return is_valid() && c.is_valid() && c._codepoint == _codepoint;
    }

   private:
    const ustring::value_type *_data, *_end, *_start;
    int32_t _size;  // todo: this can be 1 byte
    char32_t _codepoint;
  };

  class grapheme_iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ustring;
    using pointer = view *;
    using const_pointer = const view *;
    using reference = view &;
    using const_reference = const view &;
    using difference_type = ptrdiff_t;
    using size_type = ustring::size_type;

    grapheme_iterator();
    explicit grapheme_iterator(const view &str, size_type pos = 0, const char *locale = nullptr);
    explicit grapheme_iterator(const ustring &str,
                               size_type pos = 0,
                               const char *locale = nullptr);
    grapheme_iterator(const grapheme_iterator &);
    grapheme_iterator(grapheme_iterator &&);
    grapheme_iterator &operator=(const grapheme_iterator &);
    grapheme_iterator &operator=(grapheme_iterator &&);
    ~grapheme_iterator();

    // Basic iterator operations
    grapheme_iterator &operator++();
    grapheme_iterator operator++(int);
    grapheme_iterator &operator--();
    grapheme_iterator operator--(int);
    const_reference operator*() const;
    reference operator*();
    const_pointer operator->() const;
    pointer operator->();
    bool operator==(const grapheme_iterator &) const;
    bool operator!=(const grapheme_iterator &) const;

    // Additional operations for more functionality
    grapheme_iterator &operator+=(difference_type n);
    grapheme_iterator operator+(difference_type n) const;
    grapheme_iterator &operator-=(difference_type n);
    grapheme_iterator operator-(difference_type n) const;
    difference_type operator-(const grapheme_iterator &other) const;
    std::strong_ordering operator<=>(const grapheme_iterator &) const;

    // Utility functions
    size_type position() const
    {
      return _view.data() - _start;
    }
    size_type size() const
    {
      return _view.size();
    }
    bool is_end() const;
    bool is_begin() const
    {
      return _view.data() == _start;
    }

    code_point_iterator code_points_begin() const;
    code_point_iterator code_points_end() const;
    [[nodiscard]] auto code_points() const noexcept
    {
      return std::ranges::subrange<code_point_iterator>(code_points_begin(), code_points_end());
    }

   private:
    const ustring::value_type *_start, *_end;
    view _view;
    void *_break_iterator;  // UBreakIterator*
    void *_text;            // UText*
  };

  class word_iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ustring;
    using pointer = view *;
    using const_pointer = const view *;
    using reference = view &;
    using const_reference = const view &;
    using difference_type = ptrdiff_t;

    word_iterator();
    explicit word_iterator(const view &str,
                           size_type pos = 0,
                           const char *locale = nullptr,
                           WordBreak break_type = WordBreak::UBRK_WORD_NONE);
    explicit word_iterator(const ustring &str,
                           size_type pos = 0,
                           const char *locale = nullptr,
                           WordBreak break_type = WordBreak::UBRK_WORD_NONE);
    word_iterator(const word_iterator &);
    word_iterator(word_iterator &&);
    word_iterator &operator=(const word_iterator &);
    word_iterator &operator=(word_iterator &&);
    ~word_iterator();

    word_iterator &operator++();
    word_iterator operator++(int);
    word_iterator &operator--();
    word_iterator operator--(int);

    const_reference operator*() const;
    reference operator*();
    const_pointer operator->() const;
    pointer operator->();

    bool operator==(const word_iterator &) const;
    bool operator!=(const word_iterator &) const;

    // Additional operations for more functionality
    word_iterator &operator+=(difference_type n);
    word_iterator operator+(difference_type n) const;
    word_iterator &operator-=(difference_type n);
    word_iterator operator-(difference_type n) const;
    difference_type operator-(const word_iterator &other) const;
    std::strong_ordering operator<=>(const word_iterator &) const;

    size_type position() const
    {
      return _view.data() - _start;
    }
    size_type word_size() const
    {
      return _view.size();
    }
    size_type word_length() const;
    bool is_end() const;

    grapheme_iterator graphemes_begin() const;
    grapheme_iterator graphemes_end() const;
    std::pair<grapheme_iterator, grapheme_iterator> graphemes() const;

    code_point_iterator code_points_begin() const;
    code_point_iterator code_points_end() const;
    std::pair<code_point_iterator, code_point_iterator> code_points() const;

   private:
    const ustring::value_type *_start, *_end;
    view _view;
    void *_break_iterator;  // UBreakIterator*
    void *_text;            // UText*
  };

  class sentence_iterator {
   public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ustring;
    using pointer = view *;
    using const_pointer = const view *;
    using reference = view &;
    using const_reference = const view &;
    using difference_type = ptrdiff_t;
    using size_type = ustring::size_type;

    sentence_iterator();
    explicit sentence_iterator(const view &str, size_type pos = 0, const char *locale = nullptr);
    explicit sentence_iterator(const ustring &str,
                               size_type pos = 0,
                               const char *locale = nullptr);
    sentence_iterator(const sentence_iterator &);
    sentence_iterator(sentence_iterator &&);
    sentence_iterator &operator=(const sentence_iterator &);
    sentence_iterator &operator=(sentence_iterator &&);
    ~sentence_iterator();

    // Basic iterator operations
    sentence_iterator &operator++();
    sentence_iterator operator++(int);
    sentence_iterator &operator--();
    sentence_iterator operator--(int);
    const_reference operator*() const;
    reference operator*();
    const_pointer operator->() const;
    pointer operator->();
    bool operator==(const sentence_iterator &) const;
    bool operator!=(const sentence_iterator &) const;

    // Additional operations for more functionality
    sentence_iterator &operator+=(difference_type n);
    sentence_iterator operator+(difference_type n) const;
    sentence_iterator &operator-=(difference_type n);
    sentence_iterator operator-(difference_type n) const;
    difference_type operator-(const sentence_iterator &other) const;
    std::strong_ordering operator<=>(const sentence_iterator &) const;

    // Utility functions
    size_type position() const
    {
      return _view.data() - _start;
    }
    size_type size() const
    {
      return _view.size();
    }
    bool is_end() const;
    bool is_begin() const
    {
      return _view.data() == _start;
    }

    word_iterator words_begin() const;
    word_iterator words_end() const;
    std::pair<word_iterator, word_iterator> words() const;

    grapheme_iterator graphemes_begin() const;
    grapheme_iterator graphemes_end() const;
    [[nodiscard]] auto graphemes() const noexcept
    {
      return std::ranges::subrange<grapheme_iterator>(graphemes_begin(), graphemes_end());
    }

    code_point_iterator code_points_begin() const;
    code_point_iterator code_points_end() const;
    [[nodiscard]] auto code_points() const noexcept
    {
      return std::ranges::subrange(code_points_begin(), code_points_end());
    }

   private:
    const ustring::value_type *_start, *_end;
    view _view;
    void *_break_iterator;  // UBreakIterator*
    void *_text;            // UText*
  };

  static constexpr size_type npos = -1;
  static constexpr size_type max_pos = std::numeric_limits<size_type>::max();
  static constexpr size_type default_size = static_cast<size_type>(12);

  ustring();
  ustring(const ustring &other);
  ustring(ustring &&other) noexcept;
  ustring(view other);

  // explicit ustring(const icu::UnicodeString &str);
  // explicit ustring(icu::UnicodeString &&str);

  ustring(value_type c);
  explicit ustring(const char *s, bool validate = false);
  explicit ustring(const char *s, size_t length, bool validate = false);
  ustring(const char8_t *s);
  ustring(const char8_t *s, size_t length);
  ustring(const char16_t *s);
  ustring(const char16_t *s, size_t length);
  ustring(const char32_t *s);
  ustring(const char32_t *s, size_t length);
  explicit ustring(const wchar_t *s, bool validate = false);
  explicit ustring(const wchar_t *s, size_t length, bool validate = false);

  ustring(const std::string &s);
  ustring(const std::u8string &s);
  ustring(const std::u16string &s);
  ustring(const std::u32string &s);
  ustring(const std::wstring &s);

  ustring(std::string_view s);
  ustring(std::u8string_view s);
  ustring(std::u16string_view s);
  ustring(std::u32string_view s);
  ustring(std::wstring_view s);

  explicit ustring(size_type n, const char8_t *base_str);
  explicit ustring(size_type n, const ustring &base_str);

  ~ustring();

  template<class T> ustring &operator=(T &&other)
  {
    if constexpr (std::is_array_v<std::remove_reference_t<T>>) {
      *this = ustring(other);
    }
    else if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
      *this = ustring(*other);
    }
    else {
      if (this != &other) {
        *this = ustring(std::forward<T>(other));
      }
    }
    return *this;
  }
  ustring &operator=(const char8_t *other);
  ustring &operator=(const ustring &other);
  ustring &operator=(ustring &&other);

  friend std::ostream &operator<<(std::ostream &os, const ustring &str);
  friend std::istream &operator>>(std::istream &is, ustring &str);

  ustring &from_utf8(const char8_t *str, size_t size);
  ustring &from_utf16(const char16_t *str, size_t size);
  ustring &from_utf32(const char32_t *str, size_t size);

  [[nodiscard]] view to_view() const &;
  [[nodiscard]] view to_view(size_type left) const &;
  [[nodiscard]] view to_view(size_type left, size_type size) const &;
  [[nodiscard]] std::string to_string() const;
  [[nodiscard]] std::string_view to_string_view() const &;
  [[nodiscard]] std::u8string to_u8string() const;
  [[nodiscard]] std::u8string_view to_u8string_view() const &;
  [[nodiscard]] std::u16string to_u16string() const;
  [[nodiscard]] std::u32string to_u32string() const;
  [[nodiscard]] std::wstring to_wstring() const;

  template<typename View>
    requires requires(const ustring &s, const View &v) {
      {
        v.apply(s)
      } -> std::convertible_to<ustring>;
    }
  friend auto operator|(const ustring &str, const View &view)
  {
    return view.apply(str);
  }

  [[nodiscard]] iterator begin() noexcept;
  [[nodiscard]] const_iterator begin() const noexcept;
  [[nodiscard]] const_iterator cbegin() const noexcept;
  [[nodiscard]] iterator end() noexcept;
  [[nodiscard]] const_iterator end() const noexcept;
  [[nodiscard]] const_iterator cend() const noexcept;
  [[nodiscard]] reverse_iterator rbegin() noexcept;
  [[nodiscard]] const_reverse_iterator rbegin() const noexcept;
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept;
  [[nodiscard]] reverse_iterator rend() noexcept;
  [[nodiscard]] const_reverse_iterator rend() const noexcept;
  [[nodiscard]] const_reverse_iterator crend() const noexcept;

  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] size_type size() const noexcept;
  [[nodiscard]] static constexpr size_type max_size() noexcept
  {
    return view::max_size();
  }
  void reserve(size_type new_cap);
  [[nodiscard]] size_type capacity() const noexcept;
  void shrink_to_fit();
  void clear() noexcept;

  [[nodiscard]] size_type length() const noexcept;

  [[nodiscard]] reference operator[](size_type pos);
  [[nodiscard]] const_reference operator[](size_type pos) const;
  [[nodiscard]] reference at(size_type pos);
  [[nodiscard]] const_reference at(size_type pos) const;
  [[nodiscard]] reference front();
  [[nodiscard]] const_reference front() const;
  [[nodiscard]] reference back();
  [[nodiscard]] const_reference back() const;
  [[nodiscard]] std::span<value_type> span() noexcept;
  [[nodiscard]] code_point_iterator char_at(size_type index) const;
  [[nodiscard]] code_point_iterator front_char() const;
  [[nodiscard]] code_point_iterator back_char() const;

  ustring &assign(const value_type *s, size_type n);
  ustring &append(const char *s);
  ustring &append(char s);
  ustring &append(const ustring &str);
  ustring &append(const value_type *s);
  ustring &append(const value_type *s, size_type n);
  ustring &append(size_type n, value_type c);
  template<class InputIt> ustring &append(InputIt first, InputIt last)
  {
    while (first != last) {
      push_back(*first);
      ++first;
    }
    return *this;
  }

  ustring &operator+=(const ustring &str);
  ustring &operator+=(value_type ch);
  ustring &operator+=(const value_type *s);
  ustring &operator+=(const char *s);
  ustring &operator+=(char s);

  void push_back(value_type ch);
  void pop_back();

  ustring &set_char(size_type char_index, char32_t code);
  ustring &insert(size_type pos, const ustring &str);
  ustring &insert(size_type pos, const char *s);
  ustring &insert(size_type pos, const char *s, size_type n);
  ustring &insert(size_type pos, const value_type *s);
  ustring &insert(size_type pos, const value_type *s, size_type n);
  ustring &insert(size_type pos, size_type n, value_type c);
  iterator insert(const_iterator pos, value_type c);
  iterator insert(const_iterator pos, size_type n, value_type c);
  template<class InputIt> iterator insert(const_iterator pos, InputIt first, InputIt last)
  {
    size_type offset = pos - cbegin();
#ifdef _DEBUG
    if (offset > _size) {
      throw std::out_of_range("insert position out of range");
    }
#endif

    std::vector<value_type> temp;
    while (first != last) {
      temp.push_back(*first);
      ++first;
    }

    insert(offset, temp.data(), temp.size());
    return begin() + offset;
  }

  ustring &erase(size_type pos = 0, size_type n = npos);
  iterator erase(const_iterator pos);
  iterator erase(const_iterator first, const_iterator last);

  void resize(size_type n);
  void resize(size_type n, value_type c);
  void swap(ustring &other) noexcept;

  [[nodiscard]] ustring copy() const;
  [[nodiscard]] size_type copy(value_type *dest, size_type n, size_type pos = 0) const;
  [[nodiscard]] ustring substr(size_type pos = 0, size_type n = npos) const;
  [[nodiscard]] view substr_view(size_type pos = 0, size_type n = npos) const;

  [[nodiscard]] size_type find(const ustring &str, size_type pos = 0) const noexcept;
  [[nodiscard]] size_type find(const value_type *s, size_type pos, size_type n) const;
  [[nodiscard]] size_type find(const value_type *s, size_type pos = 0) const;
  [[nodiscard]] size_type find(value_type c, size_type pos = 0) const noexcept;

  [[nodiscard]] size_type rfind(const ustring &str, size_type pos = npos) const noexcept;
  [[nodiscard]] size_type rfind(const value_type *s, size_type pos, size_type n) const;
  [[nodiscard]] size_type rfind(const value_type *s, size_type pos = npos) const;
  [[nodiscard]] size_type rfind(value_type c, size_type pos = npos) const noexcept;

  [[nodiscard]] size_type find_first_of(const ustring &str, size_type pos = 0) const noexcept;
  [[nodiscard]] size_type find_first_of(const value_type *s, size_type pos, size_type n) const;
  [[nodiscard]] size_type find_first_of(const value_type *s, size_type pos = 0) const;
  [[nodiscard]] size_type find_first_of(value_type c, size_type pos = 0) const noexcept;

  [[nodiscard]] size_type find_last_of(const ustring &str, size_type pos = npos) const noexcept;
  [[nodiscard]] size_type find_last_of(const value_type *s, size_type pos, size_type n) const;
  [[nodiscard]] size_type find_last_of(const value_type *s, size_type pos = npos) const;
  [[nodiscard]] size_type find_last_of(value_type c, size_type pos = npos) const noexcept;

  [[nodiscard]] size_type find_first_not_of(const ustring &str, size_type pos = 0) const noexcept;
  [[nodiscard]] size_type find_first_not_of(const value_type *s, size_type pos, size_type n) const;
  [[nodiscard]] size_type find_first_not_of(const value_type *s, size_type pos = 0) const;
  [[nodiscard]] size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept;

  [[nodiscard]] size_type find_last_not_of(const ustring &str,
                                           size_type pos = npos) const noexcept;
  [[nodiscard]] size_type find_last_not_of(const value_type *s, size_type pos, size_type n) const;
  [[nodiscard]] size_type find_last_not_of(const value_type *s, size_type pos = npos) const;
  [[nodiscard]] size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept;

  [[nodiscard]] size_t count(const ustring &str) const noexcept;
  [[nodiscard]] size_t count(const value_type *s) const;
  [[nodiscard]] size_t count(char32_t c) const;
  [[nodiscard]] size_t count(std::function<bool(char32_t)> f) const;

  [[nodiscard]] bool contains(const ustring &str) const noexcept;
  [[nodiscard]] bool contains(const value_type *s) const;
  [[nodiscard]] bool contains(char32_t c) const noexcept;
  [[nodiscard]] bool contains(value_type c) const noexcept;
  [[nodiscard]] bool contains(std::function<bool(char32_t)> f) const;

  [[nodiscard]] int compare(const ustring &str) const noexcept;
  [[nodiscard]] int compare(size_type pos1, size_type n1, const ustring &str) const;
  [[nodiscard]] int compare(
      size_type pos1, size_type n1, const ustring &str, size_type pos2, size_type n2 = npos) const;
  [[nodiscard]] int compare(const value_type *s) const;
  [[nodiscard]] int compare(size_type pos1, size_type n1, const value_type *s) const;
  [[nodiscard]] int compare(size_type pos1, size_type n1, const value_type *s, size_type n2) const;

  bool operator==(const char *rhs) const noexcept;
  std::strong_ordering operator<=>(const char *rhs) const noexcept;
  bool operator==(const char8_t *rhs) const noexcept;
  std::strong_ordering operator<=>(const char8_t *rhs) const noexcept;
  bool operator==(const char16_t *rhs) const noexcept;
  std::strong_ordering operator<=>(const char16_t *rhs) const noexcept;
  bool operator==(const char32_t *rhs) const noexcept;
  std::strong_ordering operator<=>(const char32_t *rhs) const noexcept;
  friend bool operator==(const ustring &lhs, const ustring &rhs) noexcept;
  friend std::strong_ordering operator<=>(const ustring &lhs, const ustring &rhs) noexcept;
  template<typename T>
    requires std::convertible_to<T, ustring>
  friend bool operator==(const ustring &lhs, T &&rhs) noexcept
  {
    return lhs == ustring(std::forward<T>(rhs));
  }
  template<typename T>
    requires std::convertible_to<T, ustring>
  friend auto operator<=>(const ustring &lhs, const T *rhs) noexcept
  {
    return lhs <=> ustring(std::forward<T>(rhs));
  }

  friend ustring operator+(const ustring &lhs, const ustring &rhs);
  friend ustring operator+(const ustring &lhs, value_type rhs);
  friend ustring operator+(value_type lhs, const ustring &rhs);
  friend ustring operator+(const value_type *lhs, const ustring &rhs);
  friend ustring operator+(const ustring &lhs, const value_type *rhs);

  [[nodiscard]] bool is_alpha() const noexcept;
  [[nodiscard]] bool is_digit() const noexcept;
  [[nodiscard]] bool is_alnum() const noexcept;
  [[nodiscard]] bool is_space() const noexcept;
  [[nodiscard]] bool is_lower() const noexcept;
  [[nodiscard]] bool is_upper() const noexcept;
  [[nodiscard]] bool is_title() const noexcept;
  [[nodiscard]] bool is_normalized(const NormalizationConfig &config) const;

  ustring &filter(std::function<bool(char32_t, size_type)> &&codepoint_filter);
  ustring &transform(std::function<char32_t(char32_t, size_type)> &&codepoint_transformer);
  ustring &to_lower(bool any_lower = false);
  ustring &to_upper(bool any_upper = false);
  ustring &capitalize(const char *locale = nullptr);
  ustring &swap_case();
  ustring &trim();
  ustring &title(const char *locale = nullptr, ToTitleOptions options = ToTitleOptions::DEFAULT);
  ustring &strip(const value_type *ch = u8" ");
  ustring &strip(const ustring &ch);
  ustring &normalize(const NormalizationConfig &config);

  ustring filtered(std::function<bool(char32_t, size_type)> &&codepoint_filter) const;
  ustring transformed(std::function<char32_t(char32_t, size_type)> &&codepoint_transformer) const;
  ustring lowered(bool any_lower = false) const;
  ustring uppered(bool any_upper = false) const;
  ustring capitalized() const;
  ustring case_swapped() const;
  ustring trimmed() const;
  ustring titled(const char *locale = nullptr,
                 ToTitleOptions options = ToTitleOptions::DEFAULT) const;
  ustring stripped(const value_type *ch = u8" ") const;
  ustring normalized(const NormalizationConfig &config) const;

  // iterators
  [[nodiscard]] code_point_iterator code_points_begin() const noexcept
  {
    return code_point_iterator(*this);
  }
  [[nodiscard]] code_point_iterator code_points_end() const noexcept
  {
    return code_point_iterator(*this, size());
  }
  [[nodiscard]] auto code_points() const noexcept
  {
    return std::ranges::subrange<code_point_iterator>(code_points_begin(), code_points_end());
  }
  [[nodiscard]] grapheme_iterator graphemes_begin() const noexcept
  {
    return grapheme_iterator(*this);
  }
  [[nodiscard]] grapheme_iterator graphemes_end() const noexcept
  {
    return grapheme_iterator(*this, size());
  }
  [[nodiscard]] auto graphemes() const noexcept
  {
    return std::ranges::subrange<grapheme_iterator>(graphemes_begin(), graphemes_end());
  }
  [[nodiscard]] word_iterator words_begin() const noexcept
  {
    return word_iterator(*this);
  }
  [[nodiscard]] word_iterator words_end() const noexcept
  {
    return word_iterator(*this, size());
  }
  [[nodiscard]] auto words() const noexcept
  {
    return std::ranges::subrange<word_iterator>(words_begin(), words_end());
  }
  [[nodiscard]] sentence_iterator sentences_begin() const noexcept
  {
    return sentence_iterator(*this);
  }
  [[nodiscard]] sentence_iterator sentences_end() const noexcept
  {
    return sentence_iterator(*this, size());
  }
  [[nodiscard]] auto sentences() const noexcept
  {
    return std::ranges::subrange<sentence_iterator>(sentences_begin(), sentences_end());
  }

  // ranges support
  template<typename UnaryPredicate> [[nodiscard]] bool all_of(UnaryPredicate p) const
  {
    return std::ranges::all_of(code_points(), std::move(p));
  }

  template<typename UnaryPredicate> [[nodiscard]] bool any_of(UnaryPredicate p) const
  {
    return std::ranges::any_of(code_points(), std::move(p));
  }

  template<typename UnaryPredicate> [[nodiscard]] bool none_of(UnaryPredicate p) const
  {
    return std::ranges::none_of(code_points(), std::move(p));
  }

  template<typename T> [[nodiscard]] size_type count(const T &value) const
  {
    return std::ranges::count(code_points(), value);
  }

  template<typename UnaryPredicate> [[nodiscard]] size_type count_if(UnaryPredicate p) const
  {
    return std::ranges::count_if(code_points(), std::move(p));
  }

  [[nodiscard]] ustring sort() const;
  [[nodiscard]] ustring unique() const;
  [[nodiscard]] std::vector<view> split(char32_t delimiter) const;
  [[nodiscard]] std::vector<view> split(std::unordered_set<char32_t> &&delimiter) const;
  [[nodiscard]] std::vector<view> split(const ustring &delimiter) const;
  [[nodiscard]] std::vector<view> split_words(const char *locale) const;

  // ranges 支持
  template<std::ranges::range R>
  [[nodiscard]] static ustring join(R &&range, const ustring &delimiter = ustring())
  {
    return std::accumulate(
        std::next(std::begin(range)),
        std::end(range),
        *std::begin(range),
        [&delimiter](ustring a, const auto &b) { return std::move(a) + delimiter + ustring(b); });
  }

  //   template <typename Pred> [[nodiscard]] auto split_if(Pred pred) const {
  //     return std::views::split(*this, std::move(pred)) |
  //            std::views::transform(
  //                [](auto &&r) { return ustring(std::string_view(r)); });
  //   }

  //   [[nodiscard]] auto lines() const {
  //     return split_if([](char32_t c) { return c == '\n' || c == '\r'; });
  //   }

  //   [[nodiscard]] auto chunks(size_type n) const {
  //     return std::views::chunk(*this, n) | std::views::transform([](auto &&r)
  //     {
  //              return ustring(std::string_view(r));
  //            });
  //   }

  [[nodiscard]] size_t hash() const noexcept
  {
    return std::hash<std::u8string_view>{}(std::u8string_view(data(), size()));
  }

  //[[nodiscard]] bool matches(const ustring &pattern,
  //                           const pattern_options &options = {}) const;
  //[[nodiscard]] std::vector<std::pair<size_type, size_type>>
  // find_all_matches(const ustring &pattern,
  //                 const pattern_options &options = {}) const;
  //[[nodiscard]] ustring
  // replace_all_matches(const ustring &pattern, const ustring &replacement,
  //                    const pattern_options &options = {}) const;

  [[nodiscard]] ustring &to_halfwidth();  // 全角转半角
  [[nodiscard]] ustring &to_fullwidth();  // 半角转全角
  [[nodiscard]] ustring &normalize_whitespace(bool including_zero_width = true);  // 规范化空白字符
  [[nodiscard]] ustring &normalize_quotes();  // 规范化引号
  [[nodiscard]] ustring &normalize_dashes();  // 规范化破折号
  [[nodiscard]] ustring &simplify();          // 简化文本（如中文简繁转换）
  [[nodiscard]] ustring &traditionalize();    // 繁化文本（如中文简繁转换）

  [[nodiscard]] ustring escape_html() const;
  [[nodiscard]] ustring unescape_html() const;
  [[nodiscard]] ustring escape_json() const;
  [[nodiscard]] ustring unescape_json() const;
  [[nodiscard]] ustring escape_xml() const;
  [[nodiscard]] ustring unescape_xml() const;

  [[nodiscard]] pointer data() noexcept;
  [[nodiscard]] const_pointer data() const noexcept;

 private:
  bool is_using_buffer() const
  {
    return _using_buffer;
  }
  void preallocate(size_type size = 0);
  union {
    value_type _buf[default_size];
    value_type *_ptr;
  };

  struct {
    bool _using_buffer : 1;
    size_type _capacity : sizeof(size_type) * 8 - 1;
  };
  size_type _size;
};

using ustring_view = ustring::view;

template<> struct std::formatter<ustring> : std::formatter<std::string_view> {
  auto format(const ustring &str, format_context &ctx) const
  {
    return formatter<string_view>::format(str.to_string_view(), ctx);
  }
};
