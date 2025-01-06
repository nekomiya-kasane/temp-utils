#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <locale>
#include <chrono>
#include <cstdint>

namespace locale {

// Locale category flags
enum class category : unsigned {
    none        = 0,
    collate     = 1 << 0,
    ctype       = 1 << 1,
    monetary    = 1 << 2,
    numeric     = 1 << 3,
    time        = 1 << 4,
    messages    = 1 << 5,
    all         = collate | ctype | monetary | numeric | time | messages
};

// Script types
enum class script {
    unknown,
    latin,
    han,
    hiragana,
    katakana,
    hangul,
    arabic,
    devanagari,
    thai,
    cyrillic,
    greek,
    hebrew,
    bengali,
    gujarati,
    gurmukhi,
    kannada,
    malayalam,
    oriya,
    tamil,
    telugu,
    tibetan,
    myanmar,
    georgian,
    ethiopic,
    cherokee,
    canadian_aboriginal,
    mongolian,
    khmer
};

// Calendar types
enum class calendar_type {
    gregorian,
    buddhist,
    chinese,
    hebrew,
    islamic,
    islamic_civil,
    japanese,
    persian,
    indian,
    coptic,
    ethiopic
};

// Date format style
enum class date_style {
    full,       // "Tuesday, April 12, 1952 AD"
    long_,      // "April 12, 1952"
    medium,     // "Apr 12, 1952"
    short_      // "04/12/52"
};

// Time format style
enum class time_style {
    full,       // "10:10:10 Pacific Daylight Time"
    long_,      // "10:10:10 PDT"
    medium,     // "10:10:10"
    short_      // "10:10"
};

// Number format style
enum class number_style {
    standard,   // 1,234.56
    scientific, // 1.23456E3
    spellout,   // one thousand two hundred thirty-four point five six
    ordinal,    // 1,234th
    currency,   // $1,234.56
    percent     // 123,456%
};

// String normalization form
enum class normalize_form {
    nfc,  // Canonical Decomposition followed by Canonical Composition
    nfd,  // Canonical Decomposition
    nfkc, // Compatibility Decomposition followed by Canonical Composition
    nfkd  // Compatibility Decomposition
};

// Text break iterator type
enum class break_type {
    character,  // Character boundaries
    word,       // Word boundaries
    line,       // Line break boundaries
    sentence,   // Sentence boundaries
    title       // Title-casing boundaries
};

// Word type returned by break iterator
enum class word_type {
    unknown,
    none,
    number,
    letter,
    kana,
    ideo,       // Ideographic
    emoji
};

// Locale stack management
void push_locale(const char* locale_name, category cat = category::all);
void pop_locale();
[[nodiscard]] int get_locale_stack_depth() noexcept;

// Core locale functions
void set_default_locale(const char* locale_name);
[[nodiscard]] const char* get_current_locale_name() noexcept;
[[nodiscard]] const char* get_system_locale_name() noexcept;
[[nodiscard]] bool is_valid_locale(const char* locale_name) noexcept;

// Locale information
[[nodiscard]] const char* get_language(const char* locale_name = nullptr);
[[nodiscard]] const char* get_script(const char* locale_name = nullptr);
[[nodiscard]] const char* get_country(const char* locale_name = nullptr);
[[nodiscard]] const char* get_variant(const char* locale_name = nullptr);
[[nodiscard]] const char* get_name(const char* locale_name = nullptr);
[[nodiscard]] std::string get_display_name(const char* locale_name = nullptr,
                                         const char* display_locale = nullptr);
[[nodiscard]] std::string get_display_language(const char* locale_name = nullptr,
                                             const char* display_locale = nullptr);
[[nodiscard]] std::string get_display_script(const char* locale_name = nullptr,
                                           const char* display_locale = nullptr);
[[nodiscard]] std::string get_display_country(const char* locale_name = nullptr,
                                            const char* display_locale = nullptr);
[[nodiscard]] std::string get_display_variant(const char* locale_name = nullptr,
                                            const char* display_locale = nullptr);

// Locale properties
[[nodiscard]] bool is_right_to_left(const char* locale_name = nullptr) noexcept;
[[nodiscard]] bool has_script(const char* locale_name = nullptr) noexcept;
[[nodiscard]] script get_script_type(const char* locale_name = nullptr) noexcept;
[[nodiscard]] const char* get_exemplar_characters(const char* locale_name = nullptr);

// Character type information
[[nodiscard]] bool is_letter(char32_t c) noexcept;
[[nodiscard]] bool is_digit(char32_t c) noexcept;
[[nodiscard]] bool is_whitespace(char32_t c) noexcept;
[[nodiscard]] bool is_punctuation(char32_t c) noexcept;
[[nodiscard]] bool is_symbol(char32_t c) noexcept;
[[nodiscard]] bool is_control(char32_t c) noexcept;
[[nodiscard]] bool is_upper(char32_t c) noexcept;
[[nodiscard]] bool is_lower(char32_t c) noexcept;
[[nodiscard]] bool is_titlecase(char32_t c) noexcept;
[[nodiscard]] bool is_defined(char32_t c) noexcept;
[[nodiscard]] bool is_emoji(char32_t c) noexcept;
[[nodiscard]] bool is_ideographic(char32_t c) noexcept;
[[nodiscard]] script get_script(char32_t c) noexcept;
[[nodiscard]] int get_numeric_value(char32_t c) noexcept;

// Case conversion
[[nodiscard]] char32_t to_upper(char32_t c) noexcept;
[[nodiscard]] char32_t to_lower(char32_t c) noexcept;
[[nodiscard]] char32_t to_title(char32_t c) noexcept;
[[nodiscard]] char32_t to_fold(char32_t c) noexcept;  // Case folding

// String case conversion
[[nodiscard]] std::u32string to_upper(std::u32string_view str);
[[nodiscard]] std::u32string to_lower(std::u32string_view str);
[[nodiscard]] std::u32string to_title(std::u32string_view str);
[[nodiscard]] std::u32string to_fold(std::u32string_view str);
[[nodiscard]] std::u32string normalize(std::u32string_view str, normalize_form form);

// String break iteration
std::vector<size_t> find_breaks(std::u32string_view str, break_type type);
std::vector<std::u32string_view> split(std::u32string_view str, break_type type);
[[nodiscard]] word_type get_word_type(std::u32string_view word) noexcept;

// Collation support
[[nodiscard]] int compare(std::string_view str1, std::string_view str2,
                         bool ignore_case = false, bool ignore_accents = false);
[[nodiscard]] bool is_greater(std::string_view str1, std::string_view str2,
                            bool ignore_case = false, bool ignore_accents = false);
[[nodiscard]] bool is_less(std::string_view str1, std::string_view str2,
                          bool ignore_case = false, bool ignore_accents = false);
[[nodiscard]] uint32_t hash(std::string_view str, bool ignore_case = false);
[[nodiscard]] std::string sort_key(std::string_view str);

// Number formatting
[[nodiscard]] std::string format_number(double number, number_style style = number_style::standard);
[[nodiscard]] std::string format_integer(int64_t number, number_style style = number_style::standard);
[[nodiscard]] std::string format_currency(double amount, const char* currency_code);
[[nodiscard]] std::string format_percent(double value);
[[nodiscard]] bool parse_number(const char* str, double& result);
[[nodiscard]] bool parse_currency(const char* str, double& amount, std::string& currency);
[[nodiscard]] const char* get_decimal_separator();
[[nodiscard]] const char* get_grouping_separator();
[[nodiscard]] const char* get_currency_symbol(const char* currency_code = nullptr);
[[nodiscard]] int get_fraction_digits(const char* currency_code = nullptr);

// Date and time formatting
[[nodiscard]] std::string format_date(const std::chrono::system_clock::time_point& time,
                                    date_style style = date_style::medium);
[[nodiscard]] std::string format_time(const std::chrono::system_clock::time_point& time,
                                    time_style style = time_style::medium);
[[nodiscard]] std::string format_datetime(const std::chrono::system_clock::time_point& time,
                                        date_style date_style = date_style::medium,
                                        time_style time_style = time_style::medium);
[[nodiscard]] bool parse_date(const char* str, std::chrono::system_clock::time_point& time);
[[nodiscard]] bool parse_time(const char* str, std::chrono::system_clock::time_point& time);
[[nodiscard]] bool parse_datetime(const char* str, std::chrono::system_clock::time_point& time);

// Calendar support
[[nodiscard]] calendar_type get_calendar_type(const char* locale_name = nullptr);
[[nodiscard]] std::vector<const char*> get_month_names(bool abbreviated = false);
[[nodiscard]] std::vector<const char*> get_day_names(bool abbreviated = false);
[[nodiscard]] std::vector<const char*> get_quarter_names(bool abbreviated = false);
[[nodiscard]] std::vector<const char*> get_era_names(bool abbreviated = false);
[[nodiscard]] std::vector<const char*> get_am_pm_markers();
[[nodiscard]] const char* get_first_day_of_week();
[[nodiscard]] int get_minimal_days_in_first_week();
[[nodiscard]] bool is_weekend(const std::chrono::system_clock::time_point& time);

// Time zone support
void set_time_zone(const char* timezone_id);
[[nodiscard]] const char* get_time_zone_id();
[[nodiscard]] int get_time_zone_offset(const char* timezone_id = nullptr);
[[nodiscard]] bool is_daylight_time(const char* timezone_id = nullptr);
[[nodiscard]] std::string get_time_zone_display_name(const char* timezone_id = nullptr,
                                                    bool daylight = false,
                                                    bool abbreviated = true);

// Global utility functions
[[nodiscard]] std::vector<const char*> get_available_locales();
[[nodiscard]] std::vector<const char*> get_available_time_zones();
[[nodiscard]] std::vector<const char*> get_available_calendars(const char* locale_name = nullptr);
[[nodiscard]] std::vector<const char*> get_available_currencies();
[[nodiscard]] bool is_valid_time_zone(const char* timezone_id);
[[nodiscard]] bool is_valid_currency(const char* currency_code);
[[nodiscard]] const char* get_current_time_zone();

// Error handling
[[nodiscard]] const char* get_last_error() noexcept;
void clear_error() noexcept;

} // namespace locale

inline locale::category operator|(locale::category a, locale::category b) {
    return static_cast<locale::category>(
        static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline locale::category operator&(locale::category a, locale::category b) {
    return static_cast<locale::category>(
        static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

