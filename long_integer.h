#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>
#include <stdexcept>

// Forward declarations
struct uint128_t;
struct int128_t;
struct uint256_t;
struct int256_t;

// 128-bit unsigned integer
struct uint128_t {
  uint64_t low;
  uint64_t high;

  constexpr uint128_t() : low(0), high(0) {}

  template<typename T>
    requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) <= 8)
  explicit constexpr uint128_t(T l) : low(l), high(l < 0 ? -1 : 0)
  {
  }
  template<typename T>
    requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) <= 8)
  explicit constexpr uint128_t(T l) : low(l), high(0)
  {
  }
  explicit constexpr uint128_t(const int128_t &v);
  explicit constexpr uint128_t(uint64_t l) : low(l), high(0) {}
  constexpr uint128_t(uint64_t h, uint64_t l) : low(l), high(h) {}

  // Additional conversion operators
  explicit constexpr operator bool() const noexcept
  {
    return high != 0 || low != 0;
  }
  explicit constexpr operator uint64_t() const noexcept
  {
    return low;
  }
  explicit constexpr operator uint32_t() const noexcept
  {
    return static_cast<uint32_t>(low);
  }
  explicit constexpr operator uint16_t() const noexcept
  {
    return static_cast<uint16_t>(low);
  }
  explicit constexpr operator uint8_t() const noexcept
  {
    return static_cast<uint8_t>(low);
  }
  explicit constexpr operator int64_t() const noexcept
  {
    return static_cast<int64_t>(low);
  }
  explicit constexpr operator int32_t() const noexcept
  {
    return static_cast<int32_t>(low);
  }
  explicit constexpr operator int16_t() const noexcept
  {
    return static_cast<int16_t>(low);
  }
  explicit constexpr operator int8_t() const noexcept
  {
    return static_cast<int8_t>(low);
  }
  explicit constexpr operator double() const noexcept
  {
    constexpr double two_64 = 18446744073709551616.0;  // 2^64
    return static_cast<double>(high) * two_64 + static_cast<double>(low);
  }
  explicit constexpr operator float() const noexcept
  {
    return static_cast<float>(static_cast<double>(*this));
  }

  // Arithmetic operators
  constexpr uint128_t &operator+=(const uint128_t &rhs)
  {
    uint64_t old_low = low;
    low += rhs.low;
    high += rhs.high + (low < old_low ? 1 : 0);
    return *this;
  }

  constexpr uint128_t &operator-=(const uint128_t &rhs)
  {
    uint64_t old_low = low;
    low -= rhs.low;
    high -= rhs.high + (low > old_low ? 1 : 0);
    return *this;
  }

  constexpr uint128_t &operator*=(const uint128_t &rhs)
  {
    uint128_t result = static_cast<uint128_t>(0);
    for (int i = 0; i < 128; ++i) {
      if (rhs.bit(i))
        result += (*this << i);
    }
    *this = result;
    return *this;
  }

  constexpr uint128_t &operator/=(const uint128_t &rhs)
  {
    if (rhs == 0)
      throw std::domain_error("Division by zero");
    if (rhs == 1)
      return *this;

    uint128_t quotient = static_cast<uint128_t>(0);
    uint128_t remainder = *this;

    while (remainder >= rhs) {
      int shift = std::countl_zero(rhs.high);
      uint128_t shifted = rhs << shift;
      if (shifted > remainder) {
        shifted >>= 1;
        --shift;
      }
      remainder -= shifted;
      quotient |= uint128_t(1) << shift;
    }

    *this = quotient;
    return *this;
  }

  // Bitwise operators
  constexpr uint128_t &operator&=(const uint128_t &rhs)
  {
    low &= rhs.low;
    high &= rhs.high;
    return *this;
  }

  constexpr uint128_t &operator|=(const uint128_t &rhs)
  {
    low |= rhs.low;
    high |= rhs.high;
    return *this;
  }

  constexpr uint128_t &operator^=(const uint128_t &rhs)
  {
    low ^= rhs.low;
    high ^= rhs.high;
    return *this;
  }

  constexpr uint128_t &operator<<=(int shift)
  {
    if (shift >= 128) {
      high = low = 0;
    }
    else if (shift >= 64) {
      high = low << (shift - 64);
      low = 0;
    }
    else if (shift > 0) {
      high = (high << shift) | (low >> (64 - shift));
      low <<= shift;
    }
    return *this;
  }

  constexpr uint128_t &operator>>=(int shift)
  {
    if (shift >= 128) {
      high = low = 0;
    }
    else if (shift >= 64) {
      low = high >> (shift - 64);
      high = 0;
    }
    else if (shift > 0) {
      low = (low >> shift) | (high << (64 - shift));
      high >>= shift;
    }
    return *this;
  }

  constexpr uint128_t operator~() const
  {
    return {~high, ~low};
  }
  constexpr bool bit(int i) const
  {
    return i < 64 ? (low >> i) & 1 : (high >> (i - 64)) & 1;
  }

  // Binary operators
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr uint128_t operator+(const uint128_t &lhs, T rhs)
  {
    return lhs + static_cast<uint128_t>(rhs);
  }
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr uint128_t operator-(const uint128_t &lhs, T rhs)
  {
    return lhs - static_cast<uint128_t>(rhs);
  }
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr uint128_t operator*(const uint128_t &lhs, T rhs)
  {
    return lhs * static_cast<uint128_t>(rhs);
  }
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr uint128_t operator/(const uint128_t &lhs, T rhs)
  {
    return lhs / static_cast<uint128_t>(rhs);
  }
  friend constexpr uint128_t operator+(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy += rhs;
  }
  friend constexpr uint128_t operator-(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy -= rhs;
  }
  friend constexpr uint128_t operator*(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy *= rhs;
  }
  friend constexpr uint128_t operator/(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy /= rhs;
  }
  friend constexpr uint128_t operator&(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }
  friend constexpr uint128_t operator|(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }
  friend constexpr uint128_t operator^(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }
  friend constexpr uint128_t operator<<(const uint128_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy <<= shift;
  }
  friend constexpr uint128_t operator>>(const uint128_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy >>= shift;
  }

  // Additional operators for built-in types
  template<typename T>
    requires std::is_integral_v<T>
  uint128_t &operator&=(T rhs)
  {
    low &= rhs;
    high = 0;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  uint128_t &operator|=(T rhs)
  {
    low |= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  uint128_t &operator^=(T rhs)
  {
    low ^= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend uint128_t operator&(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend uint128_t operator|(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend uint128_t operator^(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend uint128_t operator&(T lhs, const uint128_t &rhs)
  {
    auto rhs_copy = rhs;
    return rhs_copy &= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend uint128_t operator|(T lhs, const uint128_t &rhs)
  {
    auto rhs_copy = rhs;
    return rhs_copy |= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend uint128_t operator^(T lhs, const uint128_t &rhs)
  {
    auto rhs_copy = rhs;
    return rhs_copy ^= lhs;
  }

  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T>
  friend constexpr std::strong_ordering operator<=>(const uint128_t &lhs, const T &rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T>
  friend constexpr bool operator==(const uint128_t &lhs, const T &rhs) noexcept;

  friend constexpr std::strong_ordering operator<=>(const uint128_t &lhs,
                                                    const uint128_t &rhs) noexcept;
  friend constexpr bool operator==(const uint128_t &lhs, const uint128_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const uint128_t &lhs,
                                                    const int128_t &rhs) noexcept;
  friend constexpr bool operator==(const uint128_t &lhs, const int128_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const uint128_t &lhs,
                                                    const uint256_t &rhs) noexcept;
  friend constexpr bool operator==(const uint128_t &lhs, const uint256_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const uint128_t &lhs,
                                                    const int256_t &rhs) noexcept;
  friend constexpr bool operator==(const uint128_t &lhs, const int256_t &rhs) noexcept;
};

// 128-bit signed integer
struct int128_t {
  uint128_t value;

  constexpr int128_t() = default;
  template<typename T>
    requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) <= 8)
  explicit constexpr int128_t(T v) : value(v)
  {
  }
  template<typename T>
    requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) <= 8)
  explicit constexpr int128_t(T v) : value(v)
  {
  }
  explicit constexpr int128_t(const uint128_t &v) : value(v) {}

  // Additional conversion operators
  explicit constexpr operator bool() const noexcept
  {
    return static_cast<bool>(value);
  }
  explicit constexpr operator uint64_t() const noexcept
  {
    return value.low;
  }
  explicit constexpr operator uint32_t() const noexcept
  {
    return static_cast<uint32_t>(value.low);
  }
  explicit constexpr operator uint16_t() const noexcept
  {
    return static_cast<uint16_t>(value.low);
  }
  explicit constexpr operator uint8_t() const noexcept
  {
    return static_cast<uint8_t>(value.low);
  }
  explicit constexpr operator int64_t() const noexcept
  {
    bool is_negative = value.high & (static_cast<uint64_t>(1) << 63);
    if (!is_negative) {
      return static_cast<int64_t>(value.low);
    }
    else {
      return -static_cast<int64_t>(~value.low + 1);
    }
  }
  explicit constexpr operator int32_t() const noexcept
  {
    return static_cast<int32_t>(static_cast<int64_t>(*this));
  }
  explicit constexpr operator int16_t() const noexcept
  {
    return static_cast<int16_t>(static_cast<int64_t>(*this));
  }
  explicit constexpr operator int8_t() const noexcept
  {
    return static_cast<int8_t>(static_cast<int64_t>(*this));
  }
  explicit constexpr operator double() const noexcept
  {
    bool is_negative = value.high & (static_cast<uint64_t>(1) << 63);
    if (!is_negative) {
      return static_cast<double>(value);
    }
    constexpr double two_64 = 18446744073709551616.0;  // 2^64
    return -(static_cast<double>(~value.high) * two_64 + static_cast<double>(~value.low) + 1.0);
  }
  explicit constexpr operator float() const noexcept
  {
    return static_cast<float>(static_cast<double>(*this));
  }

  // Arithmetic operators
  int128_t &operator+=(const int128_t &rhs)
  {
    value += rhs.value;
    return *this;
  }
  int128_t &operator-=(const int128_t &rhs)
  {
    value -= rhs.value;
    return *this;
  }
  int128_t &operator*=(const int128_t &rhs)
  {
    value *= rhs.value;
    return *this;
  }
  int128_t &operator/=(const int128_t &rhs)
  {
    value /= rhs.value;
    return *this;
  }

  // Bitwise operators
  int128_t &operator&=(const int128_t &rhs)
  {
    value &= rhs.value;
    return *this;
  }
  int128_t &operator|=(const int128_t &rhs)
  {
    value |= rhs.value;
    return *this;
  }
  int128_t &operator^=(const int128_t &rhs)
  {
    value ^= rhs.value;
    return *this;
  }
  int128_t &operator<<=(int shift)
  {
    value <<= shift;
    return *this;
  }
  int128_t &operator>>=(int shift)
  {
    value >>= shift;
    return *this;
  }

  int128_t operator~() const
  {
    return int128_t{~value};
  }
  int128_t operator-() const
  {
    return int128_t{~value + uint128_t(1)};
  }

  // Binary operators
  friend int128_t operator+(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy += rhs;
  }
  friend int128_t operator-(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy -= rhs;
  }
  friend int128_t operator*(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy *= rhs;
  }
  friend int128_t operator/(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy /= rhs;
  }
  friend int128_t operator&(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }
  friend int128_t operator|(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }
  friend int128_t operator^(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }
  friend int128_t operator<<(const int128_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy <<= shift;
  }
  friend int128_t operator>>(const int128_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy >>= shift;
  }

  // Additional operators for built-in types and int128_t
  template<typename T>
    requires std::is_integral_v<T>
  int128_t &operator&=(T rhs)
  {
    value &= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  int128_t &operator|=(T rhs)
  {
    value |= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  int128_t &operator^=(T rhs)
  {
    value ^= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int128_t operator&(int128_t lhs, T rhs)
  {
    return lhs &= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int128_t operator|(int128_t lhs, T rhs)
  {
    return lhs |= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int128_t operator^(int128_t lhs, T rhs)
  {
    return lhs ^= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int128_t operator&(T lhs, int128_t rhs)
  {
    return rhs &= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend int128_t operator|(T lhs, int128_t rhs)
  {
    return rhs |= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend int128_t operator^(T lhs, int128_t rhs)
  {
    return rhs ^= lhs;
  }

  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr std::strong_ordering operator<=>(const int128_t &lhs, const T &rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr bool operator==(const int128_t &lhs, const T &rhs) noexcept;

  friend constexpr std::strong_ordering operator<=>(const int128_t &lhs,
                                                    const uint128_t &rhs) noexcept;
  friend constexpr bool operator==(const int128_t &lhs, const uint128_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const int128_t &lhs,
                                                    const int128_t &rhs) noexcept;
  friend constexpr bool operator==(const int128_t &lhs, const int128_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const int128_t &lhs,
                                                    const uint256_t &rhs) noexcept;
  friend constexpr bool operator==(const int128_t &lhs, const uint256_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const int128_t &lhs,
                                                    const int256_t &rhs) noexcept;
  friend constexpr bool operator==(const int128_t &lhs, const int256_t &rhs) noexcept;
};

constexpr uint128_t::uint128_t(const int128_t &v) : uint128_t(v.value) {}

// 256-bit unsigned integer
struct uint256_t {
  uint128_t low;
  uint128_t high;

  constexpr uint256_t() = default;
  template<typename T>
    requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) <= 8)
  explicit constexpr uint256_t(T l) : low(l), high(l < 0 ? -1 : 0)
  {
  }
  template<typename T>
    requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) <= 8)
  explicit constexpr uint256_t(T l) : low(l), high(0)
  {
  }
  explicit constexpr uint256_t(const int128_t &l)
      : low{const_cast<int128_t &>(l)}, high(l < 0 ? -1 : 0)
  {
  }
  explicit constexpr uint256_t(const uint128_t &l) : low(l) {}
  constexpr uint256_t(const uint256_t &v) = default;
  constexpr uint256_t(uint128_t h, uint128_t l) : low(l), high(h) {}
  constexpr uint256_t(uint64_t hh, uint64_t h, uint64_t l, uint64_t ll) : low(l, ll), high(hh, h)
  {
  }

  // Additional conversion operators
  explicit constexpr operator bool() const noexcept
  {
    return static_cast<bool>(high) || static_cast<bool>(low);
  }
  explicit constexpr operator uint128_t() const noexcept
  {
    return low;
  }
  explicit constexpr operator uint64_t() const noexcept
  {
    return low.low;
  }
  explicit constexpr operator uint32_t() const noexcept
  {
    return static_cast<uint32_t>(low.low);
  }
  explicit constexpr operator uint16_t() const noexcept
  {
    return static_cast<uint16_t>(low.low);
  }
  explicit constexpr operator uint8_t() const noexcept
  {
    return static_cast<uint8_t>(low.low);
  }
  explicit constexpr operator int128_t() const noexcept
  {
    return static_cast<int128_t>(low);
  }
  explicit constexpr operator int64_t() const noexcept
  {
    return static_cast<int64_t>(low.low);
  }
  explicit constexpr operator int32_t() const noexcept
  {
    return static_cast<int32_t>(low.low);
  }
  explicit constexpr operator int16_t() const noexcept
  {
    return static_cast<int16_t>(low.low);
  }
  explicit constexpr operator int8_t() const noexcept
  {
    return static_cast<int8_t>(low.low);
  }
  explicit constexpr operator double() const noexcept
  {
    constexpr double two_128 = 3.402823669209384634633746074317682114550e+38;  // 2^128
    return static_cast<double>(high) * two_128 + static_cast<double>(low);
  }
  explicit constexpr operator float() const noexcept
  {
    return static_cast<float>(static_cast<double>(*this));
  }

  // Arithmetic operators
  uint256_t &operator+=(const uint256_t &rhs)
  {
    uint128_t old_low = low;
    low += rhs.low;
    high += rhs.high + (low < old_low ? uint128_t(1) : uint128_t(0));
    return *this;
  }

  uint256_t &operator-=(const uint256_t &rhs)
  {
    uint128_t old_low = low;
    low -= rhs.low;
    high -= rhs.high + (low > old_low ? uint128_t(1) : uint128_t(0));
    return *this;
  }

  // Bitwise operators
  uint256_t &operator&=(const uint256_t &rhs)
  {
    low &= rhs.low;
    high &= rhs.high;
    return *this;
  }

  uint256_t &operator|=(const uint256_t &rhs)
  {
    low |= rhs.low;
    high |= rhs.high;
    return *this;
  }

  uint256_t &operator^=(const uint256_t &rhs)
  {
    low ^= rhs.low;
    high ^= rhs.high;
    return *this;
  }

  uint256_t &operator<<=(int shift)
  {
    if (shift >= 256) {
      high = low = static_cast<uint128_t>(0);
    }
    else if (shift >= 128) {
      high = low << (shift - 128);
      low = static_cast<uint128_t>(0);
    }
    else if (shift > 0) {
      high = (high << shift) | (low >> (128 - shift));
      low <<= shift;
    }
    return *this;
  }

  uint256_t &operator>>=(int shift)
  {
    if (shift >= 256) {
      high = low = static_cast<uint128_t>(0);
    }
    else if (shift >= 128) {
      low = high >> (shift - 128);
      high = static_cast<uint128_t>(0);
    }
    else if (shift > 0) {
      low = (low >> shift) | (high << (128 - shift));
      high >>= shift;
    }
    return *this;
  }

  uint256_t operator~() const
  {
    return {~high, ~low};
  }

  // Binary operators
  friend uint256_t operator+(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy += rhs;
  }
  friend uint256_t operator-(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy -= rhs;
  }
  friend uint256_t operator&(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }
  friend uint256_t operator|(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }
  friend uint256_t operator^(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }
  friend uint256_t operator<<(const uint256_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy <<= shift;
  }
  friend uint256_t operator>>(const uint256_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy >>= shift;
  }

  // Additional operators for built-in types and uint128_t
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator&=(T rhs)
  {
    low &= rhs;
    high = static_cast<uint128_t>(0);
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator|=(T rhs)
  {
    low |= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator^=(T rhs)
  {
    low ^= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator&(uint256_t lhs, T rhs)
  {
    return lhs &= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator|(uint256_t lhs, T rhs)
  {
    return lhs |= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator^(uint256_t lhs, T rhs)
  {
    return lhs ^= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator&(T lhs, uint256_t rhs)
  {
    return rhs &= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator|(T lhs, uint256_t rhs)
  {
    return rhs |= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator^(T lhs, uint256_t rhs)
  {
    return rhs ^= lhs;
  }

  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr std::strong_ordering operator<=>(const uint256_t &lhs, const T &rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr bool operator==(const uint256_t &lhs, const T &rhs) noexcept;

  friend constexpr std::strong_ordering operator<=>(const uint256_t &lhs,
                                                    const int256_t &rhs) noexcept;
  friend constexpr bool operator==(const uint256_t &lhs, const int256_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const uint256_t &lhs,
                                                    const uint256_t &rhs) noexcept;
  friend constexpr bool operator==(const uint256_t &lhs, const uint256_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const uint256_t &lhs,
                                                    const int128_t &rhs) noexcept;
  friend constexpr bool operator==(const uint256_t &lhs, const int128_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const uint256_t &lhs,
                                                    const uint128_t &rhs) noexcept;
  friend constexpr bool operator==(const uint256_t &lhs, const uint128_t &rhs) noexcept;
};

// 256-bit signed integer
struct int256_t {
  uint256_t value;

  constexpr int256_t() : value() {}
  template<typename T>
    requires std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) <= 8)
  explicit constexpr int256_t(T l) : value{l}
  {
  }
  template<typename T>
    requires std::is_integral_v<T> && std::is_unsigned_v<T> && (sizeof(T) <= 8)
  explicit constexpr int256_t(T l) : value{l}
  {
  }
  constexpr int256_t(const int256_t &v) = default;
  constexpr int256_t(const int128_t &v)
      : value(v < 0 ? static_cast<uint128_t>(-1) : static_cast<uint128_t>(0), v.value)
  {
  }
  constexpr int256_t(const uint256_t &v) : value(v)
  {
    // todo: should set first bit to zero?
  }
  constexpr int256_t(const uint128_t &h, const uint128_t &l) : value{h, l} {}
  constexpr int256_t(const uint64_t hh, const uint64_t hl, const uint64_t lh, const uint64_t ll)
      : value{hh, hl, lh, ll}
  {
  }

  // Additional conversion operators
  explicit constexpr operator bool() const noexcept
  {
    return static_cast<bool>(value);
  }
  explicit constexpr operator uint128_t() const noexcept
  {
    return value.low;
  }
  explicit constexpr operator uint64_t() const noexcept
  {
    return value.low.low;
  }
  explicit constexpr operator uint32_t() const noexcept
  {
    return static_cast<uint32_t>(value.low.low);
  }
  explicit constexpr operator uint16_t() const noexcept
  {
    return static_cast<uint16_t>(value.low.low);
  }
  explicit constexpr operator uint8_t() const noexcept
  {
    return static_cast<uint8_t>(value.low.low);
  }
  explicit constexpr operator int128_t() const noexcept
  {
    bool negative = value.high.high & (static_cast<uint64_t>(1) << 63);
    return static_cast<int128_t>(negative ? value.low | (static_cast<uint64_t>(1) << 63) :
                                            value.low);
  }
  explicit constexpr operator int64_t() const noexcept
  {
    bool is_negative = value.high.high & (static_cast<uint64_t>(1) << 63);
    if (!is_negative) {
      return static_cast<int64_t>(value.low.low);
    }
    else {
      return -static_cast<int64_t>(~value.low.low + 1);
    }
  }
  explicit constexpr operator int32_t() const noexcept
  {
    return static_cast<int32_t>(static_cast<int64_t>(*this));
  }
  explicit constexpr operator int16_t() const noexcept
  {
    return static_cast<int16_t>(static_cast<int64_t>(*this));
  }
  explicit constexpr operator int8_t() const noexcept
  {
    return static_cast<int8_t>(static_cast<int64_t>(*this));
  }
  explicit constexpr operator double() const noexcept
  {
    bool is_negative = value.high.high & (static_cast<uint64_t>(1) << 63);
    if (!is_negative) {
      constexpr double two_128 = 3.402823669209384634633746074317682114550e+38;  // 2^128
      return static_cast<double>(value.high) * two_128 + static_cast<double>(value.low);
    }
    else {
      constexpr double two_128 = 3.402823669209384634633746074317682114550e+38;  // 2^128
      uint256_t abs_val = ~value + uint256_t(1);
      return -(static_cast<double>(abs_val.high) * two_128 + static_cast<double>(abs_val.low));
    }
  }
  explicit constexpr operator float() const noexcept
  {
    return static_cast<float>(static_cast<double>(*this));
  }

  // Arithmetic operators
  int256_t &operator+=(const int256_t &rhs)
  {
    value += rhs.value;
    return *this;
  }
  int256_t &operator-=(const int256_t &rhs)
  {
    value -= rhs.value;
    return *this;
  }

  // Bitwise operators
  int256_t &operator&=(const int256_t &rhs)
  {
    value &= rhs.value;
    return *this;
  }
  int256_t &operator|=(const int256_t &rhs)
  {
    value |= rhs.value;
    return *this;
  }
  int256_t &operator^=(const int256_t &rhs)
  {
    value ^= rhs.value;
    return *this;
  }
  int256_t &operator<<=(const int shift)
  {
    value <<= shift;
    return *this;
  }
  int256_t &operator>>=(const int shift)
  {
    value >>= shift;
    return *this;
  }

  int256_t operator~() const
  {
    return {~value};
  }
  int256_t operator-() const
  {
    return {~value + uint256_t(1)};
  }

  // Binary operators
  friend int256_t operator+(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy += rhs;
  }
  friend int256_t operator-(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy -= rhs;
  }
  friend int256_t operator&(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }
  friend int256_t operator|(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }
  friend int256_t operator^(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }
  friend int256_t operator<<(const int256_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy <<= shift;
  }
  friend int256_t operator>>(const int256_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy >>= shift;
  }

  // Additional operators for built-in types and int128_t
  template<typename T>
    requires std::is_integral_v<T>
  int256_t &operator&=(T rhs)
  {
    value &= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  int256_t &operator|=(T rhs)
  {
    value |= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  int256_t &operator^=(T rhs)
  {
    value ^= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int256_t operator&(int256_t lhs, T rhs)
  {
    return lhs &= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int256_t operator|(int256_t lhs, T rhs)
  {
    return lhs |= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int256_t operator^(int256_t lhs, T rhs)
  {
    return lhs ^= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int256_t operator&(T lhs, int256_t rhs)
  {
    return rhs &= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int256_t operator|(T lhs, int256_t rhs)
  {
    return rhs |= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T>
  friend int256_t operator^(T lhs, int256_t rhs)
  {
    return rhs ^= lhs;
  }

  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T>
  friend constexpr std::strong_ordering operator<=>(const int256_t &lhs, const T &rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T>
  friend constexpr bool operator==(const int256_t &lhs, const T &rhs) noexcept;

  friend constexpr std::strong_ordering operator<=>(const int256_t &lhs,
                                                    const int256_t &rhs) noexcept;
  friend constexpr bool operator==(const int256_t &lhs, const int256_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const int256_t &lhs,
                                                    const uint256_t &rhs) noexcept;
  friend constexpr bool operator==(const int256_t &lhs, const uint256_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const int256_t &lhs,
                                                    const int128_t &rhs) noexcept;
  friend constexpr bool operator==(const int256_t &lhs, const int128_t &rhs) noexcept;
  friend constexpr std::strong_ordering operator<=>(const int256_t &lhs,
                                                    const uint128_t &rhs) noexcept;
  friend constexpr bool operator==(const int256_t &lhs, const uint128_t &rhs) noexcept;
};

template<> class std::numeric_limits<uint128_t> {
 public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = false;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
#if __cplusplus > 202012L
  static constexpr auto has_denorm = denorm_absent;
#endif
  static constexpr bool has_denorm_loss = false;
  static constexpr float_round_style round_style = round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = true;
  static constexpr int digits = 128;
  static constexpr int digits10 = 38;
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = true;
  static constexpr bool tinyness_before = false;

  static constexpr uint128_t min() noexcept
  {
    return static_cast<uint128_t>(0);
  }
  static constexpr uint128_t lowest() noexcept
  {
    return min();
  }
  static constexpr uint128_t max() noexcept
  {
    return {~static_cast<uint64_t>(0), ~static_cast<uint64_t>(0)};
  }
  static constexpr uint128_t epsilon() noexcept
  {
    return static_cast<uint128_t>(0);
  }
  static constexpr uint128_t round_error() noexcept
  {
    return static_cast<uint128_t>(0);
  }
  static constexpr uint128_t infinity() noexcept
  {
    return static_cast<uint128_t>(0);
  }
  static constexpr uint128_t quiet_NaN() noexcept
  {
    return static_cast<uint128_t>(0);
  }
  static constexpr uint128_t signaling_NaN() noexcept
  {
    return static_cast<uint128_t>(0);
  }
  static constexpr uint128_t denorm_min() noexcept
  {
    return static_cast<uint128_t>(0);
  }
};

template<> class std::numeric_limits<int128_t> {
 public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = true;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
#if __cplusplus > 202012L
  static constexpr auto has_denorm = denorm_absent;
#endif
  static constexpr bool has_denorm_loss = false;
  static constexpr float_round_style round_style = round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = true;
  static constexpr int digits = 127;
  static constexpr int digits10 = 38;
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = true;
  static constexpr bool tinyness_before = false;

  static constexpr int128_t min() noexcept
  {
    return int128_t{uint128_t(static_cast<uint64_t>(1) << 63, 0)};
  }
  static constexpr int128_t lowest() noexcept
  {
    return min();
  }
  static constexpr int128_t max() noexcept
  {
    return int128_t{uint128_t(~(static_cast<uint64_t>(1) << 63), ~static_cast<uint64_t>(0))};
  }
  static constexpr int128_t epsilon() noexcept
  {
    return static_cast<int128_t>(0);
  }
  static constexpr int128_t round_error() noexcept
  {
    return static_cast<int128_t>(0);
  }
  static constexpr int128_t infinity() noexcept
  {
    return static_cast<int128_t>(0);
  }
  static constexpr int128_t quiet_NaN() noexcept
  {
    return static_cast<int128_t>(0);
  }
  static constexpr int128_t signaling_NaN() noexcept
  {
    return static_cast<int128_t>(0);
  }
  static constexpr int128_t denorm_min() noexcept
  {
    return static_cast<int128_t>(0);
  }
};

template<> class std::numeric_limits<uint256_t> {
 public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = false;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
#if __cplusplus > 202012L
  static constexpr auto has_denorm = denorm_absent;
#endif
  static constexpr bool has_denorm_loss = false;
  static constexpr float_round_style round_style = round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = true;
  static constexpr int digits = 256;
  static constexpr int digits10 = 77;
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = true;
  static constexpr bool tinyness_before = false;

  static constexpr uint256_t min() noexcept
  {
    return static_cast<uint256_t>(0);
  }
  static constexpr uint256_t lowest() noexcept
  {
    return min();
  }
  static constexpr uint256_t max() noexcept
  {
    return {~static_cast<uint64_t>(0),
            ~static_cast<uint64_t>(0),
            ~static_cast<uint64_t>(0),
            ~static_cast<uint64_t>(0)};
  }
  static constexpr uint256_t epsilon() noexcept
  {
    return static_cast<uint256_t>(0);
  }
  static constexpr uint256_t round_error() noexcept
  {
    return static_cast<uint256_t>(0);
  }
  static constexpr uint256_t infinity() noexcept
  {
    return static_cast<uint256_t>(0);
  }
  static constexpr uint256_t quiet_NaN() noexcept
  {
    return static_cast<uint256_t>(0);
  }
  static constexpr uint256_t signaling_NaN() noexcept
  {
    return static_cast<uint256_t>(0);
  }
  static constexpr uint256_t denorm_min() noexcept
  {
    return static_cast<uint256_t>(0);
  }
};

template<> class std::numeric_limits<int256_t> {
 public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = true;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
#if __cplusplus > 202012L
  static constexpr auto has_denorm = denorm_absent;
#endif
  static constexpr bool has_denorm_loss = false;
  static constexpr float_round_style round_style = round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = true;
  static constexpr int digits = 255;
  static constexpr int digits10 = 76;
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = true;
  static constexpr bool tinyness_before = false;

  static constexpr int256_t min() noexcept
  {
    return {uint128_t(static_cast<uint64_t>(1) << 63, 0), {0, 0}};
  }
  static constexpr int256_t lowest() noexcept
  {
    return min();
  }
  static constexpr int256_t max() noexcept
  {
    return {~(static_cast<uint64_t>(1) << 63),
            ~static_cast<uint64_t>(0),
            ~static_cast<uint64_t>(0),
            ~static_cast<uint64_t>(0)};
  }
  static constexpr int256_t epsilon() noexcept
  {
    return {};
  }
  static constexpr int256_t round_error() noexcept
  {
    return {};
  }
  static constexpr int256_t infinity() noexcept
  {
    return {};
  }
  static constexpr int256_t quiet_NaN() noexcept
  {
    return {};
  }
  static constexpr int256_t signaling_NaN() noexcept
  {
    return {};
  }
  static constexpr int256_t denorm_min() noexcept
  {
    return {};
  }
};

template<> struct std::is_integral<uint256_t> {
  static constexpr bool value = true;
};

template<> struct std::is_integral<int256_t> {
  static constexpr bool value = true;
};

template<> struct std::is_signed<uint256_t> {
  static constexpr bool value = false;
};

template<> struct std::is_signed<int256_t> {
  static constexpr bool value = true;
};

template<> struct std::is_unsigned<uint256_t> {
  static constexpr bool value = true;
};

template<> struct std::is_unsigned<int256_t> {
  static constexpr bool value = false;
};

//// ReSharper disable once CppUseTypeTraitAlias
// template<> constexpr bool std::is_integral_v<uint256_t> = std::is_integral<uint256_t>::value;
//// ReSharper disable once CppUseTypeTraitAlias
// template<> constexpr bool std::is_integral_v<int256_t> = std::is_integral<int256_t>::value;
//  ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_signed_v<int256_t> = std::is_signed<int256_t>::value;
// ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_signed_v<uint256_t> = std::is_signed<uint256_t>::value;
// ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_unsigned_v<int256_t> = std::is_unsigned<int256_t>::value;
// ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_unsigned_v<uint256_t> = std::is_unsigned<uint256_t>::value;

template<> struct std::is_integral<uint128_t> {
  static constexpr bool value = true;
};

template<> struct std::is_integral<int128_t> {
  static constexpr bool value = true;
};

template<> struct std::is_signed<uint128_t> {
  static constexpr bool value = false;
};

template<> struct std::is_signed<int128_t> {
  static constexpr bool value = true;
};

template<> struct std::is_unsigned<uint128_t> {
  static constexpr bool value = true;
};

template<> struct std::is_unsigned<int128_t> {
  static constexpr bool value = false;
};

//// ReSharper disable once CppUseTypeTraitAlias
// template<> constexpr bool std::is_integral_v<uint128_t> = std::is_integral<uint128_t>::value;
//// ReSharper disable once CppUseTypeTraitAlias
// template<> constexpr bool std::is_integral_v<int128_t> = std::is_integral<int128_t>::value;
//  ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_signed_v<int128_t> = std::is_signed<int128_t>::value;
// ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_signed_v<uint128_t> = std::is_signed<uint128_t>::value;
// ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_unsigned_v<int128_t> = std::is_unsigned<int128_t>::value;
// ReSharper disable once CppUseTypeTraitAlias
template<> constexpr bool std::is_unsigned_v<uint128_t> = std::is_unsigned<uint128_t>::value;

namespace traits {

  // clang-format off
  template<size_t S>
    requires(S == 1 || S == 2 || S == 4 || S == 8 || S == 16 || S == 32)
  using sized_unsigned_integer = std::conditional<
      S == 1,  uint8_t,   std::conditional<
      S == 2,  uint16_t,  std::conditional<
      S == 4,  uint32_t,  std::conditional<
      S == 8,  uint64_t,  std::conditional<
      S == 16, uint128_t,
               uint256_t
    >>>>>;

  template<size_t S>
    requires(S == 1 || S == 2 || S == 4 || S == 8 || S == 16 || S == 32)
  using sized_signed_integer = std::conditional<
      S == 1,  int8_t,   std::conditional<
      S == 2,  int16_t,  std::conditional<
      S == 4,  int32_t,  std::conditional<
      S == 8,  int64_t,  std::conditional<
      S == 16, int128_t,
               int256_t
    >>>>>;
  // clang-format on

  template<typename T>
  concept long_integer = std::is_same_v<T, int128_t> || std::is_same_v<T, uint128_t> ||
                         std::is_same_v<T, int256_t> || std::is_same_v<T, uint256_t>;

}  // namespace traits

// uint128_t comparison operators
template<typename T>
  requires std::is_integral_v<T>
constexpr std::strong_ordering operator<=>(const uint128_t &lhs, const T &rhs) noexcept
{
  if (lhs.high > 0)
    return std::strong_ordering::greater;
  return lhs.low <=> static_cast<uint64_t>(rhs);
}

template<typename T>
  requires std::is_integral_v<T>
constexpr bool operator==(const uint128_t &lhs, const T &rhs) noexcept
{
  return lhs.high == 0 && lhs.low == static_cast<uint64_t>(rhs);
}

constexpr std::strong_ordering operator<=>(const uint128_t &lhs, const int128_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (rhs_negative)
    return std::strong_ordering::greater;
  return lhs <=> rhs.value;
}

constexpr bool operator==(const uint128_t &lhs, const int128_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);
  return !rhs_negative && lhs == rhs.value;
}

constexpr std::strong_ordering operator<=>(const uint128_t &lhs, const uint256_t &rhs) noexcept
{
  if (rhs.high != 0)
    return std::strong_ordering::less;
  return lhs <=> rhs.low;
}

constexpr bool operator==(const uint128_t &lhs, const uint256_t &rhs) noexcept
{
  return rhs.high == 0 && lhs == rhs.low;
}

constexpr std::strong_ordering operator<=>(const uint128_t &lhs, const int256_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (rhs_negative)
    return std::strong_ordering::greater;
  return lhs <=> rhs.value.low;
}

constexpr bool operator==(const uint128_t &lhs, const int256_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  return !rhs_negative && lhs == rhs.value.low;
}

// int128_t comparison operators
template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr std::strong_ordering operator<=>(const int128_t &lhs, const T &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (std::is_signed_v<T>) {
    bool rhs_negative = rhs < 0;
    if (lhs_negative != rhs_negative)
      return rhs_negative <=> lhs_negative;
    if (!lhs_negative)
      return lhs.value.low <=> static_cast<uint64_t>(rhs);
    // Both negative
    // todo: possibly wrong here
    if ((lhs.value.high ^ 0x7FFF'FFFF'FFFF'FFFF) != 0 && lhs.value.high & 0x8000'0000'0000'0000) {
      return std::strong_ordering::less;
    }
    return static_cast<int64_t>(rhs) <=> lhs.value.low;
  }
  else {
    // Unsigned comparison
    if (lhs_negative)
      return std::strong_ordering::less;
    return lhs.value.low <=> static_cast<uint64_t>(rhs);
  }
}

template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr bool operator==(const int128_t &lhs, const T &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (std::is_signed_v<T>) {
    bool rhs_negative = rhs < 0;
    return lhs_negative == rhs_negative &&
           lhs.value.low == static_cast<uint64_t>(std::abs(static_cast<int64_t>(rhs)));
  }
  else {
    return !lhs_negative && lhs.value.low == static_cast<uint64_t>(rhs);
  }
}

constexpr std::strong_ordering operator<=>(const int128_t &lhs, const uint256_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (lhs_negative)
    return std::strong_ordering::less;
  if (rhs.high != 0)
    return std::strong_ordering::less;
  return lhs.value <=> rhs.low;
}

constexpr bool operator==(const int128_t &lhs, const uint256_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  return !lhs_negative && rhs.high == 0 && lhs.value == rhs.low;
}

constexpr std::strong_ordering operator<=>(const int128_t &lhs, const int256_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (lhs_negative != rhs_negative)
    return rhs_negative <=> lhs_negative;
  if (!lhs_negative)
    return lhs.value <=> rhs.value.low;
  return rhs.value.low <=> lhs.value;
}

constexpr bool operator==(const int128_t &lhs, const int256_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  return lhs_negative == rhs_negative && lhs.value == rhs.value.low;
}

// uint256_t comparison operators
template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr std::strong_ordering operator<=>(const uint256_t &lhs, const T &rhs) noexcept
{
  if (lhs.high != 0)
    return std::strong_ordering::greater;
  return lhs.low <=> static_cast<uint128_t>(rhs);
}

template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr bool operator==(const uint256_t &lhs, const T &rhs) noexcept
{
  return lhs.high == 0 && lhs.low == static_cast<uint128_t>(rhs);
}

constexpr std::strong_ordering operator<=>(const uint256_t &lhs, const int256_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (rhs_negative)
    return std::strong_ordering::greater;
  return lhs <=> rhs.value;
}

constexpr bool operator==(const uint256_t &lhs, const int256_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  return !rhs_negative && lhs == rhs.value;
}

// int256_t comparison operators
template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr std::strong_ordering operator<=>(const int256_t &lhs, const T &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (std::is_signed_v<T>) {
    bool rhs_negative = rhs < 0;
    if (lhs_negative != rhs_negative)
      return rhs_negative <=> lhs_negative;
    if (!lhs_negative)
      return lhs.value.low.low <=> static_cast<uint64_t>(rhs);
    // Both negative
    return static_cast<uint64_t>(rhs) <=> lhs.value.low.low;
  }
  else {
    // Unsigned comparison
    if (lhs_negative)
      return std::strong_ordering::less;
    if (lhs.value.high.high != 0 || lhs.value.high.low != 0 || lhs.value.low.high != 0)
      return std::strong_ordering::greater;
    return lhs.value.low.low <=> static_cast<uint64_t>(rhs);
  }
}

template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr bool operator==(const int256_t &lhs, const T &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (std::is_signed_v<T>) {
    bool rhs_negative = rhs < 0;
    return lhs_negative == rhs_negative && lhs.value.high.high == 0 && lhs.value.high.low == 0 &&
           lhs.value.low.high == 0 &&
           lhs.value.low.low == static_cast<uint64_t>(std::abs(static_cast<int64_t>(rhs)));
  }
  else {
    return !lhs_negative && lhs.value.high.high == 0 && lhs.value.high.low == 0 &&
           lhs.value.low.high == 0 && lhs.value.low.low == static_cast<uint64_t>(rhs);
  }
}

// Same type comparison operators
constexpr std::strong_ordering operator<=>(const uint128_t &lhs, const uint128_t &rhs) noexcept
{
  if (auto cmp = lhs.high <=> rhs.high; cmp != 0)
    return cmp;
  return lhs.low <=> rhs.low;
}

constexpr bool operator==(const uint128_t &lhs, const uint128_t &rhs) noexcept
{
  return lhs.high == rhs.high && lhs.low == rhs.low;
}

constexpr std::strong_ordering operator<=>(const int128_t &lhs, const int128_t &rhs) noexcept
{
  // Handle sign bit first
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);

  if (lhs_negative != rhs_negative)
    return rhs_negative <=> lhs_negative;  // Negative < Positive

  // Same sign, compare magnitudes
  if (!lhs_negative)
    return lhs.value <=> rhs.value;
  else
    return rhs.value <=> lhs.value;  // For negative numbers, reverse comparison
}

constexpr bool operator==(const int128_t &lhs, const int128_t &rhs) noexcept
{
  return lhs.value == rhs.value;
}

constexpr std::strong_ordering operator<=>(const uint256_t &lhs, const uint256_t &rhs) noexcept
{
  if (auto cmp = lhs.high <=> rhs.high; cmp != 0)
    return cmp;
  return lhs.low <=> rhs.low;
}

constexpr bool operator==(const uint256_t &lhs, const uint256_t &rhs) noexcept
{
  return lhs.high == rhs.high && lhs.low == rhs.low;
}

constexpr std::strong_ordering operator<=>(const int256_t &lhs, const int256_t &rhs) noexcept
{
  // Handle sign bit first
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  bool rhs_negative = rhs.value.high.high & (static_cast<uint64_t>(1) << 63);

  if (lhs_negative != rhs_negative)
    return rhs_negative <=> lhs_negative;  // Negative < Positive

  // Same sign, compare magnitudes
  return lhs.value <=> rhs.value;
}

constexpr bool operator==(const int256_t &lhs, const int256_t &rhs) noexcept
{
  return lhs.value == rhs.value;
}

// Missing comparison operators between different types

// int128_t <=> uint128_t
constexpr std::strong_ordering operator<=>(const int128_t &lhs, const uint128_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (lhs_negative)
    return std::strong_ordering::less;
  return lhs.value <=> rhs;
}

constexpr bool operator==(const int128_t &lhs, const uint128_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high & (static_cast<uint64_t>(1) << 63);
  return !lhs_negative && lhs.value == rhs;
}

// uint256_t <=> uint128_t
constexpr std::strong_ordering operator<=>(const uint256_t &lhs, const uint128_t &rhs) noexcept
{
  if (lhs.high != 0)
    return std::strong_ordering::greater;
  return lhs.low <=> rhs;
}

constexpr bool operator==(const uint256_t &lhs, const uint128_t &rhs) noexcept
{
  return lhs.high == 0 && lhs.low == rhs;
}

// uint256_t <=> int128_t
constexpr std::strong_ordering operator<=>(const uint256_t &lhs, const int128_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (rhs_negative)
    return std::strong_ordering::greater;
  if (lhs.high != 0)
    return std::strong_ordering::greater;
  return lhs.low <=> rhs.value;
}

constexpr bool operator==(const uint256_t &lhs, const int128_t &rhs) noexcept
{
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);
  return !rhs_negative && lhs.high == 0 && lhs.low == rhs.value;
}

// int256_t <=> uint128_t
constexpr std::strong_ordering operator<=>(const int256_t &lhs, const uint128_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (lhs_negative)
    return std::strong_ordering::less;
  if (lhs.value.high != 0)
    return std::strong_ordering::greater;
  return lhs.value.low <=> rhs;
}

constexpr bool operator==(const int256_t &lhs, const uint128_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  return !lhs_negative && lhs.value.high == 0 && lhs.value.low == rhs;
}

// int256_t <=> int128_t
constexpr std::strong_ordering operator<=>(const int256_t &lhs, const int128_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);
  if (lhs_negative != rhs_negative)
    return rhs_negative <=> lhs_negative;
  if (!lhs_negative) {
    if (lhs.value.high != 0)
      return std::strong_ordering::greater;
    return lhs.value.low <=> rhs.value;
  }
  // Both negative
  if (lhs.value.high != 0)
    return std::strong_ordering::less;
  return rhs.value <=> lhs.value.low;
}

constexpr bool operator==(const int256_t &lhs, const int128_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  bool rhs_negative = rhs.value.high & (static_cast<uint64_t>(1) << 63);
  return lhs_negative == rhs_negative && lhs.value.high == 0 && lhs.value.low == rhs.value;
}

// int256_t <=> uint256_t
constexpr std::strong_ordering operator<=>(const int256_t &lhs, const uint256_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  if (lhs_negative)
    return std::strong_ordering::less;
  return lhs.value <=> rhs;
}

constexpr bool operator==(const int256_t &lhs, const uint256_t &rhs) noexcept
{
  bool lhs_negative = lhs.value.high.high & (static_cast<uint64_t>(1) << 63);
  return !lhs_negative && lhs.value == rhs;
}

template<typename T1, typename T2>
  requires traits::long_integer<T1> && traits::long_integer<T2> && (sizeof(T1) > sizeof(T2))
constexpr auto operator&(const T1 &lhs, const T2 &rhs) noexcept
{
  T2 t1{lhs};
  return t1 &= rhs;
}

template<typename T1, typename T2>
  requires traits::long_integer<T1> && traits::long_integer<T2> && (sizeof(T1) > sizeof(T2))
constexpr auto operator|(const T1 &lhs, const T2 &rhs) noexcept
{
  T2 t1{lhs};
  return t1 |= rhs;
}

template<typename T1, typename T2>
  requires traits::long_integer<T1> && traits::long_integer<T2> && (sizeof(T1) > sizeof(T2))
constexpr auto operator^(const T1 &lhs, const T2 &rhs) noexcept
{
  T2 t1{lhs};
  return t1 ^= rhs;
}

template<typename T1, typename T2>
  requires traits::long_integer<T1> && traits::long_integer<T2> && (sizeof(T1) < sizeof(T2))
constexpr auto operator&(const T1 &lhs, const T2 &rhs) noexcept
{
  T1 t2{rhs};
  return t2 &= lhs;
}

template<typename T1, typename T2>
  requires traits::long_integer<T1> && traits::long_integer<T2> && (sizeof(T1) < sizeof(T2))
constexpr auto operator|(const T1 &lhs, const T2 &rhs) noexcept
{
  T1 t2{rhs};
  return t2 |= lhs;
}

template<typename T1, typename T2>
  requires traits::long_integer<T1> && traits::long_integer<T2> && (sizeof(T1) < sizeof(T2))
constexpr auto operator^(const T1 &lhs, const T2 &rhs) noexcept
{
  T1 t2{rhs};
  return t2 ^= lhs;
}
