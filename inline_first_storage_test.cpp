#include "inline_first_storage.h"
#include <gtest/gtest.h>
#include <string>
#include <algorithm>
#include <memory>
#include <vector>

// Helper class to track allocations
template<typename T>
class tracking_allocator : public std::allocator<T> {
public:
    using base = std::allocator<T>;
    using value_type = typename base::value_type;
    using pointer = typename base::pointer;
    using size_type = typename base::size_type;

    static size_t allocation_count;
    static size_t deallocation_count;

    tracking_allocator() noexcept = default;
    template<typename U>
    tracking_allocator(const tracking_allocator<U>&) noexcept {}

    pointer allocate(size_type n) {
        ++allocation_count;
        return base::allocate(n);
    }

    void deallocate(pointer p, size_type n) {
        ++deallocation_count;
        base::deallocate(p, n);
    }

    static void reset() {
        allocation_count = 0;
        deallocation_count = 0;
    }
};

template<typename T>
size_t tracking_allocator<T>::allocation_count = 0;

template<typename T>
size_t tracking_allocator<T>::deallocation_count = 0;

class InlineFirstStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        tracking_allocator<uint8_t>::reset();
    }
};

// Basic construction and assignment tests
TEST_F(InlineFirstStorageTest, DefaultConstruction) {
    inline_first_storage<16> storage;
    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0);
    EXPECT_EQ(storage.capacity(), 16);
}

TEST_F(InlineFirstStorageTest, CustomAllocatorConstruction) {
    tracking_allocator<uint8_t> alloc;
    inline_first_storage<16, uint8_t, tracking_allocator<uint8_t>> storage(alloc);
    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0);
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, 0);
}

// Push back and element access tests
TEST_F(InlineFirstStorageTest, PushBackInline) {
    inline_first_storage<4> storage;
    storage.push_back(1);
    storage.push_back(2);
    storage.push_back(3);

    EXPECT_EQ(storage.size(), 3);
    EXPECT_EQ(storage[0], 1);
    EXPECT_EQ(storage[1], 2);
    EXPECT_EQ(storage[2], 3);
    EXPECT_EQ(storage.capacity(), 4);
}

TEST_F(InlineFirstStorageTest, PushBackHeap) {
    inline_first_storage<2, uint8_t, tracking_allocator<uint8_t>> storage;
    
    // Fill inline buffer
    storage.push_back(1);
    storage.push_back(2);
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, 0);
    
    // Force heap allocation
    storage.push_back(3);
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, 1);
    
    EXPECT_EQ(storage.size(), 3);
    EXPECT_EQ(storage[0], 1);
    EXPECT_EQ(storage[1], 2);
    EXPECT_EQ(storage[2], 3);
}

// Iterator tests
TEST_F(InlineFirstStorageTest, Iterators) {
    inline_first_storage<4> storage;
    for (uint8_t i = 0; i < 4; ++i) {
        storage.push_back(i);
    }

    // Test forward iteration
    uint8_t expected = 0;
    for (const auto& value : storage) {
        EXPECT_EQ(value, expected++);
    }

    // Test reverse iteration
    expected = 3;
    for (auto it = storage.end(); it != storage.begin();) {
        EXPECT_EQ(*--it, expected--);
    }
}

// Copy and move tests
TEST_F(InlineFirstStorageTest, CopyConstruction) {
    inline_first_storage<4, uint8_t, tracking_allocator<uint8_t>> source;
    for (uint8_t i = 0; i < 6; ++i) {  // Force heap allocation
        source.push_back(i);
    }
    
    size_t alloc_count = tracking_allocator<uint8_t>::allocation_count;
    auto copy = source;  // Copy construction
    
    EXPECT_EQ(copy.size(), source.size());
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, alloc_count + 1);
    
    for (size_t i = 0; i < copy.size(); ++i) {
        EXPECT_EQ(copy[i], source[i]);
    }
}

TEST_F(InlineFirstStorageTest, MoveConstruction) {
    inline_first_storage<4, uint8_t, tracking_allocator<uint8_t>> source;
    for (uint8_t i = 0; i < 6; ++i) {  // Force heap allocation
        source.push_back(i);
    }
    
    size_t alloc_count = tracking_allocator<uint8_t>::allocation_count;
    auto moved = std::move(source);  // Move construction
    
    EXPECT_EQ(moved.size(), 6);
    EXPECT_EQ(source.size(), 0);  // Source should be empty
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, alloc_count);  // No new allocations
}

// Capacity and growth tests
TEST_F(InlineFirstStorageTest, Reserve) {
    inline_first_storage<4, uint8_t, tracking_allocator<uint8_t>> storage;
    
    // Reserve within inline capacity
    storage.reserve(3);
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, 0);
    
    // Reserve beyond inline capacity
    storage.reserve(8);
    EXPECT_EQ(tracking_allocator<uint8_t>::allocation_count, 1);
    EXPECT_GE(storage.capacity(), 8);
}

TEST_F(InlineFirstStorageTest, ShrinkToFit) {
    inline_first_storage<4, uint8_t, tracking_allocator<uint8_t>> storage;
    
    // Push beyond inline capacity
    for (uint8_t i = 0; i < 8; ++i) {
        storage.push_back(i);
    }
    
    // Remove some elements
    for (int i = 0; i < 5; ++i) {
        storage.pop_back();
    }
    
    size_t alloc_count = tracking_allocator<uint8_t>::allocation_count;
    storage.shrink_to_fit();
    
    // Should convert back to inline storage
    EXPECT_EQ(storage.capacity(), 4);
    EXPECT_EQ(tracking_allocator<uint8_t>::deallocation_count, alloc_count);
}

// Exception safety tests
class ThrowingType {
public:
    static bool should_throw;
    
    ThrowingType(int v = 0) : value(v) {
        if (should_throw) throw std::runtime_error("test exception");
    }
    
    ThrowingType(const ThrowingType& other) : value(other.value) {
        if (should_throw) throw std::runtime_error("test exception");
    }
    
    int value;
};

bool ThrowingType::should_throw = false;

TEST_F(InlineFirstStorageTest, ExceptionSafety) {
    inline_first_storage<2, ThrowingType> storage;
    
    // Test push_back exception safety
    storage.push_back(ThrowingType(1));
    storage.push_back(ThrowingType(2));
    
    ThrowingType::should_throw = true;
    EXPECT_THROW(storage.push_back(ThrowingType(3)), std::runtime_error);
    
    EXPECT_EQ(storage.size(), 2);
    EXPECT_EQ(storage[0].value, 1);
    EXPECT_EQ(storage[1].value, 2);
}

// Performance test (optional, can be disabled)
TEST_F(InlineFirstStorageTest, DISABLED_Performance) {
    constexpr size_t iterations = 1000000;
    constexpr size_t inline_size = 16;
    
    // Test with inline_first_storage
    {
        inline_first_storage<inline_size> storage;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            storage.push_back(static_cast<uint8_t>(i & 0xFF));
            if (i % inline_size == 0) {
                storage.clear();
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "inline_first_storage time: " << duration.count() << "ms\n";
    }
    
    // Compare with std::vector
    {
        std::vector<uint8_t> vec;
        auto start = std::chrono::high_resolution_clock::now();
        
        for (size_t i = 0; i < iterations; ++i) {
            vec.push_back(static_cast<uint8_t>(i & 0xFF));
            if (i % inline_size == 0) {
                vec.clear();
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "std::vector time: " << duration.count() << "ms\n";
    }
}
