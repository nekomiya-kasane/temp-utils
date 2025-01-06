#include "ustring.h"
#include <benchmark/benchmark.h>

// Benchmark data setup
static const std::u8string empty_str = u8"";
static const std::u8string ascii_str = u8"Hello, World! This is a test string with numbers 123 and symbols !@#";
static const std::u8string chinese_str = u8"这是一个包含中文字符的测试字符串";
static const std::u8string emoji_str = u8"😀🌍🎉🚀💻🎮🎨🎭";
static const std::u8string mixed_str = u8"Hello世界!😀 Testing混合文本 with emojis🌍";
static const std::u8string math_str = u8"∑∫≠±×÷√∞∝∂∇∈∉∋∌∏∐∑∏∐";
static const std::u8string combining_str = u8"éèêëāīūēōǖ";
static const std::u8string long_text = u8R"(
Lorem ipsum dolor sit amet, consectetur adipiscing elit. 你好，世界！
Testing various characters: αβγδ 1234567890 !@#$%^&*
Emoji test: 😀🌍🎉 Mathematical symbols: ∑∫≠±
Combined text with ASCII, CJK汉字, Emoji😊, and Math∞
)";

// Benchmark has_property with different properties and string types
static void BM_HasProperty_Empty(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(empty_str, CharProperty::ALPHABETIC));
    }
}
BENCHMARK(BM_HasProperty_Empty);

static void BM_HasProperty_ASCII(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(ascii_str, CharProperty::ALPHABETIC));
    }
}
BENCHMARK(BM_HasProperty_ASCII);

static void BM_HasProperty_Chinese(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(chinese_str, CharProperty::IDEOGRAPHIC));
    }
}
BENCHMARK(BM_HasProperty_Chinese);

static void BM_HasProperty_Emoji(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(emoji_str, CharProperty::EMOJI));
    }
}
BENCHMARK(BM_HasProperty_Emoji);

static void BM_HasProperty_Math(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(math_str, CharProperty::MATH));
    }
}
BENCHMARK(BM_HasProperty_Math);

static void BM_HasProperty_Combining(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(combining_str, CharProperty::COMBINING_MARK));
    }
}
BENCHMARK(BM_HasProperty_Combining);

// Benchmark get_property with different character types
static void BM_GetProperty_ASCII(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(get_property(u8"A"));
    }
}
BENCHMARK(BM_GetProperty_ASCII);

static void BM_GetProperty_Chinese(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(get_property(u8"世"));
    }
}
BENCHMARK(BM_GetProperty_Chinese);

static void BM_GetProperty_Emoji(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(get_property(u8"😀"));
    }
}
BENCHMARK(BM_GetProperty_Emoji);

static void BM_GetProperty_Math(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(get_property(u8"∑"));
    }
}
BENCHMARK(BM_GetProperty_Math);

// Benchmark codepoint conversion with different character types
static void BM_ToCodepoint_ASCII(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(to_codepoint(u8"A"));
    }
}
BENCHMARK(BM_ToCodepoint_ASCII);

static void BM_ToCodepoint_Chinese(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(to_codepoint(u8"世"));
    }
}
BENCHMARK(BM_ToCodepoint_Chinese);

static void BM_ToCodepoint_Emoji(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(to_codepoint(u8"😀"));
    }
}
BENCHMARK(BM_ToCodepoint_Emoji);

// Benchmark string operations with properties
static void BM_String_Contains_Property(benchmark::State& state) {
    ustring str(reinterpret_cast<const char*>(long_text.c_str()));
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.contains(CharProperty::IDEOGRAPHIC));
    }
}
BENCHMARK(BM_String_Contains_Property);

static void BM_String_Count_Codepoint(benchmark::State& state) {
    ustring str(reinterpret_cast<const char*>(long_text.c_str()));
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.count(U'世'));
    }
}
BENCHMARK(BM_String_Count_Codepoint);

// Benchmark property checks on long text
static void BM_LongText_HasProperty(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(long_text, CharProperty::ALPHABETIC));
    }
}
BENCHMARK(BM_LongText_HasProperty);

static void BM_LongText_GetProperty(benchmark::State& state) {
    for (auto _ : state) {
        for (size_t i = 0; i < long_text.size(); i++) {
            benchmark::DoNotOptimize(get_property(std::u8string_view(&long_text[i], 1)));
        }
    }
}
BENCHMARK(BM_LongText_GetProperty);

// Benchmark different property combinations
static void BM_Property_Combinations(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(mixed_str, 
            static_cast<CharProperty>(
                static_cast<int>(CharProperty::ALPHABETIC) | 
                static_cast<int>(CharProperty::IDEOGRAPHIC) |
                static_cast<int>(CharProperty::EMOJI)
            )
        ));
    }
}
BENCHMARK(BM_Property_Combinations);

// Benchmark code point iteration
static void BM_CodePoint_Iteration(benchmark::State& state) {
    ustring str(reinterpret_cast<const char*>(long_text.c_str()));
    for (auto _ : state) {
        for (auto it = str.code_points_begin(); it != str.code_points_end(); ++it) {
            benchmark::DoNotOptimize(*it);
        }
    }
}
BENCHMARK(BM_CodePoint_Iteration);

// Range-based benchmarks with different sizes
static void BM_Property_Check_Range(benchmark::State& state) {
    const int N = state.range(0);
    std::u8string test_str;
    test_str.reserve(N);
    for (int i = 0; i < N; ++i) {
        test_str += u8"A世😀∑é"[i % 5];
    }
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(has_property(test_str, CharProperty::ALPHABETIC));
    }
}
BENCHMARK(BM_Property_Check_Range)->Range(8, 8<<10);

static void BM_CodePoint_Conversion_Range(benchmark::State& state) {
    const int N = state.range(0);
    std::vector<std::u8string> test_strings;
    test_strings.reserve(N);
    for (int i = 0; i < N; ++i) {
        test_strings.push_back(std::u8string(1, u8"A世😀∑é"[i % 5]));
    }
    
    for (auto _ : state) {
        for (const auto& s : test_strings) {
            benchmark::DoNotOptimize(to_codepoint(s));
        }
    }
}
BENCHMARK(BM_CodePoint_Conversion_Range)->Range(8, 8<<10);

BENCHMARK_MAIN();
