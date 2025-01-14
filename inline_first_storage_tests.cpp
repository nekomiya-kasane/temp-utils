#include "inline_first_storage.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <format>

TEST(InlineFirstStorageTest, DefaultConstruction)
{
  inline_first_storage<16> storage;
  EXPECT_EQ(storage.capacity(), 16);
}

TEST(InlineFirstStorageTest, InlineBufferUsage)
{
  inline_first_storage<16> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4, 5};
  storage = inline_first_storage<16>(data.data(), data.size());

  EXPECT_EQ(storage.capacity(), 16);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 5, data.begin()));
}

TEST(InlineFirstStorageTest, HeapAllocation)
{
  inline_first_storage<4> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8};  // More than inline capacity
  storage = inline_first_storage<4>(data.data(), data.size());

  EXPECT_GE(storage.capacity(), data.size());
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 8, data.begin()));
}

TEST(InlineFirstStorageTest, CopyConstruction)
{
  std::vector<uint8_t> data = {1, 2, 3, 4};
  inline_first_storage<4> storage1(data.data(), data.size());
  inline_first_storage<4> storage2(storage1);

  EXPECT_EQ(storage1.capacity(), storage2.capacity());
  EXPECT_EQ(storage1.capacity(), 4);
  EXPECT_EQ(storage2.capacity(), 4);
  EXPECT_TRUE(std::equal(storage1.begin(), storage1.end(), storage2.begin()));
}

TEST(InlineFirstStorageTest, MoveConstruction)
{
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};
  inline_first_storage<4> storage1(data.data(), data.size());
  inline_first_storage<4> storage2(std::move(storage1));

  // Moved from storage should be empty, that is the size is reset to inline capacity
  EXPECT_EQ(storage1.capacity(), 4);
  EXPECT_GE(storage2.capacity(), data.size());
  EXPECT_TRUE(std::equal(storage2.begin(), storage2.begin() + 6, data.begin()));
}

TEST(InlineFirstStorageTest, ResizeGrowth)
{
  inline_first_storage<4> storage;

  // Test growing within inline capacity
  storage.reserve(3);
  EXPECT_EQ(storage.capacity(), 4);

  // Test growing beyond inline capacity
  storage.reserve(8);
  EXPECT_EQ(storage.capacity(), 8);

  // Test shrinking
  storage.reserve(2);
  EXPECT_EQ(storage.capacity(), 8);
}

TEST(InlineFirstStorageTest, Assignment)
{
  std::vector<uint8_t> data1 = {1, 2, 3};
  std::vector<uint8_t> data2 = {4, 5, 6, 7, 8};

  inline_first_storage<4> storage1(data1.data(), data1.size());
  inline_first_storage<4> storage2(data2.data(), data2.size());

  storage1 = storage2;
  EXPECT_GE(storage1.capacity(), data2.size());
  EXPECT_TRUE(std::equal(storage1.begin(), storage1.begin() + 5, data2.begin()));
}

TEST(InlineFirstStorageTest, MoveAssignment)
{
  std::vector<uint8_t> data = {1, 2, 3, 4, 5};
  inline_first_storage<4> storage1(data.data(), data.size());
  inline_first_storage<4> storage2;

  storage2 = std::move(storage1);
  EXPECT_GE(storage2.capacity(), data.size());
  EXPECT_TRUE(std::equal(storage2.begin(), storage2.begin() + 5, data.begin()));
  EXPECT_GE(storage1.capacity(), 5);
}

TEST(InlineFirstStorageTest, EdgeCases)
{
  inline_first_storage<1> storage;

  // Test empty storage
  EXPECT_EQ(storage.capacity(), 1);

  // Test single element
  storage.reserve(1);
  storage[0] = 42;
  EXPECT_EQ(storage[0], 42);

  // Test reserve to zero
  storage.reserve(0);
  EXPECT_EQ(storage.capacity(), 1);

  // Test growth size calculation
#ifdef _DEBUG
  EXPECT_THROW({ storage.reserve(storage.max_capacity() + 1); }, std::length_error);
#endif
}

TEST(InlineFirstStorageTest, DifferentSizesCopy)
{
  inline_first_storage<8> storage1;
  std::vector<uint8_t> data = {1, 2, 3, 4};
  storage1 = inline_first_storage<8>(data.data(), data.size());

  inline_first_storage<4> storage2(storage1);
  EXPECT_GE(storage2.capacity(), data.size());
  EXPECT_TRUE(std::equal(storage2.begin(), storage2.begin() + 4, data.begin()));

  inline_first_storage<2> storage3(storage1, 2);  // Partial copy
  EXPECT_EQ(storage3.capacity(), 2);
  EXPECT_TRUE(std::equal(storage3.begin(), storage3.begin() + 2, data.begin()));
}

TEST(InlineFirstStorageTest, DataAccess)
{
  inline_first_storage<4> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4};
  storage = inline_first_storage<4>(data.data(), data.size());

  // Test operator[]
  for (size_t i = 0; i < storage.capacity(); ++i) {
    EXPECT_EQ(storage[i], data[i]);
  }

  // Test data() access
  const uint8_t *ptr = storage.data();
  EXPECT_TRUE(std::equal(ptr, ptr + 4, data.begin()));
}

TEST(InlineFirstStorageTest, SpanConversion)
{
  inline_first_storage<4> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4};
  storage = inline_first_storage<4>(data.data(), data.size());

  auto [ptr, size] = storage.to_span();
  EXPECT_GE(size, data.size());
  EXPECT_TRUE(std::equal(ptr, ptr + 4, data.begin()));
}

TEST(InlineFirstStorageTest, IteratorOperations)
{
  inline_first_storage<4> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4};
  storage = inline_first_storage<4>(data.data(), data.size());

  // Test iterator arithmetic
  auto it = storage.begin();
  EXPECT_EQ(*it, 1);
  EXPECT_EQ(*(it + 1), 2);
  EXPECT_EQ(*(it + 2), 3);
  EXPECT_EQ(*(it + 3), 4);

  // Test iterator comparison
  EXPECT_TRUE(it < storage.end());
  EXPECT_TRUE(it + 4 <= storage.end());

  // Test const iterator
  const auto &const_storage = storage;
  auto const_it = const_storage.begin();
  EXPECT_EQ(*const_it, 1);
}

TEST(InlineFirstStorageTest, SelfAssignment)
{
  inline_first_storage<4> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4};
  storage = inline_first_storage<4>(data.data(), data.size());

  // Test self copy assignment
  storage = storage;
  EXPECT_GE(storage.capacity(), 4);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 4, data.begin()));

  // Test self move assignment
  storage = std::move(storage);
  EXPECT_GE(storage.capacity(), 4);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 4, data.begin()));
}

TEST(InlineFirstStorageTest, MoveWithDifferentSizes)
{
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};

  // Move from smaller to larger inline capacity
  inline_first_storage<4> small(data.data(), 4);
  inline_first_storage<8> large(std::move(small));
  EXPECT_EQ(large.capacity(), 8);
  EXPECT_EQ(small.capacity(), 4);

  // Move from larger to smaller inline capacity
  inline_first_storage<8> large2(data.data(), 6);
  inline_first_storage<4> small2(std::move(large2));
  EXPECT_GE(small2.capacity(), 6);
  EXPECT_EQ(large2.capacity(), 8);
}

TEST(InlineFirstStorageTest, PartialMove)
{
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6};
  inline_first_storage<8> source(data.data(), data.size());

  // Move partial content
  inline_first_storage<4> dest(std::move(source), 3);
  EXPECT_EQ(dest.capacity(), 4);
  EXPECT_EQ(source.capacity(), 8);

  // Verify content
  EXPECT_TRUE(std::equal(dest.begin(), dest.begin() + 3, data.begin()));
}

TEST(InlineFirstStorageTest, ResizeEdgeCases)
{
  inline_first_storage<4> storage;

  // Test reserve to same size
  storage.reserve(0);
  EXPECT_EQ(storage.capacity(), 4);

  // Test reserve within inline capacity
  storage.reserve(4);
  EXPECT_GE(storage.capacity(), 4);

  // Test reserve to trigger heap allocation
  storage.reserve(8);
  EXPECT_GE(storage.capacity(), 8);

  // Test reserve down while using heap
  storage.reserve(6);
  EXPECT_GE(storage.capacity(), 6);

  // Test reserve down to inline capacity
  storage.reserve(4);
  EXPECT_GE(storage.capacity(), 4);

  // Test reserve to zero while using heap
  storage.reserve(8);
  storage.reserve(0);
  EXPECT_GE(storage.capacity(), 8);
}

TEST(InlineFirstStorageTest, DataConsistency)
{
  inline_first_storage<4> storage;
  std::vector<uint8_t> data = {1, 2, 3, 4};

  // Test data consistency during reserve operations
  storage = inline_first_storage<4>(data.data(), data.size());
  storage.reserve(8);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 4, data.begin()));

  // Test data consistency during shrink
  storage.reserve(2);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 4, data.begin()));

  // Test data consistency after multiple operations
  storage.reserve(6);
  storage[4] = 5;
  storage[5] = 6;
  storage.reserve(4);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 4, data.begin()));
}

TEST(InlineFirstStorageTest, CopyBetweenDifferentSizes)
{
  std::vector<uint8_t> data(100, 42);  // 100 elements of value 42

  // Test copy between different inline capacities
  inline_first_storage<16> small(data.data(), 10);
  inline_first_storage<32> medium(small);
  inline_first_storage<64> large(medium);

  EXPECT_EQ(small.capacity(), 16);
  EXPECT_EQ(small[9], 42);
  EXPECT_NE(small[10], 42);
  EXPECT_EQ(medium.capacity(), 32);
  EXPECT_EQ(medium[9], 42);
  EXPECT_NE(medium[10], 42);
  EXPECT_EQ(large.capacity(), 64);
  EXPECT_EQ(large[9], 42);
  EXPECT_NE(large[10], 42);

  // Test partial copy between different capacities
  inline_first_storage<8> tiny(large, 5);
  EXPECT_EQ(tiny.capacity(), 8);
  EXPECT_TRUE(std::equal(tiny.begin(), tiny.begin() + 5, data.begin()));
}

TEST(InlineFirstStorageTest, StressTest)
{
  constexpr size_t test_size = 1000;
  std::vector<uint8_t> data(test_size);
  for (size_t i = 0; i < test_size; ++i) {
    data[i] = static_cast<uint8_t>(i % 256);
  }

  inline_first_storage<16> storage;

  // Test multiple reserve operations
  for (size_t size = 0; size <= test_size; size += 100) {
    storage.reserve(size);
    EXPECT_GE(storage.capacity(), size);

    if (size > 0) {
      std::copy_n(data.begin(), size, storage.begin());
      EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + size, data.begin()));
    }
  }
}

TEST(InlineFirstStorageTest, ShrinkOperation)
{
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8};
  inline_first_storage<4> storage(data.data(), data.size());

  // Test shrink with data copy
  EXPECT_TRUE(storage.shrink(6, true));
  EXPECT_GE(storage.capacity(), 6);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 6, data.begin()));

  // Test shrink without data copy
  auto storage2 = storage;
  EXPECT_TRUE(storage2.shrink(4, false));
  EXPECT_GE(storage2.capacity(), 4);
  EXPECT_FALSE(std::equal(storage2.begin(), storage2.begin() + 3, data.begin()));

  // Test shrink to inline with data copy
  EXPECT_TRUE(storage.shrink(3, true, true));
  EXPECT_EQ(storage.capacity(), 4);  // Should be using inline buffer now
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 3, data.begin()));

  // Test shrink failure (size >= capacity)
  EXPECT_FALSE(storage.shrink(8));
  EXPECT_EQ(storage.capacity(), 4);

  // Test shrink with heap storage
  storage = inline_first_storage<4>(data.data(), data.size());
  EXPECT_TRUE(storage.shrink(5, true, false));
  EXPECT_GE(storage.capacity(), 5);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 5, data.begin()));
}

TEST(InlineFirstStorageTest, ResetOperation)
{
  std::vector<uint8_t> data = {1, 2, 3, 4};
  inline_first_storage<4> storage;

  // Test reset with new data
  uint8_t* new_data = new uint8_t[6]{5, 6, 7, 8, 9, 10};
  storage.reset(new_data, 6);
  EXPECT_EQ(storage.capacity(), 6);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 6, new_data));

  // Test reset from inline to heap
  storage = inline_first_storage<4>(data.data(), data.size());
  uint8_t* heap_data = new uint8_t[8]{1, 2, 3, 4, 5, 6, 7, 8};
  storage.reset(heap_data, 8);
  EXPECT_EQ(storage.capacity(), 8);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 8, heap_data));

  // Test reset from heap to heap
  uint8_t* another_data = new uint8_t[5]{11, 12, 13, 14, 15};
  storage.reset(another_data, 5);
  EXPECT_EQ(storage.capacity(), 5);
  EXPECT_TRUE(std::equal(storage.begin(), storage.begin() + 5, another_data));
}

TEST(InlineFirstStorageTest, ToString) {
  inline_first_storage<16> storage;
  EXPECT_EQ(to_string(storage), "inline_first_storage{size=0, capacity=16, data=[]}");

  // Add some data
  uint8_t data[] = {1, 2, 3, 4, 5};
  storage.assign(data, sizeof(data));
  EXPECT_EQ(to_string(storage), "inline_first_storage{size=5, capacity=16, data=[1, 2, 3, 4, 5]}");

  // Test with larger data
  inline_first_storage<4> small_storage;
  uint8_t large_data[] = {10, 20, 30, 40, 50, 60};
  small_storage.assign(large_data, sizeof(large_data));
  EXPECT_EQ(to_string(small_storage), 
            "inline_first_storage{size=6, capacity=6, data=[10, 20, 30, 40, 50, 60]}");

  // Test with single byte
  inline_first_storage<4> single_byte;
  single_byte.push_back(42);
  EXPECT_EQ(to_string(single_byte), "inline_first_storage{size=1, capacity=4, data=[42]}");
}

TEST(InlineFirstStorageTest, FormatSupport) {
  inline_first_storage<8> storage;
  uint8_t data[] = {1, 2, 3};
  storage.assign(data, sizeof(data));

  EXPECT_EQ(std::format("{}", storage), 
            "inline_first_storage{size=3, capacity=8, data=[1, 2, 3]}");
  EXPECT_EQ(std::format("{:s}", storage), 
            "inline_first_storage{size=3, capacity=8, data=[1, 2, 3]}");
}
