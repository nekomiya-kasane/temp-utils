#include "ustring.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <locale>
#include <memory>
#include <numeric>
#include <set>
#include <unordered_map>
#include <unordered_set>

#define U_CHARSET_IS_UTF8 1

#include <unicode/brkiter.h>
#include <unicode/casemap.h>
#include <unicode/coleitr.h>
#include <unicode/coll.h>
#include <unicode/errorcode.h>
#include <unicode/localpointer.h>
#include <unicode/locid.h>
#include <unicode/schriter.h>
#include <unicode/tblcoll.h>
#include <unicode/translit.h>
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

auto init = []() {
  UErrorCode status = U_ZERO_ERROR;
  std::locale::global(std::locale("en_US.UTF-8"));
  u_init(&status);  // 确保 ICU 正确初始化
  if (U_FAILURE(status)) {
    std::cerr << "ICU initialization failed: " << u_errorName(status) << std::endl;
  }
  return status;
}();

namespace {

static_assert((int)Normalization2Mode::COMPOSE == (int)UNormalization2Mode::UNORM2_COMPOSE);
static_assert((int)Normalization2Mode::COMPOSE_CONTIGUOUS ==
              (int)UNormalization2Mode::UNORM2_COMPOSE_CONTIGUOUS);
static_assert((int)Normalization2Mode::DECOMPOSE == (int)UNormalization2Mode::UNORM2_DECOMPOSE);
static_assert((int)Normalization2Mode::FCD == (int)UNormalization2Mode::UNORM2_FCD);

static_assert((int)ToTitleOptions::WHOLE_STRING == U_TITLECASE_WHOLE_STRING);
static_assert((int)ToTitleOptions::SENTENCES == U_TITLECASE_SENTENCES);
static_assert((int)ToTitleOptions::NO_LOWERCASE == U_TITLECASE_NO_LOWERCASE);
static_assert((int)ToTitleOptions::NO_BREAK_ADJUSTMENT == U_TITLECASE_NO_BREAK_ADJUSTMENT);
static_assert((int)ToTitleOptions::ADJUST_TO_CASED == U_TITLECASE_ADJUST_TO_CASED);

auto _swap_case(char32_t c, ustring::size_type)
{
  if (u_islower(c)) {
    return (char32_t)u_toupper(c);
  }
  else if (u_isupper(c)) {
    return (char32_t)(u_tolower(c));
  }
  return c;
}

auto _to_lower(char32_t c, ustring::size_type)
{
  return u_tolower(c);
}

auto _to_upper(char32_t c, ustring::size_type)
{
  return u_toupper(c);
}

const char *_get_normalization_data_file(const NormalizationConfig &config)
{
  switch (config.data_file) {
    case NormalizationDataFile::NFC:
      return "nfc";
    case NormalizationDataFile::NFKC:
      return "nfkc";
    case NormalizationDataFile::NFKC_CF:
      return "nfkc_cf";
    case NormalizationDataFile::NFKC_SCF:
      return "nfkc_scf";
    case NormalizationDataFile::CUSTOM:
      return config.custom_data_file;
  }
  return nullptr;
}

}  // namespace

std::string to_utf8(char32_t codepoint)
{
  std::string sResult;
  if (codepoint <= 0x7F) {
    sResult.resize(1);
    sResult[0] = codepoint;
  }
  else if (codepoint <= 0x7FF) {
    sResult.resize(2);
    sResult[0] = 0xC0 | ((codepoint >> 6) & 0x1F);
    sResult[1] = 0x80 | (codepoint & 0x3F);
  }
  else if (codepoint <= 0xFFFF) {
    sResult.resize(3);
    sResult[0] = 0xE0 | ((codepoint >> 12) & 0x0F);
    sResult[1] = 0x80 | ((codepoint >> 6) & 0x3F);
    sResult[2] = 0x80 | (codepoint & 0x3F);
  }
  else if (codepoint <= 0x1FFFFF) {
    sResult.resize(4);
    sResult[0] = 0xF0 | ((codepoint >> 18) & 0x07);
    sResult[1] = 0x80 | ((codepoint >> 12) & 0x3F);
    sResult[2] = 0x80 | ((codepoint >> 6) & 0x3F);
    sResult[3] = 0x80 | (codepoint & 0x3F);
  }
  else {
    sResult = "\xEF\xBF\xBD";  // placeholder
  }
  return sResult;
}

inline CharProperty operator|(CharProperty a, CharProperty b)
{
  return static_cast<CharProperty>(static_cast<int>(a) | static_cast<int>(b));
}

bool has_property(std::u8string_view str, CharProperty property)
{
  if (str.empty()) {
    return false;
  }

  int32_t offset = 0, len = str.size();
  UChar32 codePoint;

  U8_NEXT(str.data(), offset, len, codePoint);
  if (codePoint < 0 || len < str.size()) {
    return false;
  }

  return has_property(codePoint, property);
}

bool has_property(const char8_t *str, CharProperty property)
{
  return str && has_property(to_codepoint(str), property);
}

bool has_property(char32_t codepoint, CharProperty property)
{
  auto result = get_property(codepoint);
  return (static_cast<int>(result) & static_cast<int>(property)) != 0;
}

CharProperty get_property(std::u8string_view str)
{
  return str.empty() ? CharProperty::NONE : get_property(to_codepoint(str.data()));
}

CharProperty get_property(const char8_t *str)
{
  return str ? get_property(to_codepoint(str)) : CharProperty::NONE;
}

CharProperty get_property(char32_t codepoint)
{
  uint32_t result = 0;
  if (u_isalpha(codepoint))
    result |= static_cast<uint32_t>(CharProperty::ALPHABETIC);
  if (u_islower(codepoint))
    result |= static_cast<uint32_t>(CharProperty::LOWERCASE);
  if (u_isupper(codepoint))
    result |= static_cast<uint32_t>(CharProperty::UPPERCASE);
  if (u_isspace(codepoint))
    result |= static_cast<uint32_t>(CharProperty::WHITESPACE);
  if (u_isdigit(codepoint))
    result |= static_cast<uint32_t>(CharProperty::DIGIT);
  if (u_ispunct(codepoint))
    result |= static_cast<uint32_t>(CharProperty::PUNCTUATION);
  if (u_iscntrl(codepoint))
    result |= static_cast<uint32_t>(CharProperty::CONTROL);
  if (u_hasBinaryProperty(codepoint, UCHAR_EMOJI))
    result |= static_cast<uint32_t>(CharProperty::EMOJI);
  if (u_hasBinaryProperty(codepoint, UCHAR_IDEOGRAPHIC))
    result |= static_cast<uint32_t>(CharProperty::IDEOGRAPHIC);
  if (u_isalpha(codepoint))
    result |= static_cast<uint32_t>(CharProperty::LETTER);
  if (u_hasBinaryProperty(codepoint, UCHAR_MATH))
    result |= static_cast<uint32_t>(CharProperty::MATH);
  if (u_isxdigit(codepoint))
    result |= static_cast<uint32_t>(CharProperty::HEXDIGIT);
  if (u_charType(codepoint) == U_COMBINING_SPACING_MARK)
    result |= static_cast<uint32_t>(CharProperty::COMBINING_MARK);
  if (u_hasBinaryProperty(codepoint, UCHAR_DASH))
    result |= static_cast<uint32_t>(CharProperty::DASH);
  if (u_hasBinaryProperty(codepoint, UCHAR_DIACRITIC))
    result |= static_cast<uint32_t>(CharProperty::DIACRITIC);
  if (u_hasBinaryProperty(codepoint, UCHAR_EXTENDER))
    result |= static_cast<uint32_t>(CharProperty::EXTENDER);
  if (u_hasBinaryProperty(codepoint, UCHAR_GRAPHEME_BASE))
    result |= static_cast<uint32_t>(CharProperty::GRAPHEME_BASE);
  if (u_hasBinaryProperty(codepoint, UCHAR_GRAPHEME_EXTEND))
    result |= static_cast<uint32_t>(CharProperty::GRAPHEME_EXTEND);
  if (u_hasBinaryProperty(codepoint, UCHAR_GRAPHEME_LINK))
    result |= static_cast<uint32_t>(CharProperty::GRAPHEME_LINK);
  if (u_hasBinaryProperty(codepoint, UCHAR_IDS_BINARY_OPERATOR))
    result |= static_cast<uint32_t>(CharProperty::IDS_BINARY_OPERATOR);
  if (u_hasBinaryProperty(codepoint, UCHAR_IDS_TRINARY_OPERATOR))
    result |= static_cast<uint32_t>(CharProperty::IDS_TRINARY_OPERATOR);
  if (u_hasBinaryProperty(codepoint, UCHAR_JOIN_CONTROL))
    result |= static_cast<uint32_t>(CharProperty::JOIN_CONTROL);
  if (u_hasBinaryProperty(codepoint, UCHAR_LOGICAL_ORDER_EXCEPTION))
    result |= static_cast<uint32_t>(CharProperty::LOGICAL_ORDER_EXCEPTION);
  if (u_hasBinaryProperty(codepoint, UCHAR_NONCHARACTER_CODE_POINT))
    result |= static_cast<uint32_t>(CharProperty::NONCHARACTER_CODE_POINT);
  if (u_hasBinaryProperty(codepoint, UCHAR_QUOTATION_MARK))
    result |= static_cast<uint32_t>(CharProperty::QUOTATION_MARK);
  if (u_hasBinaryProperty(codepoint, UCHAR_RADICAL))
    result |= static_cast<uint32_t>(CharProperty::RADICAL);
  if (u_hasBinaryProperty(codepoint, UCHAR_SOFT_DOTTED))
    result |= static_cast<uint32_t>(CharProperty::SOFT_DOTTED);
  if (u_hasBinaryProperty(codepoint, UCHAR_TERMINAL_PUNCTUATION))
    result |= static_cast<uint32_t>(CharProperty::TERMINAL_PUNCTUATION);
  if (u_hasBinaryProperty(codepoint, UCHAR_UNIFIED_IDEOGRAPH))
    result |= static_cast<uint32_t>(CharProperty::UNIFIED_IDEOGRAPH);
  if (u_hasBinaryProperty(codepoint, UCHAR_VARIATION_SELECTOR))
    result |= static_cast<uint32_t>(CharProperty::VARIATION_SELECTOR);
  if (u_isUWhiteSpace(codepoint))
    result |= static_cast<uint32_t>(CharProperty::SPACE);
  // if (u_hasBinaryProperty(codepoint, UCHAR_PRIVATE_USE))
  //   result |= static_cast<uint32_t>(CharProperty::PRIVATE_USE);
  return static_cast<CharProperty>(result);
}

char32_t to_codepoint(std::u8string_view str)
{
  return str.empty() ? 0 : to_codepoint(str.data());
}

char32_t to_codepoint(const char8_t *str)
{
  return to_codepoint(reinterpret_cast<const char *>(str), 0);
}

char32_t to_codepoint(const char *str, size_t offset)
{
  const uint8_t *bytes = reinterpret_cast<const uint8_t *>(str + offset);
  if (!bytes[0]) {
    return 0;
  }
  if (bytes[0] < 0x80) {
    return bytes[0];
  }
  if ((bytes[0] & 0xE0) == 0xC0) {
    return bytes[1] ? (bytes[0] & 0x1F) << 6 | bytes[1] & 0x3F : 0;
  }
  if ((bytes[0] & 0xF0) == 0xE0) {
    return bytes[1] && bytes[2] ?
               (bytes[0] & 0x0F) << 12 | (bytes[1] & 0x3F) << 6 | (bytes[2] & 0x3F) :
               0;
  }
  if ((bytes[0] & 0xF8) == 0xF0) {
    return bytes[1] && bytes[2] && bytes[3] ? (bytes[0] & 0x07) << 18 | ((bytes[1] & 0x3F) << 12) |
                                                  (bytes[2] & 0x3F) << 6 | (bytes[3] & 0x3F) :
                                              0;
  }
  return 0;
}

void ustring::preallocate(ustring::size_type size)
{
  if (size <= default_size) {
    _using_buffer = true;
    _capacity = default_size;
    // if (size < default_size) {
    //   _buf[size] = '\0';
    // }
  }
  else {
    _using_buffer = false;
    _capacity = size;
    _ptr = new value_type[_capacity];
  }
  _size = size;
}

ustring::ustring()
{
  preallocate();
}

ustring::ustring(const ustring &other)
{
  _using_buffer = other._using_buffer;
  _capacity = other._capacity;
  if (!is_using_buffer()) {
    _ptr = new value_type[_capacity];
  }
  std::memcpy(data(), other.data(), other.size());
  _size = other._size;
}

ustring::ustring(ustring &&other) noexcept
{
  _using_buffer = other._using_buffer;
  _capacity = other._capacity;
  other._capacity = default_size;
  if (is_using_buffer()) {
    std::memmove(data(), other.data(), other.size());
  }
  else {
    _ptr = other._ptr;
    other._ptr = nullptr;
    other._using_buffer = true;
  }
  _size = other._size;
  other._size = 0;
  other._buf[0] = '\0';
}

ustring::ustring(view other)
{
  if (other.size() <= default_size) {
    _using_buffer = true;
    _capacity = default_size;
  }
  else {
    _using_buffer = false;
    _capacity = other.size();
    _ptr = new value_type[_capacity];
  }
  std::memmove(data(), other.data(), other.size());
  _size = other.size();
}

ustring::~ustring()
{
  if (!is_using_buffer()) {
    delete[] _ptr;
  }
}

ustring &ustring::operator=(const char8_t *other)
{
  _size = strlen((const char *)other);
  preallocate(_size);
  std::memmove(data(), other, _size);
  return *this;
}

ustring &ustring::operator=(const ustring &other)
{
  _using_buffer = other._using_buffer;
  _capacity = other._capacity;
  if (!is_using_buffer()) {
    _ptr = new value_type[_capacity + 1];
  }
  std::memcpy(data(), other.data(), other.size());
  _size = other._size;

  return *this;
}

ustring &ustring::operator=(ustring &&other)
{
  _using_buffer = other._using_buffer;
  _capacity = other._capacity;
  other._capacity = default_size;
  if (is_using_buffer()) {
    std::memmove(data(), other.data(), other.size());
  }
  else {
    _ptr = other._ptr;
    other._ptr = nullptr;
    other._using_buffer = true;
  }
  _size = other._size;
  other._size = 0;
  other._buf[0] = '\0';

  return *this;
}

std::ostream &operator<<(std::ostream &os, const ustring &str)
{
  return os << str.to_string_view();
}

std::istream &operator>>(std::istream &is, ustring &str)
{
  std::string temp;
  is >> temp;
  str = ustring(temp);
  return is;
}

//// Constructor from UnicodeString
// ustring::ustring(const icu::UnicodeString &str)
//{
//   int32_t utf8Length;
//   u_strToUTF8(nullptr, 0, &utf8Length, str.getBuffer(), str.length(), nullptr);
//   preallocate(utf8Length);
//   u_strToUTF8(reinterpret_cast<char *>(data()),
//               _capacity,
//               &utf8Length,
//               str.getBuffer(),
//               str.length(),
//               nullptr);
// }
//
//// Move constructor from UnicodeString
// ustring::ustring(icu::UnicodeString &&str)
//{
//   int32_t utf8Length;
//   u_strToUTF8(nullptr, 0, &utf8Length, str.getBuffer(), str.length(), nullptr);
//   preallocate(utf8Length);
//   u_strToUTF8(reinterpret_cast<char *>(data()),
//               _capacity,
//               &utf8Length,
//               str.getBuffer(),
//               str.length(),
//               nullptr);
// }

ustring::ustring(ustring::value_type c)
{
  preallocate();
  push_back(c);
}

// Constructor from char*
ustring::ustring(const char *s, bool validate) : ustring(s, std::strlen(s), validate) {}

// Constructor from char* with length
ustring::ustring(const char *s, size_t length, bool validate)
{
  if (!s) {
    preallocate();
    return;
  }
  _size = length;

  // Validate and measure UTF-8
  if (validate) {
    int32_t length = 0;
    UErrorCode status = U_ZERO_ERROR;
    u_strFromUTF8(nullptr, 0, &length, s, -1, &status);
    if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
      preallocate(0);
      return;
    }
  }

  // Get actual UTF-8 length
  preallocate(_size);
  std::memcpy(data(), s, _size);
}

// Constructor from char8_t*
ustring::ustring(const char8_t *s) : ustring(s, std::strlen(reinterpret_cast<const char *>(s))) {}

// Constructor from char8_t* with length
ustring::ustring(const char8_t *s, size_t length)
{
  preallocate(length);
  std::memcpy(data(), s, length);
}

// Constructor from char16_t*
ustring::ustring(const char16_t *s) : ustring(s, -1) {}

// Constructor from char16_t* with length
ustring::ustring(const char16_t *s, size_t length)
{
  preallocate();
  if (!s || length == 0) {
    return;
  }

  from_utf16(s, length);
}

// Constructor from char32_t*
ustring::ustring(const char32_t *s) : ustring(s, -1) {}

// Constructor from char32_t* with length
ustring::ustring(const char32_t *s, size_t length)
{
  preallocate();
  if (!s || length == 0) {
    return;
  }

  from_utf32(s, length);
}

// Constructor from wchar_t*
ustring::ustring(const wchar_t *s, bool validate) : ustring(s, -1, validate) {}

// Constructor from wchar_t* with length
ustring::ustring(const wchar_t *s, size_t length, bool validate)
{
  if (!s || length == 0) {
    preallocate(0);
    return;
  }

  if constexpr (sizeof(wchar_t) == 2) {
    // Windows: wchar_t is UTF-16
    // Get required UTF-8 length
    int32_t utf8Length;
    {
      UErrorCode status;
      u_strToUTF8(nullptr, 0, &utf8Length, reinterpret_cast<const UChar *>(s), length, &status);
      preallocate(utf8Length);
    }

    {
      // Convert to UTF-8
      UErrorCode status;
      u_strToUTF8(reinterpret_cast<char *>(data()),
                  _capacity,
                  &utf8Length,
                  reinterpret_cast<const UChar *>(s),
                  length,
                  &status);
      if (U_FAILURE(status)) {
        throw std::runtime_error("Failed to convert ustring to u16string");
      }
    }
  }
  else {
    // Unix-like: wchar_t is UTF-32
    // todo: on AIX is 2 bytes
    *this = ustring(reinterpret_cast<const char32_t *>(s), length);
  }
}

// Standard string constructors
ustring::ustring(const std::string &s) : ustring(s.c_str(), s.length()) {}
ustring::ustring(const std::u8string &s) : ustring(s.c_str(), s.length()) {}
ustring::ustring(const std::u16string &s) : ustring(s.c_str(), s.length()) {}
ustring::ustring(const std::u32string &s) : ustring(s.c_str(), s.length()) {}
ustring::ustring(const std::wstring &s) : ustring(s.c_str(), s.length()) {}

// String view constructors
ustring::ustring(std::string_view s) : ustring(s.data(), s.length()) {}
ustring::ustring(std::u8string_view s) : ustring(s.data(), s.length()) {}
ustring::ustring(std::u16string_view s) : ustring(s.data(), s.length()) {}
ustring::ustring(std::u32string_view s) : ustring(s.data(), s.length()) {}
ustring::ustring(std::wstring_view s) : ustring(s.data(), s.length()) {}

ustring::ustring(size_type n, const char8_t *base_str)
{
  _size = strlen((const char *)base_str);
  preallocate(_size * n);
  for (size_type i = 0; i < n; ++i) {
    append(base_str);
  }
}

ustring::ustring(size_type n, const ustring &base_str)
{
  preallocate(base_str.size() * n);
  for (size_type i = 0; i < n; ++i) {
    append(base_str);
  }
}

ustring &ustring::from_utf8(const char8_t *str, size_t size)
{
  assign(str, size);
  return *this;
}

ustring &ustring::from_utf16(const char16_t *str, size_t size)
{
  // Get required UTF-8 length
  int32_t utf8Length = 0;
  {
    UErrorCode status = UErrorCode::U_ZERO_ERROR;
    u_strToUTF8(nullptr, 0, &utf8Length, reinterpret_cast<const UChar *>(str), size, &status);
    resize(utf8Length);
  }

  {
    // Convert to UTF-8
    UErrorCode status = UErrorCode::U_ZERO_ERROR;
    u_strToUTF8(reinterpret_cast<char *>(data()),
                _capacity,
                &utf8Length,
                reinterpret_cast<const UChar *>(str),
                size,
                &status);
    if (U_FAILURE(status)) {
      throw std::runtime_error("Failed to convert ustring to u16string");
    }
  }

  return *this;
}

ustring &ustring::from_utf32(const char32_t *str, size_t size)
{
  std::string result;
  result.reserve(size * 4);
  for (size_t i = 0; i < size; ++i) {
    char32_t ch = str[i];
    result += to_utf8(str[i]);
  }

  resize(result.size());
  std::memcpy(data(), result.c_str(), result.size() + 1);

  return *this;
}

ustring::view ustring::to_view() const &
{
  return view(*this);
}

ustring::view ustring::to_view(size_type left) const &
{
  assert(left <= size());
  return view(data() + left, size() - left);
}

ustring::view ustring::to_view(size_type left, size_type len) const &
{
  assert(left + len <= size());
  return view(data() + left, std::min(size() - left, len));
}

std::string ustring::view::to_string() const
{
  return std::string(reinterpret_cast<const char *>(data()), _size);
}

std::u8string ustring::view::to_u8string() const
{
  return std::u8string(reinterpret_cast<const char8_t *>(data()), _size);
}

std::string_view ustring::view::to_string_view() const &
{
  return std::string_view(reinterpret_cast<const char *>(data()), _size);
}

std::u8string_view ustring::view::to_u8string_view() const &
{
  return std::u8string_view(reinterpret_cast<const char8_t *>(data()), _size);
}

std::u16string ustring::view::to_u16string() const
{
  std::u16string result;
  int32_t length;
  UErrorCode status = U_ZERO_ERROR;
  u_strFromUTF8(nullptr, 0, &length, reinterpret_cast<const char *>(data()), _size, &status);

  status = U_ZERO_ERROR;
  result.resize(length);
  u_strFromUTF8(&result[0],
                result.capacity(),
                nullptr,
                reinterpret_cast<const char *>(data()),
                _size,
                &status);
  if (U_FAILURE(status)) {
    throw std::runtime_error("Failed to convert ustring to u16string");
  }
  return result;
}

std::u32string ustring_view::view::to_u32string() const
{
  std::u32string result;

  const_iterator it = data();
  int32_t offset = 0, len = size();
  UChar32 codePoint;

  result.reserve(length());

  while (offset < len) {
    U8_NEXT(it, offset, len, codePoint);
    if (codePoint < 0) {
      throw std::runtime_error("Failed to convert ustring to u32string");
    }
    result.push_back(codePoint);
  }

  return result;
}

std::wstring ustring::view::to_wstring() const
{
  std::wstring result;
  if constexpr (sizeof(wchar_t) == 2) {
    int32_t length;
    UErrorCode status = U_ZERO_ERROR;
    u_strFromUTF8(nullptr, 0, &length, reinterpret_cast<const char *>(data()), _size, &status);
    if (U_FAILURE(status)) {
      throw std::runtime_error("Failed to convert ustring to u16string");
    }

    status = U_ZERO_ERROR;
    result.resize(length);
    u_strFromUTF8((UChar *)&result[0],
                  result.capacity(),
                  nullptr,
                  reinterpret_cast<const char *>(data()),
                  _size,
                  &status);
    if (U_FAILURE(status)) {
      throw std::runtime_error("Failed to convert ustring to u16string");
    }
  }
  else if constexpr (sizeof(wchar_t) == 4) {
    const_iterator it = data();
    int32_t offset = 0, len = size();
    UChar32 codePoint;

    while (offset < len) {
      U8_NEXT(it, offset, len, codePoint);
      if (codePoint < 0) {
        throw std::runtime_error("Failed to convert ustring to u32string");
      }
      result.push_back(codePoint);
    }
  }
  else {
    assert(false);
  }
  return result;
}

std::string ustring::to_string() const
{
  return to_view().to_string();
}

std::u8string ustring::to_u8string() const
{
  return to_view().to_u8string();
}

std::string_view ustring::to_string_view() const &
{
  return to_view().to_string_view();
}

std::u8string_view ustring::to_u8string_view() const &
{
  return to_view().to_u8string_view();
}

std::u16string ustring::to_u16string() const
{
  return to_view().to_u16string();
}

std::u32string ustring::to_u32string() const
{
  return to_view().to_u32string();
}

std::wstring ustring::to_wstring() const
{
  return to_view().to_wstring();
}

// Iterator functions
ustring::iterator ustring::begin() noexcept
{
  return data();
}

ustring::const_iterator ustring::begin() const noexcept
{
  return data();
}

ustring::const_iterator ustring::cbegin() const noexcept
{
  return data();
}

ustring::iterator ustring::end() noexcept
{
  return data() + _size;
}

ustring::const_iterator ustring::end() const noexcept
{
  return data() + _size;
}

ustring::const_iterator ustring::cend() const noexcept
{
  return data() + _size;
}

ustring::reverse_iterator ustring::rbegin() noexcept
{
  return reverse_iterator(end());
}

ustring::const_reverse_iterator ustring::rbegin() const noexcept
{
  return const_reverse_iterator(end());
}

ustring::const_reverse_iterator ustring::crbegin() const noexcept
{
  return const_reverse_iterator(end());
}

ustring::reverse_iterator ustring::rend() noexcept
{
  return reverse_iterator(begin());
}

ustring::const_reverse_iterator ustring::rend() const noexcept
{
  return const_reverse_iterator(begin());
}

ustring::const_reverse_iterator ustring::crend() const noexcept
{
  return const_reverse_iterator(begin());
}

//  Capacity functions
bool ustring::empty() const noexcept
{
  return _size == 0;
}

ustring::size_type ustring::size() const noexcept
{
  return _size;
}

ustring::size_type ustring::view::length() const noexcept
{
  if (!_data) {
    return 0;
  }

  int32_t offset = 0;
  int32_t length = static_cast<int32_t>(size());
  UChar32 codepoint;
  size_type count = 0;

  while (offset < length) {
    U8_NEXT(data(), offset, length, codepoint);

    if (codepoint < 0) {
      continue;
    }
    count++;
  }
  return count;
}

ustring::size_type ustring::length() const noexcept
{
  return to_view().length();
}

ustring::size_type ustring::capacity() const noexcept
{
  return _capacity;
}

void ustring::reserve(size_type new_cap)
{
#ifdef _DEBUG
  if (new_cap > max_size()) {
    throw std::length_error("ustring::reserve: capacity exceeds maximum");
  }
#endif

  if (new_cap > capacity()) {
    value_type *new_data = nullptr;
    new_data = new value_type[new_cap];
    std::copy_n(data(), _size, new_data);

    if (!is_using_buffer()) {
      delete[] _ptr;
    }

    _ptr = new_data;
    _capacity = new_cap;
    _using_buffer = false;
  }
}

void ustring::shrink_to_fit()
{
  if (!is_using_buffer() && _size <= default_size) {
    // Can switch back to using buffer
    auto old_ptr = _ptr;
    std::copy_n(_ptr, _size, _buf);
    delete[] old_ptr;
    _using_buffer = true;
    _capacity = default_size;
  }
  else if (!is_using_buffer() && _size < capacity()) {
    // Reallocate to exact size
    value_type *new_data = new value_type[_size];
    std::copy_n(_ptr, _size, new_data);
    delete[] _ptr;
    _ptr = new_data;
    _capacity = _size;
  }
}

void ustring::clear() noexcept
{
  _size = 0;
}

// Element access with conditional exception handling
ustring::reference ustring::front()
{
#ifdef _DEBUG
  if (empty()) {
    throw std::out_of_range("ustring::front: string is empty");
  }
#endif
  return data()[0];
}

ustring::const_reference ustring::front() const
{
#ifdef _DEBUG
  if (empty()) {
    throw std::out_of_range("ustring::front: string is empty");
  }
#endif
  return data()[0];
}

ustring::reference ustring::back()
{
#ifdef _DEBUG
  if (empty()) {
    throw std::out_of_range("ustring::back: string is empty");
  }
#endif
  return data()[_size - 1];
}

ustring::const_reference ustring::back() const
{
#ifdef _DEBUG
  if (empty()) {
    throw std::out_of_range("ustring::back: string is empty");
  }
#endif
  return data()[_size - 1];
}

ustring::reference ustring::operator[](size_type pos)
{
  return data()[pos];
}

ustring::const_reference ustring::operator[](size_type pos) const
{
  return data()[pos];
}

std::span<ustring::value_type> ustring::span() noexcept
{
  return {data(), static_cast<size_t>(size())};
}

ustring::reference ustring::at(size_type pos)
{
#ifdef _DEBUG
  if (pos >= _size) {
    throw std::out_of_range("ustring::at: position out of range");
  }
#endif
  return data()[pos];
}

ustring::const_reference ustring::at(size_type pos) const
{
#ifdef _DEBUG
  if (pos >= _size) {
    throw std::out_of_range("ustring::at: position out of range");
  }
#endif
  return data()[pos];
}

ustring::pointer ustring::data() noexcept
{
  return is_using_buffer() ? _buf : _ptr;
}

ustring::const_pointer ustring::data() const noexcept
{
  return is_using_buffer() ? _buf : _ptr;
}

// Modification functions
ustring &ustring::append(const ustring &str)
{
#ifdef _DEBUG
  if (max_size() - _size < str.size()) {
    throw std::length_error("ustring::append: length would exceed maximum");
  }
#endif
  return append(str.data(), str.size());
}

ustring &ustring::append(const value_type *s)
{
#ifdef _DEBUG
  if (!s) {
    throw std::invalid_argument("ustring::append: null pointer");
  }
#endif
  if (s) {
    return append(s, strlen(reinterpret_cast<const char *>(s)));
  }
  return *this;
}

ustring &ustring::append(const value_type *s, size_type n)
{
#ifdef _DEBUG
  if (!s && n > 0) {
    throw std::invalid_argument("ustring::append: null pointer");
  }
#endif

  if (!s || n == 0)
    return *this;

#ifdef _DEBUG
  if (max_size() - _size < n) {
    throw std::length_error("ustring::append: length would exceed maximum");
  }
#endif

  size_type new_size = _size + n;
  if (new_size > capacity()) {
    reserve(std::max(new_size, capacity() * 2));
  }

  std::copy_n(s, n, data() + _size);
  _size = new_size;
  return *this;
}

ustring &ustring::append(size_type n, value_type c)
{
#ifdef _DEBUG
  if (max_size() - _size < n) {
    throw std::length_error("ustring::append: length would exceed maximum");
  }
#endif

  if (n == 0)
    return *this;

  size_type new_size = _size + n;
  if (new_size > capacity()) {
    reserve(std::max(new_size, capacity() * 2));
  }

  std::fill_n(data() + _size, n, c);
  _size = new_size;
  return *this;
}

ustring &ustring::operator+=(const ustring &str)
{
  return append(str);
}

ustring &ustring::operator+=(value_type ch)
{
  push_back(ch);
  return *this;
}

ustring &ustring::operator+=(const value_type *s)
{
  return append(s);
}

ustring &ustring::assign(const value_type *s, size_type n)
{
  if (!n) {
    clear();
    return *this;
  }

  if (is_using_buffer() && n <= default_size) {
    std::memcpy(data(), s, n * sizeof(value_type));
    _size = n;
    // #ifdef _DEBUG
    //     if (n < default_size) {
    //       data()[n] = '\0';
    //     }
    // #endif
    return *this;
  }

  _using_buffer = false;
  _size = n;
  clear();
  reserve(n);
  append(s, n);

  return *this;
}

ustring &ustring::append(const char *s)
{
  if (!s) {
    return *this;
  }
  return append(reinterpret_cast<const value_type *>(s));
}

ustring &ustring::append(char s)
{
  push_back(static_cast<value_type>(s));
  return *this;
}

ustring &ustring::operator+=(const char *s)
{
  return append(s);
}

ustring &ustring::operator+=(char s)
{
  return append(s);
}

void ustring::push_back(value_type ch)
{
#ifdef _DEBUG
  if (_size == max_size()) {
    throw std::length_error("ustring::push_back: length exceeds maximum");
  }
  if (_size > capacity()) {
    throw std::length_error("ustring::push_back: bad size");
  }
#endif

  if (_size == capacity()) {
    reserve(capacity() * 3 / 2);
  }
  data()[_size++] = ch;
}

void ustring::pop_back()
{
#ifdef _DEBUG
  if (empty()) {
    throw std::out_of_range("ustring::pop_back: string is empty");
  }
#endif
  if (!empty()) {
    --_size;
  }
}

ustring &ustring::insert(size_type pos, const ustring &str)
{
#ifdef _DEBUG
  if (pos > _size) {
    throw std::out_of_range("ustring::insert: position out of range");
  }
#endif

#ifdef _DEBUG
  if (max_size() - _size < str.size()) {
    throw std::length_error("ustring::insert: length would exceed maximum");
  }
#endif

  if (pos <= _size) {
    return insert(pos, str.data(), str.size());
  }
  return *this;
}

ustring &ustring::insert(size_type pos, const char *s)
{
  static_assert(sizeof(value_type) == sizeof(char));
  return insert(pos, reinterpret_cast<const value_type *>(s));
}

ustring &ustring::insert(size_type pos, const char *s, size_type n)
{
  static_assert(sizeof(value_type) == sizeof(char));
  return insert(pos, reinterpret_cast<const value_type *>(s), n);
}

ustring &ustring::insert(size_type pos, const value_type *s)
{
#ifdef _DEBUG
  if (!s) {
    throw std::invalid_argument("ustring::insert: null pointer");
  }
#endif
  if (s) {
    return insert(pos, s, strlen(reinterpret_cast<const char *>(s)));
  }
  return *this;
}

ustring::iterator ustring::insert(const_iterator pos, value_type c)
{
  size_type offset = pos - begin();
#ifdef _DEBUG
  if (offset > _size) {
    throw std::out_of_range("ustring::insert: iterator out of range");
  }
#endif
  if (offset <= _size) {
    insert(offset, 1, c);
    return begin() + offset;
  }
  return begin() + _size;
}

ustring &ustring::insert(size_type pos, size_type n, value_type c)
{
#ifdef _DEBUG
  if (pos > _size) {
    throw std::out_of_range("ustring::insert: position out of range");
  }
  if (max_size() - _size < n) {
    throw std::length_error("ustring::insert: length would exceed maximum");
  }
#endif

  if (n == 0) {
    return *this;
  }

  size_type new_size = _size + n;
  if (new_size > capacity()) {
    reserve(std::max(new_size, capacity() * 2));
  }

  if (pos < _size) {
    std::copy_backward(data() + pos, data() + _size, data() + new_size);
  }

  std::fill_n(data() + pos, n, c);
  _size = new_size;

  return *this;
}

ustring::iterator ustring::insert(const_iterator pos, size_type n, value_type c)
{
  if (n == 0)
    return const_cast<iterator>(pos);

  size_type offset = pos - cbegin();
#ifdef _DEBUG
  if (offset > _size) {
    throw std::out_of_range("insert position out of range");
  }
#endif

  insert(offset, n, c);
  return begin() + offset;
}

ustring &ustring::insert(size_type pos, const value_type *s, size_type n)
{
#ifdef _DEBUG
  if (pos > _size) {
    throw std::out_of_range("insert position out of range");
  }
#endif
  if (!s || n == 0)
    return *this;

  // Calculate new size and reserve space
  size_type new_size = _size + n;
  if (new_size > _capacity) {
    size_type new_capacity = std::max(new_size, _capacity * 2);
    value_type *new_data = new value_type[new_capacity + 1];

    // Copy data before pos
    std::copy(data(), data() + pos, new_data);
    // Copy new data
    std::copy(s, s + n, new_data + pos);
    // Copy data after pos
    std::copy(data() + pos, data() + _size, new_data + pos + n);

    // new_data[new_size] = '\0';

    if (!is_using_buffer()) {
      delete[] _ptr;
    }
    _ptr = new_data;
    _using_buffer = false;
    _capacity = new_capacity;
  }
  else {
    // Shift existing data
    std::copy_backward(data() + pos, data() + _size, data() + new_size);
    // Insert new data
    std::copy(s, s + n, data() + pos);
  }

  _size = new_size;
  return *this;
}

ustring::iterator ustring::erase(const_iterator pos)
{
  return erase(pos, pos + 1);
}

ustring::iterator ustring::erase(const_iterator first, const_iterator last)
{
  size_type pos = first - cbegin(), len = last - first;
#ifdef _DEBUG
  if (pos > _size || pos + len > _size) {
    throw std::out_of_range("erase range out of range");
  }
#endif

  if (len == 0)
    return first;

  // Move remaining characters
  std::copy(data() + pos + len, data() + _size, data() + pos);
  _size -= len;
  // data()[_size] = '\0';

  return begin() + pos;
}

ustring &ustring::erase(size_type pos, size_type n)
{
#ifdef _DEBUG
  if (pos > _size) {
    throw std::out_of_range("ustring::erase: position out of range");
  }
#endif

  if (pos < _size) {
    if (n == npos || pos + n > _size) {
      n = _size - pos;
    }

    if (n > 0) {
      std::copy(data() + pos + n, data() + _size, data() + pos);
      _size -= n;
    }
  }

  return *this;
}

void ustring::resize(size_type n)
{
#ifdef _DEBUG
  if (n > max_size()) {
    throw std::length_error("ustring::resize: length exceeds maximum");
  }
#endif

  if (n > capacity()) {
    reserve(n);
  }

  _size = n;
}

void ustring::resize(size_type n, value_type c)
{
#ifdef _DEBUG
  if (n > max_size()) {
    throw std::length_error("ustring::resize: length exceeds maximum");
  }
#endif

  if (n > _size) {
    // Need to grow and fill new elements with c
    if (n > capacity()) {
      reserve(n);
    }
    std::fill_n(data() + _size, n - _size, c);
  }
  _size = n;
}

void ustring::swap(ustring &other) noexcept
{
  if (is_using_buffer() && other.is_using_buffer()) {
    // Both using buffer, swap buffers
    std::swap_ranges(_buf, _buf + default_size, other._buf);
  }
  else if (!is_using_buffer() && !other.is_using_buffer()) {
    // Both using heap, swap pointers
    std::swap(_ptr, other._ptr);
  }
  else {
    // One buffer, one heap - need to do a full swap
    value_type temp_buf[default_size];
    if (is_using_buffer()) {
      // this is buffer, other is heap
      std::copy_n(_buf, default_size, temp_buf);
      _ptr = other._ptr;
      std::copy_n(temp_buf, default_size, other._buf);
    }
    else {
      // this is heap, other is buffer
      std::copy_n(other._buf, default_size, temp_buf);
      other._ptr = _ptr;
      std::copy_n(temp_buf, default_size, _buf);
    }
  }

  // Swap metadata using temporary variables since we can't swap bit fields
  // directly
  bool temp_using_buffer = _using_buffer;
  size_type temp_capacity = _capacity;
  size_type temp_size = _size;

  _using_buffer = other._using_buffer;
  _capacity = other._capacity;
  _size = other._size;

  other._using_buffer = temp_using_buffer;
  other._capacity = temp_capacity;
  other._size = temp_size;
}

ustring ustring::copy() const
{
  return ustring(*this);
}

ustring::size_type ustring::copy(value_type *dest, size_type n, size_type pos) const
{
  if (pos >= _size)
    return 0;

  const size_type len = std::min(n == npos ? max_pos : n, _size - pos);
  std::copy_n(data() + pos, len, dest);
  return len;
}

ustring_view ustring::view::substr_view(size_type pos, size_type n) const
{
  if (pos >= _size)
    return ustring_view(data() + size(), 0);

  const size_type len = std::min(n == npos ? max_pos : n, _size - pos);
  return ustring_view(data() + pos, len);
}

ustring_view ustring::substr_view(size_type pos, size_type n) const
{
  return to_view().substr_view(pos, n);
}

ustring ustring::view::substr(size_type pos, size_type n) const
{
  return ustring(substr_view(pos, n));
}

ustring ustring::substr(size_type pos, size_type n) const
{
  return to_view().substr(pos, n);
}

ustring::size_type ustring::view::find(const ustring &str, size_type pos) const noexcept
{
  if (pos >= _size || str._size > _size - pos)
    return npos;
  if (str.empty())
    return pos;

  // Simple implementation of Boyer-Moore-Horspool algorithm
  const value_type *const pat = str.data();
  const size_type pat_len = str._size;
  const value_type *const text = data();
  const size_type text_len = _size;

  // Build bad character table
  size_type skip[256];
  for (size_type i = 0; i < 256; i++)
    skip[i] = pat_len;
  for (size_type i = 0; i < pat_len - 1; i++) {
    skip[static_cast<unsigned char>(pat[i])] = pat_len - 1 - i;
  }

  // Search
  size_type i = pos;
  while (i <= text_len - pat_len) {
    size_type j = pat_len - 1;
    while (j != npos && pat[j] == text[i + j])
      --j;
    if (j == npos)
      return i;
    i += skip[static_cast<unsigned char>(text[i + pat_len - 1])];
  }

  return npos;
}

ustring::size_type ustring::find(const ustring &str, size_type pos) const noexcept
{
  return to_view().find(str, pos);
}

ustring::size_type ustring::view::find(const value_type *s, size_type pos, size_type n) const
{
  if (!s || pos >= _size)
    return npos;
  return find(ustring(s, n), pos);
}

ustring::size_type ustring::find(const value_type *s, size_type pos, size_type n) const
{
  return to_view().find(s, pos, n);
}

ustring::size_type ustring::view::find(const value_type *s, size_type pos) const
{
  if (!s || pos >= _size)
    return npos;
  return find(s, pos, strlen(reinterpret_cast<const char *>(s)));
}

ustring::size_type ustring::find(const value_type *s, size_type pos) const
{
  return to_view().find(s, pos);
}

ustring::size_type ustring::view::find(value_type c, size_type pos) const noexcept
{
  if (pos >= _size)
    return npos;

  const value_type *p = data() + pos;
  const value_type *const end = data() + _size;
  while (p != end) {
    if (*p == c)
      return p - data();
    ++p;
  }
  return npos;
}

ustring::size_type ustring::find(value_type c, size_type pos) const noexcept
{
  return to_view().find(c, pos);
}

ustring::size_type ustring::view::rfind(const ustring &str, size_type pos) const noexcept
{
  if (str.empty())
    return std::min(pos == npos ? max_pos : pos, _size);
  if (str._size > _size)
    return npos;

  pos = std::min(pos == npos ? max_pos : pos, _size - str._size);
  const value_type *const data_ptr = data();
  for (auto i = pos + 1; i-- > 0;) {
    if (std::equal(str.data(), str.data() + str._size, data_ptr + i)) {
      return i;
    }
  }
  return npos;
}

ustring::size_type ustring::rfind(const ustring &str, size_type pos) const noexcept
{
  return to_view().rfind(str, pos);
}

ustring::size_type ustring::view::rfind(const value_type *s, size_type pos, size_type n) const
{
  if (!s)
    return npos;
  return rfind(ustring(s, n), pos);
}

ustring::size_type ustring::rfind(const value_type *s, size_type pos, size_type n) const
{
  return to_view().rfind(s, pos, n);
}

ustring::size_type ustring::view::rfind(const value_type *s, size_type pos) const
{
  if (!s)
    return npos;
  return rfind(s, pos, strlen(reinterpret_cast<const char *>(s)));
}

ustring::size_type ustring::rfind(const value_type *s, size_type pos) const
{
  return to_view().rfind(s, pos);
}

ustring::size_type ustring::view::rfind(value_type c, size_type pos) const noexcept
{
  if (_size == 0)
    return npos;

  pos = std::min(pos == npos ? max_pos : pos, _size - 1);
  const value_type *p = data() + pos;
  while (true) {
    if (*p == c)
      return p - data();
    if (p == data())
      break;
    --p;
  }
  return npos;
}

ustring::size_type ustring::rfind(value_type c, size_type pos) const noexcept
{
  return to_view().rfind(c, pos);
}

ustring::size_type ustring::view::find_first_of(const ustring &str, size_type pos) const noexcept
{
  if (pos >= _size || str.empty())
    return npos;

  const value_type *const chars = str.data();
  const size_type chars_len = str._size;
  const value_type *p = data() + pos;
  const value_type *const end = data() + _size;

  while (p != end) {
    for (size_type i = 0; i < chars_len; ++i) {
      if (*p == chars[i])
        return p - data();
    }
    ++p;
  }
  return npos;
}

ustring::size_type ustring::find_first_of(const ustring &str, size_type pos) const noexcept
{
  return to_view().find_first_of(str, pos);
}

ustring::size_type ustring::view::find_first_of(const value_type *s,
                                                size_type pos,
                                                size_type n) const
{
  if (!s || pos >= _size)
    return npos;
  return find_first_of(ustring(s, n), pos);
}

ustring::size_type ustring::find_first_of(const value_type *s, size_type pos, size_type n) const
{
  return to_view().find_first_of(s, pos, n);
}

ustring::size_type ustring::view::find_first_of(const value_type *s, size_type pos) const
{
  if (!s || pos >= _size)
    return npos;
  return find_first_of(s, pos, strlen(reinterpret_cast<const char *>(s)));
}

ustring::size_type ustring::find_first_of(const value_type *s, size_type pos) const
{
  return to_view().find_first_of(s, pos);
}

ustring::size_type ustring::view::find_first_of(value_type c, size_type pos) const noexcept
{
  return find(c, pos);
}

ustring::size_type ustring::find_first_of(value_type c, size_type pos) const noexcept
{
  return to_view().find_first_of(c, pos);
}

ustring::size_type ustring::view::find_last_of(const ustring &str, size_type pos) const noexcept
{
  if (str.empty() || _size == 0)
    return npos;

  pos = std::min(pos == npos ? max_pos : pos, _size - 1);
  const value_type *const chars = str.data();
  const size_type chars_len = str._size;
  const value_type *p = data() + pos;

  while (true) {
    for (size_type i = 0; i < chars_len; ++i) {
      if (*p == chars[i])
        return p - data();
    }
    if (p == data())
      break;
    --p;
  }
  return npos;
}

ustring::size_type ustring::find_last_of(const ustring &str, size_type pos) const noexcept
{
  return to_view().find_last_of(str, pos);
}

ustring::size_type ustring::view::find_last_of(const value_type *s,
                                               size_type pos,
                                               size_type n) const
{
  if (!s)
    return npos;
  return find_last_of(ustring(s, n), pos);
}

ustring::size_type ustring::find_last_of(const value_type *s, size_type pos, size_type n) const
{
  return to_view().find_last_of(s, pos, n);
}

ustring::size_type ustring::view::find_last_of(const value_type *s, size_type pos) const
{
  if (!s)
    return npos;
  return find_last_of(s, pos, strlen(reinterpret_cast<const char *>(s)));
}

ustring::size_type ustring::find_last_of(const value_type *s, size_type pos) const
{
  return to_view().find_last_of(s, pos);
}

ustring::size_type ustring::view::find_last_of(value_type c, size_type pos) const noexcept
{
  return rfind(c, pos);
}

ustring::size_type ustring::find_last_of(value_type c, size_type pos) const noexcept
{
  return to_view().find_last_of(c, pos);
}

ustring::size_type ustring::view::find_first_not_of(const ustring &str,
                                                    size_type pos) const noexcept
{
  if (pos >= _size)
    return npos;
  if (str.empty())
    return pos;

  const value_type *const chars = str.data();
  const size_type chars_len = str._size;
  const value_type *p = data() + pos;
  const value_type *const end = data() + _size;

  while (p != end) {
    bool found = false;
    for (size_type i = 0; i < chars_len && !found; ++i) {
      if (*p == chars[i])
        found = true;
    }
    if (!found)
      return p - data();
    ++p;
  }
  return npos;
}

ustring::size_type ustring::find_first_not_of(const ustring &str, size_type pos) const noexcept
{
  return to_view().find_first_not_of(str, pos);
}

ustring::size_type ustring::view::find_first_not_of(const value_type *s,
                                                    size_type pos,
                                                    size_type n) const
{
  if (!s)
    return pos;
  return find_first_not_of(ustring(s, n), pos);
}

ustring::size_type ustring::find_first_not_of(const value_type *s,
                                              size_type pos,
                                              size_type n) const
{
  return to_view().find_first_not_of(s, pos, n);
}

ustring::size_type ustring::view::find_first_not_of(const value_type *s, size_type pos) const
{
  if (!s)
    return pos;
  return find_first_not_of(s, pos, strlen(reinterpret_cast<const char *>(s)));
}

ustring::size_type ustring::find_first_not_of(const value_type *s, size_type pos) const
{
  return to_view().find_first_not_of(s, pos);
}

ustring::size_type ustring::view::find_first_not_of(value_type c, size_type pos) const noexcept
{
  if (pos >= _size)
    return npos;

  const value_type *p = data() + pos;
  const value_type *const end = data() + _size;
  while (p != end) {
    if (*p != c)
      return p - data();
    ++p;
  }
  return npos;
}

ustring::size_type ustring::find_first_not_of(value_type c, size_type pos) const noexcept
{
  return to_view().find_first_not_of(c, pos);
}

ustring::size_type ustring::view::find_last_not_of(const ustring &str,
                                                   size_type pos) const noexcept
{
  if (_size == 0)
    return npos;
  if (str.empty())
    return std::min(pos == npos ? max_pos : pos, _size - 1);

  pos = std::min(pos == npos ? max_pos : pos, _size - 1);
  const value_type *const chars = str.data();
  const size_type chars_len = str._size;
  const value_type *p = data() + pos;

  while (true) {
    bool found = false;
    for (size_type i = 0; i < chars_len && !found; ++i) {
      if (*p == chars[i])
        found = true;
    }
    if (!found)
      return p - data();
    if (p == data())
      break;
    --p;
  }
  return npos;
}

ustring::size_type ustring::find_last_not_of(const ustring &str, size_type pos) const noexcept
{
  return to_view().find_last_not_of(str, pos);
}

ustring::size_type ustring::view::find_last_not_of(const value_type *s,
                                                   size_type pos,
                                                   size_type n) const
{
  if (!s)
    return pos;
  return find_last_not_of(ustring(s, n), pos);
}

ustring::size_type ustring::find_last_not_of(const value_type *s, size_type pos, size_type n) const
{
  return to_view().find_last_not_of(s, pos, n);
}

ustring::size_type ustring::view::find_last_not_of(const value_type *s, size_type pos) const
{
  if (!s)
    return pos;
  return find_last_not_of(s, pos, strlen(reinterpret_cast<const char *>(s)));
}

ustring::size_type ustring::find_last_not_of(const value_type *s, size_type pos) const
{
  return to_view().find_last_not_of(s, pos);
}

ustring::size_type ustring::view::find_last_not_of(value_type c, size_type pos) const noexcept
{
  if (_size == 0)
    return npos;

  pos = std::min(pos == npos ? max_pos : pos, _size - 1);
  const value_type *p = data() + pos;
  while (true) {
    if (*p != c)
      return p - data();
    if (p == data())
      break;
    --p;
  }
  return npos;
}

ustring::size_type ustring::find_last_not_of(value_type c, size_type pos) const noexcept
{
  return to_view().find_last_not_of(c, pos);
}

size_t ustring::view::count(const ustring &str) const noexcept
{
  // todo:
  if (size() < str.size()) {
    return 0;
  }

  size_t count = 0;
  size_t pos = 0;
  while (pos < size() - str.size() + 1) {
    for (size_t i = 0; i < str.size(); i++) {
      if (data()[pos + i] != str.data()[i]) {
        break;
      }
      if (i == str.size() - 1) {
        count++;
      }
    }
    pos++;
  }

  return count;
}

size_t ustring::count(const ustring &str) const noexcept
{
  return to_view().count(str);
}

size_t ustring::view::count(const value_type *s) const
{
  if (empty()) {
    return 0;
  }

  size_t count = 0, str_len = strlen(reinterpret_cast<const char *>(s));
  size_t pos = 0;
  while (pos < size() - str_len + 1) {
    for (size_t i = 0; i < str_len; i++) {
      if (data()[pos + i] != s[i]) {
        break;
      }
      if (i == str_len - 1) {
        count++;
      }
    }
    pos++;
  }

  return count;
}

size_t ustring::count(const value_type *s) const
{
  return to_view().count(s);
}

size_t ustring::view::count(char32_t c) const
{
  size_t count = 0;
  for (const auto &it : code_points()) {
    if (it == c) {
      count++;
    }
  }

  return count;
}

size_t ustring::count(char32_t c) const
{
  return to_view().count(c);
}

size_t ustring::view::count(std::function<bool(char32_t)> f) const
{
  size_t count = 0;
  for (const auto &it : code_points()) {
    if (f(it)) {
      count++;
    }
  }

  return count;
}

size_t ustring::count(std::function<bool(char32_t)> f) const
{
  return to_view().count(f);
}

bool ustring::view::contains(const ustring &str) const noexcept
{
  return find(str) != npos;
}

bool ustring::contains(const ustring &str) const noexcept
{
  return to_view().contains(str);
}

bool ustring::view::contains(const value_type *s) const
{
  return find(s) != npos;
}

bool ustring::contains(const value_type *s) const
{
  return to_view().contains(s);
}

bool ustring::view::contains(char32_t c) const noexcept
{
  for (const auto &it : code_points()) {
    if (it == c) {
      return true;
    }
  }
  return false;
}

bool ustring::contains(char32_t c) const noexcept
{
  return to_view().contains(c);
}

bool ustring::view::contains(value_type c) const noexcept
{
  return find(c) != npos;
}

bool ustring::contains(value_type c) const noexcept
{
  return to_view().contains(c);
}

bool ustring::view::contains(std::function<bool(char32_t)> f) const
{
  for (const auto &it : code_points()) {
    if (f(it)) {
      return true;
    }
  }
  return false;
}

bool ustring::contains(std::function<bool(char32_t)> f) const
{
  return to_view().contains(f);
}

int ustring::view::compare(const ustring &str) const noexcept
{
  const size_type len = std::min(_size, str._size);
  int result = std::memcmp(data(), str.data(), len * sizeof(value_type));
  if (result != 0)
    return result;
  if (_size < str._size)
    return -1;
  if (_size > str._size)
    return 1;
  return 0;
}

int ustring::compare(const ustring &str) const noexcept
{
  return to_view().compare(str);
}

int ustring::view::compare(size_type pos1, size_type n1, const ustring &str) const
{
#ifdef _DEBUG
  if (pos1 > _size) {
    throw std::out_of_range("ustring::compare: position out of range");
  }
#endif
  return compare(pos1, n1, str.data(), str._size);
}

int ustring::compare(size_type pos1, size_type n1, const ustring &str) const
{
  return to_view().compare(pos1, n1, str);
}

int ustring::view::compare(
    size_type pos1, size_type n1, const ustring &str, size_type pos2, size_type n2) const
{
#ifdef _DEBUG
  if (pos1 > _size || pos2 > str._size) {
    throw std::out_of_range("ustring::compare: position out of range");
  }
#endif

  n1 = std::min(n1, _size - pos1);
  n2 = std::min(n2, str._size - pos2);
  const size_type len = std::min(n1, n2);
  int result = std::memcmp(data() + pos1, str.data() + pos2, len * sizeof(value_type));
  if (result != 0)
    return result;
  if (n1 < n2)
    return -1;
  if (n1 > n2)
    return 1;
  return 0;
}

int ustring::compare(
    size_type pos1, size_type n1, const ustring &str, size_type pos2, size_type n2) const
{
  return to_view().compare(pos1, n1, str, pos2, n2);
}

int ustring::view::compare(const value_type *s) const
{
  if (!s)
    return 1;
  const size_type s_len = strlen(reinterpret_cast<const char *>(s));
  return compare(0, _size, s, s_len);
}

int ustring::compare(const value_type *s) const
{
  return to_view().compare(s);
}

int ustring::view::compare(size_type pos1, size_type n1, const value_type *s) const
{
  if (!s)
    return 1;
  return compare(pos1, n1, s, strlen(reinterpret_cast<const char *>(s)));
}

int ustring::compare(size_type pos1, size_type n1, const value_type *s) const
{
  return to_view().compare(pos1, n1, s);
}

int ustring::view::compare(size_type pos1, size_type n1, const value_type *s, size_type n2) const
{
#ifdef _DEBUG
  if (pos1 > _size) {
    throw std::out_of_range("ustring::compare: position out of range");
  }
  if (!s) {
    throw std::invalid_argument("ustring::compare: null pointer");
  }
#endif

  if (!s)
    return 1;
  n1 = std::min(n1, _size - pos1);
  const size_type len = std::min(n1, n2);
  int result = std::memcmp(data() + pos1, s, len * sizeof(value_type));
  if (result != 0)
    return result;
  if (n1 < n2)
    return -1;
  if (n1 > n2)
    return 1;
  return 0;
}

int ustring::compare(size_type pos1, size_type n1, const value_type *s, size_type n2) const
{
  return to_view().compare(pos1, n1, s, n2);
}

// int ustring::compare(const ustring &other, collation_strength strength, const char *locale)
// const
//{
//   UErrorCode status = U_ZERO_ERROR;
//   UCollator *coll = ucol_open(locale, &status);
//   if (U_FAILURE(status)) {
//     return this->compare(other);
//   }
//
//   ucol_setStrength(coll, static_cast<UCollationStrength>(strength));
//
//   int result = ucol_strcoll(coll,
//                             reinterpret_cast<const UChar *>(data()),
//                             _size,
//                             reinterpret_cast<const UChar *>(other.data()),
//                             other._size);
//
//   ucol_close(coll);
//   return result;
// }

// bool ustring::equals(const ustring &other, collation_strength strength,
//                      const char *locale) const {
//   return compare(other, strength, locale) == 0;
// }
//
// ustring sort_key(const ustring &str, collation_strength strength,
//                  const char *locale) {
//   UErrorCode status = U_ZERO_ERROR;
//   UCollator *coll = ucol_open(locale, &status);
//   if (U_FAILURE(status)) {
//     return "";
//   }
//
//   ucol_setStrength(coll,
//                    static_cast<UCollationStrength>(static_cast<int>(strength)));
//
//   int32_t key_len =
//       ucol_getSortKey(coll, reinterpret_cast<const UChar *>(str.data()),
//                       str.size(), nullptr, 0);
//
//   if (key_len == 0) {
//     ucol_close(coll);
//     return "";
//   }
//
//   std::vector<uint8_t> key_buffer(key_len);
//   ucol_getSortKey(coll, reinterpret_cast<const UChar *>(key_buffer.data()),
//                   key_buffer.size(), key_buffer.data(), key_buffer.size());
//
//   ucol_close(coll);
//
//   return ustring(reinterpret_cast<const char *>(key_buffer.data()),
//                  key_buffer.size() - 1);
// }
//
// Operator overloads
bool ustring::view::operator==(const char *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(reinterpret_cast<const value_type *>(rhs)) == 0;
}

bool ustring::operator==(const char *rhs) const noexcept
{
  return to_view() == rhs;
}

std::strong_ordering ustring::view::operator<=>(const char *rhs) const noexcept
{
  if (!rhs)
    return _size == 0 ? std::strong_ordering::equal : std::strong_ordering::greater;
  return compare(reinterpret_cast<const value_type *>(rhs)) <=> 0;
}

std::strong_ordering ustring::operator<=>(const char *rhs) const noexcept
{
  return to_view() <=> rhs;
}

bool ustring::view::operator==(const char8_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(reinterpret_cast<const value_type *>(rhs)) == 0;
}

bool ustring::operator==(const char8_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(reinterpret_cast<const value_type *>(rhs)) == 0;
}

std::strong_ordering ustring::view::operator<=>(const char8_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0 ? std::strong_ordering::equal : std::strong_ordering::greater;
  return compare(reinterpret_cast<const value_type *>(rhs)) <=> 0;
}

std::strong_ordering ustring::operator<=>(const char8_t *rhs) const noexcept
{
  return to_view() <=> rhs;
}

bool ustring::view::operator==(const char16_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(ustring(rhs)) == 0;  // todo: need improve
}

bool ustring::operator==(const char16_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(ustring(rhs)) == 0;
}

std::strong_ordering ustring::view::operator<=>(const char16_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0 ? std::strong_ordering::equal : std::strong_ordering::greater;
  return compare(ustring(rhs)) <=> 0;
}

std::strong_ordering ustring::operator<=>(const char16_t *rhs) const noexcept
{
  return to_view() <=> rhs;
}

bool ustring::view::operator==(const char32_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(ustring(rhs)) == 0;
}

bool ustring::operator==(const char32_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0;
  return compare(ustring(rhs)) == 0;
}

std::strong_ordering ustring::view::operator<=>(const char32_t *rhs) const noexcept
{
  if (!rhs)
    return _size == 0 ? std::strong_ordering::equal : std::strong_ordering::greater;
  return compare(reinterpret_cast<const value_type *>(rhs)) <=> 0;
}

std::strong_ordering ustring::operator<=>(const char32_t *rhs) const noexcept
{
  return to_view() <=> rhs;
}

bool operator==(const ustring::view &lhs, const ustring::view &rhs) noexcept
{
  if (lhs.size() != rhs.size()) {
    return false;
  }
  auto lhs_size = lhs.size(), rhs_size = rhs.size();
  if (lhs_size == rhs_size) {
    return std::memcmp(lhs.data(), rhs.data(), lhs_size) == 0;
  }
  return false;
}

bool operator==(const ustring &lhs, const ustring &rhs) noexcept
{
  return static_cast<const ustring::view &>(lhs.to_view()) ==
         static_cast<const ustring::view &>(rhs.to_view());
}
std::strong_ordering operator<=>(const ustring::view &lhs, const ustring::view &rhs) noexcept
{
  size_t min_len = std::min(lhs.size(), rhs.size());

  int result = std::memcmp(lhs.data(), rhs.data(), min_len * sizeof(ustring::value_type));

  if (result != 0) {
    return result < 0 ? std::strong_ordering::less : std::strong_ordering::greater;
  }

  if (lhs.size() < rhs.size())
    return std::strong_ordering::less;
  if (lhs.size() > rhs.size())
    return std::strong_ordering::greater;
  return std::strong_ordering::equal;
}

std::strong_ordering operator<=>(const ustring &lhs, const ustring &rhs) noexcept
{
  return static_cast<const ustring::view &>(lhs.to_view()) <=>
         static_cast<const ustring::view &>(rhs.to_view());
}

ustring operator+(const ustring &lhs, const ustring &rhs)
{
  ustring result = lhs;
  result.append(rhs);
  return result;
}

ustring operator+(const ustring &lhs, ustring::value_type rhs)
{
  ustring result = lhs;
  result.append(rhs);
  return result;
}

ustring operator+(ustring::value_type lhs, const ustring &rhs)
{
  ustring result{lhs};
  result.append(rhs);
  return result;
}

ustring operator+(const ustring &lhs, const ustring::value_type *rhs)
{
  if (!rhs)
    return lhs;
  const auto rhs_len = strlen(reinterpret_cast<const char *>(rhs));
  ustring result = lhs;
  result.reserve(lhs._size + rhs_len);
  result.append(rhs);
  return result;
}

ustring operator+(const ustring::value_type *lhs, const ustring &rhs)
{
  if (!lhs)
    return rhs;
  const auto lhs_len = strlen(reinterpret_cast<const char *>(lhs));
  ustring result{lhs};
  result.reserve(lhs_len + rhs._size);
  result.append(rhs);
  return result;
}

bool ustring::is_alpha() const noexcept
{
  if (empty())
    return false;

  //  for (auto c : code_points()) {
  //    if (!u_isalpha(c))
  //      return false;
  //  }
  //
  //  return true;
  //}

  UErrorCode status = U_ZERO_ERROR;
  UChar32 c;
  int32_t i = 0;
  while (i < _size) {
    U8_NEXT(data(), i, _size, c);
    if (status != U_ZERO_ERROR || !u_isalpha(c))
      return false;
  }
  return true;
}

bool ustring::is_digit() const noexcept
{
  if (empty())
    return false;
  UErrorCode status = U_ZERO_ERROR;
  UChar32 c;
  int32_t i = 0;
  while (i < _size) {
    U8_NEXT(data(), i, _size, c);
    if (status != U_ZERO_ERROR || !u_isdigit(c))
      return false;
  }
  return true;
}

bool ustring::is_alnum() const noexcept
{
  if (empty())
    return false;
  UErrorCode status = U_ZERO_ERROR;
  UChar32 c;
  int32_t i = 0;
  while (i < _size) {
    U8_NEXT(data(), i, _size, c);
    if (status != U_ZERO_ERROR || !u_isalnum(c))
      return false;
  }
  return true;
}

bool ustring::is_space() const noexcept
{
  if (empty())
    return false;
  UErrorCode status = U_ZERO_ERROR;
  UChar32 c;
  int32_t i = 0;
  while (i < _size) {
    U8_NEXT(data(), i, _size, c);
    if (status != U_ZERO_ERROR || !u_isspace(c))
      return false;
  }
  return true;
}

bool ustring::is_lower() const noexcept
{
  if (empty())
    return false;
  UErrorCode status = U_ZERO_ERROR;
  UChar32 c;
  int32_t i = 0;
  while (i < _size) {
    U8_NEXT(data(), i, _size, c);
    if (status != U_ZERO_ERROR || !u_islower(c))
      return false;
  }
  return true;
}

bool ustring::is_upper() const noexcept
{
  if (empty())
    return false;
  UErrorCode status = U_ZERO_ERROR;
  UChar32 c;
  int32_t i = 0;
  while (i < _size) {
    U8_NEXT(data(), i, _size, c);
    if (status != U_ZERO_ERROR || !u_isupper(c))
      return false;
  }
  return true;
}

bool ustring::is_title() const noexcept
{
  if (empty())
    return false;

  auto icu_str = icu::UnicodeString::fromUTF8({(const char *)data(), (int32_t)size()});
  if (icu_str.isBogus())
    return false;

  UErrorCode status = U_ZERO_ERROR;
  UBreakIterator *bi = ubrk_open(
      UBRK_WORD, nullptr, icu_str.getBuffer(), icu_str.length(), &status);
  if (U_FAILURE(status))
    return false;

  int32_t start = ubrk_first(bi);
  int32_t end = ubrk_next(bi);
  bool is_title = true;

  while (end != UBRK_DONE) {
    if (ubrk_getRuleStatus(bi) == UBRK_WORD_NONE) {
      start = end;
      end = ubrk_next(bi);
      continue;
    }

    UChar32 c = icu_str.char32At(start);
    if (!u_istitle(c) && !u_isupper(c)) {
      is_title = false;
      break;
    }
    for (auto i = start + 1; i < end; i++) {
      c = icu_str.char32At(i);
      if (u_istitle(c) || u_isupper(c)) {
        is_title = false;
        break;
      }
    }

    start = end;
    end = ubrk_next(bi);
  }

  ubrk_close(bi);
  return is_title;
}

bool ustring::is_normalized(const NormalizationConfig &config) const
{
  const char *data_file = _get_normalization_data_file(config);

  UErrorCode status = U_ZERO_ERROR;
  const UNormalizer2 *normalizer = unorm2_getInstance(
      nullptr, data_file, static_cast<UNormalization2Mode>(config.mode), &status);

  if (U_FAILURE(status)) {
    return false;
  }

  std::u16string utf16_data = to_u16string();
  UBool result = unorm2_isNormalized(normalizer, utf16_data.data(), utf16_data.size(), &status);

  return U_SUCCESS(status) && result;
}

ustring &ustring::filter(std::function<bool(char32_t, size_type)> &&codepoint_filter)
{
  if (empty())
    return *this;

  UErrorCode status = U_ZERO_ERROR;

  auto src = data();
  int32_t srcLength = _size;
  int32_t i = 0, dest_i = 0;
  size_type count = 0;
  UChar32 c;

  while (i < srcLength) {
    U8_NEXT(src, i, srcLength, c);

    if (codepoint_filter(c, count) && i != dest_i) {
      U8_APPEND_UNSAFE(src, dest_i, c);
    }

    count++;
  }

  _size = dest_i;
  return *this;
}

ustring &ustring::transform(std::function<char32_t(char32_t, size_type)> &&codepoint_transformer)
{
  if (empty())
    return *this;

  UErrorCode status = U_ZERO_ERROR;
  auto utf8Buffer = new char8_t[_size * 4];

  const char8_t *src = data();
  int32_t srcLength = _size;
  int32_t i = 0, dest_i = 0;
  size_t count = 0;
  UChar32 c;

  while (i < srcLength) {
    U8_NEXT(src, i, srcLength, c);
    if (c < 0)
      break;  // Invalid UTF-8 sequence
    c = codepoint_transformer(c, count);

    U8_APPEND_UNSAFE(utf8Buffer, dest_i, c);
    count++;
  }
  // #ifdef _DEBUG
  //   if (dest_i < _size * 4 - 1) {
  //     utf8Buffer[dest_i] = '\0';
  //   }
  // #endif

  if (U_SUCCESS(status)) {
    assign(utf8Buffer, dest_i);
  }

  delete[] utf8Buffer;
  return *this;
}

ustring &ustring::to_lower(bool any_lower)
{
  if (!any_lower)
    return transform(_to_lower);

  icu::ErrorCode icu_status;
  ustring out;
  out.resize(size() * 2);
  auto real_size = icu::CaseMap::utf8ToLower("" /* root locale */,
                                             0 /* default options */,
                                             reinterpret_cast<const char *>(data()),
                                             size(),
                                             reinterpret_cast<char *>(out.data()),
                                             out.size(),
                                             nullptr /* edits - unused */,
                                             icu_status);
  out.resize(real_size);
  if (icu_status.isFailure()) {
    std::cout << icu_status.errorName();
    icu_status.reset();
    return *this;
  }

  return *this = std::move(out);
}

ustring &ustring::to_upper(bool any_upper)
{
  if (!any_upper)
    return transform(_to_upper);

  icu::ErrorCode icu_status;
  ustring out;
  out.resize(size() * 2);
  auto real_size = icu::CaseMap::utf8ToUpper("" /* root locale */,
                                             0 /* default options */,
                                             reinterpret_cast<const char *>(data()),
                                             size(),
                                             reinterpret_cast<char *>(out.data()),
                                             out.size(),
                                             nullptr /* edits - unused */,
                                             icu_status);
  out.resize(real_size);
  if (icu_status.isFailure()) {
    std::cout << icu_status.errorName();
    icu_status.reset();
    return *this;
  }
  return *this = std::move(out);
}

ustring &ustring::capitalize(const char *locale)
{
  if (empty())
    return *this;

  icu::Locale icuLocale(locale ? locale : std::locale().name().c_str());

  UErrorCode status = U_ZERO_ERROR;
  icu::BreakIterator *bi = icu::BreakIterator::createSentenceInstance(icuLocale, status);
  if (U_FAILURE(status) && status != U_USING_DEFAULT_WARNING)
    return *this;

  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  bi->setText(ustr);

  int32_t start = bi->first();
  int32_t end = bi->next();
  while (end != icu::BreakIterator::DONE) {
    if (start < ustr.length()) {
      ustr.setCharAt(start, u_toupper(ustr.char32At(start)));
      for (int32_t i = start + 1; i < end; ++i) {
        ustr.setCharAt(i, u_tolower(ustr.char32At(i)));
      }
    }
    start = end;
    end = bi->next();
  }

  std::string utf8;
  ustr.toUTF8String(utf8);
  assign(reinterpret_cast<const char8_t *>(utf8.c_str()), utf8.length());

  delete bi;
  return *this;
}

ustring &ustring::swap_case()
{
  return transform(_swap_case);
}

ustring &ustring::title(const char *locale, ToTitleOptions options)
{
  icu::ErrorCode icu_status;
  ustring out;
  out.resize(size() * 2);
  auto real_size = icu::CaseMap::utf8ToTitle("",
                                             static_cast<uint32_t>(options),
                                             nullptr,
                                             reinterpret_cast<const char *>(data()),
                                             size(),
                                             reinterpret_cast<char *>(out.data()),
                                             out.capacity(),
                                             nullptr,
                                             icu_status);
  out.resize(real_size);
  if (icu_status.isFailure()) {
    std::cout << icu_status.errorName();
    icu_status.reset();
    return *this;
  }

  return *this = std::move(out);
}

ustring &ustring::trim()
{
  UErrorCode status = U_ZERO_ERROR;
  auto unicodeStr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  unicodeStr.trim();

  std::string utf8TitleStr;
  unicodeStr.toUTF8String(utf8TitleStr);

  assign(reinterpret_cast<const value_type *>(utf8TitleStr.data()), utf8TitleStr.size());

  return *this;
}

ustring &ustring::strip(const value_type *ch)
{
  if (empty() || !ch)
    return *this;

  std::unordered_set<UChar32> chars;

  const char8_t *p = ch;
  int32_t i = 0;
  while (*(p + i)) {
    UChar32 c;
    U8_NEXT(p, i, -1, c);
    if (c < 0)
      break;
    chars.insert(c);
  }

  const char8_t *src = data();
  int32_t srcLength = _size;
  int32_t start = 0, end = srcLength;
  UChar32 c;

  while (start < srcLength) {
    auto old_start = start;
    U8_NEXT(src, start, srcLength, c);
    if (c < 0 || !chars.contains(c)) {
      start = old_start;
      break;
    }
  }

  while (end > start) {
    auto old_end = end;
    U8_PREV(src, 0, end, c);
    if (c < 0 || !chars.contains(c)) {
      end = old_end;
      break;
    }
  }

  if (start > 0 || end < srcLength) {
    assign(src + start, end - start);
  }

  return *this;
}

ustring &ustring::strip(const ustring &ch)
{
  if (empty() || !ch.size())
    return *this;

  std::unordered_set<UChar32> chars;

  const char8_t *p = ch.data();
  int32_t i = 0;
  while (i < size()) {
    UChar32 c;
    U8_NEXT(p, i, -1, c);
    if (c < 0)
      break;
    chars.insert(c);
  }

  const char8_t *src = data();
  int32_t srcLength = _size;
  int32_t start = 0, end = srcLength;
  UChar32 c;

  while (start < srcLength) {
    U8_NEXT(src, start, srcLength, c);
    if (c < 0 || !chars.contains(c))
      break;
  }

  while (end > start) {
    U8_PREV(src, 0, end, c);
    if (c < 0 || !chars.contains(c))
      break;
  }

  if (start > 0 || end < srcLength) {
    assign(src + start, end - start);
  }

  return *this;
}

ustring &ustring::normalize(const NormalizationConfig &config)
{
  const char *data_file = _get_normalization_data_file(config);

  UErrorCode status = U_ZERO_ERROR;
  const UNormalizer2 *normalizer = unorm2_getInstance(
      nullptr, data_file, static_cast<UNormalization2Mode>(config.mode), &status);

  if (U_FAILURE(status)) {
    return *this;
  }

  std::u16string utf16_data = to_u16string();
  std::vector<UChar> buffer(utf16_data.size() * 2);
  int32_t length = unorm2_normalize(
      normalizer, utf16_data.data(), utf16_data.size(), buffer.data(), buffer.size(), &status);

  if (status == U_BUFFER_OVERFLOW_ERROR) {
    status = U_ZERO_ERROR;
    buffer.resize(length);
    length = unorm2_normalize(
        normalizer, utf16_data.data(), utf16_data.size(), buffer.data(), buffer.size(), &status);
  }

  if (U_SUCCESS(status)) {
    *this = from_utf16(buffer.data(), length);
  }
  return *this;
}

ustring ustring::filtered(std::function<bool(char32_t, size_type)> &&codepoint_filter) const
{
  ustring ret{*this};
  ret.filter(std::forward<std::function<bool(char32_t, size_type)>>(codepoint_filter));
  return ret;
}

ustring ustring::transformed(
    std::function<char32_t(char32_t, size_type)> &&codepoint_transformer) const
{
  ustring ret{*this};
  ret.transform(std::forward<std::function<char32_t(char32_t, size_type)>>(codepoint_transformer));
  return ret;
}

ustring ustring::lowered(bool any_lower) const
{
  ustring ret{*this};
  ret.to_lower(any_lower);
  return ret;
}

ustring ustring::uppered(bool any_upper) const
{
  ustring ret{*this};
  ret.to_upper(any_upper);
  return ret;
}

ustring ustring::capitalized() const
{
  ustring ret{*this};
  ret.capitalize();
  return ret;
}

ustring ustring::case_swapped() const
{
  ustring ret{*this};
  ret.swap_case();
  return ret;
}

ustring ustring::trimmed() const
{
  ustring ret{*this};
  ret.trim();
  return ret;
}

ustring ustring::titled(const char *locale, ToTitleOptions options) const
{
  ustring ret{*this};
  ret.title(locale, options);
  return ret;
}

ustring ustring::stripped(const value_type *ch) const
{
  ustring ret{*this};
  ret.strip(ch);
  return ret;
}

ustring ustring::normalized(const NormalizationConfig &config) const
{
  ustring ret{*this};
  ret.normalize(config);
  return ret;
}

ustring &ustring::to_halfwidth()
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  auto trans = std::unique_ptr<icu::Transliterator>(
      icu::Transliterator::createInstance("Fullwidth-Halfwidth", UTRANS_FORWARD, status));
  if (U_SUCCESS(status)) {
    trans->transliterate(ustr);
    std::string utf8;
    ustr.toUTF8String(utf8);
    assign(reinterpret_cast<const value_type *>(utf8.data()), utf8.length());
  }
  return *this;
}

ustring &ustring::to_fullwidth()
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  auto trans = std::unique_ptr<icu::Transliterator>(
      icu::Transliterator::createInstance("Halfwidth-Fullwidth", UTRANS_FORWARD, status));
  if (U_SUCCESS(status)) {
    trans->transliterate(ustr);
    std::string utf8;
    ustr.toUTF8String(utf8);
    assign(reinterpret_cast<const value_type *>(utf8.data()), utf8.length());
  }
  return *this;
}

ustring &ustring::normalize_whitespace(bool including_zero_width)
{
  if (empty())
    return *this;

  UErrorCode status = U_ZERO_ERROR;

  auto src = data();
  int32_t srcLength = _size;
  int32_t i = 0, dest_i = 0;
  UChar32 c = 0, last_c;

  bool last_is_space = false;
  bool last_c_is_space = false, c_is_space = false;
  while (i < srcLength) {
    last_c = c;
    last_c_is_space = c_is_space;
    U8_NEXT(src, i, srcLength, c);

    c_is_space = u_isspace(c) ||
                 (including_zero_width ? c == U'\u200B' || c == U'\u200C' || c == U'\u200D' :
                                         false);
    if (c_is_space) {
      if (!last_c || last_c_is_space) {
        continue;
      }
      U8_APPEND_UNSAFE(src, dest_i, ' ');
      last_is_space = true;
    }
    else {
      U8_APPEND_UNSAFE(src, dest_i, c);
      last_is_space = false;
    }
  }

  _size = last_is_space ? dest_i - 1 : dest_i /* minus last space */;
  return *this;
}

ustring &ustring::normalize_quotes()
{
  return transform([](char32_t c, size_type) {
    switch (c) {
      case U'『':
      case U'』':
      case U'«':
      case U'»':
      case U'“':
      case U'”':
      case U'〝':
      case U'〞':
      case U'„':
        return U'"';
      case U'「':
      case U'」':
      case U'﹁':  // should this be ""?
      case U'﹂':
      case U'‹':
      case U'›':
      case U'‘':
      case U'’':
      case U'‛':
        return U'\'';
      default:
        return c;
    }
  });
}

ustring &ustring::normalize_dashes()
{
  return transform([](char32_t c, size_type) {
    switch (c) {
      case U'‒':
      case U'–':
      case U'—':
      case U'―':
      case U'-':
      case U'‐':
      case U'‑':
      case U'⁃':
      case U'⸺':
      case U'⸻':
        return U'-';
      default:
        return c;
    }
  });
}

ustring &ustring::simplify()
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  auto trans = std::unique_ptr<icu::Transliterator>(
      icu::Transliterator::createInstance("Traditional-Simplified", UTRANS_FORWARD, status));
  if (U_SUCCESS(status)) {
    trans->transliterate(ustr);
    std::string utf8;
    ustr.toUTF8String(utf8);
    assign(reinterpret_cast<const value_type *>(utf8.data()), utf8.length());
  }
  return *this;
}

ustring &ustring::traditionalize()
{
  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  auto trans = std::unique_ptr<icu::Transliterator>(
      icu::Transliterator::createInstance("Simplified-Traditional", UTRANS_FORWARD, status));
  if (U_SUCCESS(status)) {
    trans->transliterate(ustr);
    std::string utf8;
    ustr.toUTF8String(utf8);
    assign(reinterpret_cast<const value_type *>(utf8.data()), utf8.length());
  }
  return *this;
}

ustring ustring::sort() const
{
  // todo:
  return *this;
}

ustring ustring::unique() const
{
  if (empty())
    return *this;

  ustring ret(*this);

  UErrorCode status = U_ZERO_ERROR;

  auto src = ret.data();
  int32_t srcLength = _size;
  int32_t i = 0, dest_i = 0;
  UChar32 c;

  std::unordered_set<UChar32> seen;

  while (i < srcLength) {
    U8_NEXT(src, i, srcLength, c);

    if (seen.insert(c).second && i != dest_i) {
      U8_APPEND_UNSAFE(src, dest_i, c);
    }
  }

  ret._size = dest_i;
  return *this;
}

std::vector<ustring::view> ustring::view::split(char32_t delimiter) const
{
  return split(std::unordered_set<char32_t>{delimiter});
}

std::vector<ustring::view> ustring::split(char32_t delimiters) const
{
  return to_view().split(delimiters);
}

std::vector<ustring::view> ustring::view::split(std::unordered_set<char32_t> &&delimiter) const
{
  std::vector<view> result;

  UErrorCode status = U_ZERO_ERROR;
  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));

  icu::StringCharacterIterator iter(ustr);
  int32_t start = 0;
  UChar32 c;
  while ((c = iter.next32()) != icu::StringCharacterIterator::DONE) {
    if (delimiter.contains(c)) {
      int32_t end = iter.getIndex() - 1;
      result.emplace_back(data() + start, end - start);
      start = iter.getIndex();
    }
  }
  if (start < ustr.length()) {
    result.emplace_back(data() + start, ustr.length() - start);
  }

  return result;
}

std::vector<ustring::view> ustring::split(std::unordered_set<char32_t> &&delimiter) const
{
  return to_view().split(std::move(delimiter));
}

std::vector<ustring::view> ustring::view::split(const ustring &delimiter) const
{
  if (delimiter.empty()) {
    return {};
  }

  std::vector<view> result;

  icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
  icu::UnicodeString delimStr = icu::UnicodeString::fromUTF8(
      icu::StringPiece(reinterpret_cast<const char *>(delimiter.data()), delimiter.size()));

  int32_t start = 0;
  int32_t pos;
  while ((pos = ustr.indexOf(delimStr, start)) != -1) {
    result.emplace_back(data() + start, pos - start);
    start = pos + delimStr.length();
  }
  if (start < ustr.length()) {
    result.emplace_back(data() + start, ustr.length() - start);
  }

  return result;
}

std::vector<ustring::view> ustring::split(const ustring &delimiter) const
{
  return to_view().split(delimiter);
}

std::vector<ustring::view> ustring::view::split_words(const char *locale) const
{
  if (!locale) {
    return {};
  }

  std::vector<view> result;

  UErrorCode status = U_ZERO_ERROR;
  icu::BreakIterator *bi = icu::BreakIterator::createWordInstance(
      icu::Locale::getUS() /* todo: other according to locale */, status);
  if (U_SUCCESS(status)) {
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(
        icu::StringPiece(reinterpret_cast<const char *>(data()), size()));
    bi->setText(ustr);
    int32_t start = bi->first();
    for (int32_t end = bi->next(); end != icu::BreakIterator::DONE; start = end, end = bi->next())
    {
      result.emplace_back(data() + start, end - start);
    }
  }

  return result;
}

std::vector<ustring::view> ustring::split_words(const char *locale) const
{
  return to_view().split_words(locale);
}

ustring::word_iterator ustring::words_begin() const
{
  return word_iterator(*this, 0);
}

ustring::word_iterator ustring::words_end() const
{
  return word_iterator(*this, size());
}

ustring::code_point_iterator::code_point_iterator() : _data{nullptr}, _size{0}, _codepoint{0} {}

ustring::code_point_iterator::code_point_iterator(const ustring &str, size_type pos)
    : _data(str.data() + pos), _size{0}, _codepoint{0}
{
#ifdef _DEBUG
  _owner = &str;
#endif
  if (pos < str.size()) {
    U8_NEXT_UNSAFE(_data, _size, _codepoint);
  }
  if (pos >= str.size()) {
    _data = str.data() + str.size();
  }
  _end = str.data() + str.size();
  _start = str.data();
}

ustring::code_point_iterator::code_point_iterator(const ustring::code_point_iterator &other)
    : _data(other._data),
      _size(other._size),
      _codepoint(other._codepoint),
      _start(other._start),
      _end(other._end),
#ifdef _DEBUG
      _owner(other._owner)
#endif
{
}

ustring::code_point_iterator::code_point_iterator(ustring::code_point_iterator &&other) noexcept
    : _data(other._data),
      _size(other._size),
      _codepoint(other._codepoint),
      _start(other._start),
      _end(other._end),
#ifdef _DEBUG
      _owner(other._owner)
#endif
{
  other._data = nullptr;
  other._size = 0;
}

ustring::code_point_iterator &ustring::code_point_iterator::operator=(
    const ustring::code_point_iterator &other)
{
  _data = other._data;
  _size = other._size;
  _codepoint = other._codepoint;
  _start = other._start;
  _end = other._end;
#ifdef _DEBUG
  _owner = other._owner;
#endif
  return *this;
}

ustring::code_point_iterator &ustring::code_point_iterator::operator=(
    ustring::code_point_iterator &&other) noexcept
{
  _data = other._data;
  _size = other._size;
  _codepoint = other._codepoint;
  _start = other._start;
  _end = other._end;
#ifdef _DEBUG
  _owner = other._owner;
#endif
  other._data = other._start = other._end = nullptr;
  other._size = 0;
  return *this;
}

ustring::code_point_iterator &ustring::code_point_iterator::operator++()
{
#ifdef _DEBUG
  assert(_data < _owner->end());
#endif
  _data += _size;
  _size = 0;
  if (_data < _end) {
    U8_NEXT(_data, _size, -1, _codepoint);
  }
  else {
    _codepoint = 0;
  }
  return *this;
}

ustring::code_point_iterator &ustring::code_point_iterator::operator++(int)
{
  return ++*this;
}

ustring::code_point_iterator &ustring::code_point_iterator::operator+=(size_t step)
{
  while (step-- > 0) {
    _data += _size;
    _size = 0;
    if (_data < _end) {
      U8_NEXT(_data, _size, -1, _codepoint);
    }
    else {
      _codepoint = 0;
      break;
    }
  }

  return *this;
}

ustring::code_point_iterator ustring::code_point_iterator::operator+(size_t step) const
{
  code_point_iterator tmp(*this);
  return tmp += step;
}

ustring::code_point_iterator &ustring::code_point_iterator::operator--()
{
  _size = 0;
  if (_data > _start) {
    int32_t diff = _data - _start, oldDiff = diff;
    U8_PREV(_start, 0, diff, _codepoint);
    _size = oldDiff - diff;
    _data -= _size;
  }
  else {
    _codepoint = 0;
  }
  return *this;
}

ustring::code_point_iterator ustring::code_point_iterator::operator-=(size_t step)
{
  while (step-- > 0) {
    _size = 0;
    if (_data > _start) {
      int32_t diff = _data - _start, oldDiff = diff;
      U8_PREV(_start, 0, diff, _codepoint);
      _size = oldDiff - diff;
      _data -= _size;
    }
    else {
      _codepoint = 0;
      break;
    }
  }
  return *this;
}

ustring::code_point_iterator ustring::code_point_iterator::operator-(size_t step) const
{
  code_point_iterator tmp(*this);
  return tmp -= step;
}

ustring::grapheme_iterator::grapheme_iterator(const ustring &str,
                                              size_type pos,
                                              const char *locale)
    : _view(str.data() + pos, 0),
      _end(str.data() + str.size()),
      _start(str.data()),
      _break_iterator(nullptr),
      _text(nullptr)
{
#ifdef _DEBUG
  _owner = &str;
#endif
  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_open(UBRK_CHARACTER, locale, nullptr, 0, &status);
  if (!U_SUCCESS(status)) {
    throw "Failed to create grapheme iterator";
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    throw "Failed to open utf-8 string";
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (!U_SUCCESS(status)) {
    utext_close(text_it);
    ubrk_close(break_it);
    throw "Failed to set utext";
  }

  auto curr_pos = _view.data() - _start;
  curr_pos = curr_pos > 0 ? ubrk_following(break_it, curr_pos) : ubrk_first(break_it);

  if (curr_pos == UBRK_DONE) {
    _view = {_end, 0};
    return;
  }

  _view = {_start + curr_pos, 0};
  int32_t next = ubrk_next(break_it);
  if (next != UBRK_DONE) {
    _view = {_view.data(), static_cast<size_type>(next - curr_pos)};
  }
}

ustring::grapheme_iterator::grapheme_iterator(const grapheme_iterator &other)
    : _view(other._view),
      _end(other._end),
      _start(other._start),
      _break_iterator(nullptr),
      _text(nullptr)
{
#ifdef _DEBUG
  _owner = other._owner;
#endif
  if (!other._break_iterator) {
    return;
  }

  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_safeClone(
      static_cast<UBreakIterator *>(other._break_iterator), nullptr, nullptr, &status);
  if (!U_SUCCESS(status)) {
    return;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    _break_iterator = nullptr;
    return;
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (U_SUCCESS(status)) {
    int32_t pos = std::max(0, static_cast<int32_t>(_view.data() - _start));
    ubrk_following(break_it, pos);
  }
}

ustring::grapheme_iterator::~grapheme_iterator()
{
  if (_text) {
    utext_close(static_cast<UText *>(_text));
  }
  if (_break_iterator) {
    ubrk_close(static_cast<UBreakIterator *>(_break_iterator));
  }
}

ustring::grapheme_iterator &ustring::grapheme_iterator::operator=(const grapheme_iterator &other)
{
  if (this == &other) {
    return *this;
  }

  _view = other._view;
  _end = other._end;
  _start = other._start;
#ifdef _DEBUG
  _owner = other._owner;
#endif

  if (_text) {
    utext_close(static_cast<UText *>(_text));
    _text = nullptr;
  }
  if (_break_iterator) {
    ubrk_close(static_cast<UBreakIterator *>(_break_iterator));
    _break_iterator = nullptr;
  }

  if (!other._break_iterator) {
    return *this;
  }

  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_safeClone(
      static_cast<UBreakIterator *>(other._break_iterator), nullptr, nullptr, &status);
  if (!U_SUCCESS(status)) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    _break_iterator = nullptr;
    return *this;
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (U_SUCCESS(status)) {
    int32_t pos = std::max(0, static_cast<int32_t>(_view.data() - _start));
    ubrk_following(break_it, pos);
  }
  return *this;
}

ustring::grapheme_iterator &ustring::grapheme_iterator::operator++()
{
  if (is_end() || !_break_iterator) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);

  int32_t curr_pos = _view.data() - _start + _view.size();
  int32_t next = ubrk_following(break_it, curr_pos);
  if (next == UBRK_DONE) {
    _view = {_end, 0};
    return *this;
  }
  _view = {_start + curr_pos, (next - curr_pos)};
  return *this;
}

ustring::grapheme_iterator &ustring::grapheme_iterator::operator--()
{
  if (!_break_iterator || _view.data() == _start) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);

  // If we're at the end, we need to find the last boundary
  if (is_end()) {
    int32_t last = ubrk_last(break_it);
    if (last == UBRK_DONE) {
      return *this;
    }
    int32_t prev = ubrk_previous(break_it);
    if (prev != UBRK_DONE) {
      _view = {_start + prev, static_cast<size_type>(last - prev)};
    }
    else {
      _view = {_start, static_cast<size_type>(last)};
    }
    return *this;
  }

  // Get current position and find previous boundary
  int32_t curr_pos = _view.data() - _start;
  ubrk_isBoundary(break_it, curr_pos);  // Set iterator to current position
  int32_t prev = ubrk_previous(break_it);
  if (prev == UBRK_DONE) {
    return *this;
  }

  int32_t prev2 = ubrk_previous(break_it);
  if (prev2 != UBRK_DONE) {
    _view = {_start + prev2, static_cast<size_type>(prev - prev2)};
    ubrk_next(break_it);  // Restore iterator position
  }
  else {
    _view = {_start, static_cast<size_type>(prev)};
  }

  return *this;
}

ustring::grapheme_iterator ustring::grapheme_iterator::operator++(int)
{
  grapheme_iterator tmp(*this);
  ++*this;
  return tmp;
}

ustring::grapheme_iterator ustring::grapheme_iterator::operator--(int)
{
  grapheme_iterator tmp(*this);
  --*this;
  return tmp;
}

ustring::grapheme_iterator::const_reference ustring::grapheme_iterator::operator*() const
{
  return _view;
}

ustring::grapheme_iterator::reference ustring::grapheme_iterator::operator*()
{
  return _view;
}

ustring::grapheme_iterator::const_pointer ustring::grapheme_iterator::operator->() const
{
  return &_view;
}

ustring::grapheme_iterator::pointer ustring::grapheme_iterator::operator->()
{
  return &_view;
}

bool ustring::grapheme_iterator::operator==(const grapheme_iterator &other) const
{
  return _view.data() == other._view.data();
}

bool ustring::grapheme_iterator::operator!=(const grapheme_iterator &other) const
{
  return !(*this == other);
}

bool ustring::grapheme_iterator::is_end() const
{
  assert(_view.data() <= _end);
  return _view.data() == _end;
}

// Helper functions implementation
ustring::grapheme_iterator ustring::graphemes_begin() const
{
  return grapheme_iterator(*this, 0);
}

ustring::grapheme_iterator ustring::graphemes_end() const
{
  return grapheme_iterator(*this, size());
}

std::pair<ustring::grapheme_iterator, ustring::grapheme_iterator> ustring::graphemes() const
{
  return {graphemes_begin(), graphemes_end()};
}

ustring::word_iterator::word_iterator(const ustring &str,
                                      size_type pos,
                                      const char *locale,
                                      WordBreak break_type)
    : _view(str.data() + pos, 0),
      _end(str.data() + str.size()),
      _start(str.data()),
      _break_iterator(nullptr),
      _text(nullptr)
{
#ifdef _DEBUG
  _owner = &str;
#endif
  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_open(UBRK_WORD, locale, nullptr, 0, &status);
  if (!U_SUCCESS(status)) {
    throw "Failed to create word iterator";
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    throw "Failed to open utf-8 string";
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (!U_SUCCESS(status)) {
    utext_close(text_it);
    ubrk_close(break_it);
    throw "Failed to set utext";
  }

  auto curr_pos = _view.data() - _start;
  curr_pos = curr_pos > 0 ? ubrk_following(break_it, curr_pos) : ubrk_first(break_it);

  if (curr_pos == UBRK_DONE) {
    _view = {_end, 0};
    return;
  }

  _view = {_start + curr_pos, 0};
  int32_t next = ubrk_next(break_it);
  if (next != UBRK_DONE) {
    _view = {_view.data(), static_cast<size_type>(next - curr_pos)};
  }
}

ustring::word_iterator::~word_iterator()
{
  if (_text) {
    utext_close(static_cast<UText *>(_text));
  }
  if (_break_iterator) {
    ubrk_close(static_cast<UBreakIterator *>(_break_iterator));
  }
}

ustring::word_iterator::word_iterator(const word_iterator &other)
    : _view(other._view),
      _end(other._end),
      _start(other._start),
      _break_iterator(nullptr),
      _text(nullptr)
{
#ifdef _DEBUG
  _owner = other._owner;
#endif
  if (!other._break_iterator) {
    return;
  }

  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_safeClone(
      static_cast<UBreakIterator *>(other._break_iterator), nullptr, nullptr, &status);
  if (!U_SUCCESS(status)) {
    return;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    _break_iterator = nullptr;
    return;
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (U_SUCCESS(status)) {
    int32_t pos = std::max(0, static_cast<int32_t>(_view.data() - _start));
    ubrk_following(break_it, pos);
  }
}

ustring::word_iterator &ustring::word_iterator::operator=(const word_iterator &other)
{
  if (this == &other) {
    return *this;
  }

  _view = other._view;
  _end = other._end;
  _start = other._start;
#ifdef _DEBUG
  _owner = other._owner;
#endif

  if (_text) {
    utext_close(static_cast<UText *>(_text));
    _text = nullptr;
  }
  if (_break_iterator) {
    ubrk_close(static_cast<UBreakIterator *>(_break_iterator));
    _break_iterator = nullptr;
  }

  if (!other._break_iterator) {
    return *this;
  }

  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_safeClone(
      static_cast<UBreakIterator *>(other._break_iterator), nullptr, nullptr, &status);
  if (!U_SUCCESS(status)) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    _break_iterator = nullptr;
    return *this;
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (U_SUCCESS(status)) {
    int32_t pos = std::max(0, static_cast<int32_t>(_view.data() - _start));
    ubrk_following(break_it, pos);
  }
  return *this;
}

ustring::word_iterator &ustring::word_iterator::operator++()
{
  if (is_end() || !_break_iterator) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);

  int32_t curr_pos = _view.data() - _start + _view.size();
  int32_t next = ubrk_next(break_it);
  if (next != UBRK_DONE) {
    _view = {_start + curr_pos, static_cast<size_type>(next - curr_pos)};
    // ubrk_previous(break_it);  // Restore iterator position
  }
  else {
    _view = {_end, 0};
  }

  return *this;
}

ustring::word_iterator ustring::word_iterator::operator++(int)
{
  word_iterator tmp(*this);
  ++*this;
  return tmp;
}

ustring::word_iterator::const_reference ustring::word_iterator::operator*() const
{
  return _view;
}

ustring::word_iterator::reference ustring::word_iterator::operator*()
{
  return _view;
}

ustring::word_iterator::const_pointer ustring::word_iterator::operator->() const
{
  return &_view;
}

ustring::word_iterator::pointer ustring::word_iterator::operator->()
{
  return &_view;
}

bool ustring::word_iterator::operator==(const word_iterator &other) const
{
  return _view.data() == other._view.data();
}

bool ustring::word_iterator::operator!=(const word_iterator &other) const
{
  return !(*this == other);
}

ustring::size_type ustring::word_iterator::word_length() const
{
  if (is_end() || _view.empty()) {
    return 0;
  }

  size_type count = 0;
  const char *p = reinterpret_cast<const char *>(_view.data());
  const char *end = p + _view.size();
  while (p < end) {
    UChar32 c;
    auto i = p - reinterpret_cast<const char *>(_start);
    U8_NEXT(p, i, end - p, c);
    if (c >= 0) {
      count++;
    }
  }
  return count;
}

bool ustring::word_iterator::is_end() const
{
  assert(_view.data() <= _end);
  return _view.data() == _end;
}

ustring::word_iterator &ustring::word_iterator::operator--()
{
  if (!_break_iterator || _view.data() == _start) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);

  // If we're at the end, we need to find the last boundary
  if (is_end()) {
    int32_t last = ubrk_last(break_it);
    if (last == UBRK_DONE) {
      return *this;
    }
    int32_t prev = ubrk_previous(break_it);
    if (prev != UBRK_DONE) {
      _view = {_start + prev, static_cast<size_type>(last - prev)};
    }
    else {
      _view = {_start, static_cast<size_type>(last)};
    }
    return *this;
  }

  // Get current position and find previous boundary
  int32_t curr_pos = _view.data() - _start;
  ubrk_isBoundary(break_it, curr_pos);  // Set iterator to current position
  int32_t prev = ubrk_previous(break_it);
  if (prev == UBRK_DONE) {
    return *this;
  }

  int32_t prev2 = ubrk_previous(break_it);
  if (prev2 != UBRK_DONE) {
    _view = {_start + prev2, static_cast<size_type>(prev - prev2)};
    ubrk_next(break_it);  // Restore iterator position
  }
  else {
    _view = {_start, static_cast<size_type>(prev)};
  }

  return *this;
}

ustring::word_iterator ustring::word_iterator::operator--(int)
{
  word_iterator tmp(*this);
  --*this;
  return tmp;
}

ustring::sentence_iterator::sentence_iterator(const ustring &str,
                                              size_type pos,
                                              const char *locale)
    : _view(str.data() + pos, 0),
      _end(str.data() + str.size()),
      _start(str.data()),
      _break_iterator(nullptr),
      _text(nullptr)
{
#ifdef _DEBUG
  _owner = &str;
#endif
  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_open(UBRK_SENTENCE, locale, nullptr, 0, &status);
  if (!U_SUCCESS(status)) {
    throw "Failed to create sentence iterator";
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    throw "Failed to open utf-8 string";
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (!U_SUCCESS(status)) {
    utext_close(text_it);
    ubrk_close(break_it);
    throw "Failed to set utext";
  }

  auto curr_pos = _view.data() - _start;
  curr_pos = curr_pos > 0 ? ubrk_following(break_it, curr_pos) : ubrk_first(break_it);

  if (curr_pos == UBRK_DONE) {
    _view = {_end, 0};
    return;
  }

  _view = {_start + curr_pos, 0};
  int32_t next = ubrk_following(break_it, curr_pos);
  if (next != UBRK_DONE) {
    _view = {_view.data(), static_cast<size_type>(next - curr_pos)};
  }
}

ustring::sentence_iterator::sentence_iterator(const sentence_iterator &other)
    : _view(other._view),
      _end(other._end),
      _start(other._start),
      _break_iterator(nullptr),
      _text(nullptr)
{
#ifdef _DEBUG
  _owner = other._owner;
#endif
  if (!other._break_iterator) {
    return;
  }

  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_safeClone(
      static_cast<UBreakIterator *>(other._break_iterator), nullptr, nullptr, &status);
  if (!U_SUCCESS(status)) {
    return;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    _break_iterator = nullptr;
    return;
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (U_SUCCESS(status)) {
    int32_t pos = std::max(0, static_cast<int32_t>(_view.data() - _start));
    ubrk_following(break_it, pos);
  }
}

ustring::sentence_iterator::~sentence_iterator()
{
  if (_text) {
    utext_close(static_cast<UText *>(_text));
  }
  if (_break_iterator) {
    ubrk_close(static_cast<UBreakIterator *>(_break_iterator));
  }
}

ustring::sentence_iterator &ustring::sentence_iterator::operator=(const sentence_iterator &other)
{
  if (this == &other) {
    return *this;
  }

  _view = other._view;
  _end = other._end;
  _start = other._start;
#ifdef _DEBUG
  _owner = other._owner;
#endif

  if (_text) {
    utext_close(static_cast<UText *>(_text));
    _text = nullptr;
  }
  if (_break_iterator) {
    ubrk_close(static_cast<UBreakIterator *>(_break_iterator));
    _break_iterator = nullptr;
  }

  if (!other._break_iterator) {
    return *this;
  }

  UErrorCode status = U_ZERO_ERROR;
  _break_iterator = ubrk_safeClone(
      static_cast<UBreakIterator *>(other._break_iterator), nullptr, nullptr, &status);
  if (!U_SUCCESS(status)) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);
  status = U_ZERO_ERROR;
  _text = utext_openUTF8(nullptr, reinterpret_cast<const char *>(_start), _end - _start, &status);
  if (!U_SUCCESS(status)) {
    ubrk_close(break_it);
    _break_iterator = nullptr;
    return *this;
  }

  auto text_it = static_cast<UText *>(_text);
  status = U_ZERO_ERROR;
  ubrk_setUText(break_it, text_it, &status);
  if (U_SUCCESS(status)) {
    int32_t pos = std::max(0, static_cast<int32_t>(_view.data() - _start));
    ubrk_following(break_it, pos);
  }
  return *this;
}

ustring::sentence_iterator &ustring::sentence_iterator::operator++()
{
  if (is_end() || !_break_iterator) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);

  int32_t curr_pos = _view.data() - _start + _view.size();
  int32_t next = ubrk_following(break_it, curr_pos);
  if (next == UBRK_DONE) {
    _view = {_end, 0};
    return *this;
  }
  _view = {_start + curr_pos, (next - curr_pos)};
  return *this;
}

ustring::sentence_iterator &ustring::sentence_iterator::operator--()
{
  if (!_break_iterator || _view.data() == _start) {
    return *this;
  }

  auto break_it = static_cast<UBreakIterator *>(_break_iterator);

  // If we're at the end, we need to find the last boundary
  if (is_end()) {
    int32_t last = ubrk_last(break_it);
    if (last == UBRK_DONE) {
      return *this;
    }
    int32_t prev = ubrk_previous(break_it);
    if (prev != UBRK_DONE) {
      _view = {_start + prev, static_cast<size_type>(last - prev)};
    }
    else {
      _view = {_start, static_cast<size_type>(last)};
    }
    return *this;
  }

  // Get current position and find previous boundary
  int32_t curr_pos = _view.data() - _start;
  ubrk_isBoundary(break_it, curr_pos);  // Set iterator to current position
  int32_t prev = ubrk_previous(break_it);
  if (prev == UBRK_DONE) {
    return *this;
  }

  int32_t prev2 = ubrk_previous(break_it);
  if (prev2 != UBRK_DONE) {
    _view = {_start + prev2, static_cast<size_type>(prev - prev2)};
    ubrk_next(break_it);  // Restore iterator position
  }
  else {
    _view = {_start, static_cast<size_type>(prev)};
  }

  return *this;
}

ustring::sentence_iterator ustring::sentence_iterator::operator++(int)
{
  sentence_iterator tmp(*this);
  ++*this;
  return tmp;
}

ustring::sentence_iterator ustring::sentence_iterator::operator--(int)
{
  sentence_iterator tmp(*this);
  --*this;
  return tmp;
}

ustring::sentence_iterator::const_reference ustring::sentence_iterator::operator*() const
{
  return _view;
}

ustring::sentence_iterator::reference ustring::sentence_iterator::operator*()
{
  return _view;
}

ustring::sentence_iterator::const_pointer ustring::sentence_iterator::operator->() const
{
  return &_view;
}

ustring::sentence_iterator::pointer ustring::sentence_iterator::operator->()
{
  return &_view;
}

bool ustring::sentence_iterator::operator==(const sentence_iterator &other) const
{
  return _view.data() == other._view.data();
}

bool ustring::sentence_iterator::operator!=(const sentence_iterator &other) const
{
  return !(*this == other);
}

bool ustring::sentence_iterator::is_end() const
{
  assert(_view.data() <= _end);
  return _view.data() == _end;
}

ustring::sentence_iterator ustring::sentences_begin() const
{
  return sentence_iterator(*this, 0);
}

ustring::sentence_iterator ustring::sentences_end() const
{
  return sentence_iterator(*this, size());
}

std::pair<ustring::sentence_iterator, ustring::sentence_iterator> ustring::sentences() const
{
  return {sentences_begin(), sentences_end()};
}
