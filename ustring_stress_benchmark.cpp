#include "ustring.h"
#include <benchmark/benchmark.h>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>

// Advanced test data generation
namespace test_data {
    // Unicode ranges for different scripts
    struct UnicodeRange {
        char32_t start;
        char32_t end;
        const char* name;
    };

    static const UnicodeRange unicode_ranges[] = {
        {0x0020, 0x007F, "Basic Latin"},
        {0x3000, 0x303F, "CJK Symbols"},
        {0x4E00, 0x9FFF, "CJK Unified Ideographs"},
        {0x1F300, 0x1F9FF, "Emoji"},
        {0x0900, 0x097F, "Devanagari"},
        {0x0600, 0x06FF, "Arabic"},
        {0x0400, 0x04FF, "Cyrillic"},
        {0x0370, 0x03FF, "Greek"},
        {0x0E00, 0x0E7F, "Thai"}
    };

    // Generate a string with characters from multiple Unicode ranges
    static std::u32string generate_mixed_unicode(size_t length) {
        static std::mt19937 rng(std::random_device{}());
        std::u32string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; ++i) {
            const auto& range = unicode_ranges[i % std::size(unicode_ranges)];
            std::uniform_int_distribution<char32_t> dist(range.start, range.end);
            result.push_back(dist(rng));
        }
        return result;
    }

    // Generate a string with random combining characters
    static std::u32string generate_combining_chars(size_t length) {
        static std::mt19937 rng(std::random_device{}());
        static const std::vector<char32_t> combining_marks = {
            0x0300, 0x0301, 0x0302, 0x0303, 0x0304, 0x0305, // Combining diacritical marks
            0x0901, 0x0902, 0x0903, // Devanagari marks
            0x3099, 0x309A  // Japanese marks
        };
        
        std::u32string result;
        result.reserve(length * 2); // Reserve extra space for combining marks
        
        for (size_t i = 0; i < length; ++i) {
            // Add base character
            result.push_back(U'a' + (i % 26));
            
            // Add 0-3 combining marks
            std::uniform_int_distribution<size_t> num_marks_dist(0, 3);
            size_t num_marks = num_marks_dist(rng);
            
            for (size_t j = 0; j < num_marks; ++j) {
                std::uniform_int_distribution<size_t> mark_dist(0, combining_marks.size() - 1);
                result.push_back(combining_marks[mark_dist(rng)]);
            }
        }
        return result;
    }
}

// Stress Test Benchmarks
static void BM_StressTest_LargeString_Construction(benchmark::State& state) {
    const size_t size = state.range(0);
    auto large_unicode = test_data::generate_mixed_unicode(size);
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(ustring(reinterpret_cast<const char*>(large_unicode.c_str())));
    }
}
BENCHMARK(BM_StressTest_LargeString_Construction)
    ->RangeMultiplier(4)
    ->Range(1<<10, 1<<20); // Test from 1KB to 1MB

static void BM_StressTest_ConcurrentAccess(benchmark::State& state) {
    const int num_threads = state.range(0);
    ustring shared_str(test_data::generate_mixed_unicode(1000).c_str());
    
    for (auto _ : state) {
        std::vector<std::thread> threads;
        threads.reserve(num_threads);
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&shared_str]() {
                for (int j = 0; j < 1000; ++j) {
                    benchmark::DoNotOptimize(shared_str.size());
                    benchmark::DoNotOptimize(shared_str.capacity());
                    benchmark::DoNotOptimize(shared_str.empty());
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
    }
}
BENCHMARK(BM_StressTest_ConcurrentAccess)
    ->RangeMultiplier(2)
    ->Range(2, 32); // Test with 2 to 32 threads

static void BM_StressTest_RandomAccess(benchmark::State& state) {
    const size_t size = state.range(0);
    ustring str(test_data::generate_mixed_unicode(size).c_str());
    std::vector<size_t> random_positions(1000);
    
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, str.size() - 1);
    std::generate(random_positions.begin(), random_positions.end(),
                 [&]() { return dist(rng); });
    
    for (auto _ : state) {
        for (size_t pos : random_positions) {
            benchmark::DoNotOptimize(str[pos]);
        }
    }
}
BENCHMARK(BM_StressTest_RandomAccess)
    ->RangeMultiplier(4)
    ->Range(1<<10, 1<<20);

static void BM_StressTest_IntensiveModification(benchmark::State& state) {
    const size_t initial_size = state.range(0);
    const size_t num_operations = 1000;
    
    for (auto _ : state) {
        state.PauseTiming();
        ustring str(test_data::generate_mixed_unicode(initial_size).c_str());
        std::mt19937 rng(std::random_device{}());
        state.ResumeTiming();
        
        for (size_t i = 0; i < num_operations; ++i) {
            size_t op = i % 4;
            size_t pos = rng() % (str.size() + 1);
            
            switch (op) {
                case 0: // insert
                    str.insert(pos, "test");
                    break;
                case 1: // erase
                    if (pos < str.size()) {
                        str.erase(pos, 1);
                    }
                    break;
                case 2: // append
                    str.append("test");
                    break;
                case 3: // replace
                    if (pos < str.size()) {
                        str.replace(pos, 1, "new");
                    }
                    break;
            }
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_StressTest_IntensiveModification)
    ->RangeMultiplier(4)
    ->Range(1<<10, 1<<18);

// Memory Stress Tests
static void BM_StressTest_MemoryReallocation(benchmark::State& state) {
    const size_t num_operations = state.range(0);
    
    for (auto _ : state) {
        ustring str;
        for (size_t i = 0; i < num_operations; ++i) {
            str.reserve(str.capacity() * 2 + 1);
            str.append(test_data::generate_mixed_unicode(100).c_str());
            if (i % 2 == 0) {
                str.shrink_to_fit();
            }
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_StressTest_MemoryReallocation)
    ->RangeMultiplier(2)
    ->Range(1<<4, 1<<12);

// Unicode Normalization Stress Tests
static void BM_StressTest_ComplexNormalization(benchmark::State& state) {
    const size_t size = state.range(0);
    auto combining_str = test_data::generate_combining_chars(size);
    ustring str(reinterpret_cast<const char*>(combining_str.c_str()));
    
    for (auto _ : state) {
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFKC}));
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFKD}));
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFC}));
        benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFD}));
    }
}
BENCHMARK(BM_StressTest_ComplexNormalization)
    ->RangeMultiplier(4)
    ->Range(1<<8, 1<<16);

// Iterator Stress Tests
static void BM_StressTest_IteratorOperations(benchmark::State& state) {
    const size_t size = state.range(0);
    ustring str(test_data::generate_mixed_unicode(size).c_str());
    
    for (auto _ : state) {
        auto it = str.begin();
        auto end = str.end();
        while (it != end) {
            benchmark::DoNotOptimize(*it);
            auto next = it;
            ++next;
            if (next != end) {
                it = next;
            } else {
                break;
            }
        }
    }
}
BENCHMARK(BM_StressTest_IteratorOperations)
    ->RangeMultiplier(4)
    ->Range(1<<10, 1<<18);

// Search Pattern Stress Tests
static void BM_StressTest_ComplexSearch(benchmark::State& state) {
    const size_t size = state.range(0);
    ustring haystack(test_data::generate_mixed_unicode(size).c_str());
    ustring needle(test_data::generate_mixed_unicode(size/100).c_str());
    
    for (auto _ : state) {
        size_t pos = 0;
        while ((pos = haystack.find(needle, pos)) != ustring::npos) {
            benchmark::DoNotOptimize(pos);
            pos++;
        }
    }
}
BENCHMARK(BM_StressTest_ComplexSearch)
    ->RangeMultiplier(4)
    ->Range(1<<10, 1<<18);

// Comparison Stress Tests
static void BM_StressTest_Comparison(benchmark::State& state) {
    const size_t size = state.range(0);
    std::vector<ustring> strings;
    for (int i = 0; i < 100; ++i) {
        strings.emplace_back(test_data::generate_mixed_unicode(size).c_str());
    }
    
    for (auto _ : state) {
        for (size_t i = 0; i < strings.size(); ++i) {
            for (size_t j = i + 1; j < strings.size(); ++j) {
                benchmark::DoNotOptimize(strings[i].compare(strings[j]));
            }
        }
    }
}
BENCHMARK(BM_StressTest_Comparison)
    ->RangeMultiplier(4)
    ->Range(1<<6, 1<<12);

// Mixed Operation Stress Test
static void BM_StressTest_MixedOperations(benchmark::State& state) {
    const size_t size = state.range(0);
    std::vector<ustring> strings;
    for (int i = 0; i < 10; ++i) {
        strings.emplace_back(test_data::generate_mixed_unicode(size).c_str());
    }
    
    for (auto _ : state) {
        for (auto& str : strings) {
            // Perform a mix of operations
            str.append(test_data::generate_mixed_unicode(10).c_str());
            benchmark::DoNotOptimize(str.find('a'));
            str.insert(str.size()/2, "test");
            benchmark::DoNotOptimize(str.normalized({Normalization2Mode::NFC}));
            str.erase(0, 5);
            benchmark::DoNotOptimize(str.compare(strings[0]));
        }
    }
}
BENCHMARK(BM_StressTest_MixedOperations)
    ->RangeMultiplier(4)
    ->Range(1<<8, 1<<16);

BENCHMARK_MAIN();
