#include "ustring.h"
#include <gtest/gtest.h>
#include <stdexcept>

class UStringVisitTest : public ::testing::Test {
 protected:
  void SetUp() override
  {
    // Create test strings
    empty_str = ustring();
    small_str = ustring("Hello");  // Small string optimization
    large_str = ustring("This is a longer string that should not fit in the small string buffer");
  }

  ustring empty_str;
  ustring small_str;
  ustring large_str;
};

// Test operator[] access
TEST_F(UStringVisitTest, SubscriptOperator)
{
  // Test non-const operator[]
  EXPECT_EQ(small_str[0], u8'H');
  EXPECT_EQ(small_str[4], u8'o');

  // Test const operator[]
  const ustring& const_str = small_str;
  EXPECT_EQ(const_str[0], u8'H');
  EXPECT_EQ(const_str[4], u8'o');

  // Test large string
  EXPECT_EQ(large_str[0], u8'T');
  EXPECT_EQ(large_str[1], u8'h');
  EXPECT_EQ(large_str[2], u8'i');
  EXPECT_EQ(large_str[3], u8's');

  // Modify string using operator[]
  small_str[0] = u8'h';
  EXPECT_EQ(small_str[0], u8'h');
  EXPECT_EQ(std::string(small_str.begin(), small_str.end()), "hello");
}

// Test at() function
TEST_F(UStringVisitTest, AtFunction)
{
  // Test non-const at()
  EXPECT_EQ(small_str.at(0), u8'H');
  EXPECT_EQ(small_str.at(4), u8'o');

  // Test const at()
  const ustring& const_str = small_str;
  EXPECT_EQ(const_str.at(0), u8'H');
  EXPECT_EQ(const_str.at(4), u8'o');

  // Test at() with invalid index
#ifdef _DEBUG
  EXPECT_THROW((void)small_str.at(5), std::out_of_range);
  EXPECT_THROW((void)const_str.at(5), std::out_of_range);
  EXPECT_THROW((void)empty_str.at(0), std::out_of_range);
#endif

  // Modify string using at()
  small_str.at(1) = u8'E';
  EXPECT_EQ(small_str.at(1), u8'E');
  EXPECT_EQ(std::string(small_str.begin(), small_str.end()), "HEllo");
}

// Test front() function
TEST_F(UStringVisitTest, FrontFunction)
{
  // Test non-const front()
  EXPECT_EQ(small_str.front(), u8'H');
  EXPECT_EQ(large_str.front(), u8'T');

  // Test const front()
  const ustring& const_str = small_str;
  EXPECT_EQ(const_str.front(), u8'H');

  // Modify front element
  small_str.front() = u8'h';
  EXPECT_EQ(small_str.front(), u8'h');
  EXPECT_EQ(std::string(small_str.begin(), small_str.end()), "hello");

  // Test front() with empty string
#ifdef _DEBUG
  EXPECT_THROW((void)empty_str.front(), std::out_of_range);
  const ustring& const_empty = empty_str;
  EXPECT_THROW((void)const_empty.front(), std::out_of_range);
#endif
}

// Test back() function
TEST_F(UStringVisitTest, BackFunction)
{
  // Test non-const back()
  EXPECT_EQ(small_str.back(), u8'o');
  EXPECT_EQ(large_str.back(), u8'r');

  // Test const back()
  const ustring& const_str = small_str;
  EXPECT_EQ(const_str.back(), u8'o');

  // Modify back element
  small_str.back() = u8'O';
  EXPECT_EQ(small_str.back(), u8'O');
  EXPECT_EQ(std::string(small_str.begin(), small_str.end()), "HellO");

  // Test back() with empty string
  EXPECT_THROW((void)empty_str.back(), std::out_of_range);
  const ustring& const_empty = empty_str;
  EXPECT_THROW((void)const_empty.back(), std::out_of_range);
}

// Test boundary conditions
TEST_F(UStringVisitTest, BoundaryConditions)
{
  // Test operator[] with boundary indices
  EXPECT_NO_THROW((void)small_str[0]);
  EXPECT_NO_THROW((void)small_str[small_str.size() - 1]);

  // Test at() with boundary indices
  EXPECT_NO_THROW((void)small_str.at(0));
  EXPECT_NO_THROW((void)small_str.at(small_str.size() - 1));
  EXPECT_THROW((void)small_str.at(small_str.size()), std::out_of_range);

  // Test modification at boundaries
  small_str[0] = u8'H';
  small_str[small_str.size() - 1] = u8'O';
  EXPECT_EQ(std::string(small_str.begin(), small_str.end()), "HellO");
}

// Test const correctness
TEST_F(UStringVisitTest, ConstCorrectness)
{
  const ustring const_str("Test");

  // These should compile and work
  EXPECT_NO_THROW({
    (void)const_str[0];
    (void)const_str.at(0);
    (void)const_str.front();
    (void)const_str.back();
  });

  // Verify values
  EXPECT_EQ(const_str[0], u8'T');
  EXPECT_EQ(const_str.at(0), u8'T');
  EXPECT_EQ(const_str.front(), u8'T');
  EXPECT_EQ(const_str.back(), u8't');
}
