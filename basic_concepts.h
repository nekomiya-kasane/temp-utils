#pragma once

#include <format>
#include <type_traits>

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

}  // namespace traits