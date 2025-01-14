#pragma once

#include "ustring.h"
#include <concepts>
#include <format>
#include <ostream>
#include <string>
#include <type_traits>

template<typename T>
concept has_ostream_operator = requires(std::ostream &os, const T &t) {
  {
    os << t
  } -> std::convertible_to<std::ostream &>;
};

template<typename T>
concept has_member_to_string = requires(const T &t) {
  {
    t.to_string()
  } -> std::convertible_to<std::string>;
};

template<typename T>
concept has_member_to_ustring = requires(const T &t) {
  {
    t.to_ustring()
  } -> std::convertible_to<ustring>;
};

template<typename T>
concept has_global_to_string = requires(const T &t) {
  {
    to_string(t)
  } -> std::convertible_to<std::string>;
};

template<typename T>
concept has_global_to_ustring = requires(const T &t) {
  {
    to_ustring(t)
  } -> std::convertible_to<ustring>;
};

template<typename T>
concept has_std_to_string = requires(const T &t) {
  {
    std::to_string(t)
  } -> std::convertible_to<std::string>;
};

template<typename T>
concept has_std_formatter = requires(const T &t) {
  {
    std::format("{}", t)
  } -> std::convertible_to<std::string>;
};

template<typename T>
concept stringifiable = !has_ostream_operator<T> &&
                        (has_member_to_string<T> || has_member_to_ustring<T> ||
                         has_global_to_string<T> || has_global_to_ustring<T> ||
                         has_std_to_string<T> || has_std_formatter<T>);

namespace detail {

template<typename T> auto to_string_impl(const T &value)
{
  if constexpr (has_member_to_ustring<T>) {
    return value.to_ustring();
  }
  else if constexpr (has_global_to_ustring<T>) {
    return to_ustring(value);
  }
  else if constexpr (has_member_to_string<T>) {
    return ustring(value.to_string());
  }
  else if constexpr (has_global_to_string<T>) {
    return ustring(to_string(value));
  }
  else if constexpr (has_std_to_string<T>) {
    return ustring(std::to_string(value));
  }
  else if constexpr (has_std_formatter<T>) {
    return ustring(std::format("{}", value));
  }
  else if constexpr (has_ostream_operator<T>) {
    std::ostringstream oss;
    oss << value;
    return ustring(oss.str());
  }
  else {
    static_assert(has_ostream_operator<T> || has_member_to_string<T> || has_member_to_ustring<T> ||
                      has_global_to_string<T> || has_global_to_ustring<T> ||
                      has_std_to_string<T> || has_std_formatter<T>,
                  "Type must either:\n"
                  "1. Have a member function to_string() returning std::string\n"
                  "2. Have a member function to_ustring() returning ustring\n"
                  "3. Have a global function to_string(T) returning std::string\n"
                  "4. Have a global function to_ustring(T) returning ustring\n"
                  "5. Have a std::to_string(T) overload\n"
                  "6. Have a std::formatter specialization\n"
                  "7. Already have an operator<< for std::ostream");
  }
}
}  // namespace detail

}  // namespace ustd

template<typename T>
  requires stringifiable<T>
std::ostream &operator<<(std::ostream &os, const T &value)
{
  return os << detail::to_string_impl(value);
}

template<typename T>
  requires stringifiable<T> && (!requires { typename std::formatter<T>; })
struct std::formatter<T> {
  constexpr auto parse(std::format_parse_context &ctx)
  {
    return ctx.begin();
  }

  auto format(const T &value, std::format_context &ctx) const
  {
    return std::format_to(ctx.out(), "{}", detail::to_string_impl(value));
  }
};
