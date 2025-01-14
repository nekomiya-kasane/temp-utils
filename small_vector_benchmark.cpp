#include "small_vector.h"
#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include <vector>

// Helper function to generate random strings
std::vector<std::string> generate_random_strings(size_t count, size_t avg_length = 16) {
    std::vector<std::string> result;
    result.reserve(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> len_dis(1, avg_length * 2);
    std::uniform_int_distribution<> char_dis('a', 'z');
    
    for (size_t i = 0; i < count; ++i) {
        std::string str;
        str.reserve(len_dis(gen));
        for (size_t j = 0; j < str.capacity(); ++j) {
            str.push_back(static_cast<char>(char_dis(gen)));
        }
        result.push_back(std::move(str));
    }
    return result;
}

// Helper function to generate random integers
std::vector<int> generate_random_ints(size_t count) {
    std::vector<int> result;
    result.reserve(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1000000);
    
    for (size_t i = 0; i < count; ++i) {
        result.push_back(dis(gen));
    }
    return result;
}

template<typename Vec>
static void BM_Creation(benchmark::State& state) {
    const size_t count = state.range(0);
    auto data = generate_random_ints(count);
    
    for (auto _ : state) {
        Vec v;
        benchmark::DoNotOptimize(v);
        for (const auto& item : data) {
            v.push_back(item);
        }
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_EmplaceBack(benchmark::State& state) {
    const size_t count = state.range(0);
    auto strings = generate_random_strings(count);
    
    for (auto _ : state) {
        Vec v;
        benchmark::DoNotOptimize(v);
        for (const auto& s : strings) {
            v.emplace_back(s);
        }
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_InsertMiddle(benchmark::State& state) {
    const size_t count = state.range(0);
    auto data = generate_random_ints(count / 2);  // Insert half as many items
    
    for (auto _ : state) {
        Vec v(count / 2, 42);  // Start with half-full vector
        benchmark::DoNotOptimize(v);
        auto it = v.begin() + v.size() / 2;
        for (const auto& item : data) {
            it = v.insert(it, item);
            ++it;
        }
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count / 2));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_EraseMiddle(benchmark::State& state) {
    const size_t count = state.range(0);
    
    for (auto _ : state) {
        Vec v(count, 42);
        benchmark::DoNotOptimize(v);
        auto it = v.begin() + v.size() / 4;
        for (size_t i = 0; i < count / 2 && it != v.end(); ++i) {
            it = v.erase(it);
        }
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count / 2));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_RandomAccess(benchmark::State& state) {
    const size_t count = state.range(0);
    std::vector<size_t> indices;
    indices.reserve(count);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(0, count - 1);
    for (size_t i = 0; i < count; ++i) {
        indices.push_back(dis(gen));
    }
    
    Vec v(count, 42);
    
    for (auto _ : state) {
        size_t sum = 0;
        for (size_t idx : indices) {
            sum += v[idx];
            benchmark::DoNotOptimize(sum);
        }
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_Sort(benchmark::State& state) {
    const size_t count = state.range(0);
    auto data = generate_random_ints(count);
    
    for (auto _ : state) {
        Vec v(data.begin(), data.end());
        benchmark::DoNotOptimize(v);
        std::sort(v.begin(), v.end());
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_CopyConstruct(benchmark::State& state) {
    const size_t count = state.range(0);
    Vec source(count, 42);
    
    for (auto _ : state) {
        Vec v(source);
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
    state.SetLabel(typeid(Vec).name());
}

template<typename Vec>
static void BM_MoveConstruct(benchmark::State& state) {
    const size_t count = state.range(0);
    
    for (auto _ : state) {
        Vec source(count, 42);
        benchmark::DoNotOptimize(source);
        Vec v(std::move(source));
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
    
    state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
    state.SetLabel(typeid(Vec).name());
}

// Register benchmarks for both vector types
#define REGISTER_BENCHMARK(name, range_start, range_end) \
    BENCHMARK(name<std::vector<int>>)->Range(range_start, range_end); \
    BENCHMARK(name<small_vector<int, 16>>)->Range(range_start, range_end)

REGISTER_BENCHMARK(BM_Creation, 1, 1 << 16);
// todo: bugs
//REGISTER_BENCHMARK(BM_EmplaceBack, 1, 1 << 16);
REGISTER_BENCHMARK(BM_InsertMiddle, 1, 1 << 16);
REGISTER_BENCHMARK(BM_EraseMiddle, 1, 1 << 16);
REGISTER_BENCHMARK(BM_RandomAccess, 1, 1 << 16);
REGISTER_BENCHMARK(BM_Sort, 1, 1 << 16);
REGISTER_BENCHMARK(BM_CopyConstruct, 1, 1 << 16);
REGISTER_BENCHMARK(BM_MoveConstruct, 1, 1 << 16);

BENCHMARK_MAIN();
