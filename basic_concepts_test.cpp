#include <format>
#include <list>
#include <ranges>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "basic_concepts.h"
#include "small_vector.h"

using namespace traits;

class TestClass {
 public:
  int memberFunc(int x, double y)
  {
    return x + static_cast<int>(y);
  }
  std::string constMemberFunc(int x) const
  {
    return std::to_string(x);
  }
};

int freeFunc(int x, double y)
{
  return x + static_cast<int>(y);
}

// 1. is_formattable
TEST(BasicConceptsTest, Formattable)
{
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
TEST(BasicConceptsTest, Ranged)
{
  EXPECT_TRUE(is_ranged<std::vector<int>>);
  EXPECT_TRUE(is_ranged<std::list<int>>);
  EXPECT_TRUE(is_ranged<std::string>);

  int arr[5];
  EXPECT_FALSE(is_ranged<decltype(arr)>);

  struct NonRange {};
  EXPECT_FALSE(is_ranged<NonRange>);
  EXPECT_FALSE(is_ranged<int>);
  // todo: but if we try std::begin(small_vector<int>) in the concept definition, it will be false
  // and actually this phrase won't compile
  EXPECT_TRUE((is_ranged<small_vector<int, 10>>));
  EXPECT_TRUE((is_ranged<small_vector<NonRange, 10>>));
}

// 3. function_traits
TEST(BasicConceptsTest, FreeFunctionTraits)
{
  using Traits = function_traits<decltype(&freeFunc)>;

  EXPECT_TRUE((std::is_same_v<Traits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<Traits::args_tuple, std::tuple<int, double>>));
  EXPECT_EQ(Traits::arity, 2);
}

TEST(BasicConceptsTest, MemberFunctionTraits)
{
  using Traits = function_traits<decltype(&TestClass::memberFunc)>;

  EXPECT_TRUE((std::is_same_v<Traits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<Traits::args_tuple, std::tuple<int, double>>));
  EXPECT_TRUE((std::is_same_v<Traits::class_type, TestClass>));
  EXPECT_EQ(Traits::arity, 2);
}

TEST(BasicConceptsTest, ConstMemberFunctionTraits)
{
  using Traits = function_traits<decltype(&TestClass::constMemberFunc)>;

  EXPECT_TRUE((std::is_same_v<Traits::result_type, std::string>));
  EXPECT_TRUE((std::is_same_v<Traits::args_tuple, std::tuple<int>>));
  EXPECT_TRUE((std::is_same_v<Traits::class_type, TestClass>));
  EXPECT_EQ(Traits::arity, 1);
}

TEST(BasicConceptsTest, CompoundTests)
{
  EXPECT_TRUE(is_formattable<std::string>);
  EXPECT_TRUE(is_ranged<std::string>);

  auto lambda = [](int x) -> std::string { return std::to_string(x); };
  using LambdaType = decltype(lambda);

  EXPECT_TRUE((is_formattable<std::invoke_result_t<LambdaType, int>>));
}

TEST(BasicConceptsTest, EdgeCases)
{
  std::vector<int> empty_vec;
  EXPECT_TRUE((is_ranged<decltype(empty_vec)>));

  EXPECT_TRUE((is_formattable<const int>));
  EXPECT_TRUE((is_ranged<const std::vector<int>>));

  EXPECT_TRUE((is_formattable<int &>));
  EXPECT_TRUE((is_ranged<std::vector<int> &>));
}

TEST(BasicConceptsTest, LambdaAndFunctorTraits)
{
  [[maybe_unused]] auto stateless_lambda = [](int x) { return x * 2; };
  using StatelessLambdaType = decltype(stateless_lambda);
  using StatelessTraits = function_traits<decltype(&StatelessLambdaType::operator())>;
  EXPECT_TRUE((std::is_same_v<StatelessTraits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<StatelessTraits::args_tuple, std::tuple<int>>));
  EXPECT_EQ(StatelessTraits::arity, 1);

  int multiplier = 3;
  [[maybe_unused]] auto stateful_lambda = [multiplier](int x) { return x * multiplier; };
  using StatefulLambdaType = decltype(stateful_lambda);
  using StatefulTraits = function_traits<decltype(&StatefulLambdaType::operator())>;
  EXPECT_TRUE((std::is_same_v<StatefulTraits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<StatefulTraits::args_tuple, std::tuple<int>>));
  EXPECT_EQ(StatefulTraits::arity, 1);

  struct Functor {
    double operator()(int x, float y) const
    {
      return x + y;
    }
    // ReSharper disable once CppMemberFunctionMayBeConst
    int operator()(std::string s)
    {
      return std::stoi(s);
    }
  };

  [[maybe_unused]] double (Functor::*func1)(int, float) const = &Functor::operator();
  using ConstFunctorTraits1 = function_traits<decltype(func1)>;
  EXPECT_TRUE((std::is_same_v<ConstFunctorTraits1::result_type, double>));
  EXPECT_TRUE((std::is_same_v<ConstFunctorTraits1::args_tuple, std::tuple<int, float>>));
  EXPECT_EQ(ConstFunctorTraits1::arity, 2);

  [[maybe_unused]] int (Functor::*func2)(std::string) = &Functor::operator();
  using ConstFunctorTraits2 = function_traits<decltype(func2)>;
  EXPECT_TRUE((std::is_same_v<ConstFunctorTraits2::result_type, int>));
  EXPECT_TRUE((std::is_same_v<ConstFunctorTraits2::args_tuple, std::tuple<std::string>>));
  EXPECT_EQ(ConstFunctorTraits2::arity, 1);
}

TEST(BasicConceptsTest, ReferenceQualifiedMemberFunctionTraits)
{
  struct RefQualifiedMethods {
    // 基本引用限定符
    void lvalueMethod() & {}
    void rvalueMethod() && {}

    // const + 引用限定符
    void constLvalueMethod() const & {}
    void constRvalueMethod() const && {}

    // volatile + 引用限定符
    void volatileLvalueMethod() volatile & {}
    void volatileRvalueMethod() volatile && {}

    // const volatile + 引用限定符
    void constVolatileLvalueMethod() const volatile & {}
    void constVolatileRvalueMethod() const volatile && {}

    // 可变参数 + 引用限定符
    void variadicLvalueMethod(const char *fmt, ...) & {}
    void variadicRvalueMethod(const char *fmt, ...) && {}

    // const + 可变参数 + 引用限定符
    void constVariadicLvalueMethod(const char *fmt, ...) const & {}
    void constVariadicRvalueMethod(const char *fmt, ...) const && {}

    // volatile + 可变参数 + 引用限定符
    void volatileVariadicLvalueMethod(const char *fmt, ...) volatile & {}
    void volatileVariadicRvalueMethod(const char *fmt, ...) volatile && {}

    // const volatile + 可变参数 + 引用限定符
    void constVolatileVariadicLvalueMethod(const char *fmt, ...) const volatile & {}
    void constVolatileVariadicRvalueMethod(const char *fmt, ...) const volatile && {}
  };

  // 测试基本引用限定符
  {
    using LvalueTraits = function_traits<decltype(&RefQualifiedMethods::lvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<LvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<LvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<LvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(LvalueTraits::arity, 0);
    EXPECT_FALSE(LvalueTraits::is_variadic);

    using RvalueTraits = function_traits<decltype(&RefQualifiedMethods::rvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<RvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<RvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<RvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(RvalueTraits::arity, 0);
    EXPECT_FALSE(RvalueTraits::is_variadic);
  }

  // 测试 const + 引用限定符
  {
    using ConstLvalueTraits = function_traits<decltype(&RefQualifiedMethods::constLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstLvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<ConstLvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<ConstLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstLvalueTraits::arity, 0);
    EXPECT_FALSE(ConstLvalueTraits::is_variadic);

    using ConstRvalueTraits = function_traits<decltype(&RefQualifiedMethods::constRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstRvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<ConstRvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<ConstRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstRvalueTraits::arity, 0);
    EXPECT_FALSE(ConstRvalueTraits::is_variadic);
  }

  // 测试 volatile + 引用限定符
  {
    using VolatileLvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::volatileLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<VolatileLvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<VolatileLvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<VolatileLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(VolatileLvalueTraits::arity, 0);
    EXPECT_FALSE(VolatileLvalueTraits::is_variadic);

    using VolatileRvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::volatileRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<VolatileRvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<VolatileRvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<VolatileRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(VolatileRvalueTraits::arity, 0);
    EXPECT_FALSE(VolatileRvalueTraits::is_variadic);
  }

  // 测试 const volatile + 引用限定符
  {
    using ConstVolatileLvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::constVolatileLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstVolatileLvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<ConstVolatileLvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<ConstVolatileLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstVolatileLvalueTraits::arity, 0);
    EXPECT_FALSE(ConstVolatileLvalueTraits::is_variadic);

    using ConstVolatileRvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::constVolatileRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstVolatileRvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<ConstVolatileRvalueTraits::args_tuple, std::tuple<>>));
    EXPECT_TRUE((std::is_same_v<ConstVolatileRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstVolatileRvalueTraits::arity, 0);
    EXPECT_FALSE(ConstVolatileRvalueTraits::is_variadic);
  }

  // 测试可变参数 + 引用限定符
  {
    using VariadicLvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::variadicLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<VariadicLvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<VariadicLvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE((std::is_same_v<VariadicLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(VariadicLvalueTraits::arity, 1);
    EXPECT_TRUE(VariadicLvalueTraits::is_variadic);

    using VariadicRvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::variadicRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<VariadicRvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<VariadicRvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE((std::is_same_v<VariadicRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(VariadicRvalueTraits::arity, 1);
    EXPECT_TRUE(VariadicRvalueTraits::is_variadic);
  }

  // 测试 const + 可变参数 + 引用限定符
  {
    using ConstVariadicLvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::constVariadicLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstVariadicLvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<ConstVariadicLvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE((std::is_same_v<ConstVariadicLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstVariadicLvalueTraits::arity, 1);
    EXPECT_TRUE(ConstVariadicLvalueTraits::is_variadic);

    using ConstVariadicRvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::constVariadicRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstVariadicRvalueTraits::result_type, void>));
    EXPECT_TRUE((std::is_same_v<ConstVariadicRvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE((std::is_same_v<ConstVariadicRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstVariadicRvalueTraits::arity, 1);
    EXPECT_TRUE(ConstVariadicRvalueTraits::is_variadic);
  }

  // 测试 volatile + 可变参数 + 引用限定符
  {
    using VolatileVariadicLvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::volatileVariadicLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<VolatileVariadicLvalueTraits::result_type, void>));
    EXPECT_TRUE(
        (std::is_same_v<VolatileVariadicLvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE((std::is_same_v<VolatileVariadicLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(VolatileVariadicLvalueTraits::arity, 1);
    EXPECT_TRUE(VolatileVariadicLvalueTraits::is_variadic);

    using VolatileVariadicRvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::volatileVariadicRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<VolatileVariadicRvalueTraits::result_type, void>));
    EXPECT_TRUE(
        (std::is_same_v<VolatileVariadicRvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE((std::is_same_v<VolatileVariadicRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(VolatileVariadicRvalueTraits::arity, 1);
    EXPECT_TRUE(VolatileVariadicRvalueTraits::is_variadic);
  }

  // 测试 const volatile + 可变参数 + 引用限定符
  {
    using ConstVolatileVariadicLvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::constVolatileVariadicLvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstVolatileVariadicLvalueTraits::result_type, void>));
    EXPECT_TRUE(
        (std::is_same_v<ConstVolatileVariadicLvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE(
        (std::is_same_v<ConstVolatileVariadicLvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstVolatileVariadicLvalueTraits::arity, 1);
    EXPECT_TRUE(ConstVolatileVariadicLvalueTraits::is_variadic);

    using ConstVolatileVariadicRvalueTraits =
        function_traits<decltype(&RefQualifiedMethods::constVolatileVariadicRvalueMethod)>;
    EXPECT_TRUE((std::is_same_v<ConstVolatileVariadicRvalueTraits::result_type, void>));
    EXPECT_TRUE(
        (std::is_same_v<ConstVolatileVariadicRvalueTraits::args_tuple, std::tuple<const char *>>));
    EXPECT_TRUE(
        (std::is_same_v<ConstVolatileVariadicRvalueTraits::class_type, RefQualifiedMethods>));
    EXPECT_EQ(ConstVolatileVariadicRvalueTraits::arity, 1);
    EXPECT_TRUE(ConstVolatileVariadicRvalueTraits::is_variadic);
  }
}

int printf_like(const char *fmt, ...)
{
  return 0;
}

TEST(BasicConceptsTest, VariadicFunctionTraits)
{
  // C 风格可变参数函数

  using PrintfLikeType = decltype(&printf_like);
  using PrintfTraits = function_traits<PrintfLikeType>;

  EXPECT_TRUE((std::is_same_v<PrintfTraits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<PrintfTraits::args_tuple, std::tuple<const char *>>));
  EXPECT_EQ(PrintfTraits::arity, 1);
  EXPECT_TRUE(PrintfTraits::is_variadic);

  // 可变参数成员函数
  struct VariadicClass {
    void printf_like(const char *fmt, ...) {}
    void const_printf_like(const char *fmt, ...) const {}
  };

  // 非 const 可变参数成员函数
  using VariadicMethodTraits = function_traits<decltype(&VariadicClass::printf_like)>;
  EXPECT_TRUE((std::is_same_v<VariadicMethodTraits::result_type, void>));
  EXPECT_TRUE((std::is_same_v<VariadicMethodTraits::args_tuple, std::tuple<const char *>>));
  EXPECT_TRUE((std::is_same_v<VariadicMethodTraits::class_type, VariadicClass>));
  EXPECT_EQ(VariadicMethodTraits::arity, 1);
  EXPECT_TRUE(VariadicMethodTraits::is_variadic);

  // const 可变参数成员函数
  using ConstVariadicMethodTraits = function_traits<decltype(&VariadicClass::const_printf_like)>;
  EXPECT_TRUE((std::is_same_v<ConstVariadicMethodTraits::result_type, void>));
  EXPECT_TRUE((std::is_same_v<ConstVariadicMethodTraits::args_tuple, std::tuple<const char *>>));
  EXPECT_TRUE((std::is_same_v<ConstVariadicMethodTraits::class_type, VariadicClass>));
  EXPECT_EQ(ConstVariadicMethodTraits::arity, 1);
  EXPECT_TRUE(ConstVariadicMethodTraits::is_variadic);

  // 标准库可变参数函数
  using StdPrintfType = decltype(&std::printf);
  using StdPrintfTraits = function_traits<StdPrintfType>;
  EXPECT_TRUE((std::is_same_v<StdPrintfTraits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<StdPrintfTraits::args_tuple, std::tuple<const char *>>));
  EXPECT_EQ(StdPrintfTraits::arity, 1);
  EXPECT_TRUE(StdPrintfTraits::is_variadic);
}

TEST(BasicConceptsTest, FunctionReferenceAndPointerTraits)
{
  // 函数指针类型
  using FuncPtr = int (*)(double);
  using FuncPtrTraits = function_traits<FuncPtr>;
  EXPECT_TRUE((std::is_same_v<FuncPtrTraits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<FuncPtrTraits::args_tuple, std::tuple<double>>));
  EXPECT_EQ(FuncPtrTraits::arity, 1);

  // 成员函数指针类型
  using MemFuncPtr = int (TestClass::*)(double);
  using MemFuncPtrTraits = function_traits<MemFuncPtr>;
  EXPECT_TRUE((std::is_same_v<MemFuncPtrTraits::result_type, int>));
  EXPECT_TRUE((std::is_same_v<MemFuncPtrTraits::args_tuple, std::tuple<double>>));
  EXPECT_TRUE((std::is_same_v<MemFuncPtrTraits::class_type, TestClass>));
  EXPECT_EQ(MemFuncPtrTraits::arity, 1);
}

TEST(BasicConceptsTest, QualifiedMemberFunctionTraits)
{
  struct QualifiedMethods {
    void constMethod() const {}
    void volatileMethod() volatile {}
    void constVolatileMethod() const volatile {}
    void refQualifiedMethod() & {}
    void constRefMethod() const & {}
    void rvalueMethod() && {}
    void variadicVolatileMethod(const char *fmt, ...) volatile {}
    void variadicConstVolatileMethod(const char *fmt, ...) const volatile {}
  };

  // const 方法
  using ConstMethodTraits = function_traits<decltype(&QualifiedMethods::constMethod)>;
  EXPECT_TRUE((std::is_same_v<ConstMethodTraits::result_type, void>));
  EXPECT_TRUE((std::is_same_v<ConstMethodTraits::args_tuple, std::tuple<>>));
  EXPECT_TRUE((std::is_same_v<ConstMethodTraits::class_type, QualifiedMethods>));
  EXPECT_EQ(ConstMethodTraits::arity, 0);
  EXPECT_FALSE((ConstMethodTraits::is_variadic));

  // volatile 方法
  using VolatileMethodTraits = function_traits<decltype(&QualifiedMethods::volatileMethod)>;
  EXPECT_TRUE((std::is_same_v<VolatileMethodTraits::result_type, void>));
  EXPECT_TRUE((std::is_same_v<VolatileMethodTraits::args_tuple, std::tuple<>>));
  EXPECT_TRUE((std::is_same_v<VolatileMethodTraits::class_type, QualifiedMethods>));
  EXPECT_EQ(VolatileMethodTraits::arity, 0);
  EXPECT_FALSE(VolatileMethodTraits::is_variadic);

  // const volatile 方法
  using ConstVolatileMethodTraits =
      function_traits<decltype(&QualifiedMethods::constVolatileMethod)>;
  EXPECT_TRUE((std::is_same_v<ConstVolatileMethodTraits::result_type, void>));
  EXPECT_TRUE((std::is_same_v<ConstVolatileMethodTraits::args_tuple, std::tuple<>>));
  EXPECT_TRUE((std::is_same_v<ConstVolatileMethodTraits::class_type, QualifiedMethods>));
  EXPECT_EQ(ConstVolatileMethodTraits::arity, 0);
  EXPECT_FALSE(ConstVolatileMethodTraits::is_variadic);

  // volatile 可变参数方法
  using VolatileVariadicMethodTraits =
      function_traits<decltype(&QualifiedMethods::variadicVolatileMethod)>;
  EXPECT_TRUE((std::is_same_v<VolatileVariadicMethodTraits::result_type, void>));
  EXPECT_TRUE(
      (std::is_same_v<VolatileVariadicMethodTraits::args_tuple, std::tuple<const char *>>));
  EXPECT_TRUE((std::is_same_v<VolatileVariadicMethodTraits::class_type, QualifiedMethods>));
  EXPECT_EQ(VolatileVariadicMethodTraits::arity, 1);
  EXPECT_TRUE(VolatileVariadicMethodTraits::is_variadic);

  // const volatile 可变参数方法
  using ConstVolatileVariadicMethodTraits =
      function_traits<decltype(&QualifiedMethods::variadicConstVolatileMethod)>;
  EXPECT_TRUE((std::is_same_v<ConstVolatileVariadicMethodTraits::result_type, void>));
  EXPECT_TRUE(
      (std::is_same_v<ConstVolatileVariadicMethodTraits::args_tuple, std::tuple<const char *>>));
  EXPECT_TRUE((std::is_same_v<ConstVolatileVariadicMethodTraits::class_type, QualifiedMethods>));
  EXPECT_EQ(ConstVolatileVariadicMethodTraits::arity, 1);
  EXPECT_TRUE(ConstVolatileVariadicMethodTraits::is_variadic);
}

TEST(BasicConceptsTest, ComplexReturnTypeTraits)
{
  struct ComplexReturnTypes {
    std::vector<int> returnVector()
    {
      return {};
    }
    std::map<std::string, double> returnMap()
    {
      return {};
    }
    auto returnAuto()
    {
      return 42;
    }
    decltype(auto) returnDecltype()
    {
      return std::string("hello");
    }
  };

  using VectorReturnTraits = function_traits<decltype(&ComplexReturnTypes::returnVector)>;
  EXPECT_TRUE((std::is_same_v<VectorReturnTraits::result_type, std::vector<int>>));

  using MapReturnTraits = function_traits<decltype(&ComplexReturnTypes::returnMap)>;
  EXPECT_TRUE((std::is_same_v<MapReturnTraits::result_type, std::map<std::string, double>>));

  using AutoReturnTraits = function_traits<decltype(&ComplexReturnTypes::returnAuto)>;
  EXPECT_TRUE((std::is_same_v<AutoReturnTraits::result_type, int>));

  using DecltypeReturnTraits = function_traits<decltype(&ComplexReturnTypes::returnDecltype)>;
  EXPECT_TRUE((std::is_same_v<DecltypeReturnTraits::result_type, std::string>));
}
