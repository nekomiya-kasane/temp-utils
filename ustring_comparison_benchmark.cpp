#include "ustring.h"
#include <benchmark/benchmark.h>
#include <unicode/unistr.h>
#include <string>
#include <random>
#include <array>
#include <algorithm>

namespace test_data {
    // Test data generation utilities
    static std::string generate_ascii(size_t length) {
        static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);
        
        std::string str(length, 0);
        std::generate_n(str.begin(), length, [&]() { return charset[dist(rng)]; });
        return str;
    }

    // Unicode test data
    static const std::array<std::u8string_view, 5> test_strings = {{
        u8"Hello, World!",  // ASCII
        u8"你好，世界！",   // Chinese
        u8"こんにちは、世界！", // Japanese
        u8"안녕하세요, 세계!", // Korean
        u8"Hello, 世界! 🌍" // Mixed with emoji
    }};

    // Large text samples
    static const char8_t* large_mixed_text = u8R"(
Lorem ipsum dolor sit amet, consectetur adipiscing elit.
你好，世界！这是一个测试文本。
こんにちは、世界！これはテストテキストです。
안녕하세요, 세계! 이것은 테스트 텍스트입니다.
Hello, World! 🌍 This is a test text with emojis 😊 and symbols ∑∫≠±
)";

    // Generate text with combining characters
    static std::u8string generate_combining_text(size_t length) {
        static const std::array<char8_t, 5> base_chars = {u8'a', u8'e', u8'i', u8'o', u8'u'};
        static const std::array<char8_t, 3> combining_marks = {u8'\u0301', u8'\u0302', u8'\u0303'};
        static std::mt19937 rng(std::random_device{}());
        
        std::u8string result;
        result.reserve(length * 2);
        
        for (size_t i = 0; i < length; ++i) {
            result += base_chars[i % base_chars.size()];
            result += combining_marks[i % combining_marks.size()];
        }
        return result;
    }
}

// Construction Benchmarks
template<typename T>
static void BM_Construction_Empty(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(T());
    }
}
BENCHMARK_TEMPLATE(BM_Construction_Empty, ustring);
BENCHMARK_TEMPLATE(BM_Construction_Empty, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Construction_Empty, std::u8string);

template<typename T>
static void BM_Construction_ASCII(benchmark::State& state) {
    std::string ascii = test_data::generate_ascii(state.range(0));
    for (auto _ : state) {
        if constexpr (std::is_same_v<T, ustring>) {
            benchmark::DoNotOptimize(T(ascii.c_str()));
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            benchmark::DoNotOptimize(T(ascii.c_str()));
        } else {
            benchmark::DoNotOptimize(T(reinterpret_cast<const char8_t*>(ascii.c_str())));
        }
    }
}
BENCHMARK_TEMPLATE(BM_Construction_ASCII, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Construction_ASCII, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Construction_ASCII, std::u8string)->Range(8, 8<<10);

// Copy Operations
template<typename T>
static void BM_Copy(benchmark::State& state) {
    T str(reinterpret_cast<const char*>(test_data::large_mixed_text));
    for (auto _ : state) {
        benchmark::DoNotOptimize(T(str));
    }
}
BENCHMARK_TEMPLATE(BM_Copy, ustring);
BENCHMARK_TEMPLATE(BM_Copy, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Copy, std::u8string);

// Assignment Operations
template<typename T>
static void BM_Assignment(benchmark::State& state) {
    T source(reinterpret_cast<const char*>(test_data::large_mixed_text));
    T dest;
    for (auto _ : state) {
        dest = source;
        benchmark::DoNotOptimize(dest);
    }
}
BENCHMARK_TEMPLATE(BM_Assignment, ustring);
BENCHMARK_TEMPLATE(BM_Assignment, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Assignment, std::u8string);

// Append Operations
template<typename T>
static void BM_Append(benchmark::State& state) {
    const size_t size = state.range(0);
    std::string append_text = test_data::generate_ascii(size);
    T str;
    
    for (auto _ : state) {
        state.PauseTiming();
        str.clear();
        state.ResumeTiming();
        
        if constexpr (std::is_same_v<T, ustring>) {
            str.append(append_text.c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str.append(icu::UnicodeString(append_text.c_str()));
        } else {
            str.append(reinterpret_cast<const char8_t*>(append_text.c_str()));
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_Append, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Append, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Append, std::u8string)->Range(8, 8<<10);

// Find Operations
template<typename T>
static void BM_Find(benchmark::State& state) {
    T haystack(reinterpret_cast<const char*>(test_data::large_mixed_text));
    T needle(reinterpret_cast<const char*>(u8"世界"));
    
    for (auto _ : state) {
        if constexpr (std::is_same_v<T, ustring>) {
            benchmark::DoNotOptimize(haystack.find(needle));
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            benchmark::DoNotOptimize(haystack.indexOf(needle));
        } else {
            benchmark::DoNotOptimize(haystack.find(needle));
        }
    }
}
BENCHMARK_TEMPLATE(BM_Find, ustring);
BENCHMARK_TEMPLATE(BM_Find, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Find, std::u8string);

// Substring Operations
template<typename T>
static void BM_Substring(benchmark::State& state) {
    T str(reinterpret_cast<const char*>(test_data::large_mixed_text));
    size_t len = str.length() / 2;
    
    for (auto _ : state) {
        if constexpr (std::is_same_v<T, ustring>) {
            benchmark::DoNotOptimize(str.substr(0, len));
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            benchmark::DoNotOptimize(str.tempSubString(0, len));
        } else {
            benchmark::DoNotOptimize(str.substr(0, len));
        }
    }
}
BENCHMARK_TEMPLATE(BM_Substring, ustring);
BENCHMARK_TEMPLATE(BM_Substring, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Substring, std::u8string);

// Compare Operations
template<typename T>
static void BM_Compare(benchmark::State& state) {
    std::vector<T> strings;
    for (const auto& test_str : test_data::test_strings) {
        strings.emplace_back(reinterpret_cast<const char*>(test_str.data()));
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < strings.size(); ++i) {
            for (size_t j = i + 1; j < strings.size(); ++j) {
                if constexpr (std::is_same_v<T, ustring>) {
                    benchmark::DoNotOptimize(strings[i].compare(strings[j]));
                } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                    benchmark::DoNotOptimize(strings[i].compare(strings[j]));
                } else {
                    benchmark::DoNotOptimize(strings[i].compare(strings[j]));
                }
            }
        }
    }
}
BENCHMARK_TEMPLATE(BM_Compare, ustring);
BENCHMARK_TEMPLATE(BM_Compare, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Compare, std::u8string);

// Iteration Performance
template<typename T>
static void BM_Iteration(benchmark::State& state) {
    T str(reinterpret_cast<const char*>(test_data::large_mixed_text));
    
    for (auto _ : state) {
        if constexpr (std::is_same_v<T, ustring>) {
            for (auto it = str.code_points_begin(); it != str.code_points_end(); ++it) {
                benchmark::DoNotOptimize(*it);
            }
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            for (int32_t i = 0; i < str.length(); ++i) {
                benchmark::DoNotOptimize(str.char32At(i));
            }
        } else {
            for (auto ch : str) {
                benchmark::DoNotOptimize(ch);
            }
        }
    }
}
BENCHMARK_TEMPLATE(BM_Iteration, ustring);
BENCHMARK_TEMPLATE(BM_Iteration, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Iteration, std::u8string);

// Memory Usage Patterns
template<typename T>
static void BM_MemoryGrowth(benchmark::State& state) {
    const size_t initial_size = state.range(0);
    std::string append_text = test_data::generate_ascii(100);
    
    for (auto _ : state) {
        T str;
        for (size_t i = 0; i < initial_size; i += 100) {
            if constexpr (std::is_same_v<T, ustring>) {
                str.append(append_text.c_str());
            } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                str.append(icu::UnicodeString(append_text.c_str()));
            } else {
                str.append(reinterpret_cast<const char8_t*>(append_text.c_str()));
            }
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_MemoryGrowth, ustring)->Range(1<<10, 1<<20);
BENCHMARK_TEMPLATE(BM_MemoryGrowth, icu::UnicodeString)->Range(1<<10, 1<<20);
BENCHMARK_TEMPLATE(BM_MemoryGrowth, std::u8string)->Range(1<<10, 1<<20);

// Unicode Normalization (where applicable)
static void BM_Normalize_NFKC(benchmark::State& state) {
    const size_t size = state.range(0);
    std::u8string combining_text = test_data::generate_combining_text(size);
    
    ustring ustr(reinterpret_cast<const char*>(combining_text.c_str()));
    icu::UnicodeString istr(reinterpret_cast<const char*>(combining_text.c_str()));
    
    for (auto _ : state) {
        // ustring normalization
        benchmark::DoNotOptimize(ustr.normalized({Normalization2Mode::NFKC}));
        
        // ICU normalization
        UErrorCode error = U_ZERO_ERROR;
        icu::UnicodeString result;
        benchmark::DoNotOptimize(istr.normalize(UNORM2_NFKC, error, result));
    }
}
BENCHMARK(BM_Normalize_NFKC)->Range(8, 8<<10);

// Mixed Operations Benchmark
template<typename T>
static void BM_MixedOperations(benchmark::State& state) {
    std::vector<std::string> operations = test_data::generate_ascii(state.range(0));
    T str;
    
    for (auto _ : state) {
        state.PauseTiming();
        str.clear();
        state.ResumeTiming();
        
        for (const auto& op : operations) {
            switch (op[0] % 4) {
                case 0: // append
                    if constexpr (std::is_same_v<T, ustring>) {
                        str.append(op.c_str());
                    } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                        str.append(icu::UnicodeString(op.c_str()));
                    } else {
                        str.append(reinterpret_cast<const char8_t*>(op.c_str()));
                    }
                    break;
                case 1: // find
                    if constexpr (std::is_same_v<T, ustring>) {
                        benchmark::DoNotOptimize(str.find('a'));
                    } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                        benchmark::DoNotOptimize(str.indexOf('a'));
                    } else {
                        benchmark::DoNotOptimize(str.find(u8'a'));
                    }
                    break;
                case 2: // substring
                    if (!str.empty()) {
                        size_t len = str.length() / 2;
                        if constexpr (std::is_same_v<T, ustring>) {
                            benchmark::DoNotOptimize(str.substr(0, len));
                        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                            benchmark::DoNotOptimize(str.tempSubString(0, len));
                        } else {
                            benchmark::DoNotOptimize(str.substr(0, len));
                        }
                    }
                    break;
                case 3: // clear
                    str.clear();
                    break;
            }
        }
    }
}
BENCHMARK_TEMPLATE(BM_MixedOperations, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_MixedOperations, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_MixedOperations, std::u8string)->Range(8, 8<<10);

// Insert Operations
template<typename T>
static void BM_Insert_Middle(benchmark::State& state) {
    const size_t size = state.range(0);
    std::string insert_text = test_data::generate_ascii(100);
    T str;
    
    for (auto _ : state) {
        state.PauseTiming();
        if constexpr (std::is_same_v<T, ustring>) {
            str = ustring(test_data::generate_ascii(size).c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str = icu::UnicodeString(test_data::generate_ascii(size).c_str());
        } else {
            str = T(reinterpret_cast<const char8_t*>(test_data::generate_ascii(size).c_str()));
        }
        state.ResumeTiming();
        
        size_t pos = str.length() / 2;
        if constexpr (std::is_same_v<T, ustring>) {
            str.insert(pos, insert_text.c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str.insert(pos, icu::UnicodeString(insert_text.c_str()));
        } else {
            str.insert(pos, reinterpret_cast<const char8_t*>(insert_text.c_str()));
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_Insert_Middle, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Insert_Middle, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Insert_Middle, std::u8string)->Range(8, 8<<10);

// Erase Operations
template<typename T>
static void BM_Erase_Middle(benchmark::State& state) {
    const size_t size = state.range(0);
    
    for (auto _ : state) {
        state.PauseTiming();
        if constexpr (std::is_same_v<T, ustring>) {
            T str(test_data::generate_ascii(size).c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            T str(test_data::generate_ascii(size).c_str());
        } else {
            T str(reinterpret_cast<const char8_t*>(test_data::generate_ascii(size).c_str()));
        }
        state.ResumeTiming();
        
        size_t pos = str.length() / 2;
        size_t len = std::min<size_t>(100, str.length() - pos);
        if constexpr (std::is_same_v<T, ustring>) {
            str.erase(pos, len);
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str.remove(pos, len);
        } else {
            str.erase(pos, len);
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_Erase_Middle, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Erase_Middle, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Erase_Middle, std::u8string)->Range(8, 8<<10);

// Replace Operations
template<typename T>
static void BM_Replace(benchmark::State& state) {
    const size_t size = state.range(0);
    std::string replace_text = test_data::generate_ascii(100);
    
    for (auto _ : state) {
        state.PauseTiming();
        T str;
        if constexpr (std::is_same_v<T, ustring>) {
            str = ustring(test_data::generate_ascii(size).c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str = icu::UnicodeString(test_data::generate_ascii(size).c_str());
        } else {
            str = T(reinterpret_cast<const char8_t*>(test_data::generate_ascii(size).c_str()));
        }
        state.ResumeTiming();
        
        size_t pos = str.length() / 2;
        size_t len = std::min<size_t>(100, str.length() - pos);
        if constexpr (std::is_same_v<T, ustring>) {
            str.replace(pos, len, replace_text.c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str.replace(pos, len, icu::UnicodeString(replace_text.c_str()));
        } else {
            str.replace(pos, len, reinterpret_cast<const char8_t*>(replace_text.c_str()));
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_Replace, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Replace, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Replace, std::u8string)->Range(8, 8<<10);

// Resize Operations
template<typename T>
static void BM_Resize(benchmark::State& state) {
    const size_t initial_size = state.range(0);
    const size_t target_size = initial_size * 2;
    
    for (auto _ : state) {
        T str;
        if constexpr (std::is_same_v<T, ustring>) {
            str = ustring(test_data::generate_ascii(initial_size).c_str());
            str.resize(target_size, 'x');
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str = icu::UnicodeString(test_data::generate_ascii(initial_size).c_str());
            str.padTrailing(target_size, 'x');
        } else {
            str = T(reinterpret_cast<const char8_t*>(test_data::generate_ascii(initial_size).c_str()));
            str.resize(target_size, u8'x');
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_Resize, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Resize, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Resize, std::u8string)->Range(8, 8<<10);

// Find First/Last Not Of Operations
template<typename T>
static void BM_Find_First_Last_Not_Of(benchmark::State& state) {
    T str;
    T chars;
    if constexpr (std::is_same_v<T, ustring>) {
        str = ustring(test_data::large_mixed_text);
        chars = ustring("aeiou");
    } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
        str = icu::UnicodeString(reinterpret_cast<const char*>(test_data::large_mixed_text));
        chars = icu::UnicodeString("aeiou");
    } else {
        str = T(test_data::large_mixed_text);
        chars = T(u8"aeiou");
    }
    
    for (auto _ : state) {
        if constexpr (std::is_same_v<T, ustring>) {
            benchmark::DoNotOptimize(str.find_first_not_of(chars));
            benchmark::DoNotOptimize(str.find_last_not_of(chars));
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            // ICU doesn't have direct equivalents, simulate with find_first_of
            for (int32_t i = 0; i < str.length(); ++i) {
                if (chars.indexOf(str.charAt(i)) == -1) {
                    benchmark::DoNotOptimize(i);
                    break;
                }
            }
            for (int32_t i = str.length() - 1; i >= 0; --i) {
                if (chars.indexOf(str.charAt(i)) == -1) {
                    benchmark::DoNotOptimize(i);
                    break;
                }
            }
        } else {
            benchmark::DoNotOptimize(str.find_first_not_of(chars));
            benchmark::DoNotOptimize(str.find_last_not_of(chars));
        }
    }
}
BENCHMARK_TEMPLATE(BM_Find_First_Last_Not_Of, ustring);
BENCHMARK_TEMPLATE(BM_Find_First_Last_Not_Of, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Find_First_Last_Not_Of, std::u8string);

// Reverse Iterator Operations
template<typename T>
static void BM_Reverse_Iterator(benchmark::State& state) {
    T str;
    if constexpr (std::is_same_v<T, ustring>) {
        str = ustring(test_data::large_mixed_text);
    } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
        str = icu::UnicodeString(reinterpret_cast<const char*>(test_data::large_mixed_text));
    } else {
        str = T(test_data::large_mixed_text);
    }
    
    for (auto _ : state) {
        if constexpr (std::is_same_v<T, ustring>) {
            for (auto it = str.rbegin(); it != str.rend(); ++it) {
                benchmark::DoNotOptimize(*it);
            }
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            for (int32_t i = str.length() - 1; i >= 0; --i) {
                benchmark::DoNotOptimize(str.charAt(i));
            }
        } else {
            for (auto it = str.rbegin(); it != str.rend(); ++it) {
                benchmark::DoNotOptimize(*it);
            }
        }
    }
}
BENCHMARK_TEMPLATE(BM_Reverse_Iterator, ustring);
BENCHMARK_TEMPLATE(BM_Reverse_Iterator, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Reverse_Iterator, std::u8string);

// Capacity Operations
template<typename T>
static void BM_Capacity_Operations(benchmark::State& state) {
    const size_t size = state.range(0);
    
    for (auto _ : state) {
        T str;
        if constexpr (std::is_same_v<T, ustring>) {
            str = ustring(test_data::generate_ascii(size).c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            str = icu::UnicodeString(test_data::generate_ascii(size).c_str());
        } else {
            str = T(reinterpret_cast<const char8_t*>(test_data::generate_ascii(size).c_str()));
        }
        
        benchmark::DoNotOptimize(str.length());
        benchmark::DoNotOptimize(str.empty());
        if constexpr (std::is_same_v<T, ustring> || std::is_same_v<T, std::u8string>) {
            benchmark::DoNotOptimize(str.capacity());
        }
        
        str.clear();
        if constexpr (std::is_same_v<T, ustring> || std::is_same_v<T, std::u8string>) {
            str.shrink_to_fit();
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK_TEMPLATE(BM_Capacity_Operations, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Capacity_Operations, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Capacity_Operations, std::u8string)->Range(8, 8<<10);

// Element Access Operations
template<typename T>
static void BM_Element_Access(benchmark::State& state) {
    T str;
    if constexpr (std::is_same_v<T, ustring>) {
        str = ustring(test_data::large_mixed_text);
    } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
        str = icu::UnicodeString(reinterpret_cast<const char*>(test_data::large_mixed_text));
    } else {
        str = T(test_data::large_mixed_text);
    }
    
    const size_t len = str.length();
    std::vector<size_t> positions;
    for (size_t i = 0; i < len; i += len/10) {
        positions.push_back(i);
    }
    
    for (auto _ : state) {
        for (size_t pos : positions) {
            if constexpr (std::is_same_v<T, ustring>) {
                benchmark::DoNotOptimize(str[pos]);
                benchmark::DoNotOptimize(str.at(pos));
            } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                benchmark::DoNotOptimize(str.charAt(pos));
            } else {
                benchmark::DoNotOptimize(str[pos]);
                benchmark::DoNotOptimize(str.at(pos));
            }
        }
    }
}
BENCHMARK_TEMPLATE(BM_Element_Access, ustring);
BENCHMARK_TEMPLATE(BM_Element_Access, icu::UnicodeString);
BENCHMARK_TEMPLATE(BM_Element_Access, std::u8string);

// String Concatenation Operations
template<typename T>
static void BM_Concatenation(benchmark::State& state) {
    const size_t size = state.range(0);
    std::vector<T> strings;
    
    state.PauseTiming();
    for (size_t i = 0; i < 10; ++i) {
        if constexpr (std::is_same_v<T, ustring>) {
            strings.emplace_back(test_data::generate_ascii(size/10).c_str());
        } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
            strings.emplace_back(test_data::generate_ascii(size/10).c_str());
        } else {
            strings.emplace_back(reinterpret_cast<const char8_t*>(test_data::generate_ascii(size/10).c_str()));
        }
    }
    state.ResumeTiming();
    
    for (auto _ : state) {
        T result;
        for (const auto& s : strings) {
            if constexpr (std::is_same_v<T, ustring>) {
                result.append(s);
            } else if constexpr (std::is_same_v<T, icu::UnicodeString>) {
                result.append(s);
            } else {
                result.append(s);
            }
        }
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK_TEMPLATE(BM_Concatenation, ustring)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Concatenation, icu::UnicodeString)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_Concatenation, std::u8string)->Range(8, 8<<10);

BENCHMARK_MAIN();
