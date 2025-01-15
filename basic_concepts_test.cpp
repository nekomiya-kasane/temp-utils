#include <format>
#include <ranges>
#include <string>
#include <vector>
#include <list>
#include <type_traits>

#include <gtest/gtest.h>

#include "small_vector.h"

template<typename T>
concept is_formattable = requires(std::format_context ctx, T value) {
  std::formatter<std::remove_cvref_t<T>>{}.format(value, ctx);
};

template<typename T>
concept is_ranged = requires {
  std::begin(std::declval<T>());
  std::end(std::declval<T>());
  std::ranges::subrange(std::begin(std::declval<T>()), std::end(std::declval<T>()));
};

template<typename T> struct function_traits;

template<typename R, typename... Args>
struct function_traits<R (*)(Args...)> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  static constexpr size_t arity = sizeof...(Args);
};

template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
};

template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
};

class TestClass {
public:
  int memberFunc(int x, double y) { return x + static_cast<int>(y); }
  std::string constMemberFunc(int x) const { return std::to_string(x); }
};

int freeFunc(int x, double y) { return x + static_cast<int>(y); }

// 1. is_formattable
TEST(TraitsConceptsTest, Formattable) {
  EXPECT_TRUE(is_formattable<int>);
  EXPECT_TRUE(is_formattable<double>);
  EXPECT_TRUE(is_formattable<std::string>);
  
  struct NonFormattable {};
  EXPECT_FALSE(is_formattable<NonFormattable>);
  
  struct Formattable {
    int value;
  };
}

// 2. is_ranged
TEST(TraitsConceptsTest, Ranged) {
  EXPECT_TRUE(is_ranged<std::vector<int>>);
  EXPECT_TRUE(is_ranged<std::list<int>>);
  EXPECT_TRUE(is_ranged<std::string>);
  
  int arr[5];
  EXPECT_FALSE(is_ranged<decltype(arr)>);
  
  struct NonRange {};
  EXPECT_FALSE(is_ranged<NonRange>);
  EXPECT_FALSE(is_ranged<int>);
  auto&& s = small_vector<int, 10>();
  auto a = std::begin(std::vector<int>());
  auto vb = std::begin(s);
  auto c = std::begin(small_vector<int>());
  EXPECT_TRUE((is_ranged<small_vector<int, 10>>));
  EXPECT_TRUE((is_ranged<small_vector<NonRange, 10>>));
}

// 3. function_traits
TEST(TraitsConceptsTest, FreeFunctionTraits) {
  using Traits = function_traits<decltype(&freeFunc)>;
  
  EXPECT_TRUE((std::is_same_v<Traits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<Traits::args_tuple, std::tuple<int, double>>));
  EXPECT_EQ(Traits::arity, 2);
}

TEST(TraitsConceptsTest, MemberFunctionTraits) {
  using Traits = function_traits<decltype(&TestClass::memberFunc)>;
  
  EXPECT_TRUE((std::is_same_v<Traits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<Traits::args_tuple, std::tuple<int, double>>));
  EXPECT_TRUE((std::is_same_v<Traits::class_type, TestClass>));
  EXPECT_EQ(Traits::arity, 2);
}

TEST(TraitsConceptsTest, ConstMemberFunctionTraits) {
  using Traits = function_traits<decltype(&TestClass::constMemberFunc)>;
  
  EXPECT_TRUE((std::is_same_v<Traits::result_type, std::string>));
  EXPECT_TRUE((std::is_same_v<Traits::args_tuple, std::tuple<int>>));
  EXPECT_TRUE((std::is_same_v<Traits::class_type, TestClass>));
  EXPECT_EQ(Traits::arity, 1);
}

TEST(TraitsConceptsTest, CompoundTests) {
  EXPECT_TRUE(is_formattable<std::string>);
  EXPECT_TRUE(is_ranged<std::string>);
  
  auto lambda = [](int x) -> std::string { return std::to_string(x); };
  using LambdaType = decltype(lambda);
  
  EXPECT_TRUE((is_formattable<std::invoke_result_t<LambdaType, int>>));
}

TEST(TraitsConceptsTest, EdgeCases) {
  std::vector<int> empty_vec;
  EXPECT_TRUE(is_ranged<decltype(empty_vec)>);
  
  EXPECT_TRUE(is_formattable<const int>);
  EXPECT_TRUE(is_ranged<const std::vector<int>>);
  
  EXPECT_TRUE(is_formattable<int&>);
  EXPECT_TRUE(is_ranged<std::vector<int>&>);
}
