#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace simd_traits {

// Platform detection
#if defined(_MSC_VER)
    #define USTRING_MSVC
#elif defined(__GNUC__)
    #define USTRING_GCC
#elif defined(__clang__)
    #define USTRING_CLANG
#endif

// Architecture detection
#if defined(_M_X64) || defined(__x86_64__)
    #define USTRING_X64
#elif defined(_M_IX86) || defined(__i386__)
    #define USTRING_X86
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define USTRING_ARM64
#elif defined(_M_ARM) || defined(__arm__)
    #define USTRING_ARM
#endif

// SIMD instruction set detection
struct simd_support {
    // x86/x64 instruction sets
    static constexpr bool has_sse = 
#if defined(USTRING_MSVC)
        #if defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 1)
            true
        #else
            false
        #endif
#elif defined(USTRING_GCC) || defined(USTRING_CLANG)
        #if defined(__SSE__)
            true#include "simd_traits.h"
            
            void process_data(float* data, size_t size) {
                if (simd_traits::simd_support::has_avx2 && 
                    simd_traits::is_aligned_for_simd(data) &&
                    simd_traits::is_size_simd_compatible<float>(size)) {
                    // Use AVX2 implementation
                } else {
                    // Use scalar fallback
                }
            }
        #else
            false
        #endif
#else
        false
#endif
        ;

    static constexpr bool has_sse2 = 
#if defined(USTRING_MSVC)
        #if defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
            true
        #else
            false
        #endif
#elif defined(USTRING_GCC) || defined(USTRING_CLANG)
        #if defined(__SSE2__)
            true
        #else
            false
        #endif
#else
        false
#endif
        ;

    static constexpr bool has_sse3 = 
#if defined(__SSE3__)
        true
#else
        false
#endif
        ;

    static constexpr bool has_ssse3 = 
#if defined(__SSSE3__)
        true
#else
        false
#endif
        ;

    static constexpr bool has_sse4_1 = 
#if defined(__SSE4_1__)
        true
#else
        false
#endif
        ;

    static constexpr bool has_sse4_2 = 
#if defined(__SSE4_2__)
        true
#else
        false
#endif
        ;

    static constexpr bool has_avx = 
#if defined(__AVX__)
        true
#else
        false
#endif
        ;

    static constexpr bool has_avx2 = 
#if defined(__AVX2__)
        true
#else
        false
#endif
        ;

    // ARM NEON support
    static constexpr bool has_neon = 
#if defined(USTRING_ARM64)
        true  // ARM64 always has NEON
#elif defined(USTRING_ARM)
    #if defined(__ARM_NEON) || defined(__ARM_NEON__)
        true
    #else
        false
    #endif
#else
        false
#endif
        ;

    // ARM SVE support
    static constexpr bool has_sve = 
#if defined(__ARM_FEATURE_SVE)
        true
#else
        false
#endif
        ;
};

// SIMD register width detection
struct simd_width {
    static constexpr size_t sse = 128;  // SSE registers are 128-bit
    static constexpr size_t avx = 256;  // AVX registers are 256-bit
    static constexpr size_t avx512 = 512;  // AVX-512 registers are 512-bit
    static constexpr size_t neon = 128;  // NEON registers are 128-bit
    static constexpr size_t sve = 0;  // SVE register size is runtime dependent

    // Get the widest available SIMD register width
    static constexpr size_t native = 
#if defined(__AVX512F__)
        avx512
#elif defined(__AVX__)
        avx
#elif defined(__SSE__) || defined(__ARM_NEON) || defined(__ARM_NEON__)
        sse
#else
        0
#endif
        ;
};

// Alignment requirements
struct simd_alignment {
    static constexpr size_t sse = 16;
    static constexpr size_t avx = 32;
    static constexpr size_t avx512 = 64;
    static constexpr size_t neon = 16;

    // Get the strictest alignment requirement for available SIMD instructions
    static constexpr size_t native = 
#if defined(__AVX512F__)
        avx512
#elif defined(__AVX__)
        avx
#elif defined(__SSE__) || defined(__ARM_NEON) || defined(__ARM_NEON__)
        sse
#else
        alignof(std::max_align_t)
#endif
        ;
};

// Helper templates for SIMD operations
template<typename T>
struct simd_traits {
    static constexpr bool is_simd_compatible = 
        std::is_arithmetic_v<T> && 
        !std::is_same_v<T, bool> &&
        (sizeof(T) <= simd_width::native / 8);

    static constexpr size_t elements_per_register = 
        is_simd_compatible ? simd_width::native / (sizeof(T) * 8) : 1;

    static constexpr size_t alignment = simd_alignment::native;
};

// Utility functions
template<typename T>
constexpr bool is_aligned_for_simd(const T* ptr) {
    return reinterpret_cast<std::uintptr_t>(ptr) % simd_alignment::native == 0;
}

template<typename T>
constexpr bool is_size_simd_compatible(size_t size) {
    return size % simd_traits<T>::elements_per_register == 0;
}

} // namespace simd_traits
