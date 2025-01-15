#pragma once

#include <format>
#include <type_traits>
#include <tuple>

namespace traits {

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

template<typename F> struct function_traits : function_traits<decltype(&F::operator())> {};

template<typename R, typename... Args> struct function_traits<R (*)(Args...)> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  static constexpr size_t arity = sizeof...(Args);
};

template<typename C, typename R, typename... Args> struct function_traits<R (C::*)(Args...)> {
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

template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) volatile> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// C 风格可变参数函数 - 自由函数
template<typename R, typename... Args>
struct function_traits<R (*)(Args..., ...)> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// C 风格可变参数函数 - 成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...)> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// C 风格可变参数函数 - const 成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) const> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// C 风格可变参数函数 - volatile 成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) volatile> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// C 风格可变参数函数 - const volatile 成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) const volatile> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// 左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// 右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// const 左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// const 右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// volatile 左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) volatile &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// volatile 右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) volatile &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// const volatile 左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// const volatile 右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const volatile &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = false;
};

// 可变参数左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// 可变参数右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// const 可变参数左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) const &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// const 可变参数右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) const &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// volatile 可变参数左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) volatile &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// volatile 可变参数右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) volatile &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// const volatile 可变参数左值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) const volatile &> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

// const volatile 可变参数右值引用成员函数
template<typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args..., ...) const volatile &&> {
  using result_type = R;
  using args_tuple = std::tuple<Args...>;
  using class_type = C;
  static constexpr size_t arity = sizeof...(Args);
  static constexpr bool is_variadic = true;
};

}  // namespace traits
