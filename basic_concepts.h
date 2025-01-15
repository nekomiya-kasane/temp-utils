#pragma once

#include <format>
#include <source_location>
#include <tuple>
#include <type_traits>

namespace traits {

  template<typename = void> consteval auto function_name()
  {
    return std::source_location::current().function_name();
  }

  template<typename T> constexpr auto type_of(T &&arg)
  {
    constexpr std::string_view fmt = function_name<decltype(arg)>();
    return fmt.substr(41, fmt.size() - 47);
  }

  template<typename T> constexpr auto type_of()
  {
    constexpr std::string_view fmt = function_name<T>();
    return fmt.substr(41, fmt.size() - 47);
  }

  template<typename T> constexpr auto bare_type_of(T &&arg)
  {
    constexpr std::string_view fmt = function_name<decltype(arg)>();
    return fmt.substr(42, fmt.size() - 49);
  }

  template<typename T> constexpr auto bare_type_of()
  {
    constexpr std::string_view fmt = function_name<T>();
    return fmt.substr(42, fmt.size() - 49);
  }

  template<typename T>
  concept formattable = requires(std::format_context ctx, T value) {
    std::formatter<std::remove_cvref_t<T>>{}.format(value, ctx);
  };

  template<typename T>
  concept ranged_type = requires(T t) {
    std::begin(t);
    std::end(t);
    std::ranges::subrange(std::begin(t), std::end(t));
  };

  template<typename T>
  concept callable = requires(T t) { (std::void_t<decltype(&T::operator())>)t; };

  // 函数类型标记
  enum class function_type {
    free_function = 0,    // 自由函数
    member_function = 1,  // 成员函数
    lambda = 2,           // Lambda 函数
    functor = 3           // 函数对象
  };

  // 函数限定符标记
  enum class function_qualifiers : uint32_t {
    none = 0,
    is_const = 1 << 0,     // const 成员函数
    is_volatile = 1 << 1,  // volatile 成员函数
    is_lvalue = 1 << 2,    // 左值引用限定符 &
    is_rvalue = 1 << 3,    // 右值引用限定符 &&
    is_variadic = 1 << 4,  // 可变参数函数
    is_noexcept = 1 << 5,  // noexcept 函数
    has_capture = 1 << 6,  // lambda 带有捕获
    is_mutable = 1 << 7,   // mutable lambda
  };

  // 位运算操作符重载
  constexpr function_qualifiers operator|(function_qualifiers a, function_qualifiers b)
  {
    return static_cast<function_qualifiers>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
  }

  constexpr function_qualifiers operator&(function_qualifiers a, function_qualifiers b)
  {
    return static_cast<function_qualifiers>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
  }

  constexpr bool has_qualifier(function_qualifiers flags, function_qualifiers qualifier)
  {
    return static_cast<uint32_t>(flags & qualifier) != 0;
  }

  // 基础 function_traits 模板
  template<typename T> struct function_traits {};

  // 普通函数特化
  template<typename R, typename... Args> struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::none;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 可变参数普通函数特化
  template<typename R, typename... Args> struct function_traits<R (*)(Args..., ...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // noexcept 函数特化
  template<typename R, typename... Args> struct function_traits<R (*)(Args...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // noexcept 可变参数函数特化
  template<typename R, typename... Args> struct function_traits<R (*)(Args..., ...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = void;
    static constexpr function_type type = function_type::free_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_noexcept |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 成员函数特化
  template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::none;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 左值引用成员函数特化
  template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...) &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 左值引用成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 左值引用成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 左值引用成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 右值引用成员函数特化
  template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...) &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 右值引用成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 右值引用成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 右值引用成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 可变参数成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 可变参数成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...)> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 可变参数成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 可变参数成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 可变参数成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args...) const volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 可变参数 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 可变参数 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 可变参数 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 可变参数 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 可变参数左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 可变参数左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 可变参数左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 可变参数左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 可变参数左值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile & noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // 可变参数右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const 可变参数右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 可变参数右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // const volatile 可变参数右值引用 noexcept 成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile && noexcept> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic |
                                                      function_qualifiers::is_noexcept;
    static constexpr size_t arity = sizeof...(Args);
  };

  // volatile 可变参数成员函数特化
  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile &> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_lvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  template<typename C, typename R, typename... Args>
  struct function_traits<R (C::*)(Args..., ...) const volatile &&> {
    using result_type = R;
    using args_tuple = std::tuple<Args...>;
    using class_type = C;
    static constexpr function_type type = function_type::member_function;
    static constexpr function_qualifiers qualifiers = function_qualifiers::is_const |
                                                      function_qualifiers::is_rvalue |
                                                      function_qualifiers::is_volatile |
                                                      function_qualifiers::is_variadic;
    static constexpr size_t arity = sizeof...(Args);
  };

  namespace detail {

    template<typename T>
    concept has_lambda_name = std::is_class_v<T> && type_of<T>().contains("<lambda");

  }

  template<typename T>
    requires callable<T> && std::is_class_v<T>
  struct function_traits<T> {
    using result_type = typename function_traits<decltype(&T::operator())>::result_type;
    using args_tuple = typename function_traits<decltype(&T::operator())>::args_tuple;
    using class_type = typename function_traits<decltype(&T::operator())>::class_type;
    static constexpr function_type type = detail::has_lambda_name<T> ? function_type::lambda :
                                                                       function_type::functor;
    static constexpr function_qualifiers qualifiers = detail::has_lambda_name<T> ?
                                                          (std::is_empty_v<T> ?
                                                               function_qualifiers::none :
                                                               function_qualifiers::has_capture) :
                                                          function_qualifiers::none;
  };

  // 辅助概念
  template<typename T>
  concept variadic_function = has_qualifier(function_traits<T>::qualifiers,
                                            function_qualifiers::is_variadic);

  template<typename T>
  concept const_function = has_qualifier(function_traits<T>::qualifiers,
                                         function_qualifiers::is_const);

  template<typename T>
  concept volatile_function = has_qualifier(function_traits<T>::qualifiers,
                                            function_qualifiers::is_volatile);

  template<typename T>
  concept lvalue_function = has_qualifier(function_traits<T>::qualifiers,
                                          function_qualifiers::is_lvalue);

  template<typename T>
  concept rvalue_function = has_qualifier(function_traits<T>::qualifiers,
                                          function_qualifiers::is_rvalue);

  template<typename T>
  concept noexcept_function = has_qualifier(function_traits<T>::qualifiers,
                                            function_qualifiers::is_noexcept);

  template<typename T>
  concept member_function = function_traits<T>::type == function_type::member_function;

  template<typename T>
  concept lambda_function = function_traits<T>::type == function_type::lambda;

  template<typename T>
  concept functor = function_traits<T>::type == function_type::functor;

  template<typename T>
  concept free_function = function_traits<T>::type == function_type::free_function;

}  // namespace traits
