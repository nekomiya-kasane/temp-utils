#pragma once

#include <cstdint>
#include <type_traits>

// Compiler detection
#if defined(_MSC_VER)
#  define COMPILER_MSVC 1
#  define COMPILER_NAME "MSVC"
#  define COMPILER_VERSION _MSC_VER
#  define COMPILER_VERSION_MAJOR (_MSC_VER / 100)
#  define COMPILER_VERSION_MINOR (_MSC_VER % 100)
#elif defined(__clang__)
#  define COMPILER_CLANG 1
#  define COMPILER_NAME "Clang"
#  define COMPILER_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#  define COMPILER_VERSION_MAJOR __clang_major__
#  define COMPILER_VERSION_MINOR __clang_minor__
#elif defined(__GNUC__)
#  define COMPILER_GCC 1
#  define COMPILER_NAME "GCC"
#  define COMPILER_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#  define COMPILER_VERSION_MAJOR __GNUC__
#  define COMPILER_VERSION_MINOR __GNUC_MINOR__
#else
#  define COMPILER_UNKNOWN 1
#  define COMPILER_NAME "Unknown"
#  define COMPILER_VERSION 0
#  define COMPILER_VERSION_MAJOR 0
#  define COMPILER_VERSION_MINOR 0
#endif

// Compiler-specific attributes and intrinsics
#ifdef COMPILER_MSVC
#  define FORCEINLINE __forceinline
#  define NOINLINE __declspec(noinline)
#  define NOVTABLE __declspec(novtable)        // Suppress vtable generation
#  define EMPTY_BASES __declspec(empty_bases)  // Enable empty base optimization
#  define VECTORCALL __vectorcall              // Vector calling convention
#  define RESTRICT __restrict
#  define DEBUGBREAK() __debugbreak()
#  define CURRENT_FUNCTION __FUNCSIG__
#  define ASSUME(cond) __assume(cond)
#  define PREFETCH(ptr) _mm_prefetch(reinterpret_cast<const char *>(ptr), _MM_HINT_T0)
#  define ASSUME_UNREACHABLE() __assume(0)
// #  define LIKELY(x) (x)
// #  define UNLIKELY(x) (x)
#  define THREAD_LOCAL __declspec(thread)
#  define DLL_EXPORT __declspec(dllexport)
#  define DLL_IMPORT __declspec(dllimport)
#  define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#  define RETURNS_NONNULL _Ret_notnull_
#  define ALLOC_ALIGN(x) __declspec(align(x))
#  define SELECTANY __declspec(selectany)
#elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  define FORCEINLINE inline __attribute__((always_inline))
#  define NOINLINE __attribute__((noinline))
#  define NOVTABLE
#  define EMPTY_BASES
#  define VECTORCALL
#  define RESTRICT __restrict__
#  define DEBUGBREAK() __builtin_trap()
#  define CURRENT_FUNCTION __PRETTY_FUNCTION__
#  define ASSUME(cond) __builtin_assume(cond)
#  define PREFETCH(ptr) __builtin_prefetch(ptr)
#  define ASSUME_UNREACHABLE() __builtin_unreachable()
// #  define LIKELY(x) __builtin_expect(!!(x), 1)
// #  define UNLIKELY(x) __builtin_expect(!!(x), 0)
#  define THREAD_LOCAL __thread
#  define DLL_EXPORT __attribute__((visibility("default")))
#  define DLL_IMPORT
#  define NO_UNIQUE_ADDRESS [[no_unique_address]]
#  define RETURNS_NONNULL __attribute__((returns_nonnull))
#  define ALLOC_ALIGN(x) __attribute__((aligned(x)))
#  define SELECTANY __attribute__((weak))
#else
#  define FORCEINLINE inline
#  define NOINLINE
#  define NOVTABLE
#  define EMPTY_BASES
#  define VECTORCALL
#  define RESTRICT
#  define DEBUGBREAK() (void)0
#  define CURRENT_FUNCTION __func__
#  define ASSUME(cond) (void)(cond)
#  define PREFETCH(ptr) (void)(ptr)
#  define ASSUME_UNREACHABLE() (void)0
// #  define LIKELY(x) (x)
// #  define UNLIKELY(x) (x)
#  define THREAD_LOCAL thread_local
#  define DLL_EXPORT
#  define DLL_IMPORT
#  define NO_UNIQUE_ADDRESS
#  define RETURNS_NONNULL
#  define ALLOC_ALIGN(x)
#  define SELECTANY
#endif

// SIMD instruction set detection and intrinsics
#if defined(COMPILER_MSVC)
#  include <intrin.h>
#  if defined(_M_X64) || defined(_M_IX86)
#    if defined(__AVX512F__)
#      define SIMD_AVX512 1
#      include <immintrin.h>
#    elif defined(__AVX2__)
#      define SIMD_AVX2 1
#      include <immintrin.h>
#    elif defined(__AVX__)
#      define SIMD_AVX 1
#      include <immintrin.h>
#    elif defined(_M_IX86_FP)
#      if _M_IX86_FP >= 2
#        define SIMD_SSE2 1
#        include <emmintrin.h>
#      elif _M_IX86_FP >= 1
#        define SIMD_SSE 1
#        include <xmmintrin.h>
#      endif
#    endif
#  elif defined(_M_ARM64)
#    define SIMD_NEON 1
#    include <arm64_neon.h>
#  elif defined(_M_ARM)
#    define SIMD_NEON 1
#    include <arm_neon.h>
#  endif
#elif defined(COMPILER_CLANG) || defined(COMPILER_GCC)
#  if defined(__AVX512F__)
#    define SIMD_AVX512 1
#    include <immintrin.h>
#  elif defined(__AVX2__)
#    define SIMD_AVX2 1
#    include <immintrin.h>
#  elif defined(__AVX__)
#    define SIMD_AVX 1
#    include <immintrin.h>
#  elif defined(__SSE2__)
#    define SIMD_SSE2 1
#    include <emmintrin.h>
#  elif defined(__SSE__)
#    define SIMD_SSE 1
#    include <xmmintrin.h>
#  elif defined(__ARM_NEON)
#    define SIMD_NEON 1
#    include <arm_neon.h>
#  endif
#endif

// Compiler-specific optimizations
#ifdef COMPILER_MSVC
#  define MSVC_INTRINSIC(x) __declspec(intrinsic) x
#  define NAKED __declspec(naked)
#  define ALIGN_STACK __declspec(align_stack(16))
#  define SAFEBUFFERS __declspec(safebuffers)
#  define GUARD_CF __declspec(guard(cf))               // Control Flow Guard
#  define STRICT_GS_CHECK __declspec(strict_gs_check)  // Stack buffer overrun detection
#  define CALL_ONCE_IN_DLL __declspec(call_once)
#  define NO_SANITIZE_ADDRESS __declspec(no_sanitize_address)
#  define NO_SANITIZE __declspec(no_sanitize)
#  define RESTRICT_REFERENCES __declspec(restrict_references)
#  define ANALYSIS_ASSUME(expr) __analysis_assume(expr)
#  define DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#elif defined(COMPILER_CLANG)
#  define MSVC_INTRINSIC(x) x
#  define NAKED __attribute__((naked))
#  define ALIGN_STACK __attribute__((force_align_arg_pointer))
#  define SAFEBUFFERS
#  define GUARD_CF
#  define STRICT_GS_CHECK
#  define CALL_ONCE_IN_DLL
#  define NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#  define NO_SANITIZE(what) __attribute__((no_sanitize(what)))
#  define RESTRICT_REFERENCES
#  define ANALYSIS_ASSUME(expr) __builtin_assume(expr)
#  define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#elif defined(COMPILER_GCC)
#  define MSVC_INTRINSIC(x) x
#  define NAKED __attribute__((naked))
#  define ALIGN_STACK __attribute__((force_align_arg_pointer))
#  define SAFEBUFFERS
#  define GUARD_CF
#  define STRICT_GS_CHECK
#  define CALL_ONCE_IN_DLL
#  define NO_SANITIZE_ADDRESS __attribute__((no_sanitize_address))
#  define NO_SANITIZE(what) __attribute__((no_sanitize(what)))
#  define RESTRICT_REFERENCES
#  define ANALYSIS_ASSUME(expr) \
    do { \
      if (!(expr)) \
        __builtin_unreachable(); \
    } while (0)
#  define DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#else
#  define MSVC_INTRINSIC(x) x
#  define NAKED
#  define ALIGN_STACK
#  define SAFEBUFFERS
#  define GUARD_CF
#  define STRICT_GS_CHECK
#  define CALL_ONCE_IN_DLL
#  define NO_SANITIZE_ADDRESS
#  define NO_SANITIZE(what)
#  define RESTRICT_REFERENCES
#  define ANALYSIS_ASSUME(expr) (void)(expr)
#  define DEPRECATED_MSG(msg)
#endif

// Memory alignment helpers
template<typename T>
FORCEINLINE constexpr bool is_aligned(const void *ptr, size_t alignment = alignof(T))
{
  return (reinterpret_cast<uintptr_t>(ptr) & (alignment - 1)) == 0;
}

template<typename T> FORCEINLINE T *align_pointer(void *ptr, size_t alignment = alignof(T))
{
  return reinterpret_cast<T *>((reinterpret_cast<uintptr_t>(ptr) + alignment - 1) &
                               ~(alignment - 1));
}

// Convenience macros for class features
#define DISABLE_COPY(Class) \
  Class(const Class &) = delete; \
  Class &operator=(const Class &) = delete

#define DISABLE_MOVE(Class) \
  Class(Class &&) = delete; \
  Class &operator=(Class &&) = delete

#define DISABLE_COPY_AND_MOVE(Class) \
  DISABLE_COPY(Class); \
  DISABLE_MOVE(Class);

#define DEFAULT_COPY(Class) \
  Class(const Class &) = default; \
  Class &operator=(const Class &) = default

#define DEFAULT_MOVE(Class) \
  Class(Class &&) = default; \
  Class &operator=(Class &&) = default

#define DEFAULT_COPY_AND_MOVE(Class) \
  DEFAULT_COPY(Class); \
  DEFAULT_MOVE(Class);

#define DISABLE_ASSIGN_OPERATOR(Class) \
  Class &operator=(const Class &) = delete; \
  Class &operator=(Class &&) = delete

#define DEFAULT_ASSIGN_OPERATOR(Class) \
  Class &operator=(const Class &) = default; \
  Class &operator=(Class &&) = default
