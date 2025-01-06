#include "locale.h"
#include <unicode/unistr.h>
#include <unicode/brkiter.h>
#include <unicode/ucnv.h>
#include <unicode/udat.h>
#include <unicode/ucal.h>
#include <unicode/unum.h>
#include <unicode/unumsys.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>
#include <unicode/uclean.h>
#include <chrono>
#include <stdexcept>
#include <algorithm>

namespace locale {

namespace {
    // Helper function to handle ICU errors
    void check_icu_status(UErrorCode status, const char* operation) {
        if (U_FAILURE(status)) {
            throw std::runtime_error(std::string(operation) + " failed: " + u_errorName(status));
        }
    }

    // Convert ICU UnicodeString to std::string
    std::string to_std_string(const icu::UnicodeString& ustr) {
        std::string result;
        ustr.toUTF8String(result);
        return result;
    }

    // Thread-local storage for current timezone
    thread_local std::string current_timezone;
}

// Core locale functions
icu::Locale create(const char* locale_name) {
    if (!locale_name) return icu::Locale::getDefault();
    return icu::Locale(locale_name);
}

icu::Locale create_from_language(const char* language, const char* country, const char* variant) {
    return icu::Locale(language, country, variant);
}

icu::Locale create_from_bcp47(const char* bcp47_tag) {
    UErrorCode status = U_ZERO_ERROR;
    char locale_id[ULOC_FULLNAME_CAPACITY];
    uloc_forLanguageTag(bcp47_tag, locale_id, ULOC_FULLNAME_CAPACITY, nullptr, &status);
    check_icu_status(status, "Create locale from BCP47");
    return icu::Locale(locale_id);
}

std::vector<icu::Locale> get_available_locales() {
    std::vector<icu::Locale> locales;
    int32_t count;
    const icu::Locale* icu_locales = icu::Locale::getAvailableLocales(count);
    locales.reserve(count);
    for (int32_t i = 0; i < count; ++i) {
        locales.emplace_back(icu_locales[i]);
    }
    return locales;
}

icu::Locale get_default() noexcept {
    return icu::Locale::getDefault();
}

icu::Locale get_system() noexcept {
    return icu::Locale(uloc_getDefault());
}

// Locale information
std::string get_language(const icu::Locale& loc) {
    return loc.getLanguage();
}

std::string get_script(const icu::Locale& loc) {
    return loc.getScript();
}

std::string get_country(const icu::Locale& loc) {
    return loc.getCountry();
}

std::string get_variant(const icu::Locale& loc) {
    return loc.getVariant();
}

std::string get_name(const icu::Locale& loc) {
    return loc.getName();
}

std::string get_display_name(const icu::Locale& loc, const icu::Locale& display_locale) {
    icu::UnicodeString result;
    loc.getDisplayName(display_locale, result);
    return to_std_string(result);
}

std::string get_display_language(const icu::Locale& loc, const icu::Locale& display_locale) {
    icu::UnicodeString result;
    loc.getDisplayLanguage(display_locale, result);
    return to_std_string(result);
}

std::string get_display_script(const icu::Locale& loc, const icu::Locale& display_locale) {
    icu::UnicodeString result;
    loc.getDisplayScript(display_locale, result);
    return to_std_string(result);
}

std::string get_display_country(const icu::Locale& loc, const icu::Locale& display_locale) {
    icu::UnicodeString result;
    loc.getDisplayCountry(display_locale, result);
    return to_std_string(result);
}

std::string get_display_variant(const icu::Locale& loc, const icu::Locale& display_locale) {
    icu::UnicodeString result;
    loc.getDisplayVariant(display_locale, result);
    return to_std_string(result);
}

std::string get_bcp47_tag(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    char tag[ULOC_FULLNAME_CAPACITY];
    uloc_toLanguageTag(loc.getName(), tag, ULOC_FULLNAME_CAPACITY, TRUE, &status);
    check_icu_status(status, "Get BCP47 tag");
    return tag;
}

// Locale properties
bool is_right_to_left(const icu::Locale& loc) noexcept {
    UErrorCode status = U_ZERO_ERROR;
    ULayoutType layout = uloc_getCharacterOrientation(loc.getName(), &status);
    return U_SUCCESS(status) && layout == ULOC_LAYOUT_RTL;
}

bool has_script(const icu::Locale& loc) noexcept {
    return *loc.getScript() != '\0';
}

// Character type information
bool is_letter(char32_t c) noexcept {
    return u_isalpha(c);
}

bool is_digit(char32_t c) noexcept {
    return u_isdigit(c);
}

bool is_whitespace(char32_t c) noexcept {
    return u_isspace(c);
}

bool is_punctuation(char32_t c) noexcept {
    return u_ispunct(c);
}

bool is_upper(char32_t c) noexcept {
    return u_isupper(c);
}

bool is_lower(char32_t c) noexcept {
    return u_islower(c);
}

bool is_titlecase(char32_t c) noexcept {
    return u_istitle(c);
}

// Case conversion
char32_t to_upper(char32_t c) noexcept {
    return u_toupper(c);
}

char32_t to_lower(char32_t c) noexcept {
    return u_tolower(c);
}

char32_t to_title(char32_t c) noexcept {
    return u_totitle(c);
}

// Collation support
std::unique_ptr<icu::Collator> create_collator(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::Collator> collator(icu::Collator::createInstance(loc, status));
    check_icu_status(status, "Create collator");
    return collator;
}

int compare(const icu::Locale& loc, const std::string& str1, const std::string& str2) {
    auto collator = create_collator(loc);
    icu::UnicodeString ustr1 = icu::UnicodeString::fromUTF8(str1);
    icu::UnicodeString ustr2 = icu::UnicodeString::fromUTF8(str2);
    UErrorCode status = U_ZERO_ERROR;
    return collator->compare(ustr1, ustr2, status);
}

bool is_greater_than(const icu::Locale& loc, const std::string& str1, const std::string& str2) {
    return compare(loc, str1, str2) > 0;
}

bool is_less_than(const icu::Locale& loc, const std::string& str1, const std::string& str2) {
    return compare(loc, str1, str2) < 0;
}

// Number formatting
std::string format_number(const icu::Locale& loc, double number) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::NumberFormat> fmt(icu::NumberFormat::createInstance(loc, status));
    check_icu_status(status, "Create number format");
    icu::UnicodeString result;
    fmt->format(number, result);
    return to_std_string(result);
}

std::string format_currency(const icu::Locale& loc, double amount, const char* currency_code) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::NumberFormat> fmt(
        icu::NumberFormat::createCurrencyInstance(loc, status));
    check_icu_status(status, "Create currency format");
    icu::UnicodeString result;
    fmt->format(amount, result);
    return to_std_string(result);
}

std::string format_percent(const icu::Locale& loc, double value) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::NumberFormat> fmt(
        icu::NumberFormat::createPercentInstance(loc, status));
    check_icu_status(status, "Create percent format");
    icu::UnicodeString result;
    fmt->format(value, result);
    return to_std_string(result);
}

bool parse_number(const icu::Locale& loc, const std::string& str, double& result) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::NumberFormat> fmt(icu::NumberFormat::createInstance(loc, status));
    if (U_FAILURE(status)) return false;
    
    icu::UnicodeString ustr = icu::UnicodeString::fromUTF8(str);
    icu::Formattable formattable;
    icu::ParsePosition pos(0);
    fmt->parse(ustr, formattable, pos);
    
    if (pos.getIndex() == 0) return false;
    result = formattable.getDouble(status);
    return U_SUCCESS(status);
}

std::string get_decimal_separator(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DecimalFormatSymbols> symbols(
        new icu::DecimalFormatSymbols(loc, status));
    check_icu_status(status, "Get decimal format symbols");
    return to_std_string(icu::UnicodeString(symbols->getSymbol(
        icu::DecimalFormatSymbols::kDecimalSeparatorSymbol)));
}

std::string get_grouping_separator(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DecimalFormatSymbols> symbols(
        new icu::DecimalFormatSymbols(loc, status));
    check_icu_status(status, "Get decimal format symbols");
    return to_std_string(icu::UnicodeString(symbols->getSymbol(
        icu::DecimalFormatSymbols::kGroupingSeparatorSymbol)));
}

// Date and time formatting
std::string format_date(const icu::Locale& loc,
                       const std::chrono::system_clock::time_point& time,
                       const std::string& pattern) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DateFormat> fmt(
        icu::DateFormat::createDateInstance(icu::DateFormat::kDefault, loc));
    check_icu_status(status, "Create date format");
    
    auto tt = std::chrono::system_clock::to_time_t(time);
    icu::UnicodeString result;
    fmt->format((UDate)(tt * 1000.0), result);
    return to_std_string(result);
}

std::string format_time(const icu::Locale& loc,
                       const std::chrono::system_clock::time_point& time,
                       const std::string& pattern) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DateFormat> fmt(
        icu::DateFormat::createTimeInstance(icu::DateFormat::kDefault, loc));
    check_icu_status(status, "Create time format");
    
    auto tt = std::chrono::system_clock::to_time_t(time);
    icu::UnicodeString result;
    fmt->format((UDate)(tt * 1000.0), result);
    return to_std_string(result);
}

std::string format_datetime(const icu::Locale& loc,
                          const std::chrono::system_clock::time_point& time,
                          const std::string& pattern) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DateFormat> fmt(
        icu::DateFormat::createDateTimeInstance(icu::DateFormat::kDefault,
                                              icu::DateFormat::kDefault,
                                              loc));
    check_icu_status(status, "Create datetime format");
    
    auto tt = std::chrono::system_clock::to_time_t(time);
    icu::UnicodeString result;
    fmt->format((UDate)(tt * 1000.0), result);
    return to_std_string(result);
}

// Calendar support
std::unique_ptr<icu::Calendar> create_calendar(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::Calendar> calendar(icu::Calendar::createInstance(loc, status));
    check_icu_status(status, "Create calendar");
    return calendar;
}

std::string get_calendar_type(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::Calendar> calendar(icu::Calendar::createInstance(loc, status));
    if (U_FAILURE(status)) return "unknown";
    return calendar->getType();
}

std::vector<std::string> get_month_names(const icu::Locale& loc, bool abbreviated) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DateFormat> fmt(
        icu::DateFormat::createDateInstance(icu::DateFormat::kDefault, loc));
    std::vector<std::string> months;
    months.reserve(12);

    icu::UnicodeString month;
    for (int i = 0; i < 12; ++i) {
        month.remove();
        fmt->getMonths(month, abbreviated ? UDATPG_ABBREVIATED : UDATPG_WIDE, i);
        months.push_back(to_std_string(month));
    }
    return months;
}

std::vector<std::string> get_day_names(const icu::Locale& loc, bool abbreviated) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::DateFormat> fmt(
        icu::DateFormat::createDateInstance(icu::DateFormat::kDefault, loc));
    std::vector<std::string> days;
    days.reserve(7);

    icu::UnicodeString day;
    for (int i = 1; i <= 7; ++i) {
        day.remove();
        fmt->getWeekdays(day, abbreviated ? UDATPG_ABBREVIATED : UDATPG_WIDE, i);
        days.push_back(to_std_string(day));
    }
    return days;
}

std::string get_first_day_of_week(const icu::Locale& loc) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::Calendar> calendar(icu::Calendar::createInstance(loc, status));
    if (U_FAILURE(status)) return "unknown";
    return get_day_names(loc)[calendar->getFirstDayOfWeek() - 1];
}

// Time zone support
void set_time_zone(const std::string& timezone_id) {
    if (!is_valid_time_zone(timezone_id)) {
        throw std::invalid_argument("Invalid timezone ID");
    }
    current_timezone = timezone_id;
}

std::string get_time_zone_id() {
    if (current_timezone.empty()) {
        icu::UnicodeString id;
        icu::TimeZone::createDefault()->getID(id);
        return to_std_string(id);
    }
    return current_timezone;
}

int get_time_zone_offset(const std::string& timezone_id) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::TimeZone> tz(
        icu::TimeZone::createTimeZone(icu::UnicodeString::fromUTF8(timezone_id)));
    int32_t offset;
    tz->getOffset(icu::Calendar::getNow(), FALSE, offset, status);
    if (U_FAILURE(status)) return 0;
    return offset / 1000 / 60; // Convert from milliseconds to minutes
}

bool is_daylight_time(const std::string& timezone_id) {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::TimeZone> tz(
        icu::TimeZone::createTimeZone(icu::UnicodeString::fromUTF8(timezone_id)));
    UBool daylight;
    int32_t offset;
    tz->getOffset(icu::Calendar::getNow(), FALSE, offset, daylight, status);
    return U_SUCCESS(status) && daylight;
}

// Global utility functions
std::vector<std::string> get_available_locale_names() {
    std::vector<std::string> names;
    int32_t count;
    const icu::Locale* icu_locales = icu::Locale::getAvailableLocales(count);
    names.reserve(count);
    for (int32_t i = 0; i < count; ++i) {
        names.push_back(icu_locales[i].getName());
    }
    return names;
}

std::vector<std::string> get_available_time_zones() {
    std::vector<std::string> zones;
    icu::StringEnumeration* ids = icu::TimeZone::createEnumeration();
    if (!ids) return zones;

    UErrorCode status = U_ZERO_ERROR;
    const char* id;
    while ((id = ids->next(nullptr, status)) != nullptr && U_SUCCESS(status)) {
        zones.push_back(id);
    }
    delete ids;
    return zones;
}

bool is_valid_locale_name(const std::string& locale_name) {
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale test(locale_name.c_str());
    return test.isBogus() == FALSE;
}

bool is_valid_time_zone(const std::string& timezone_id) {
    std::unique_ptr<icu::TimeZone> tz(
        icu::TimeZone::createTimeZone(icu::UnicodeString::fromUTF8(timezone_id)));
    return !tz->getID(icu::UnicodeString()).isEmpty();
}

std::string get_current_time_zone() {
    icu::UnicodeString id;
    icu::TimeZone::createDefault()->getID(id);
    std::string result;
    id.toUTF8String(result);
    return result;
}

} // namespace locale
