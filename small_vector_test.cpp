#include "small_vector.h"
#include <gtest/gtest.h>
#include <string>
#include <algorithm>
#include <ranges>

using namespace ustd;

TEST(SmallVectorTest, DefaultConstruction) {
    small_vector<int, 4> v;
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
    EXPECT_GE(v.capacity(), 4);
}

TEST(SmallVectorTest, ConstructWithSize) {
    small_vector<int, 4> v(3, 42);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 42);
    EXPECT_EQ(v[1], 42);
    EXPECT_EQ(v[2], 42);
}

TEST(SmallVectorTest, PushBackInline) {
    small_vector<int, 4> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(SmallVectorTest, PushBackBeyondInline) {
    small_vector<int, 4> v;
    for (int i = 0; i < 6; ++i) {
        v.push_back(i);
    }
    EXPECT_EQ(v.size(), 6);
    for (int i = 0; i < 6; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

TEST(SmallVectorTest, CopyConstruction) {
    small_vector<int, 4> v1;
    v1.push_back(1);
    v1.push_back(2);
    
    small_vector<int, 4> v2(v1);
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
}

TEST(SmallVectorTest, MoveConstruction) {
    small_vector<std::string, 4> v1;
    v1.push_back("hello");
    v1.push_back("world");
    
    small_vector<std::string, 4> v2(std::move(v1));
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[0], "hello");
    EXPECT_EQ(v2[1], "world");
    EXPECT_TRUE(v1.empty());
}

TEST(SmallVectorTest, Assignment) {
    small_vector<int, 4> v1{1, 2, 3};
    small_vector<int, 4> v2;
    v2 = v1;
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v2[2], 3);
}

TEST(SmallVectorTest, MoveAssignment) {
    small_vector<std::string, 4> v1{"hello", "world"};
    small_vector<std::string, 4> v2;
    v2 = std::move(v1);
    EXPECT_EQ(v2.size(), 2);
    EXPECT_EQ(v2[0], "hello");
    EXPECT_EQ(v2[1], "world");
    EXPECT_TRUE(v1.empty());
}

TEST(SmallVectorTest, Insert) {
    small_vector<int, 4> v{1, 2, 4};
    auto it = v.insert(v.begin() + 2, 3);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
}

TEST(SmallVectorTest, InsertRange) {
    small_vector<int, 4> v{1, 4};
    std::array<int, 2> arr{2, 3};
    v.insert(v.begin() + 1, arr.begin(), arr.end());
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
}

TEST(SmallVectorTest, Erase) {
    small_vector<int, 4> v{1, 2, 3, 4};
    auto it = v.erase(v.begin() + 1);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
    EXPECT_EQ(v[2], 4);
}

TEST(SmallVectorTest, EraseRange) {
    small_vector<int, 4> v{1, 2, 3, 4, 5};
    auto it = v.erase(v.begin() + 1, v.begin() + 4);
    EXPECT_EQ(*it, 5);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 5);
}

TEST(SmallVectorTest, Resize) {
    small_vector<int, 4> v{1, 2};
    v.resize(4, 42);
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 42);
    EXPECT_EQ(v[3], 42);
    
    v.resize(1);
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], 1);
}

TEST(SmallVectorTest, RangeSupport) {
    small_vector<int, 4> v{1, 2, 3, 4};
    
    // Test range-based for loop
    int sum = 0;
    for (const auto& x : v) {
        sum += x;
    }
    EXPECT_EQ(sum, 10);
    
    // Test with ranges
    auto even = v | std::views::filter([](int x) { return x % 2 == 0; });
    std::vector<int> result;
    std::ranges::copy(even, std::back_inserter(result));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], 2);
    EXPECT_EQ(result[1], 4);
}

TEST(SmallVectorTest, SpanSupport) {
    small_vector<int, 4> v{1, 2, 3, 4};
    auto sp = v.as_span();
    EXPECT_EQ(sp.size(), 4);
    EXPECT_EQ(sp[0], 1);
    EXPECT_EQ(sp[1], 2);
    EXPECT_EQ(sp[2], 3);
    EXPECT_EQ(sp[3], 4);
}

TEST(SmallVectorTest, Comparison) {
    small_vector<int, 4> v1{1, 2, 3};
    small_vector<int, 4> v2{1, 2, 3};
    small_vector<int, 4> v3{1, 2, 4};
    
    EXPECT_EQ(v1, v2);
    EXPECT_NE(v1, v3);
    EXPECT_LT(v1, v3);
    EXPECT_LE(v1, v2);
    EXPECT_GT(v3, v1);
    EXPECT_GE(v2, v1);
}

TEST(SmallVectorTest, EmplaceBack) {
    small_vector<std::string, 4> v;
    auto& ref = v.emplace_back("hello");
    EXPECT_EQ(ref, "hello");
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v[0], "hello");
}

TEST(SmallVectorTest, ShrinkToFit) {
    small_vector<int, 4> v;
    for (int i = 0; i < 8; ++i) {
        v.push_back(i);
    }
    auto old_capacity = v.capacity();
    v.erase(v.begin() + 2, v.end());
    v.shrink_to_fit();
    EXPECT_LE(v.capacity(), old_capacity);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[1], 1);
}

TEST(SmallVectorTest, SimdSum) {
    small_vector<float, 4> v{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    EXPECT_FLOAT_EQ(v.sum(), 36.0f);

    small_vector<double, 4> v2{1.0, 2.0, 3.0, 4.0};
    EXPECT_DOUBLE_EQ(v2.sum(), 10.0);

    small_vector<int32_t, 4> v3{1, 2, 3, 4, 5, 6, 7, 8};
    EXPECT_EQ(v3.sum(), 36);
}

TEST(SmallVectorTest, View) {
    small_vector<int, 4> v{1, 2, 3, 4};
    auto view = v.view();
    
    EXPECT_EQ(view.size(), 4);
    EXPECT_EQ(view[0], 1);
    EXPECT_EQ(view[1], 2);
    EXPECT_EQ(view[2], 3);
    EXPECT_EQ(view[3], 4);
    
    // Test modification through view
    view[0] = 10;
    EXPECT_EQ(v[0], 10);
}

TEST(SmallVectorTest, TransformInplace) {
    small_vector<int, 4> v{1, 2, 3, 4};
    v.transform_inplace([](int x) { return x * 2; });
    
    EXPECT_EQ(v[0], 2);
    EXPECT_EQ(v[1], 4);
    EXPECT_EQ(v[2], 6);
    EXPECT_EQ(v[3], 8);
    
    small_vector<int, 4> v2{1, 1, 1, 1};
    v.transform_inplace(v2, std::plus<>());
    
    EXPECT_EQ(v[0], 3);
    EXPECT_EQ(v[1], 5);
    EXPECT_EQ(v[2], 7);
    EXPECT_EQ(v[3], 9);
}

TEST(SmallVectorTest, InsertBulk) {
    small_vector<int, 4> v{1, 4};
    std::vector<int> source{2, 3};
    
    v.insert_bulk(v.begin() + 1, source.begin(), source.end());
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
}

TEST(SmallVectorTest, DebugString) {
    small_vector<int, 4> v{1, 2, 3, 4};
    std::string debug_info = v.debug_string();
    EXPECT_TRUE(debug_info.find("size=4") != std::string::npos);
    EXPECT_TRUE(debug_info.find("inline=true") != std::string::npos);
}

TEST(SmallVectorTest, ParallelTransform) {
    small_vector<int, 4> v;
    for (int i = 0; i < 1000; ++i) {
        v.push_back(i);
    }
    
    v.parallel_transform([](int x) { return x * 2; });
    
    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(v[i], i * 2);
    }
}

TEST(SmallVectorTest, StablePartitionInplace) {
    small_vector<int, 4> v{1, 2, 3, 4, 5, 6};
    v.stable_partition_inplace([](int x) { return x % 2 == 0; });
    
    std::vector<int> expected{2, 4, 6};
    EXPECT_EQ(v.size(), 3);
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(v[i], expected[i]);
    }
}

TEST(SmallVectorTest, RemoveDuplicates) {
    small_vector<int, 4> v{3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    v.remove_duplicates();
    
    std::vector<int> expected{1, 2, 3, 4, 5, 6, 9};
    EXPECT_EQ(v.size(), expected.size());
    for (size_t i = 0; i < v.size(); ++i) {
        EXPECT_EQ(v[i], expected[i]);
    }
}

TEST(SmallVectorTest, AppendRange) {
    small_vector<int, 4> v{1, 2};
    std::vector<int> source{3, 4, 5};
    
    v.append_range(source);
    EXPECT_EQ(v.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(v[i], i + 1);
    }
}

TEST(SmallVectorTest, MemoryStats) {
    small_vector<int, 4> v;
    
    auto stats = v.get_memory_stats();
    EXPECT_EQ(stats.size, 0);
    EXPECT_EQ(stats.capacity, 4);
    EXPECT_EQ(stats.inline_capacity, 4);
    EXPECT_TRUE(stats.using_inline_storage);
    EXPECT_EQ(stats.wasted_bytes, 4 * sizeof(int));
    
    for (int i = 0; i < 5; ++i) {
        v.push_back(i);
    }
    
    stats = v.get_memory_stats();
    EXPECT_EQ(stats.size, 5);
    EXPECT_FALSE(stats.using_inline_storage);
}

TEST(SmallVectorTest, SwapElements) {
    small_vector<std::string, 4> v{"a", "b", "c", "d"};
    v.swap_elements(1, 2);
    
    EXPECT_EQ(v[0], "a");
    EXPECT_EQ(v[1], "c");
    EXPECT_EQ(v[2], "b");
    EXPECT_EQ(v[3], "d");
}

TEST(SmallVectorTest, CountIfAndContains) {
    small_vector<int, 4> v{1, 2, 3, 4, 5, 6};
    
    auto even_count = v.count_if([](int x) { return x % 2 == 0; });
    EXPECT_EQ(even_count, 3);
    
    EXPECT_TRUE(v.contains(3));
    EXPECT_FALSE(v.contains(7));
}

TEST(SmallVectorTest, RemoveIf) {
    small_vector<int, 4> v{1, 2, 3, 4, 5, 6};
    auto removed = v.remove_if([](int x) { return x % 2 == 0; });
    
    EXPECT_EQ(removed, 3);
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 3);
    EXPECT_EQ(v[2], 5);
}

TEST(SmallVectorTest, ConcurrentVector) {
    concurrent_small_vector<int, 4> cv;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&cv, i] {
            cv.push_back(i);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(cv.size(), 10);
    
    // Test atomic update
    cv.atomic_update([](auto& vec) {
        vec.remove_duplicates();
    });
}

TEST(SmallVectorTest, CustomMemoryResource) {
    std::array<std::byte, 1024> buffer;
    std::pmr::monotonic_buffer_resource resource{buffer.data(), buffer.size()};
    
    small_vector<int, 4> v{1, 2, 3, 4};
    v.set_memory_resource(&resource);
    
    v.push_back(5);
    EXPECT_EQ(v.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(v[i], i + 1);
    }
}

class Resource {
public:
    Resource() = default;
    virtual ~Resource() = default;
    virtual int getValue() const { return 42; }
};

class DerivedResource : public Resource {
public:
    int getValue() const override { return 84; }
};

TEST(SmallVectorTest, DestroyAndClear) {
    // 测试原始指针
    small_vector<Resource*, 4> v1;
    for (int i = 0; i < 3; ++i) {
        v1.push_back(new Resource());
    }
    v1.destroy_and_clear();
    EXPECT_TRUE(v1.empty());

    // 测试智能指针
    small_vector<std::unique_ptr<Resource>, 4> v2;
    v2.push_back(std::make_unique<Resource>());
    v2.push_back(std::make_unique<DerivedResource>());
    v2.destroy_and_clear();
    EXPECT_TRUE(v2.empty());
}

TEST(SmallVectorTest, DestroyArraysAndClear) {
    // 测试原始数组指针
    small_vector<int*, 4> v1;
    for (int i = 0; i < 3; ++i) {
        v1.push_back(new int[10]);
    }
    v1.destroy_arrays_and_clear();
    EXPECT_TRUE(v1.empty());

    // 测试智能数组指针
    small_vector<std::unique_ptr<int[]>, 4> v2;
    v2.push_back(std::make_unique<int[]>(10));
    v2.push_back(std::make_unique<int[]>(20));
    v2.destroy_arrays_and_clear();
    EXPECT_TRUE(v2.empty());
}

TEST(SmallVectorTest, DestroyAndClearWithCustomDeleter) {
    struct CustomResource {
        bool* destroyed;
        explicit CustomResource(bool* d) : destroyed(d) {}
    };

    bool destroyed1 = false;
    bool destroyed2 = false;
    
    small_vector<CustomResource*, 4> v;
    v.push_back(new CustomResource(&destroyed1));
    v.push_back(new CustomResource(&destroyed2));
    
    v.destroy_and_clear_with([](CustomResource* r) {
        *r->destroyed = true;
        delete r;
    });
    
    EXPECT_TRUE(v.empty());
    EXPECT_TRUE(destroyed1);
    EXPECT_TRUE(destroyed2);
}

TEST(SmallVectorTest, MixedPointerTypes) {
    small_vector<std::unique_ptr<Resource>, 4> v;
    v.push_back(std::make_unique<Resource>());
    v.push_back(std::make_unique<DerivedResource>());
    
    // 验证多态行为
    EXPECT_EQ(v[0]->getValue(), 42);
    EXPECT_EQ(v[1]->getValue(), 84);
    
    v.destroy_and_clear();
    EXPECT_TRUE(v.empty());
}

TEST(SmallVectorTest, NullPointerHandling) {
    small_vector<Resource*, 4> v;
    v.push_back(new Resource());
    v.push_back(nullptr);
    v.push_back(new Resource());
    
    // 自定义删除器应该安全处理空指针
    v.destroy_and_clear_with([](Resource* r) {
        if (r) delete r;
    });
    
    EXPECT_TRUE(v.empty());
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
