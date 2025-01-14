#pragma once

#include <concepts>
#include <format>
#include <ranges>
#include <string_view>
#include <type_traits>

#include "basic_concepts.h"

struct format_options {
  std::string_view prefix = "[";
  std::string_view suffix = "]";
  std::string_view separator = ", ";
  std::string_view empty_range = "[]";
};

template<typename Range, typename CharT>
  requires traits::is_ranged<Range>
class std::formatter<Range, CharT> {
 public:
  constexpr auto parse(std::format_parse_context &ctx)
  {
    auto it = ctx.begin();
    auto end = ctx.end();

    while (it != end && *it != '}') {
      switch (*it) {
        case 'p':
          ++it;
          if (it != end) {
            switch (*it) {
              case '(':
                options.prefix = "(";
                options.suffix = ")";
                break;
              case '{':
                options.prefix = "{";
                options.suffix = "}";
                break;
              case '<':
                options.prefix = "<";
                options.suffix = ">";
                break;
              case '[':
                options.prefix = "[";
                options.suffix = "]";
                break;
              default:
                --it;
                break;
            }
          }
          break;
        case 's':
          ++it;
          if (it != end) {
            switch (*it) {
              case ',':
                options.separator = ", ";
                break;
              case ';':
                options.separator = "; ";
                break;
              case '|':
                options.separator = " | ";
                break;
              case ' ':
                options.separator = " ";
                break;
              default:
                --it;
                break;
            }
          }
          break;
        case 'e':
          ++it;
          if (it != end) {
            switch (*it) {
              case '0':
                options.empty_range = "";
                break;
              case 'e':
                options.empty_range = "empty";
                break;
              case 'n':
                options.empty_range = "null";
                break;
              default:
                --it;
                break;
            }
          }
          break;
        default: ;
      }
      ++it;
    }
    return it;
  }

  template<typename FormatContext> auto format(const Range &range, FormatContext &ctx) const
  {
    auto out = ctx.out();

    if (std::ranges::empty(range)) {
      return std::ranges::copy(options.empty_range, out).out;
    }

    out = std::ranges::copy(options.prefix, out).out;

    auto it = std::ranges::begin(range);
    auto end = std::ranges::end(range);

    if (it != end) {
      if constexpr (traits::is_formattable<std::ranges::range_value_t<Range>>) {
        out = std::format_to(out, "{}", *it);
        ++it;

        while (it != end) {
          out = std::ranges::copy(options.separator, out).out;
          out = std::format_to(out, "{}", *it);
          ++it;
        }
      }
      else {
        // 对于不可格式化的类型，使用类型名称
        out = std::format_to(out,
                             "{}@{:p}",
                             typeid(std::ranges::range_value_t<Range>).name(),
                             static_cast<const void *>(&(*it)));
        ++it;

        while (it != end) {
          out = std::ranges::copy(options.separator, out).out;
          out = std::format_to(out,
                               "{}@{:p}",
                               typeid(std::ranges::range_value_t<Range>).name(),
                               static_cast<const void *>(&(*it)));
          ++it;
        }
      }
    }

    return std::ranges::copy(options.suffix, out).out;
  }

 protected:
  format_options options;
};
//
//// 特化 std::formatter 为我们的 range_formatter
// template<typename T>
//   requires std::ranges::range<T>
// struct std::formatter<T> : ustd::range_formatter<T> {};
//
//// 为 small_vector 特化 formatter
// template<typename T, size_t N, typename Alloc>
// struct std::formatter<small_vector<T, N, Alloc>>
//     : ustd::range_formatter<ustd::small_vector<T, N, Alloc>> {
//
//   // 添加特定于 small_vector 的格式化选项
//   constexpr auto parse(std::format_parse_context &ctx)
//   {
//     auto it = ctx.begin();
//     auto end = ctx.end();
//
//     // 首先调用基类的解析
//     it = ustd::range_formatter<ustd::small_vector<T, N, Alloc>>::parse(ctx);
//
//     // 解析 small_vector 特定的格式选项
//     while (it != end && *it != '}') {
//       switch (*it) {
//         case 'i':  // 显示内联存储信息
//           ++it;
//           show_inline_info = true;
//           break;
//         case 'c':  // 显示容量信息
//           ++it;
//           show_capacity = true;
//           break;
//       }
//       if (it != end)
//         ++it;
//     }
//     return it;
//   }
//
//   template<typename FormatContext>
//   auto format(const ustd::small_vector<T, N, Alloc> &vec, FormatContext &ctx) const
//   {
//     auto out = ctx.out();
//
//     // 如果需要显示额外信息，添加前缀
//     if (show_inline_info || show_capacity) {
//       out = std::format_to(out, "small_vector(");
//       if (show_inline_info) {
//         out = std::format_to(out, "inline={},", vec.is_using_inline_storage());
//       }
//       if (show_capacity) {
//         out = std::format_to(out, "cap={},", vec.capacity());
//       }
//       out = std::format_to(out, "size={})", vec.size());
//     }
//
//     // 调用基类的格式化
//     return ustd::range_formatter<ustd::small_vector<T, N, Alloc>>::format(vec, ctx);
//   }
//
//  private:
//   bool show_inline_info = false;
//   bool show_capacity = false;
// };
