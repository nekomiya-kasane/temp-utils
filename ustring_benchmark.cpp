#include "ustring.h"
#include <benchmark/benchmark.h>
#include <random>

// Test data setup
static const char* const small_ascii = "Hello, World!";
static const char* const large_ascii = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. 
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.)";

static const char* const small_utf8 = "Hello, 世界! 🌍";
static const char* const large_utf8 = R"(English: Hello World! 
Chinese: 你好，世界！
Japanese: こんにちは、世界！
Korean: 안녕하세요, 세계!
Emoji: 😀🌍🎉🚀💻
Math: ∑∫≠±×÷√∞
Mixed: Hello世界!😀Testing混合文本)";

// Helper function to generate random string
static std::string generate_random_string(size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);
    
    std::string str(length, 0);
    for (size_t i = 0; i < length; ++i) {
        str[i] = charset[dist(rng)];
    }
    return str;
}

// Construction Benchmarks
static void BM_DefaultConstruction(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(ustring());
    }
}
BENCHMARK(BM_DefaultConstruction);

static void BM_CStringConstruction(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(ustring(small_ascii));
    }
}
BENCHMARK(BM_CStringConstruction);

static void BM_CopyConstruction(benchmark::State& state) {
    ustring str(large_ascii);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ustring(str));
    }
}
BENCHMARK(BM_CopyConstruction);

static void BM_MoveConstruction(benchmark::State& state) {
    for (auto _ : state) {
        ustring temp(large_ascii);
        benchmark::DoNotOptimize(ustring(std::move(temp)));
    }
}
BENCHMARK(BM_MoveConstruction);

// Assignment Benchmarks
static void BM_CopyAssignment(benchmark::State& state) {
    ustring source(large_ascii);
    ustring dest;
    for (auto _ : state) {
        dest = source;
        benchmark::DoNotOptimize(dest);
    }
}
BENCHMARK(BM_CopyAssignment);

static void BM_MoveAssignment(benchmark::State& state) {
    ustring dest;
    for (auto _ : state) {
        ustring temp(large_ascii);
        dest = std::move(temp);
        benchmark::DoNotOptimize(dest);
    }
}
BENCHMARK(BM_MoveAssignment);

// Element Access Benchmarks
static void BM_At(benchmark::State& state) {
    ustring str(large_ascii);
    size_t pos = str.size() / 2;
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.at(pos));
    }
}
BENCHMARK(BM_At);

static void BM_OperatorBracket(benchmark::State& state) {
    ustring str(large_ascii);
    size_t pos = str.size() / 2;
    for (auto _ : state) {
        benchmark::DoNotOptimize(str[pos]);
    }
}
BENCHMARK(BM_OperatorBracket);

static void BM_Front(benchmark::State& state) {
    ustring str(large_ascii);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.front());
    }
}
BENCHMARK(BM_Front);

static void BM_Back(benchmark::State& state) {
    ustring str(large_ascii);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.back());
    }
}
BENCHMARK(BM_Back);

// Iterator Benchmarks
static void BM_Iterator_Forward(benchmark::State& state) {
    ustring str(large_ascii);
    for (auto _ : state) {
        for (auto it = str.begin(); it != str.end(); ++it) {
            benchmark::DoNotOptimize(*it);
        }
    }
}
BENCHMARK(BM_Iterator_Forward);

static void BM_Iterator_Reverse(benchmark::State& state) {
    ustring str(large_ascii);
    for (auto _ : state) {
        for (auto it = str.rbegin(); it != str.rend(); ++it) {
            benchmark::DoNotOptimize(*it);
        }
    }
}
BENCHMARK(BM_Iterator_Reverse);

// Capacity Benchmarks
static void BM_Reserve(benchmark::State& state) {
    for (auto _ : state) {
        ustring str;
        str.reserve(1000);
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Reserve);

static void BM_ShrinkToFit(benchmark::State& state) {
    for (auto _ : state) {
        ustring str(large_ascii);
        str.reserve(str.size() * 2);
        str.shrink_to_fit();
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_ShrinkToFit);

// Modification Benchmarks
static void BM_Clear(benchmark::State& state) {
    for (auto _ : state) {
        ustring str(large_ascii);
        str.clear();
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Clear);

static void BM_Insert(benchmark::State& state) {
    ustring str(small_ascii);
    ustring insert_str("INSERT");
    for (auto _ : state) {
        state.PauseTiming();
        str = small_ascii;
        state.ResumeTiming();
        str.insert(5, insert_str);
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Insert);

static void BM_Erase(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        ustring str(large_ascii);
        state.ResumeTiming();
        str.erase(10, 20);
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Erase);

static void BM_PushBack(benchmark::State& state) {
    for (auto _ : state) {
        ustring str;
        for (char c = 'a'; c <= 'z'; ++c) {
            str.push_back(c);
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_PushBack);

static void BM_PopBack(benchmark::State& state) {
    for (auto _ : state) {
        state.PauseTiming();
        ustring str(small_ascii);
        state.ResumeTiming();
        while (!str.empty()) {
            str.pop_back();
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_PopBack);

// String Operations Benchmarks
static void BM_Append(benchmark::State& state) {
    ustring str(small_ascii);
    ustring append_str("APPEND");
    for (auto _ : state) {
        state.PauseTiming();
        str = small_ascii;
        state.ResumeTiming();
        str.append(append_str);
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Append);

static void BM_Compare(benchmark::State& state) {
    ustring str1(small_ascii);
    ustring str2(small_ascii);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str1.compare(str2));
    }
}
BENCHMARK(BM_Compare);

static void BM_Substr(benchmark::State& state) {
    ustring str(large_ascii);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.substr(10, 20));
    }
}
BENCHMARK(BM_Substr);

static void BM_Copy(benchmark::State& state) {
    ustring str(small_ascii);
    char buffer[100];
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.copy(buffer, str.size()));
    }
}
BENCHMARK(BM_Copy);

// Search Benchmarks
static void BM_Find(benchmark::State& state) {
    ustring str(large_ascii);
    ustring search("ipsum");
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.find(search));
    }
}
BENCHMARK(BM_Find);

static void BM_RFind(benchmark::State& state) {
    ustring str(large_ascii);
    ustring search("ipsum");
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.rfind(search));
    }
}
BENCHMARK(BM_RFind);

static void BM_FindFirstOf(benchmark::State& state) {
    ustring str(large_ascii);
    ustring chars("aeiou");
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.find_first_of(chars));
    }
}
BENCHMARK(BM_FindFirstOf);

static void BM_FindLastOf(benchmark::State& state) {
    ustring str(large_ascii);
    ustring chars("aeiou");
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.find_last_of(chars));
    }
}
BENCHMARK(BM_FindLastOf);

// UTF-8 Specific Benchmarks
static void BM_UTF8_Construction(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(ustring(large_utf8));
    }
}
BENCHMARK(BM_UTF8_Construction);

static void BM_UTF8_Iteration(benchmark::State& state) {
    ustring str(large_utf8);
    for (auto _ : state) {
        for (auto it = str.code_points_begin(); it != str.code_points_end(); ++it) {
            benchmark::DoNotOptimize(*it);
        }
    }
}
BENCHMARK(BM_UTF8_Iteration);

// Range-based Benchmarks
static void BM_Construction_Range(benchmark::State& state) {
    const int N = state.range(0);
    std::string input = generate_random_string(N);
    for (auto _ : state) {
        benchmark::DoNotOptimize(ustring(input));
    }
}
BENCHMARK(BM_Construction_Range)->Range(8, 8<<10);

static void BM_Find_Range(benchmark::State& state) {
    const int N = state.range(0);
    std::string input = generate_random_string(N);
    ustring str(input);
    ustring pattern("pattern");
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.find(pattern));
    }
}
BENCHMARK(BM_Find_Range)->Range(8, 8<<10);

static void BM_Append_Range(benchmark::State& state) {
    const int N = state.range(0);
    std::string input = generate_random_string(N);
    ustring append_str(input);
    for (auto _ : state) {
        ustring str;
        str.append(append_str);
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_Append_Range)->Range(8, 8<<10);

// Normalization Benchmarks
static void BM_Normalize_NFC(benchmark::State& state) {
    ustring str(large_utf8);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFC}));
    }
}
BENCHMARK(BM_Normalize_NFC);

static void BM_Normalize_NFD(benchmark::State& state) {
    ustring str(large_utf8);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFD}));
    }
}
BENCHMARK(BM_Normalize_NFD);

static void BM_Normalize_NFKC(benchmark::State& state) {
    ustring str(large_utf8);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFKC}));
    }
}
BENCHMARK(BM_Normalize_NFKC);

static void BM_Normalize_NFKD(benchmark::State& state) {
    ustring str(large_utf8);
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFKD}));
    }
}
BENCHMARK(BM_Normalize_NFKD);

BENCHMARK_MAIN();
