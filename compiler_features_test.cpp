#include "compiler_features.h"
#include <bitset>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <vector>

// Helper function to print compiler information
void PrintCompilerInfo()
{
  std::cout << "\nCompiler Information:\n"
            << "  Name: " << COMPILER_NAME << "\n"
            << "  Version: " << COMPILER_VERSION << "\n"
            << "  Major: " << COMPILER_VERSION_MAJOR << "\n"
            << "  Minor: " << COMPILER_VERSION_MINOR << "\n\n";
}

// Helper function to print SIMD support
void PrintSIMDSupport()
{
  std::cout << "SIMD Support:\n";
#ifdef SIMD_AVX512
  std::cout << "  AVX512: Yes\n";
#else
  std::cout << "  AVX512: No\n";
#endif

#ifdef SIMD_AVX2
  std::cout << "  AVX2: Yes\n";
#else
  std::cout << "  AVX2: No\n";
#endif

#ifdef SIMD_AVX
  std::cout << "  AVX: Yes\n";
#else
  std::cout << "  AVX: No\n";
#endif

#ifdef SIMD_SSE2
  std::cout << "  SSE2: Yes\n";
#else
  std::cout << "  SSE2: No\n";
#endif

#ifdef SIMD_NEON
  std::cout << "  NEON: Yes\n";
#else
  std::cout << "  NEON: No\n";
#endif
  std::cout << "\n";
}

// ReSharper disable once IdentifierTypo
class NOVTABLE TestNovtable {
  virtual void foo() = 0;
};

// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class EMPTY_BASES TestEmptyBases : TestNovtable {};  // NOLINT(cppcoreguidelines-virtual-class-destructor)

// Print test information before running tests
class CompilerFeaturesTest : public testing::Test {
 protected:
  static void SetUpTestSuite()
  {
    PrintCompilerInfo();
    PrintSIMDSupport();
  }
};

// Test alignment functions
TEST_F(CompilerFeaturesTest, AlignmentFunctions)
{
  alignas(16) char buffer[32];
  void *ptr = buffer;

  EXPECT_TRUE(is_aligned<int>(ptr));
  EXPECT_TRUE(is_aligned<double>(ptr));

  // Test with custom alignment
  EXPECT_TRUE(is_aligned<void>(ptr, 16));

  // Test align_pointer
  void *aligned = align_pointer<double>(ptr);
  EXPECT_TRUE(is_aligned<double>(aligned));

  std::cout << "Alignment Test:\n"
            << "  Original ptr: " << ptr << "\n"
            << "  Aligned ptr: " << aligned << "\n"
            << "  Alignment of double: " << alignof(double) << "\n\n";
}

// Test function information macros
TEST_F(CompilerFeaturesTest, FunctionInformation)
{
  std::cout << "Function Information:\n"
            << "  Current function: " << CURRENT_FUNCTION << "\n\n";
  std::string func = CURRENT_FUNCTION;
  EXPECT_FALSE(func.empty());
}

// Example of a class using various compiler-specific attributes
class EMPTY_BASES DLL_EXPORT TestClass {
 public:
  // ReSharper disable once CppMemberFunctionMayBeStatic
  FORCEINLINE int fastFunction()
  {
    return 42;
  }
  // ReSharper disable once CppMemberFunctionMayBeStatic
  NOINLINE int slowFunction()
  {
    return 43;
  }

  // Function that should never return null
  RETURNS_NONNULL int *getNonNullPtr()
  {
    return &dummy;
  }

 private:
  ALLOC_ALIGN(16) int dummy { 0 };
};

TEST_F(CompilerFeaturesTest, CompilerAttributes)
{
  TestClass test;
  EXPECT_EQ(test.fastFunction(), 42);
  EXPECT_EQ(test.slowFunction(), 43);
  EXPECT_NE(test.getNonNullPtr(), nullptr);
}

// Test prefetch hint
TEST_F(CompilerFeaturesTest, PrefetchHint)
{
  std::vector<int> data(1000);
  for (int i = 0; i < 1000; ++i) {
    PREFETCH(&data[i < 996 ? i + 4 : i]);
    data[i] = i;
  }
  EXPECT_EQ(data[999], 999);
}

// Test unreachable code optimization
NOINLINE int testUnreachable(int x)  // NOLINT(misc-use-internal-linkage)
{
  switch (x) {
    case 0:
      return 0;
    case 1:
      return 1;
    default:
      ASSUME_UNREACHABLE();
  }
}

TEST_F(CompilerFeaturesTest, UnreachableCode)
{
  EXPECT_EQ(testUnreachable(0), 0);
  EXPECT_EQ(testUnreachable(1), 1);
}

// Test deprecated functionality
class [[deprecated("Use NewClass instead")]] OldClass {};

DEPRECATED_MSG("Use new_function() instead")
void old_function() {}  // NOLINT(misc-use-internal-linkage)

TEST_F(CompilerFeaturesTest, DeprecatedFunctionality)
{
  // These lines should generate compiler warnings
  // ReSharper disable once CppDeprecatedEntity
  OldClass oldObj;  // NOLINT(clang-diagnostic-deprecated-declarations)
  // ReSharper disable once CppDeprecatedEntity
  old_function();
}
