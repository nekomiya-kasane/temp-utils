#pragma once

#include "unique_id.h"
#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>

// UUID storage using two uint64_t
struct UUIDStorage {
    uint64_t high;
    uint64_t low;

    constexpr bool operator==(const UUIDStorage&) const = default;
    constexpr auto operator<=>(const UUIDStorage&) const = default;

    // Bitwise operations
    constexpr UUIDStorage operator&(const UUIDStorage& other) const {
        return {high & other.high, low & other.low};
    }

    constexpr UUIDStorage operator|(const UUIDStorage& other) const {
        return {high | other.high, low | other.low};
    }

    constexpr UUIDStorage operator^(const UUIDStorage& other) const {
        return {high ^ other.high, low ^ other.low};
    }

    constexpr UUIDStorage operator~() const {
        return {~high, ~low};
    }

    constexpr UUIDStorage& operator&=(const UUIDStorage& other) {
        high &= other.high;
        low &= other.low;
        return *this;
    }

    constexpr UUIDStorage& operator|=(const UUIDStorage& other) {
        high |= other.high;
        low |= other.low;
        return *this;
    }

    constexpr UUIDStorage& operator^=(const UUIDStorage& other) {
        high ^= other.high;
        low ^= other.low;
        return *this;
    }
};

// UUID specialization
template<>
class UniqueId<UUIDStorage> {
public:
    using value_type = UUIDStorage;
    static constexpr size_t size = 16;

    // Default constructor generates a random UUID
    UniqueId() : value_(generate().value_) {}

    // Construct from high and low parts
    template<typename U>
        requires std::convertible_to<U, uint64_t>
    constexpr UniqueId(U high, U low) : value_{static_cast<uint64_t>(high), static_cast<uint64_t>(low)} {}

    // Construct from another UniqueId of different size (upcasting)
    template<typename U, size_t S>
        requires (S <= size)
    explicit constexpr UniqueId(const UniqueId<U, S>& other) {
        auto bytes = other.bytes();
        value_.high = 0;
        value_.low = 0;
        std::copy_n(bytes.begin(), S, reinterpret_cast<uint8_t*>(&value_));
    }

    // Truncate to smaller UniqueId
    template<typename U, size_t S>
        requires (S < size)
    explicit constexpr operator UniqueId<U, S>() const {
        UniqueId<U, S> result;
        auto src_bytes = bytes();
        std::copy_n(src_bytes.begin(), S, reinterpret_cast<uint8_t*>(&result.value()));
        return result;
    }

    // Split into multiple smaller UniqueIds
    template<typename U, size_t S>
        requires (S < size)
    std::array<UniqueId<U, S>, (size + S - 1) / S> split() const {
        std::array<UniqueId<U, S>, (size + S - 1) / S> result;
        auto src_bytes = bytes();
        
        for (size_t i = 0; i < result.size(); ++i) {
            size_t copy_size = std::min(S, size - i * S);
            std::copy_n(src_bytes.begin() + i * S, copy_size, 
                       reinterpret_cast<uint8_t*>(&result[i].value()));
        }
        
        return result;
    }

    // Copy/move operations
    constexpr UniqueId(const UniqueId&) = default;
    constexpr UniqueId& operator=(const UniqueId&) = default;
    constexpr UniqueId(UniqueId&&) noexcept = default;
    constexpr UniqueId& operator=(UniqueId&&) noexcept = default;

    // Comparison operators
    constexpr auto operator<=>(const UniqueId&) const = default;

    // Get raw value
    [[nodiscard]] constexpr const UUIDStorage& value() const noexcept { return value_; }
    [[nodiscard]] constexpr uint64_t high() const noexcept { return value_.high; }
    [[nodiscard]] constexpr uint64_t low() const noexcept { return value_.low; }

    // Get raw bytes
    [[nodiscard]] std::array<uint8_t, size> bytes() const {
        std::array<uint8_t, size> result;
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value_);
        std::copy_n(bytes, size, result.begin());
        return result;
    }

    // Set value methods
    void setValue(uint64_t high, uint64_t low) {
        value_.high = high;
        value_.low = low;
    }

    // Set from bytes
    bool setBytes(const uint8_t* data, size_t length) {
        if (length != size) return false;
        if (!data) return false;
        
        std::copy_n(data, 8, reinterpret_cast<uint8_t*>(&value_.high));
        std::copy_n(data + 8, 8, reinterpret_cast<uint8_t*>(&value_.low));
        return true;
    }

    // Set from hex string
    bool setFromHex(std::string_view hex) {
        std::string cleaned;
        cleaned.reserve(hex.length());
        
        // Remove dashes for UUID format
        for (char c : hex) {
            if (c != '-') cleaned.push_back(c);
        }

        if (cleaned.length() != size * 2) return false;

        uint64_t high = 0, low = 0;
        for (size_t i = 0; i < 8; ++i) {
            uint8_t high_nibble = detail::hex_to_int(cleaned[i * 2]);
            uint8_t low_nibble = detail::hex_to_int(cleaned[i * 2 + 1]);
            if (high_nibble == 0xFF || low_nibble == 0xFF) return false;
            high = (high << 8) | ((high_nibble << 4) | low_nibble);
        }
        for (size_t i = 8; i < 16; ++i) {
            uint8_t high_nibble = detail::hex_to_int(cleaned[i * 2]);
            uint8_t low_nibble = detail::hex_to_int(cleaned[i * 2 + 1]);
            if (high_nibble == 0xFF || low_nibble == 0xFF) return false;
            low = (low << 8) | ((high_nibble << 4) | low_nibble);
        }
        value_.high = high;
        value_.low = low;
        return true;
    }

    // Format support
    std::string format(const detail::format_spec& spec = {}) const {
        static constexpr char hex_upper[] = "0123456789ABCDEF";
        static constexpr char hex_lower[] = "0123456789abcdef";
        const char* hex = spec.uppercase ? hex_upper : hex_lower;
        
        std::string result;
        auto bytes_array = bytes();
        
        // Calculate the required size
        size_t required_size = size * 2;
        if (spec.use_dashes) required_size += 4;  // For UUID dashes
        if (spec.width > static_cast<int>(required_size)) {
            required_size = spec.width;
        }
        
        result.reserve(required_size);
        
        // Add leading fill characters if right-aligned
        if (!spec.align_left && spec.width > 0) {
            size_t fill_count = spec.width - (size * 2 + (spec.use_dashes ? 4 : 0));
            result.append(fill_count, spec.fill);
        }
        
        // Format the hex string
        for (size_t i = 0; i < size; ++i) {
            if (spec.use_dashes) {
                if (i == 4 || i == 6 || i == 8 || i == 10) {
                    result.push_back('-');
                }
            }
            
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
    [[nodiscard]] std::string toString() const {
        return format();
    }

    // Parse from string
    [[nodiscard]] static std::optional<UniqueId> fromString(std::string_view str) {
        UniqueId id;
        return id.setFromHex(str) ? std::optional<UniqueId>(id) : std::nullopt;
    }

    // Generate a new random UUID
    [[nodiscard]] static UniqueId generate() {
        static thread_local std::random_device rd;
        static thread_local std::mt19937_64 gen(rd());
        static thread_local std::uniform_int_distribution<uint64_t> dis;

        uint64_t high = dis(gen);
        uint64_t low = dis(gen);
        
        // Set UUID version (4) and variant (2)
        high = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;  // version 4
        low = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;    // variant 2
        
        return UniqueId(high, low);
    }

    // Bitwise operations with another UUID
    constexpr UniqueId operator&(const UniqueId& other) const {
        return UniqueId(value_.high & other.value_.high, value_.low & other.value_.low);
    }

    constexpr UniqueId operator|(const UniqueId& other) const {
        return UniqueId(value_.high | other.value_.high, value_.low | other.value_.low);
    }

    constexpr UniqueId operator^(const UniqueId& other) const {
        return UniqueId(value_.high ^ other.value_.high, value_.low ^ other.value_.low);
    }

    constexpr UniqueId operator~() const {
        return UniqueId(~value_.high, ~value_.low);
    }

    constexpr UniqueId& operator&=(const UniqueId& other) {
        value_.high &= other.value_.high;
        value_.low &= other.value_.low;
        return *this;
    }

    constexpr UniqueId& operator|=(const UniqueId& other) {
        value_.high |= other.value_.high;
        value_.low |= other.value_.low;
        return *this;
    }

    constexpr UniqueId& operator^=(const UniqueId& other) {
        value_.high ^= other.value_.high;
        value_.low ^= other.value_.low;
        return *this;
    }

    // Bitwise operations with integral types
    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId operator&(U value) const {
        return UniqueId(value_.high & value, value_.low & value);
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId operator|(U value) const {
        return UniqueId(value_.high | value, value_.low | value);
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId operator^(U value) const {
        return UniqueId(value_.high ^ value, value_.low ^ value);
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId& operator&=(U value) {
        value_.high &= value;
        value_.low &= value;
        return *this;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId& operator|=(U value) {
        value_.high |= value;
        value_.low |= value;
        return *this;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId& operator^=(U value) {
        value_.high ^= value;
        value_.low ^= value;
        return *this;
    }

private:
    UUIDStorage value_;
};

// Formatter specialization for UUID
template<>
struct std::formatter<UniqueId<UUIDStorage>> : std::formatter<std::string> {
    template<typename FormatContext>
    auto format(const UniqueId<UUIDStorage>& id, FormatContext& ctx) {
        return std::formatter<std::string>::format(
            id.format(detail::parse_format_spec(ctx.format_spec())), ctx);
    }
};

// Common type alias
using UUID = UniqueId<UUIDStorage>;
