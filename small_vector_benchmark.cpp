#include "small_vector.h"
#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include <vector>

// Helper function to generate random strings
std::vector<std::string> generate_random_strings(size_t count, size_t avg_length = 16)
{
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
std::vector<int> generate_random_ints(size_t count)
{
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

template<typename Vec> static void BM_Creation(benchmark::State &state)
{
  const size_t count = state.range(0);
  auto data = generate_random_ints(count);

  for (auto _ : state) {
    Vec v;
    benchmark::DoNotOptimize(v);
    for (const auto &item : data) {
      v.emplace_back(item);
    }
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
  state.SetLabel(typeid(Vec).name());
}

template<typename Vec> static void BM_EmplaceBack(benchmark::State &state)
{
  const size_t count = state.range(0);
  auto data = generate_random_ints(count);

  for (auto _ : state) {
    Vec v;
    benchmark::DoNotOptimize(v);
    for (const auto &s : data) {
      v.emplace_back(s);
    }
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
  state.SetLabel(typeid(Vec).name());
}

template<typename Vec> static void BM_InsertMiddle(benchmark::State &state)
{
  const size_t count = state.range(0);
  auto data = generate_random_ints(count / 2);  // Insert half as many items

  for (auto _ : state) {
    Vec v(count / 2, 42);  // Start with half-full vector
    benchmark::DoNotOptimize(v);
    auto it = v.begin() + v.size() / 2;
    for (const auto &item : data) {
      it = v.insert(it, item);
      ++it;
    }
    benchmark::ClobberMemory();
  }

  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count / 2));
  state.SetLabel(typeid(Vec).name());
}

template<typename Vec> static void BM_EraseMiddle(benchmark::State &state)
{
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

template<typename Vec> static void BM_RandomAccess(benchmark::State &state)
{
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

template<typename Vec> static void BM_Sort(benchmark::State &state)
{
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

template<typename Vec> static void BM_CopyConstruct(benchmark::State &state)
{
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

template<typename Vec> static void BM_MoveConstruct(benchmark::State &state)
{
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

// Complex object for testing
class ComplexObject {
 public:
  explicit ComplexObject(int v = 0) : data(std::make_unique<int>(v)) {}
  ComplexObject(const ComplexObject& other) : data(std::make_unique<int>(*other.data)) {}
  ComplexObject(ComplexObject&& other) noexcept = default;
  ComplexObject& operator=(const ComplexObject& other) {
    if (this != &other) {
      data = std::make_unique<int>(*other.data);
    }
    return *this;
  }
  ComplexObject& operator=(ComplexObject&& other) noexcept = default;
  bool operator<(const ComplexObject& other) const { return *data < *other.data; }
  int getValue() const { return *data; }
 private:
  std::unique_ptr<int> data;
};

// Test complex object operations
template<typename Vec>
static void BM_ComplexObjectOperations(benchmark::State& state) {
  const size_t count = state.range(0);
  auto data = generate_random_ints(count);
  
  for (auto _ : state) {
    Vec v;
    benchmark::DoNotOptimize(v);
    for (const auto& item : data) {
      v.emplace_back(item);
    }
    // Sort to test comparison and move operations
    std::sort(v.begin(), v.end());
    // Erase half the elements
    v.erase(v.begin(), v.begin() + v.size() / 2);
    // Insert new elements
    for (size_t i = 0; i < count / 4; ++i) {
      v.emplace(v.begin() + i * 2, i);
    }
    benchmark::ClobberMemory();
  }
  
  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
}

// Test memory locality
template<typename Vec>
static void BM_MemoryLocality(benchmark::State& state) {
  const size_t count = state.range(0);
  const size_t iterations = 100;
  
  Vec v(count);
  for (size_t i = 0; i < count; ++i) {
    v[i] = static_cast<int>(i);
  }
  
  for (auto _ : state) {
    size_t sum = 0;
    // Sequential access
    for (size_t i = 0; i < iterations; ++i) {
      for (const auto& item : v) {
        sum += item;
      }
    }
    // Strided access
    for (size_t i = 0; i < iterations; ++i) {
      for (size_t j = 0; j < v.size(); j += 16) {
        sum += v[j];
      }
    }
    benchmark::DoNotOptimize(sum);
  }
  
  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count) * iterations * 2);
}

// Test mixed small and large operations
template<typename Vec>
static void BM_MixedSizesOperations(benchmark::State& state) {
  const size_t count = state.range(0);
  auto data = generate_random_ints(count);
  
  for (auto _ : state) {
    Vec v;
    // Small operations (within inline capacity)
    for (size_t i = 0; i < 8; ++i) {
      v.push_back(static_cast<int>(i));
    }
    // Clear and repeat
    v.clear();
    for (size_t i = 0; i < 4; ++i) {
      v.push_back(static_cast<int>(i));
    }
    // Grow to large size
    v.insert(v.end(), data.begin(), data.end());
    // Shrink back to small
    v.resize(8);
    benchmark::ClobberMemory();
  }
  
  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count));
}

// Test reallocation patterns
template<typename Vec>
static void BM_ReallocationPatterns(benchmark::State& state) {
  const size_t count = state.range(0);
  
  for (auto _ : state) {
    Vec v;
    // Grow one by one to test reallocation strategy
    for (size_t i = 0; i < count; ++i) {
      v.push_back(static_cast<int>(i));
    }
    // Shrink one by one
    while (!v.empty()) {
      v.pop_back();
    }
    benchmark::ClobberMemory();
  }
  
  state.SetItemsProcessed(int64_t(state.iterations()) * int64_t(count) * 2);
}

// Register benchmarks for both vector types
#define REGISTER_BENCHMARK(name, range_start, range_end) \
  BENCHMARK(name<std::vector<int>>)->Range(range_start, range_end); \
  BENCHMARK(name<small_vector<int, 16>>)->Range(range_start, range_end)

REGISTER_BENCHMARK(BM_Creation, 1, 1 << 16);
REGISTER_BENCHMARK(BM_EmplaceBack, 1, 1 << 16);
REGISTER_BENCHMARK(BM_InsertMiddle, 1, 1 << 16);
REGISTER_BENCHMARK(BM_EraseMiddle, 1, 1 << 16);
REGISTER_BENCHMARK(BM_RandomAccess, 1, 1 << 16);
REGISTER_BENCHMARK(BM_Sort, 1, 1 << 16);
REGISTER_BENCHMARK(BM_CopyConstruct, 1, 1 << 16);
REGISTER_BENCHMARK(BM_MoveConstruct, 1, 1 << 16);

REGISTER_BENCHMARK(BM_ComplexObjectOperations, 1, 1 << 16);
REGISTER_BENCHMARK(BM_MemoryLocality, 1, 1 << 16);
REGISTER_BENCHMARK(BM_MixedSizesOperations, 1, 1 << 16);
REGISTER_BENCHMARK(BM_ReallocationPatterns, 1, 1 << 16);

using ComplexVec = std::vector<ComplexObject>;
using ComplexSmallVec = small_vector<ComplexObject, 16>;
BENCHMARK_TEMPLATE(BM_Creation, ComplexVec)->Range(1, 1 << 16);
BENCHMARK_TEMPLATE(BM_Creation, ComplexSmallVec)->Range(1, 1 << 16);
// todo:
//BENCHMARK_TEMPLATE(BM_Sort, ComplexVec)->Range(1, 1 << 16);
//BENCHMARK_TEMPLATE(BM_Sort, ComplexSmallVec)->Range(1, 1 << 16);

BENCHMARK_MAIN();
