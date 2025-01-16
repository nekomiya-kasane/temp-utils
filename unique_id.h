#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>

#include "misc_utils.h"

namespace detail {
  // Format specifier parser for UniqueId
  struct format_spec {
    bool uppercase = false;   // Use uppercase hex digits
    bool use_dashes = false;  // Use dashes in UUID format
    char fill = ' ';          // Fill character
    int width = 0;            // Minimum field width
    bool align_left = false;  // Left alignment
  };

  constexpr format_spec parse_format_spec(std::string_view fmt)
  {
    format_spec spec;

    for (size_t i = 0; i < fmt.size(); ++i) {
      switch (fmt[i]) {
        case 'X':
          spec.uppercase = true;
          break;
        case 'x':
          spec.uppercase = false;
          break;
        case '-':
          spec.use_dashes = false;
          break;
        case '<':
          spec.align_left = true;
          break;
        case '>':
          spec.align_left = false;
          break;
        case '0':
          if (i == 0 || !std::isdigit(fmt[i - 1])) {
            spec.fill = '0';
            spec.align_left = false;
          }
          break;
        default:
          if (std::isdigit(fmt[i])) {
            spec.width = spec.width * 10 + (fmt[i] - '0');
          }
      }
    }

    return spec;
  }
}  // namespace detail

template<typename T, size_t Size = sizeof(T)>
  requires(std::is_integral_v<T> || std::is_enum_v<T>) && (Size <= sizeof(T))
class UniqueId {
 public:
  using value_type = T;
  static constexpr size_t size = Size;

  UniqueId() : _value{} {}

  // Construct from raw value
  template<typename U>
    requires std::convertible_to<U, T>
  explicit constexpr UniqueId(U value) : _value(static_cast<T>(value))
  {
  }

  // Construct from another UniqueId of different size (upcasting)
  template<typename U, size_t S>
    requires(S <= Size) && std::convertible_to<U, T>
  explicit constexpr UniqueId(const UniqueId<U, S> &other) : _value(static_cast<T>(other.value()))
  {
  }

  // Truncate to smaller UniqueId
  template<typename U, size_t S>
    requires(S < Size) && std::convertible_to<T, U>
  explicit constexpr operator UniqueId<U, S>() const
  {
    return UniqueId<U, S>(static_cast<U>(_value & ((T(1) << (S * 8)) - 1)));
  }

  // Split into multiple smaller UniqueIds
  template<typename U, size_t S>
    requires(S < Size) && std::convertible_to<T, U>
  std::array<UniqueId<U, S>, (Size + S - 1) / S> split() const
  {
    std::array<UniqueId<U, S>, (Size + S - 1) / S> result;
    for (size_t i = 0; i < result.size(); ++i) {
      size_t shift = i * S * 8;
      U mask = (U(1) << (S * 8)) - 1;
      result[i] = UniqueId<U, S>(static_cast<U>((_value >> shift) & mask));
    }
    return result;
  }

  // Copy/move operations
  constexpr UniqueId(const UniqueId &) = default;
  constexpr UniqueId &operator=(const UniqueId &) = default;
  constexpr UniqueId(UniqueId &&) noexcept = default;
  constexpr UniqueId &operator=(UniqueId &&) noexcept = default;

  // Comparison operators
  constexpr auto operator<=>(const UniqueId &) const = default;

  // Get raw value
  [[nodiscard]] constexpr const T &value() const noexcept
  {
    return _value;
  }

  // Get raw bytes
  [[nodiscard]] std::array<uint8_t, size> bytes() const
  {
    std::array<uint8_t, size> result{};
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&_value);
    std::copy_n(bytes, Size, result.begin());
    return result;
  }

  // Set value methods
  template<typename U>
    requires std::convertible_to<U, T>
  void setValue(U value)
  {
    _value = static_cast<T>(value);
  }

  // Set from bytes
  bool setBytes(const uint8_t *data, size_t length)
  {
    if (length != size)
      return false;
    if (!data)
      return false;

    std::copy_n(data, Size, reinterpret_cast<uint8_t *>(&_value));
    return true;
  }

  // Set from hex string
  bool setFromHex(std::string_view hex)
  {
    if (hex.length() != size * 2)
      return false;

    T value = 0;
    for (size_t i = 0; i < size; ++i) {
      uint8_t high = detail::hex_to_int(hex[i * 2]);
      uint8_t low = detail::hex_to_int(hex[i * 2 + 1]);
      if (high == 0xFF || low == 0xFF)
        return false;
      value |= static_cast<T>((high << 4) | low) << (i * 8);
    }
    _value = value;
    return true;
  }

  // Format support
  std::string format(const detail::format_spec &spec = {}) const
  {
    static constexpr char hex_upper[] = "0123456789ABCDEF";
    static constexpr char hex_lower[] = "0123456789abcdef";
    const char *hex = spec.uppercase ? hex_upper : hex_lower;

    std::string result;
    auto bytes_array = bytes();

    // Calculate the required size
    size_t required_size = size * 2;
    if (spec.width > static_cast<int>(required_size)) {
      required_size = spec.width;
    }

    result.reserve(required_size);

    // Add leading fill characters if right-aligned
    if (!spec.align_left && spec.width > 0) {
      size_t fill_count = spec.width - (size * 2);
      result.append(fill_count, spec.fill);
    }

    // Format the hex string
    for (size_t i = 0; i < size; ++i) {
      result.push_back(hex[bytes_array[i] >> 4]);
      result.push_back(hex[bytes_array[i] & 0x0F]);
    }

    // Add trailing fill characters if left-aligned
    if (spec.align_left && spec.width > 0) {
      size_t fill_count = spec.width - result.length();
      result.append(fill_count, spec.fill);
    }

    return result;
  }

  // Convert to string (hexadecimal representation)
  [[nodiscard]] std::string toString() const
  {
    return format();
  }

  // Parse from string
  [[nodiscard]] static std::optional<UniqueId> fromString(std::string_view str)
  {
    UniqueId id;
    return id.setFromHex(str) ? std::optional<UniqueId>(id) : std::nullopt;
  }

  // Generate a new random ID
  [[nodiscard]] static UniqueId generate()
  {
    static thread_local std::random_device rd;
    static thread_local std::mt19937_64 gen(rd());
    static thread_local std::uniform_int_distribution<uint64_t> dis;

    if constexpr (size == sizeof(T)) {
      return UniqueId(static_cast<T>(dis(gen)));
    }
    else {
      constexpr T mask = (T(1) << (size * 8)) - 1;
      return UniqueId(static_cast<T>(dis(gen) & mask));
    }
  }

  // Bitwise operations with another UniqueId
  constexpr UniqueId operator&(const UniqueId &other) const
  {
    return UniqueId(_value & other._value);
  }

  constexpr UniqueId operator|(const UniqueId &other) const
  {
    return UniqueId(_value | other._value);
  }

  constexpr UniqueId operator^(const UniqueId &other) const
  {
    return UniqueId(_value ^ other._value);
  }

  constexpr UniqueId operator~() const
  {
    return UniqueId(~_value);
  }

  constexpr UniqueId &operator&=(const UniqueId &other)
  {
    _value &= other._value;
    return *this;
  }

  constexpr UniqueId &operator|=(const UniqueId &other)
  {
    _value |= other._value;
    return *this;
  }

  constexpr UniqueId &operator^=(const UniqueId &other)
  {
    _value ^= other._value;
    return *this;
  }

  // Bitwise operations with integral types
  template<typename U>
    requires std::is_integral_v<U>
  constexpr UniqueId operator&(U value) const
  {
    return UniqueId(_value & static_cast<T>(value));
  }

  template<typename U>
    requires std::is_integral_v<U>
  constexpr UniqueId operator|(U value) const
  {
    return UniqueId(_value | static_cast<T>(value));
  }

  template<typename U>
    requires std::is_integral_v<U>
  constexpr UniqueId operator^(U value) const
  {
    return UniqueId(_value ^ static_cast<T>(value));
  }

  template<typename U>
    requires std::is_integral_v<U>
  constexpr UniqueId &operator&=(U value)
  {
    _value &= static_cast<T>(value);
    return *this;
  }

  template<typename U>
    requires std::is_integral_v<U>
  constexpr UniqueId &operator|=(U value)
  {
    _value |= static_cast<T>(value);
    return *this;
  }

  template<typename U>
    requires std::is_integral_v<U>
  constexpr UniqueId &operator^=(U value)
  {
    _value ^= static_cast<T>(value);
    return *this;
  }

  // Hash support
  template<typename H> friend H AbslHashValue(H h, const UniqueId &id)
  {
    return H::combine(std::move(h), id._value);
  }

 private:
  T _value;
};

// Formatter specialization for integral types
template<typename T, size_t Size>
struct std::formatter<UniqueId<T, Size>> : std::formatter<std::string> {
  template<typename FormatContext> auto format(const UniqueId<T, Size> &id, FormatContext &ctx)
  {
    return std::formatter<std::string>::format(
        id.format(detail::parse_format_spec(ctx.format_spec())), ctx);
  }
};
