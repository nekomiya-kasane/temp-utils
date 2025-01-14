#include "inline_first_storage.h"
#include <benchmark/benchmark.h>
#include <random>
#include <vector>

// Helper function to generate random data
std::vector<uint8_t> generate_random_data(size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (size_t i = 0; i < size; ++i) {
        data[i] = static_cast<uint8_t>(dis(gen));
    }
    return data;
}

// Benchmark creation with inline storage
static void BM_InlineCreation(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    
    for (auto _ : state) {
        inline_first_storage<16> storage(data.data(), size);
        benchmark::DoNotOptimize(storage);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_InlineCreation)->Range(1, 1<<20);

// Benchmark creation with heap storage
static void BM_HeapCreation(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    
    for (auto _ : state) {
        inline_first_storage<8> storage(data.data(), size);
        benchmark::DoNotOptimize(storage);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_HeapCreation)->Range(1, 1<<20);

// Benchmark copy construction
static void BM_CopyConstruction(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    inline_first_storage<16> source(data.data(), size);
    
    for (auto _ : state) {
        inline_first_storage<16> storage(source);
        benchmark::DoNotOptimize(storage);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_CopyConstruction)->Range(1, 1<<20);

// Benchmark move construction
static void BM_MoveConstruction(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    
    for (auto _ : state) {
        state.PauseTiming();
        inline_first_storage<16> source(data.data(), size);
        state.ResumeTiming();
        
        inline_first_storage<16> storage(std::move(source));
        benchmark::DoNotOptimize(storage);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_MoveConstruction)->Range(1, 1<<20);

// Benchmark resize operations
static void BM_Resize(benchmark::State& state) {
    const size_t size = state.range(0);
    inline_first_storage<16> storage;
    
    for (auto _ : state) {
        storage.reserve(size);
        benchmark::DoNotOptimize(storage);
        storage.reserve(0);
        benchmark::DoNotOptimize(storage);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_Resize)->Range(1, 1<<20);

// Benchmark data access
static void BM_DataAccess(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    inline_first_storage<16> storage(data.data(), size);
    
    for (auto _ : state) {
        uint64_t sum = 0;
        for (size_t i = 0; i < size; ++i) {
            sum += storage[i];
        }
        benchmark::DoNotOptimize(sum);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_DataAccess)->Range(1, 1<<20);

// Benchmark iterator traversal
static void BM_IteratorTraversal(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    inline_first_storage<16> storage(data.data(), size);
    
    for (auto _ : state) {
        uint64_t sum = 0;
        for (auto it = storage.begin(); it != storage.end(); ++it) {
            sum += *it;
        }
        benchmark::DoNotOptimize(sum);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_IteratorTraversal)->Range(1, 1<<20);

// Benchmark copy between different sizes
static void BM_CrossSizeCopy(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    inline_first_storage<32> source(data.data(), size);
    
    for (auto _ : state) {
        inline_first_storage<16> storage(source);
        benchmark::DoNotOptimize(storage);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_CrossSizeCopy)->Range(1, 1<<20);

// Benchmark partial move
static void BM_PartialMove(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    
    for (auto _ : state) {
        state.PauseTiming();
        inline_first_storage<32> source(data.data(), size);
        state.ResumeTiming();
        
        inline_first_storage<16> storage(std::move(source), size/2);
        benchmark::DoNotOptimize(storage);
        benchmark::DoNotOptimize(source);
    }
    
    state.SetBytesProcessed(int64_t(state.iterations()) * int64_t(size));
}
BENCHMARK(BM_PartialMove)->Range(1, 1<<20);

BENCHMARK_MAIN();
