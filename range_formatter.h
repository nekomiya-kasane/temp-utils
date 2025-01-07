#pragma once

#include <format>
#include <ranges>
#include <string_view>
#include <concepts>
#include <type_traits>

namespace ustd {

// 用于自定义格式化选项的结构体
struct format_options {
    std::string_view prefix = "[";      // 范围的前缀
    std::string_view suffix = "]";      // 范围的后缀
    std::string_view separator = ", ";   // 元素之间的分隔符
    std::string_view empty_range = "[]"; // 空范围的表示
};

// 基础的范围格式化器
template<typename Range>
class range_formatter {
public:
    // 检查类型是否可格式化
    template<typename T>
    static constexpr bool is_formattable = 
        requires(std::format_context ctx, T value) {
            std::formatter<std::remove_cvref_t<T>>{}.format(value, ctx);
        };

    // 解析格式说明符
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();

        // 解析自定义格式选项
        while (it != end && *it != '}') {
            switch (*it) {
                case 'p': // 改变前缀/后缀样式
                    ++it;
                    if (it != end) {
                        switch (*it) {
                            case '(': options.prefix = "("; options.suffix = ")"; break;
                            case '{': options.prefix = "{"; options.suffix = "}"; break;
                            case '<': options.prefix = "<"; options.suffix = ">"; break;
                            case '[': options.prefix = "["; options.suffix = "]"; break;
                            default: --it; break;
                        }
                    }
                    break;
                case 's': // 改变分隔符
                    ++it;
                    if (it != end) {
                        switch (*it) {
                            case ',': options.separator = ", "; break;
                            case ';': options.separator = "; "; break;
                            case '|': options.separator = " | "; break;
                            case ' ': options.separator = " "; break;
                            default: --it; break;
                        }
                    }
                    break;
                case 'e': // 改变空范围表示
                    ++it;
                    if (it != end) {
                        switch (*it) {
                            case '0': options.empty_range = ""; break;
                            case 'e': options.empty_range = "empty"; break;
                            case 'n': options.empty_range = "null"; break;
                            default: --it; break;
                        }
                    }
                    break;
            }
            ++it;
        }
        return it;
    }

    // 格式化范围
    template<typename FormatContext>
    auto format(const Range& range, FormatContext& ctx) const {
        auto out = ctx.out();

        // 处理空范围
        if (std::ranges::empty(range)) {
            return std::ranges::copy(options.empty_range, out).out;
        }

        // 输出前缀
        out = std::ranges::copy(options.prefix, out).out;

        // 输出元素
        auto it = std::ranges::begin(range);
        auto end = std::ranges::end(range);

        if (it != end) {
            if constexpr (is_formattable<std::ranges::range_value_t<Range>>) {
                out = std::format_to(out, "{}", *it);
                ++it;

                while (it != end) {
                    out = std::ranges::copy(options.separator, out).out;
                    out = std::format_to(out, "{}", *it);
                    ++it;
                }
            } else {
                // 对于不可格式化的类型，使用类型名称
                out = std::format_to(out, "{}@{:p}", 
                    typeid(std::ranges::range_value_t<Range>).name(),
                    static_cast<const void*>(&(*it)));
                ++it;

                while (it != end) {
                    out = std::ranges::copy(options.separator, out).out;
                    out = std::format_to(out, "{}@{:p}",
                        typeid(std::ranges::range_value_t<Range>).name(),
                        static_cast<const void*>(&(*it)));
                    ++it;
                }
            }
        }

        // 输出后缀
        return std::ranges::copy(options.suffix, out).out;
    }

protected:
    format_options options;
};

// 特化 std::formatter 为我们的 range_formatter
template<typename T>
requires std::ranges::range<T>
struct std::formatter<T> : ustd::range_formatter<T> {};

// 为 small_vector 特化 formatter
template<typename T, size_t N, typename Alloc>
struct std::formatter<ustd::small_vector<T, N, Alloc>> 
    : ustd::range_formatter<ustd::small_vector<T, N, Alloc>> {
    
    // 添加特定于 small_vector 的格式化选项
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        auto end = ctx.end();

        // 首先调用基类的解析
        it = ustd::range_formatter<ustd::small_vector<T, N, Alloc>>::parse(ctx);

        // 解析 small_vector 特定的格式选项
        while (it != end && *it != '}') {
            switch (*it) {
                case 'i': // 显示内联存储信息
                    ++it;
                    show_inline_info = true;
                    break;
                case 'c': // 显示容量信息
                    ++it;
                    show_capacity = true;
                    break;
            }
            if (it != end) ++it;
        }
        return it;
    }

    template<typename FormatContext>
    auto format(const ustd::small_vector<T, N, Alloc>& vec, FormatContext& ctx) const {
        auto out = ctx.out();

        // 如果需要显示额外信息，添加前缀
        if (show_inline_info || show_capacity) {
            out = std::format_to(out, "small_vector(");
            if (show_inline_info) {
                out = std::format_to(out, "inline={},", vec.is_using_inline_storage());
            }
            if (show_capacity) {
                out = std::format_to(out, "cap={},", vec.capacity());
            }
            out = std::format_to(out, "size={})", vec.size());
        }

        // 调用基类的格式化
        return ustd::range_formatter<ustd::small_vector<T, N, Alloc>>::format(vec, ctx);
    }

private:
    bool show_inline_info = false;
    bool show_capacity = false;
};

} // namespace ustd
