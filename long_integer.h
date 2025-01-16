#pragma once

#include <bit>
#include <concepts>
#include <cstdint>
#include <format>
#include <limits>
#include <stdexcept>

// Forward declarations
struct uint128_t;
struct int128_t;
struct uint256_t;
struct int256_t;

#pragma region uint128_t Implementation
// 128-bit unsigned integer
struct uint128_t {
  uint64_t low;
  uint64_t high;

#pragma region Constructions
  // Constructions
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
  explicit constexpr uint128_t(double v)
  {
    // if (v < 0 || v > std::numeric_limits<uint128_t>::max()) {
    //   throw std::out_of_range("Double value out of range for uint128_t");
    // }
    uint64_t integral_part = static_cast<uint64_t>(v);
    v -= integral_part;
    v *= static_cast<double>(std::numeric_limits<uint64_t>::max()) + 1.0;
    low = integral_part | static_cast<uint64_t>(v);
    high = static_cast<uint64_t>(
        v / (static_cast<double>(std::numeric_limits<uint64_t>::max()) + 1.0));
  }
  explicit constexpr uint128_t(const int128_t &v);
  explicit constexpr uint128_t(uint64_t l) : low(l), high(0) {}
  constexpr uint128_t(uint64_t h, uint64_t l) : low(l), high(h) {}
#pragma endregion

#pragma region Caster Operators
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
#pragma endregion

#pragma region Arithmatic Assignment Operators (Self)
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
    uint128_t result{0};
    uint128_t a = *this;
    uint128_t b = rhs;
    while (b != 0) {
      if (b.low & 1)
        result += a;
      a <<= 1;
      b >>= 1;
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

    // todo: improve this
    int zero_bits = std::countl_zero(high);
    zero_bits += zero_bits == 64 ? std::countl_zero(low) : 0;

    while (remainder >= rhs) {
      int shift = std::countl_zero(rhs.high);
      shift += shift == 64 ? std::countl_zero(rhs.low) : 0;
      shift -= zero_bits;
      uint128_t shifted = rhs << shift;
      while (shifted > remainder) {
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

  constexpr uint128_t &operator%=(const uint128_t &rhs)
  {
    if (rhs == 0)
      throw std::domain_error("Modulo by zero");
    if (rhs == 1) {
      high = low = 0;
      return *this;
    }

    uint128_t remainder = *this;
    while (remainder >= rhs) {
      int shift = std::countl_zero(rhs.high);
      uint128_t shifted = rhs << shift;
      if (shifted > remainder) {
        shifted >>= 1;
        --shift;
      }
      remainder -= shifted;
    }

    *this = remainder;
    return *this;
  }
#pragma endregion

#pragma region Bit Operations
  constexpr uint128_t operator~() const
  {
    return {~high, ~low};
  }

  constexpr bool bit(size_t i) const
  {
    if (i >= 128)
      return false;
    return i < 64 ? (low >> i) & 1 : (high >> (i - 64)) & 1;
  }
#pragma endregion

#pragma region Binary Operators
  // Binary operators
  friend uint128_t operator+(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy += rhs;
  }
  friend uint128_t operator-(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy -= rhs;
  }
  friend uint128_t operator*(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy *= rhs;
  }
  friend uint128_t operator/(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy /= rhs;
  }
  friend uint128_t operator&(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }
  friend uint128_t operator|(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }
  friend uint128_t operator^(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }
  friend uint128_t operator<<(const uint128_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy <<= shift;
  }
  friend uint128_t operator>>(const uint128_t &lhs, int shift)
  {
    auto lhs_copy = lhs;
    return lhs_copy >>= shift;
  }
  friend uint128_t operator%(const uint128_t &lhs, const uint128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy %= rhs;
  }
#pragma endregion

#pragma region Additional Operators with Built-in Types
  // Additional operators for built-in types
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator&=(T rhs)
  {
    low &= rhs;
    high = 0;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator|=(T rhs)
  {
    low |= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator^=(T rhs)
  {
    low ^= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator+=(T rhs)
  {
    uint64_t old_low = low;
    low += rhs;
    high += (low < old_low ? 1 : 0);
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator-=(T rhs)
  {
    uint64_t old_low = low;
    low -= rhs;
    high -= (low > old_low ? 1 : 0);
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator*=(T rhs)
  {
    uint64_t a = low & 0xFFFFFFFF;
    uint64_t b = low >> 32;
    uint64_t c = rhs & 0xFFFFFFFF;
    uint64_t d = rhs >> 32;

    uint64_t ac = a * c;
    uint64_t bc = b * c;
    uint64_t ad = a * d;
    uint64_t bd = b * d;

    uint64_t mid34 = (ac >> 32) + (bc & 0xFFFFFFFF) + ad;
    uint64_t upper64 = bc >> 32;
    upper64 += bd + (mid34 >> 32);
    low = (ac & 0xFFFFFFFF) | (mid34 << 32);
    high = upper64 + (low < ac ? 1 : 0);
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator/=(T rhs)
  {
    return *this = *this / static_cast<uint128_t>(rhs);
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t &operator%=(T rhs)
  {
    if (rhs == 0)
      throw std::domain_error("Modulo by zero");
    if (rhs == 1 || rhs == -1) {
      low = high = 0;
      return *this;
    }

    uint128_t remainder = *this;
    uint128_t divisor = uint128_t(rhs);

    while (remainder >= divisor) {
      auto high_zero = std::countl_zero(divisor.high);
      int shift = high_zero == 64 ? high_zero + std::countl_zero(divisor.low) : high_zero;
      uint128_t shifted = divisor << shift;
      while (shifted > remainder) {
        shifted >>= 1;
        --shift;
      }
      remainder -= shifted;
    }

    *this = remainder;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator&(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy &= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator|(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy |= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator^(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy ^= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator%(const uint128_t &lhs, T rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy %= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator&(T lhs, const uint128_t &rhs)
  {
    auto rhs_copy = rhs;
    return rhs_copy &= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator|(T lhs, const uint128_t &rhs)
  {
    auto rhs_copy = rhs;
    return rhs_copy |= lhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint128_t operator^(T lhs, const uint128_t &rhs)
  {
    auto rhs_copy = rhs;
    return rhs_copy ^= lhs;
  }
#pragma endregion

#pragma region Comparison Operators
  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T>
  friend constexpr std::strong_ordering operator<=>(const uint128_t &lhs, T rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T>
  friend constexpr bool operator==(const uint128_t &lhs, T rhs) noexcept;

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
#pragma endregion
};
#pragma endregion

#pragma region int128_t Implementation
// 128-bit signed integer
struct int128_t {
  uint128_t value;

#pragma region Constructions
  // Constructors
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
  explicit constexpr int128_t(double v)
  {
    if (v >= static_cast<double>(std::numeric_limits<int64_t>::max())) {
      value = uint128_t(std::numeric_limits<int64_t>::max());
    }
    else if (v <= static_cast<double>(std::numeric_limits<int64_t>::min())) {
      value = uint128_t(std::numeric_limits<int64_t>::min());
    }
    else {
      value = uint128_t(static_cast<int64_t>(v));
    }
  }
  explicit constexpr int128_t(const uint128_t &v) : value(v) {}
  explicit constexpr int128_t(uint64_t h, uint64_t l) : value{h, l} {}

#pragma endregion

#pragma region Caster Operators
  // Conversion operators
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
#pragma endregion

#pragma region Arithmatic Assignment Operators (Self)
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
  int128_t &operator%=(const int128_t &rhs)
  {
    value %= rhs.value;
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
  constexpr bool bit(size_t i) const
  {
    return value.bit(i);
  }
#pragma endregion

#pragma region Binary Operators
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
  friend int128_t operator%(const int128_t &lhs, const int128_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy %= rhs;
  }
#pragma endregion

#pragma region Additional Operators with Built-in Types
  // Additional operators for built-in types and int128_t
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator&=(T rhs)
  {
    value &= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator|=(T rhs)
  {
    value |= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator^=(T rhs)
  {
    value ^= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator+=(T rhs)
  {
    value += rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator-=(T rhs)
  {
    value -= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator*=(T rhs)
  {
    value *= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator/=(T rhs)
  {
    value /= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t &operator%=(T rhs)
  {
    value /= rhs;
    return *this;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend int128_t operator&(int128_t lhs, T rhs)
  {
    return lhs &= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend int128_t operator|(int128_t lhs, T rhs)
  {
    return lhs |= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend int128_t operator^(int128_t lhs, T rhs)
  {
    return lhs ^= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
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
#pragma endregion

#pragma region Comparison Operators
  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr std::strong_ordering operator<=>(const int128_t &lhs, T rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr bool operator==(const int128_t &lhs, T rhs) noexcept;

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
#pragma endregion
};
#pragma endregion

#pragma region uint256_t Implementation
// 256-bit unsigned integer
struct uint256_t {
  uint128_t low;
  uint128_t high;

#pragma region Constructions
  // Constructors
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
  explicit constexpr uint256_t(double v);
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
#pragma endregion

#pragma region Caster Operators
  // Conversion operators
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
#pragma endregion

#pragma region Arithmatic Assignment Operators (Self)
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

  uint256_t &operator*=(const uint256_t &rhs)
  {
    uint256_t result = static_cast<uint256_t>(0);
    for (int i = 0; i < 256; ++i) {
      if (rhs.bit(i))
        result += (*this << i);
    }
    *this = result;
    return *this;
  }

  uint256_t &operator/=(const uint256_t &rhs)
  {
    if (rhs == 0)
      throw std::domain_error("Division by zero");
    if (rhs == 1)
      return *this;

    uint256_t quotient = static_cast<uint256_t>(0);
    uint256_t remainder = *this;

    while (remainder >= rhs) {
      uint256_t shifted = rhs;
      int shift = 0;

      // Find the largest shift that keeps shifted <= remainder
      while ((shifted << 1) <= remainder && shift < 255) {
        shifted <<= 1;
        ++shift;
      }

      remainder -= shifted;
      quotient |= uint256_t(1) << shift;
    }

    *this = quotient;
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

  uint256_t &operator%=(const uint256_t &rhs)
  {
    if (rhs == 0)
      throw std::domain_error("Modulo by zero");
    if (rhs == 1 || *this < rhs)
      return *this;

    uint256_t remainder = *this;
    while (remainder >= rhs) {
      uint256_t shifted = rhs;
      int shift = 0;
      while ((shifted << 1) <= remainder && shift < 255) {
        shifted <<= 1;
        ++shift;
      }
      remainder -= shifted;
    }
    *this = remainder;
    return *this;
  }

  uint256_t operator~() const
  {
    return {~high, ~low};
  }

  uint256_t operator-() const
  {
    if (*this == 0)
      return *this;
    return ~(*this) + 1;
  }

  constexpr bool bit(size_t i) const
  {
    if (i >= 256)
      return false;
    return i < 128 ? low.bit(i) : high.bit(i - 128);
  }
#pragma endregion

#pragma region Binary Operators
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
  friend uint256_t operator*(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy *= rhs;
  }
  friend uint256_t operator/(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy /= rhs;
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
  friend uint256_t operator%(const uint256_t &lhs, const uint256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy %= rhs;
  }
#pragma endregion

#pragma region Additional Operators with Built-in Types
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
  uint256_t &operator+=(T rhs)
  {
    uint256_t tmp(rhs);
    return *this += tmp;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator-=(T rhs)
  {
    uint256_t tmp(rhs);
    return *this -= tmp;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator*=(T rhs)
  {
    uint256_t tmp(rhs);
    return *this *= tmp;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator/=(T rhs)
  {
    uint256_t tmp(rhs);
    return *this /= tmp;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t &operator%=(T rhs)
  {
    uint256_t tmp(rhs);
    return *this %= tmp;
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
  friend uint256_t operator+(uint256_t lhs, T rhs)
  {
    return lhs += rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator-(uint256_t lhs, T rhs)
  {
    return lhs -= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator*(uint256_t lhs, T rhs)
  {
    return lhs *= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator/(uint256_t lhs, T rhs)
  {
    return lhs /= rhs;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend uint256_t operator%(uint256_t lhs, T rhs)
  {
    return lhs %= rhs;
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
#pragma endregion

#pragma region Comparison Operators
  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr std::strong_ordering operator<=>(const uint256_t &lhs, T rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr bool operator==(const uint256_t &lhs, T rhs) noexcept;

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
#pragma endregion
};
#pragma endregion

#pragma region int256_t Implementation
// 256-bit signed integer
struct int256_t {
  uint256_t value;

#pragma region Constructions
  // Constructors
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
  explicit constexpr int256_t(const int128_t &v)
      : value(v < 0 ? static_cast<uint128_t>(-1) : static_cast<uint128_t>(0), v.value)
  {
  }
  explicit constexpr int256_t(const uint256_t &v) : value(v)
  {
    // todo: should set first bit to zero?
  }
  explicit constexpr int256_t(double v);
  constexpr int256_t(const uint128_t &h, const uint128_t &l) : value{h, l} {}
  constexpr int256_t(const uint64_t hh, const uint64_t hl, const uint64_t lh, const uint64_t ll)
      : value{hh, hl, lh, ll}
  {
  }
#pragma endregion

#pragma region Caster Operators
  // Conversion operators
  explicit constexpr operator bool() const noexcept
  {
    return static_cast<bool>(value);
  }
  explicit constexpr operator uint256_t() const noexcept
  {
    return value;
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
#pragma endregion

#pragma region Arithmatic Assignment Operators (Self)
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
  int256_t &operator*=(const int256_t &rhs)
  {
    value *= rhs.value;
    return *this;
  }
  int256_t &operator/=(const int256_t &rhs);

#pragma endregion

#pragma region Bit Operations
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
  int256_t &operator<<=(int shift)
  {
    value <<= shift;
    return *this;
  }
  int256_t &operator>>=(int shift)
  {
    value >>= shift;
    return *this;
  }

  int256_t operator~() const
  {
    return int256_t{~value};
  }
  int256_t operator-() const;

  constexpr bool bit(size_t i) const
  {
    return value.bit(i);
  }
#pragma endregion

#pragma region Binary Operators
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
  friend int256_t operator*(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy *= rhs;
  }
  friend int256_t operator/(const int256_t &lhs, const int256_t &rhs)
  {
    auto lhs_copy = lhs;
    return lhs_copy /= rhs;
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
#pragma endregion

#pragma region Additional Operators with Built-in Types
  // Additional operators for built-in types and int256_t
  template<typename T>
    requires std::is_integral_v<T>
  int256_t &operator/=(T rhs)
  {
    return *this /= int256_t(rhs);
  }

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
  friend int256_t operator/(int256_t lhs, T rhs)
  {
    return lhs /= rhs;
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
  friend int256_t operator/(T lhs, const int256_t &rhs)
  {
    return int256_t(lhs) /= rhs;
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
#pragma endregion

#pragma region Comparison Operators
  // Friend declarations for comparison operators
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr std::strong_ordering operator<=>(const int256_t &lhs, T rhs) noexcept;

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  friend constexpr bool operator==(const int256_t &lhs, T rhs) noexcept;

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
#pragma endregion
};
#pragma endregion

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

inline int256_t &int256_t::operator/=(const int256_t &rhs)
{
  if (rhs == 0)
    throw std::domain_error("Division by zero");

  // Handle division by -1 separately to avoid overflow with min value
  if (rhs == -1) {
    if (*this == std::numeric_limits<int256_t>::min()) {
      throw std::overflow_error("Division overflow");
    }
    value = (-(*this)).value;
    return *this;
  }

  bool result_negative = (*this < 0) != (rhs < 0);
  auto abs_this = *this < 0 ? -(*this) : *this;
  auto abs_rhs = rhs < 0 ? -rhs : rhs;

  abs_this.value /= abs_rhs.value;
  if (result_negative) {
    *this = -abs_this;
  }
  else {
    *this = abs_this;
  }
  return *this;
}

inline int256_t int256_t::operator-() const
{
  if (*this == std::numeric_limits<int256_t>::min()) {
    throw std::overflow_error("Negation overflow");
  }
  return int256_t{-value};
}

// todo:
// constexpr uint256_t::uint256_t(double v)
//{
//  // if (v < 0 || v > std::numeric_limits<uint256_t>::max()) {
//  //   throw std::out_of_range("Double value out of range for uint256_t");
//  // }
//  uint128_t integral_part = static_cast<uint128_t>(v);
//  v -= static_cast<double>(integral_part);
//  v *= static_cast<double>(std::numeric_limits<uint128_t>::max()) + 1.0;
//  low = integral_part | static_cast<uint128_t>(v);
//  high = static_cast<uint128_t>(
//      v / (static_cast<double>(std::numeric_limits<uint128_t>::max()) + 1.0));
//}

constexpr int256_t::int256_t(double v)
{
  if (v >= static_cast<double>(std::numeric_limits<int128_t>::max())) {
    value = uint256_t(std::numeric_limits<int128_t>::max());
  }
  else if (v <= static_cast<double>(std::numeric_limits<int128_t>::min())) {
    value = uint256_t(std::numeric_limits<int128_t>::min());
  }
  else {
    value = uint256_t(static_cast<int128_t>(v));
  }
}

// uint128_t comparison operators
template<typename T>
  requires std::is_integral_v<T>
constexpr std::strong_ordering operator<=>(const uint128_t &lhs, T rhs) noexcept
{
  if (lhs.high > 0)
    return std::strong_ordering::greater;
  return lhs.low <=> static_cast<uint64_t>(rhs);
}

template<typename T>
  requires std::is_integral_v<T>
constexpr bool operator==(const uint128_t &lhs, T rhs) noexcept
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
constexpr std::strong_ordering operator<=>(const int128_t &lhs, T rhs) noexcept
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
constexpr bool operator==(const int128_t &lhs, T rhs) noexcept
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
constexpr std::strong_ordering operator<=>(const uint256_t &lhs, T rhs) noexcept
{
  if (lhs.high != 0)
    return std::strong_ordering::greater;
  return lhs.low <=> static_cast<uint128_t>(rhs);
}

template<typename T>
  requires std::is_integral_v<T> && (sizeof(T) <= 8)
constexpr bool operator==(const uint256_t &lhs, T rhs) noexcept
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
constexpr std::strong_ordering operator<=>(const int256_t &lhs, T rhs) noexcept
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
constexpr bool operator==(const int256_t &lhs, T rhs) noexcept
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

// Formatter specializations
template<> struct std::formatter<uint128_t> : std::formatter<std::string> {
  constexpr auto parse(format_parse_context &ctx)
  {
    auto it = ctx.begin();
    auto end = ctx.end();

    // Default format settings
    _width = 0;
    _base = 10;
    _prefix = false;
    _upper = false;
    _fill = ' ';
    _align = '<';

    // Parse fill and align
    // if (it != end && *(it + 1) != '}') {
    //  auto next = it + 1;
    //  if (*next == '<' || *next == '>' || *next == '^') {
    //    _fill = *it;
    //    _align = *next;
    //    it += 2;
    //  }
    //  else if (*it == '<' || *it == '>' || *it == '^') {
    //    _align = *it;
    //    ++it;
    //  }
    //}

    // Parse width
    // if (it != end && std::isdigit(*it)) {
    //  _width = 0;
    //  do {
    //    _width = _width * 10 + (*it - '0');
    //    ++it;
    //  } while (it != end && std::isdigit(*it));
    //}

    // Parse type
    if (it != end && *it != '}') {
      /*switch (*it) {
        case 'x':
          _base = 16;
          break;
        case 'X':
          _base = 16;
          _upper = true;
          break;
        case 'b':
          _base = 2;
          break;
        case 'B':
          _base = 2;
          _upper = true;
          break;
        case 'o':
          _base = 8;
          break;
        case '#':
          _prefix = true;
          ++it;
          if (it != end) {
            switch (*it) {
              case 'x':
                _base = 16;
                break;
              case 'X':
                _base = 16;
                _upper = true;
                break;
              case 'b':
                _base = 2;
                break;
              case 'B':
                _base = 2;
                _upper = true;
                break;
              case 'o':
                _base = 8;
                break;
              default:
                throw format_error("invalid format specifier");
            }
          }
          break;
        case 'd':
          _base = 10;
          break;
        default:
          throw format_error("invalid format specifier");
      }*/
      ++it;
    }

    if (it != end && *it != '}')
      throw format_error("invalid format specifier");

    return it;
  }

  template<typename FormatContext> auto format(const uint128_t &value, FormatContext &ctx) const
  {
    std::string result;

    if (value == 0) {
      result = "0";
    }
    else {
      uint128_t tmp = value;
      while (tmp > 0) {
        uint8_t digit = static_cast<uint8_t>(tmp % _base);
        char c;
        if (digit < 10)
          c = '0' + digit;
        else
          c = (_upper ? 'A' : 'a') + (digit - 10);
        result = c + result;
        tmp /= _base;
      }
    }

    if (_prefix) {
      switch (_base) {
        case 2:
          result = (_upper ? "0B" : "0b") + result;
          break;
        case 8:
          result = "0" + result;
          break;
        case 16:
          result = (_upper ? "0X" : "0x") + result;
          break;
      }
    }

    if (_width > result.length()) {
      size_t padding = _width - result.length();
      if (_align == '>') {
        result = std::string(padding, _fill) + result;
      }
      else if (_align == '^') {
        size_t left = padding / 2;
        size_t right = padding - left;
        result = std::string(left, _fill) + result + std::string(right, _fill);
      }
      else {  // _align == '<'
        result = result + std::string(padding, _fill);
      }
    }

    return std::formatter<std::string>::format(result, ctx);
  }

 private:
  size_t _width;
  int _base;
  bool _prefix;
  bool _upper;
  char _fill;
  char _align;
};

// template<> struct std::formatter<int128_t> : std::formatter<uint128_t> {
//   template<typename FormatContext> auto format(const int128_t &value, FormatContext &ctx)
//   {
//     if (value < 0) {
//       auto abs_value = static_cast<uint128_t>(-value);
//       auto result = std::format("-{}", abs_value);
//       return std::formatter<std::string>::format(result, ctx);
//     }
//     return std::formatter<uint128_t>::format(static_cast<uint128_t>(value), ctx);
//   }
// };
//
// template<> struct std::formatter<uint256_t> : std::formatter<uint128_t> {
//   template<typename FormatContext> auto format(const uint256_t &value, FormatContext &ctx)
//   {
//     if (value.high == 0) {
//       return std::formatter<uint128_t>::format(value.low, ctx);
//     }
//     auto high_str = std::format("{}", value.high);
//     auto low_str = std::format("{:016X}", value.low);
//     return std::formatter<std::string>::format(high_str + low_str, ctx);
//   }
// };
//
// template<> struct std::formatter<int256_t> : std::formatter<uint256_t> {
//   template<typename FormatContext> auto format(const int256_t &value, FormatContext &ctx)
//   {
//     if (value < 0) {
//       auto abs_value = static_cast<uint256_t>(-value);
//       auto result = std::format("-{}", abs_value);
//       return std::formatter<std::string>::format(result, ctx);
//     }
//     return std::formatter<uint256_t>::format(static_cast<uint256_t>(value), ctx);
//   }
// };

#pragma region Math Functions
namespace std {
  // abs
  inline uint128_t abs(uint128_t x) noexcept
  {
    return x;
  }

  inline uint128_t abs(int128_t x) noexcept
  {
    return x < 0 ? uint128_t(-x) : uint128_t(x);
  }

  inline uint256_t abs(uint256_t x) noexcept
  {
    return x;
  }

  inline uint256_t abs(int256_t x) noexcept
  {
    return x < 0 ? uint256_t(-x) : uint256_t(x);
  }

  // sqrt - using binary search method
  inline double sqrt(uint128_t x)
  {
    if (x == 0)
      return 0.0;
    if (x == 1)
      return 1.0;

    // Convert to double, handling large numbers
    double high_part = static_cast<double>(x.high) * std::pow(2.0, 64);
    double low_part = static_cast<double>(x.low);
    double value = high_part + low_part;

    // Use Newton's method for better precision
    double result = value;
    double prev;
    do {
      prev = result;
      result = (result + value / result) * 0.5;
    } while (std::abs(result - prev) > 1e-10);

    return result;
  }

  inline double sqrt(int128_t x)
  {
    if (x < 0) {
      throw std::domain_error("sqrt of negative number");
    }
    return sqrt(uint128_t(x));
  }

  inline double sqrt(uint256_t x)
  {
    if (x == 0)
      return 0.0;
    if (x == 1)
      return 1.0;

    // Convert to double, handling large numbers carefully
    double result;
    if (x.high == 0) {
      // If high part is 0, we can use the 128-bit sqrt directly
      result = sqrt(x.low);
    }
    else {
      // For very large numbers, first get a good initial guess
      // by working with the most significant bits
      int msb = 0;
      uint256_t tmp = x;
      while (tmp > 0) {
        tmp >>= 1;
        msb++;
      }

      // Initial guess: sqrt(2^n) = 2^(n/2)
      result = std::pow(2.0, msb / 2.0);

      // Refine using Newton's method
      double prev;
      int iterations = 0;
      do {
        prev = result;
        // To avoid overflow, compute x/result first
        uint256_t quotient = x / uint256_t(result);
        double quot_d = static_cast<double>(quotient.low.low) +
                        static_cast<double>(quotient.low.high) * std::pow(2.0, 64) +
                        static_cast<double>(quotient.high.low) * std::pow(2.0, 128);
        result = (result + quot_d) * 0.5;

        // Limit iterations to prevent infinite loops
        if (++iterations > 100)
          break;
      } while (std::abs(result - prev) > 1e-10);
    }

    return result;
  }

  inline double sqrt(int256_t x)
  {
    if (x < 0) {
      throw std::domain_error("sqrt of negative number");
    }
    return sqrt(uint256_t(x));
  }

  // pow
  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint128_t pow(uint128_t base, T exp)
  {
    if (exp < 0) {
      throw std::domain_error("negative exponent");
    }
    if (exp == 0)
      return uint128_t(1);
    if (exp == 1)
      return base;

    uint128_t result = uint128_t(1);
    while (exp > 0) {
      if (exp & 1) {
        result *= base;
      }
      base *= base;
      exp >>= 1;
    }
    return result;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int128_t pow(int128_t base, T exp)
  {
    if (exp < 0) {
      throw std::domain_error("negative exponent");
    }
    if (exp == 0)
      return int128_t(1);
    if (exp == 1)
      return base;

    bool negative = base < 0 && (exp & 1);
    uint128_t abs_base = std::abs(base);
    uint128_t result = uint128_t(1);

    while (exp > 0) {
      if (exp & 1) {
        result *= abs_base;
      }
      abs_base *= abs_base;
      exp >>= 1;
    }

    return negative ? -int128_t(result) : int128_t(result);
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  uint256_t pow(uint256_t base, T exp)
  {
    if (exp < 0) {
      throw std::domain_error("negative exponent");
    }
    if (exp == 0)
      return uint256_t(1);
    if (exp == 1)
      return base;

    uint256_t result = uint256_t(1);
    while (exp > 0) {
      if (exp & 1) {
        result *= base;
      }
      base *= base;
      exp >>= 1;
    }
    return result;
  }

  template<typename T>
    requires std::is_integral_v<T> && (sizeof(T) <= 8)
  int256_t pow(int256_t base, T exp)
  {
    if (exp < 0) {
      throw std::domain_error("negative exponent");
    }
    if (exp == 0)
      return int256_t(1);
    if (exp == 1)
      return base;

    bool negative = base < 0 && (exp & 1);
    uint256_t abs_base = std::abs(base);
    uint256_t result = uint256_t(1);

    while (exp > 0) {
      if (exp & 1) {
        result *= abs_base;
      }
      abs_base *= abs_base;
      exp >>= 1;
    }

    return negative ? -int256_t(result) : int256_t(result);
  }
}  // namespace std
#pragma endregion
