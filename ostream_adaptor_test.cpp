#include "ostream_adaptor.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace ustd;

// 已有 ostream 运算符的类
class HasOstream {
    int value;
public:
    explicit HasOstream(int v) : value(v) {}
    friend std::ostream& operator<<(std::ostream& os, const HasOstream& obj) {
        return os << "HasOstream(" << obj.value << ")";
    }
};

// 有成员 to_string 的类
class HasMemberToString {
    std::string value;
public:
    explicit HasMemberToString(std::string v) : value(std::move(v)) {}
    std::string to_string() const { return "Member:" + value; }
};

// 有成员 to_ustring 的类
class HasMemberUString {
    ustring value;
public:
    explicit HasMemberUString(ustring v) : value(std::move(v)) {}
    ustring to_ustring() const { return u"UMember:" + value; }
};

// 有全局 to_string 的类
class HasGlobalToString {
    double value;
public:
    explicit HasGlobalToString(double v) : value(v) {}
    double getValue() const { return value; }
};

std::string to_string(const HasGlobalToString& obj) {
    return "Global:" + std::to_string(obj.getValue());
}

// 有全局 to_ustring 的类
class HasGlobalUString {
    ustring value;
public:
    explicit HasGlobalUString(ustring v) : value(std::move(v)) {}
    const ustring& getValue() const { return value; }
};

ustd::ustring to_ustring(const HasGlobalUString& obj) {
    return u"UGlobal:" + obj.getValue();
}

// 使用 std::to_string 的类
class UsesStdToString {
    long value;
public:
    explicit UsesStdToString(long v) : value(v) {}
    operator long() const { return value; }
};

// 使用 std::format 的类
class UsesStdFormat {
    int x, y;
public:
    UsesStdFormat(int x_, int y_) : x(x_), y(y_) {}
};

// 显式特化 std::formatter，因为我们不能通过模板推导处理这种情况
template<>
struct std::formatter<UsesStdFormat> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    
    auto format(const UsesStdFormat& p, format_context& ctx) const {
        return format_to(ctx.out(), "Format({},{})", p.x, p.y);
    }
};

// 没有任何字符串转换方法的类
class NoStringConversion {
    int value;
public:
    explicit NoStringConversion(int v) : value(v) {}
};

TEST(OstreamAdaptorTest, ExistingOstreamOperator) {
    std::ostringstream oss;
    HasOstream obj(42);
    oss << obj;
    EXPECT_EQ(oss.str(), "HasOstream(42)");
    EXPECT_EQ(std::format("{}", obj), "HasOstream(42)");
}

TEST(OstreamAdaptorTest, MemberToString) {
    std::ostringstream oss;
    HasMemberToString obj("test");
    oss << obj;
    EXPECT_EQ(oss.str(), "Member:test");
    EXPECT_EQ(std::format("{}", obj), "Member:test");
}

TEST(OstreamAdaptorTest, MemberUString) {
    std::ostringstream oss;
    HasMemberUString obj(u"test");
    oss << obj;
    EXPECT_EQ(oss.str(), "UMember:test");
    EXPECT_EQ(std::format("{}", obj), "UMember:test");
}

TEST(OstreamAdaptorTest, GlobalToString) {
    std::ostringstream oss;
    HasGlobalToString obj(3.14);
    oss << obj;
    EXPECT_TRUE(oss.str().find("Global:3.14") != std::string::npos);
    EXPECT_TRUE(std::format("{}", obj).find("Global:3.14") != std::string::npos);
}

TEST(OstreamAdaptorTest, GlobalUString) {
    std::ostringstream oss;
    HasGlobalUString obj(u"test");
    oss << obj;
    EXPECT_EQ(oss.str(), "UGlobal:test");
    EXPECT_EQ(std::format("{}", obj), "UGlobal:test");
}

TEST(OstreamAdaptorTest, StdToString) {
    std::ostringstream oss;
    UsesStdToString obj(123);
    oss << obj;
    EXPECT_EQ(oss.str(), "123");
    EXPECT_EQ(std::format("{}", obj), "123");
}

TEST(OstreamAdaptorTest, StdFormat) {
    std::ostringstream oss;
    UsesStdFormat obj(10, 20);
    oss << obj;
    EXPECT_EQ(oss.str(), "Format(10,20)");
    EXPECT_EQ(std::format("{}", obj), "Format(10,20)");
}

TEST(OstreamAdaptorTest, ConceptChecks) {
    static_assert(has_ostream_operator<HasOstream>);
    static_assert(has_member_to_string<HasMemberToString>);
    static_assert(has_member_to_ustring<HasMemberUString>);
    static_assert(has_global_to_string<HasGlobalToString>);
    static_assert(has_global_to_ustring<HasGlobalUString>);
    static_assert(has_std_to_string<long>);
    static_assert(has_std_formatter<UsesStdFormat>);
    
    static_assert(!stringifiable<HasOstream>);
    static_assert(stringifiable<HasMemberToString>);
    static_assert(stringifiable<HasMemberUString>);
    static_assert(stringifiable<HasGlobalToString>);
    static_assert(stringifiable<HasGlobalUString>);
    static_assert(!stringifiable<NoStringConversion>);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
