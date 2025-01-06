#include <gtest/gtest.h>
#include "locale.h"
#include <chrono>

using namespace ustr::locale;

TEST(LocaleTest, CreateLocale) {
    auto loc = create("en_US");
    EXPECT_EQ(get_language(loc), "en");
    EXPECT_EQ(get_country(loc), "US");
}

TEST(LocaleTest, CreateFromLanguage) {
    auto loc = create_from_language("zh", "CN");
    EXPECT_EQ(get_language(loc), "zh");
    EXPECT_EQ(get_country(loc), "CN");
}

TEST(LocaleTest, CreateFromBCP47) {
    auto loc = create_from_bcp47("en-US");
    EXPECT_EQ(get_language(loc), "en");
    EXPECT_EQ(get_country(loc), "US");
}

TEST(LocaleTest, GetDisplayNames) {
    auto en_US = create("en_US");
    auto zh_CN = create("zh_CN");
    
    EXPECT_EQ(get_display_name(en_US), "English (United States)");
    EXPECT_EQ(get_display_language(en_US), "English");
    EXPECT_EQ(get_display_country(en_US), "United States");
    
    // Test display names in different locale
    EXPECT_NE(get_display_name(en_US, zh_CN), get_display_name(en_US));
}

TEST(LocaleTest, CharacterProperties) {
    EXPECT_TRUE(is_letter('A'));
    EXPECT_TRUE(is_digit('9'));
    EXPECT_TRUE(is_whitespace(' '));
    EXPECT_TRUE(is_punctuation('.'));
    EXPECT_TRUE(is_upper('A'));
    EXPECT_TRUE(is_lower('a'));
}

TEST(LocaleTest, CaseConversion) {
    EXPECT_EQ(to_upper('a'), 'A');
    EXPECT_EQ(to_lower('A'), 'a');
    EXPECT_EQ(to_title('a'), 'A');
}

TEST(LocaleTest, Collation) {
    auto loc = create("en_US");
    EXPECT_TRUE(is_less_than(loc, "a", "b"));
    EXPECT_TRUE(is_greater_than(loc, "b", "a"));
    EXPECT_EQ(compare(loc, "a", "a"), 0);
}

TEST(LocaleTest, NumberFormatting) {
    auto en_US = create("en_US");
    auto de_DE = create("de_DE");
    
    // US format: 1,234.56
    std::string us_num = format_number(en_US, 1234.56);
    EXPECT_TRUE(us_num.find(',') != std::string::npos);
    EXPECT_TRUE(us_num.find('.') != std::string::npos);
    
    // German format: 1.234,56
    std::string de_num = format_number(de_DE, 1234.56);
    EXPECT_TRUE(de_num.find('.') != std::string::npos);
    EXPECT_TRUE(de_num.find(',') != std::string::npos);
    
    // Currency
    std::string us_curr = format_currency(en_US, 1234.56, "USD");
    EXPECT_TRUE(us_curr.find('$') != std::string::npos);
    
    // Percent
    std::string percent = format_percent(en_US, 0.1234);
    EXPECT_TRUE(percent.find('%') != std::string::npos);
}

TEST(LocaleTest, DateTimeFormatting) {
    auto loc = create("en_US");
    auto now = std::chrono::system_clock::now();
    
    std::string date = format_date(loc, now);
    EXPECT_FALSE(date.empty());
    
    std::string time = format_time(loc, now);
    EXPECT_FALSE(time.empty());
    
    std::string datetime = format_datetime(loc, now);
    EXPECT_FALSE(datetime.empty());
}

TEST(LocaleTest, CalendarFunctions) {
    auto loc = create("en_US");
    
    auto months = get_month_names(loc);
    EXPECT_EQ(months.size(), 12);
    EXPECT_EQ(months[0], "January");
    
    auto days = get_day_names(loc);
    EXPECT_EQ(days.size(), 7);
    EXPECT_EQ(days[0], "Sunday");
    
    std::string first_day = get_first_day_of_week(loc);
    EXPECT_FALSE(first_day.empty());
}

TEST(LocaleTest, TimeZoneFunctions) {
    set_time_zone("America/New_York");
    EXPECT_EQ(get_time_zone_id(), "America/New_York");
    
    int offset = get_time_zone_offset("America/New_York");
    EXPECT_NE(offset, 0);
    
    bool is_dst = is_daylight_time("America/New_York");
    // Result depends on current date
}

TEST(LocaleTest, GlobalFunctions) {
    auto locales = get_available_locale_names();
    EXPECT_FALSE(locales.empty());
    
    auto timezones = get_available_time_zones();
    EXPECT_FALSE(timezones.empty());
    
    EXPECT_TRUE(is_valid_locale_name("en_US"));
    EXPECT_FALSE(is_valid_locale_name("invalid_locale"));
    
    EXPECT_TRUE(is_valid_time_zone("America/New_York"));
    EXPECT_FALSE(is_valid_time_zone("Invalid/TimeZone"));
    
    std::string current_tz = get_current_time_zone();
    EXPECT_FALSE(current_tz.empty());
}
