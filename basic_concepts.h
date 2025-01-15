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

}  // namespace traits
