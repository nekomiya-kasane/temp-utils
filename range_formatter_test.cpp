#include "range_formatter.h"
#include "small_vector.h"
#include <gtest/gtest.h>
#include <list>
#include <set>
#include <string>
#include <vector>

TEST(RangeFormatterTest, BasicFormatting)
{
  std::vector<int> vec{1, 2, 3, 4, 5};
  EXPECT_EQ(std::format("{}", vec), "[1, 2, 3, 4, 5]");

  std::list<std::string> list{"hello", "world"};
  EXPECT_EQ(std::format("{}", list), "[hello, world]");

  std::set<char> set{'a', 'b', 'c'};
  EXPECT_EQ(std::format("{}", set), "[a, b, c]");
}

TEST(RangeFormatterTest, CustomFormatting)
{
  std::vector<int> vec{1, 2, 3};

  // 测试不同的括号样式
  EXPECT_EQ(std::format("{:p(}", vec), "(1, 2, 3)");
  EXPECT_EQ(std::format("{:p{}", vec), "{1, 2, 3}");
  EXPECT_EQ(std::format("{:p<}", vec), "<1, 2, 3>");

  // 测试不同的分隔符
  EXPECT_EQ(std::format("{:s;}", vec), "[1; 2; 3]");
  EXPECT_EQ(std::format("{:s|}", vec), "[1 | 2 | 3]");
  EXPECT_EQ(std::format("{:s }", vec), "[1 2 3]");

  // 测试空范围表示
  std::vector<int> empty;
  EXPECT_EQ(std::format("{:e0}", empty), "");
  EXPECT_EQ(std::format("{:ee}", empty), "empty");
  EXPECT_EQ(std::format("{:en}", empty), "null");
}

TEST(RangeFormatterTest, SmallVectorFormatting)
{
  small_vector<int, 4> vec{1, 2, 3};

  // 基本格式化
  EXPECT_EQ(std::format("{}", vec), "[1, 2, 3]");

  // 显示内联存储信息
  std::string result = std::format("{:i}", vec);
  EXPECT_TRUE(result.find("inline=true") != std::string::npos);

  // 显示容量信息
  result = std::format("{:c}", vec);
  EXPECT_TRUE(result.find("cap=4") != std::string::npos);

  // 组合格式化选项
  result = std::format("{:icp(}", vec);
  EXPECT_TRUE(result.find("inline=true") != std::string::npos);
  EXPECT_TRUE(result.find("cap=4") != std::string::npos);
  EXPECT_TRUE(result.find("(1, 2, 3)") != std::string::npos);
}

TEST(RangeFormatterTest, NestedRanges)
{
  std::vector<std::vector<int>> nested{{1, 2}, {3, 4}, {5, 6}};

  EXPECT_EQ(std::format("{}", nested), "[[1, 2], [3, 4], [5, 6]]");
  EXPECT_EQ(std::format("{:p{s|}", nested), "{[1, 2] | [3, 4] | [5, 6]}");
}

struct Point {
  int x, y;
  friend std::formatter<Point>;
};

template<> struct std::formatter<Point> {
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const Point &p, std::format_context &ctx) const
  {
    return std::format_to(ctx.out(), "({},{})", p.x, p.y);
  }
};

TEST(RangeFormatterTest, CustomTypes)
{
  small_vector<Point, 4> points{{1, 2}, {3, 4}, {5, 6}};
  EXPECT_EQ(std::format("{}", points), "[(1,2), (3,4), (5,6)]");
}

TEST(RangeFormatterTest, NonFormattableTypes)
{
  struct NonFormattable {};
  small_vector<NonFormattable, 4> vec(3);

  // 对于不可格式化的类型，应该显示类型信息和地址
  std::string result = std::format("{}", vec);
  EXPECT_TRUE(result.find("NonFormattable@0x") != std::string::npos);
}
