#include "small_vector.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>
#include <format>

TEST(SmallVectorTest, DefaultConstruction)
{
  small_vector<int, 4> v;
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(v.size(), 0);
  EXPECT_GE(v.capacity(), 4);
}

TEST(SmallVectorTest, InitializerListConstruction)
{
  small_vector<int, 4> v{1, 2, 3, 4};
  EXPECT_EQ(v.size(), 4);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[3], 4);
}

TEST(SmallVectorTest, PushBackInline)
{
  small_vector<int, 4> v;
  v.push_back(1);
  v.push_back(2);
  EXPECT_EQ(v.size(), 2);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
}

TEST(SmallVectorTest, PushBackHeap)
{
  small_vector<int, 4> v;
  for (int i = 0; i < 8; ++i) {
    v.push_back(i);
  }
  EXPECT_EQ(v.size(), 8);
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(v[i], i);
  }
}

TEST(SmallVectorTest, CopyConstruction)
{
  small_vector<int, 4> v1{1, 2, 3, 4, 5};  // Will use heap
  small_vector<int, 4> v2 = v1;
  EXPECT_EQ(v1.size(), v2.size());
  for (size_t i = 0; i < v1.size(); ++i) {
    EXPECT_EQ(v1[i], v2[i]);
  }
}

TEST(SmallVectorTest, MoveConstruction)
{
  small_vector<int, 4> v1{1, 2, 3, 4, 5};  // Will use heap
  size_t size = v1.size();
  small_vector<int, 4> v2 = std::move(v1);
  EXPECT_EQ(v2.size(), size);
  EXPECT_EQ(v1.size(), 0);  // NOLINT: checking moved-from state
  for (size_t i = 0; i < size; ++i) {
    EXPECT_EQ(v2[i], i + 1);
  }
}

TEST(SmallVectorTest, CopyAssignment)
{
  small_vector<int, 4> v1{1, 2, 3};
  small_vector<int, 4> v2{4, 5, 6, 7, 8};  // Will use heap
  v1 = v2;
  EXPECT_EQ(v1.size(), v2.size());
  for (size_t i = 0; i < v1.size(); ++i) {
    EXPECT_EQ(v1[i], v2[i]);
  }
}

TEST(SmallVectorTest, MoveAssignment)
{
  small_vector<int, 4> v1{1, 2, 3};
  small_vector<int, 4> v2{4, 5, 6, 7, 8};  // Will use heap
  size_t size = v2.size();
  v1 = std::move(v2);
  EXPECT_EQ(v1.size(), size);
  EXPECT_EQ(v2.size(), 0);  // NOLINT: checking moved-from state
  for (size_t i = 0; i < size; ++i) {
    EXPECT_EQ(v1[i], i + 4);
  }
}

TEST(SmallVectorTest, Reserve)
{
  small_vector<int, 4> v;
  v.reserve(10);
  EXPECT_GE(v.capacity(), 10);
  EXPECT_EQ(v.size(), 0);
}

TEST(SmallVectorTest, Clear)
{
  small_vector<int, 4> v{1, 2, 3, 4, 5};
  v.clear();
  EXPECT_TRUE(v.empty());
  EXPECT_EQ(v.size(), 0);
}

TEST(SmallVectorTest, PopBack)
{
  small_vector<int, 4> v{1, 2, 3, 4, 5};
  v.pop_back();
  EXPECT_EQ(v.size(), 4);
  EXPECT_EQ(v.back(), 4);
}

TEST(SmallVectorTest, Resize)
{
  small_vector<int, 4> v{1, 2};
  v.resize(4, 10);
  EXPECT_EQ(v.size(), 4);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 2);
  EXPECT_EQ(v[2], 10);
  EXPECT_EQ(v[3], 10);

  v.resize(1);
  EXPECT_EQ(v.size(), 1);
  EXPECT_EQ(v[0], 1);
}

TEST(SmallVectorTest, Insert)
{
  small_vector<int, 4> v{1, 2, 4};
  auto it = v.insert(v.begin() + 2, 3);
  EXPECT_EQ(*it, 3);
  EXPECT_EQ(v.size(), 4);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], i + 1);
  }
}

TEST(SmallVectorTest, InsertRange)
{
  small_vector<int, 4> v{1, 4};
  std::vector<int> source{2, 3};
  v.insert(v.begin() + 1, source.begin(), source.end());
  EXPECT_EQ(v.size(), 4);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], i + 1);
  }
}

TEST(SmallVectorTest, Erase)
{
  small_vector<int, 4> v{1, 2, 3, 4};
  auto it = v.erase(v.begin() + 1);
  EXPECT_EQ(*it, 3);
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 3);
  EXPECT_EQ(v[2], 4);
}

TEST(SmallVectorTest, EraseRange)
{
  small_vector<int, 4> v{1, 2, 3, 4, 5};
  auto it = v.erase(v.begin() + 1, v.begin() + 4);
  EXPECT_EQ(*it, 5);
  EXPECT_EQ(v.size(), 2);
  EXPECT_EQ(v[0], 1);
  EXPECT_EQ(v[1], 5);
}

TEST(SmallVectorTest, NonTrivialType)
{
  small_vector<std::string, 4> v;
  v.push_back("hello");
  v.push_back("world");
  EXPECT_EQ(v.size(), 2);
  EXPECT_EQ(v[0], "hello");
  EXPECT_EQ(v[1], "world");
}

TEST(SmallVectorTest, Iterator)
{
  small_vector<int, 4> v{1, 2, 3, 4};
  int sum = 0;
  for (const auto &x : v) {
    sum += x;
  }
  EXPECT_EQ(sum, 10);
}

TEST(SmallVectorTest, ReverseIterator)
{
  small_vector<int, 4> v{1, 2, 3, 4};
  std::vector<int> result;
  for (auto it = v.rbegin(); it != v.rend(); ++it) {
    result.push_back(*it);
  }
  EXPECT_EQ(result.size(), 4);
  EXPECT_EQ(result[0], 4);
  EXPECT_EQ(result[3], 1);
}

TEST(SmallVectorTest, Comparison)
{
  small_vector<int, 4> v1{1, 2, 3};
  small_vector<int, 4> v2{1, 2, 3};
  small_vector<int, 4> v3{1, 2, 4};
  small_vector<int, 4> v4{1, 2};
  small_vector<int, 4> v5{1, 2, 3, 4};

  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 == v3);
  EXPECT_FALSE(v1 == v4);
  EXPECT_FALSE(v1 == v5);
}

TEST(SmallVectorTest, AssignRange)
{
  small_vector<int, 4> v;
  std::vector<int> source{1, 2, 3, 4};
  v.assign(source.begin(), source.end());
  EXPECT_EQ(v.size(), 4);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], source[i]);
  }
}

TEST(SmallVectorTest, AssignInitializerList)
{
  small_vector<int, 4> v;
  v.assign({1, 2, 3, 4});
  EXPECT_EQ(v.size(), 4);
  for (size_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(v[i], i + 1);
  }
}

TEST(SmallVectorTest, EmplaceBack)
{
  small_vector<std::string, 4> v;
  v.emplace_back(3, 'a');
  EXPECT_EQ(v.size(), 1);
  EXPECT_EQ(v[0], "aaa");
}

TEST(SmallVectorTest, Emplace)
{
  small_vector<std::string, 4> v{"hello", "world"};
  auto it = v.emplace(v.begin() + 1, 3, 'a');
  EXPECT_EQ(*it, "aaa");
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], "hello");
  EXPECT_EQ(v[1], "aaa");
  EXPECT_EQ(v[2], "world");
}

// Non-default constructible type
class NonDefaultConstructible {
 public:
  explicit NonDefaultConstructible(int v) : value(v) {}
  NonDefaultConstructible(const NonDefaultConstructible&) = default;
  NonDefaultConstructible& operator=(const NonDefaultConstructible&) = default;
  bool operator==(const NonDefaultConstructible& other) const { return value == other.value; }
  int value;
};

// Move-only type
class MoveOnly {
 public:
  explicit MoveOnly(int v) : value(new int(v)) {}
  MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = nullptr; }
  MoveOnly& operator=(MoveOnly&& other) noexcept {
    if (this != &other) {
      delete value;
      value = other.value;
      other.value = nullptr;
    }
    return *this;
  }
  ~MoveOnly() { delete value; }
  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  bool operator==(const MoveOnly& other) const { return *value == *other.value; }
  int getValue() const { return *value; }
 private:
  int* value;
};

// Type with throwing constructor
class ThrowingCtor {
 public:
  explicit ThrowingCtor(bool should_throw = false) {
    if (should_throw) throw std::runtime_error("Constructor throw");
    value = new int(42);
  }
  ThrowingCtor(const ThrowingCtor& other) : value(new int(*other.value)) {}
  ThrowingCtor& operator=(const ThrowingCtor& other) {
    if (this != &other) {
      delete value;
      value = new int(*other.value);
    }
    return *this;
  }
  ~ThrowingCtor() { delete value; }
  bool operator==(const ThrowingCtor& other) const { return *value == *other.value; }
 private:
  int* value;
};

TEST(SmallVectorTest, NonDefaultConstructibleType) {
  small_vector<NonDefaultConstructible, 4> v;
  v.emplace_back(1);
  v.emplace_back(2);
  v.emplace(v.begin() + 1, 3);
  //
  //EXPECT_EQ(v.size(), 3);
  //EXPECT_EQ(v[0].value, 1);
  //EXPECT_EQ(v[1].value, 3);
  //EXPECT_EQ(v[2].value, 2);
}

TEST(SmallVectorTest, MoveOnlyType) {
  small_vector<MoveOnly, 4> v;
  v.emplace_back(1);
  v.emplace_back(2);
  v.emplace_back(3);
  
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0].getValue(), 1);
  EXPECT_EQ(v[1].getValue(), 2);
  EXPECT_EQ(v[2].getValue(), 3);
  
  // Test move construction
  small_vector<MoveOnly, 4> v2 = std::move(v);
  EXPECT_EQ(v2.size(), 3);
  EXPECT_EQ(v2[0].getValue(), 1);
  EXPECT_EQ(v2[1].getValue(), 2);
  EXPECT_EQ(v2[2].getValue(), 3);
}

TEST(SmallVectorTest, ThrowingConstructor) {
  small_vector<ThrowingCtor, 4> v;
  
  // Non-throwing case
  EXPECT_NO_THROW({
    v.emplace_back(false);
    v.emplace_back(false);
  });
  
  EXPECT_EQ(v.size(), 2);
  
  // Throwing case
  EXPECT_THROW(v.emplace_back(true), std::runtime_error);
  
  // Size should remain unchanged after exception
  EXPECT_EQ(v.size(), 2);
}

TEST(SmallVectorTest, ComplexReallocation) {
  small_vector<std::vector<int>, 2> v;
  
  // Add some vectors with data
  v.emplace_back(std::vector<int>{1, 2, 3});
  v.emplace_back(std::vector<int>{4, 5, 6});
  
  // This will cause reallocation
  v.emplace_back(std::vector<int>{7, 8, 9});
  
  EXPECT_EQ(v.size(), 3);
  EXPECT_EQ(v[0], (std::vector<int>{1, 2, 3}));
  EXPECT_EQ(v[1], (std::vector<int>{4, 5, 6}));
  EXPECT_EQ(v[2], (std::vector<int>{7, 8, 9}));
}

TEST(SmallVectorTest, MixedOperations) {
  small_vector<std::string, 4> v;
  
  // Test various operations mixed together
  v.emplace_back("hello");
  v.emplace(v.begin(), "world");
  v.push_back("!");
  EXPECT_EQ(v.size(), 3);
  
  // Insert in the middle
  v.insert(v.begin() + 1, {"there", "beautiful"});
  
  // Erase and replace
  v.erase(v.begin() + 2);
  v[2] = "gorgeous";
  
  EXPECT_EQ(v.size(), 4);
  EXPECT_EQ(v[0], "world");
  EXPECT_EQ(v[1], "there");
  EXPECT_EQ(v[2], "gorgeous");
  EXPECT_EQ(v[3], "!");
}

TEST(SmallVectorTest, VectorConversion) {
  small_vector<int> sv = {1, 2, 3, 4, 5};
  
  // Implicit conversion
  std::vector<int> v1 = sv;
  EXPECT_EQ(v1.size(), 5);
  EXPECT_TRUE(std::equal(v1.begin(), v1.end(), sv.begin()));

  // Explicit conversion
  auto v2 = sv.to_vector();
  EXPECT_EQ(v2.size(), 5);
  EXPECT_TRUE(std::equal(v2.begin(), v2.end(), sv.begin()));
}

TEST(SmallVectorTest, ViewSupport) {
  small_vector<int> sv = {1, 2, 3, 4, 5};
  auto view = sv.to_view();

  // Basic properties
  EXPECT_EQ(view.size(), 5);
  EXPECT_FALSE(view.empty());
  EXPECT_EQ(view.front(), 1);
  EXPECT_EQ(view.back(), 5);

  // Iterator access
  EXPECT_TRUE(std::equal(view.begin(), view.end(), sv.begin()));
  EXPECT_TRUE(std::equal(view.rbegin(), view.rend(), sv.rbegin()));

  // Element access
  EXPECT_EQ(view[2], 3);
  EXPECT_EQ(view.at(2), 3);
  EXPECT_THROW((void)view.at(5), std::out_of_range);

  // Subview operations
  auto view2 = view;
  view2.remove_prefix(2);
  EXPECT_EQ(view2.size(), 3);
  EXPECT_EQ(view2.front(), 3);

  view2 = view;
  view2.remove_suffix(2);
  EXPECT_EQ(view2.size(), 3);
  EXPECT_EQ(view2.back(), 3);

  // Vector conversion
  std::vector<int> v1 = view;
  EXPECT_EQ(v1.size(), 5);
  EXPECT_TRUE(std::equal(v1.begin(), v1.end(), sv.begin()));

  auto v2 = view.to_vector();
  EXPECT_EQ(v2.size(), 5);
  EXPECT_TRUE(std::equal(v2.begin(), v2.end(), sv.begin()));

  // Comparison
  auto view3 = sv.to_view();
  EXPECT_EQ(view, view3);
  view3.remove_prefix(1);
  EXPECT_NE(view, view3);
}

//TEST(SmallVectorTest, ToString) {
//  small_vector<int> v = {1, 2, 3, 4, 5};
//  EXPECT_EQ(to_string(v), "[1, 2, 3, 4, 5]");
//
//  small_vector<std::string> sv = {"hello", "world"};
//  EXPECT_EQ(to_string(sv), "[\"hello\", \"world\"]");
//
//  small_vector<double> dv = {1.5, 2.7, 3.14};
//  EXPECT_EQ(to_string(dv), "[1.500000, 2.700000, 3.140000]");
//
//  // Test empty vector
//  small_vector<int> empty;
//  EXPECT_EQ(to_string(empty), "[]");
//
//  // Test single element
//  small_vector<int> single = {42};
//  EXPECT_EQ(to_string(single), "[42]");
//
//  // Test with custom type
//  struct Point {
//    int x, y;
//    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
//      return os << "(" << p.x << "," << p.y << ")";
//    }
//  };
//  small_vector<Point> pv;
//  pv.emplace_back(Point{1, 2});
//  pv.emplace_back(Point{3, 4});
//  EXPECT_EQ(to_string(pv), "[(1,2), (3,4)]");
//}
//
//TEST(SmallVectorTest, FormatSupport) {
//  small_vector<int> v = {1, 2, 3};
//  EXPECT_EQ(std::format("{}", v), "[1, 2, 3]");
//  EXPECT_EQ(std::format("{:s}", v), "[1, 2, 3]");
//
//  small_vector<std::string> sv = {"hello", "world"};
//  EXPECT_EQ(std::format("{}", sv), "[\"hello\", \"world\"]");
//}
