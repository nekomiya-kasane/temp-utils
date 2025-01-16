#pragma once

#include "long_integer.h"
#include "unique_id.h"
#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <random>
#include <string>
#include <string_view>
#include <type_traits>

// UUID specialization using uint128_t
template<>
class UniqueId<uint128_t> {
public:
    using value_type = uint128_t;
    static constexpr size_t size = 16;

    // Default constructor generates a random UUID
    UniqueId() : value_(generate().value_) {}

    // Construct from high and low parts
    template<typename U>
        requires std::convertible_to<U, uint64_t>
    constexpr UniqueId(U high, U low) : value_(uint128_t(static_cast<uint64_t>(high), static_cast<uint64_t>(low))) {}

    // Construct from 8-4-4-4-12 format components
    constexpr UniqueId(uint32_t time_low, uint16_t time_mid, uint16_t time_hi_and_version,
                      uint16_t clock_seq, uint64_t node) {
        // Combine the components into high and low 64-bit values
        uint64_t high = (static_cast<uint64_t>(time_low) << 32) |
                       (static_cast<uint64_t>(time_mid) << 16) |
                       static_cast<uint64_t>(time_hi_and_version);
        
        uint64_t low = (static_cast<uint64_t>(clock_seq) << 48) |
                      (node & 0x0000FFFFFFFFFFFFULL);
        
        value_ = uint128_t(high, low);
    }

    // Construct from another UniqueId of different size (upcasting)
    template<typename U, size_t S>
        requires (S <= size)
    explicit constexpr UniqueId(const UniqueId<U, S>& other) {
        auto bytes = other.bytes();
        value_ = uint128_t(0);
        std::copy_n(bytes.begin(), S, reinterpret_cast<uint8_t*>(&value_));
    }

    // Truncate to smaller UniqueId
    template<typename U, size_t S>
        requires (S < size)
    explicit constexpr operator UniqueId<U, S>() const {
        UniqueId<U, S> result;
        auto src_bytes = bytes();
        std::copy_n(src_bytes.begin(), S, reinterpret_cast<uint8_t*>(&result.value_));
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
                       reinterpret_cast<uint8_t*>(&result[i].value_));
        }
        
        return result;
    }

    // Get UUID components in 8-4-4-4-12 format
    constexpr uint32_t time_low() const { 
        return static_cast<uint32_t>(value_.high >> 32); 
    }
    
    constexpr uint16_t time_mid() const { 
        return static_cast<uint16_t>(value_.high >> 16); 
    }
    
    constexpr uint16_t time_hi_and_version() const { 
        return static_cast<uint16_t>(value_.high); 
    }
    
    constexpr uint16_t clock_seq() const { 
        return static_cast<uint16_t>(value_.low >> 48); 
    }
    
    constexpr uint64_t node() const { 
        return value_.low & 0x0000FFFFFFFFFFFFULL; 
    }

    // Copy/move operations
    constexpr UniqueId(const UniqueId&) = default;
    constexpr UniqueId& operator=(const UniqueId&) = default;
    constexpr UniqueId(UniqueId&&) noexcept = default;
    constexpr UniqueId& operator=(UniqueId&&) noexcept = default;

    // Comparison operators
    constexpr auto operator<=>(const UniqueId&) const = default;

    // Access the underlying value
    constexpr const value_type& value() const { return value_; }
    constexpr value_type& value() { return value_; }

    // Get raw bytes
    constexpr std::array<uint8_t, size> bytes() const {
        std::array<uint8_t, size> result;
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value_);
        std::copy_n(ptr, size, result.begin());
        return result;
    }

    // Set value methods
    void setValue(uint64_t high, uint64_t low) {
        value_ = uint128_t(high, low);
    }

    // Set from bytes
    bool setBytes(const uint8_t* data, size_t length) {
        if (length != size) return false;
        if (!data) return false;
        
        std::copy_n(data, size, reinterpret_cast<uint8_t*>(&value_));
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
        value_ = uint128_t(high, low);
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
        UniqueId result;
        result.value_ = value_ & other.value_;
        return result;
    }

    constexpr UniqueId operator|(const UniqueId& other) const {
        UniqueId result;
        result.value_ = value_ | other.value_;
        return result;
    }

    constexpr UniqueId operator^(const UniqueId& other) const {
        UniqueId result;
        result.value_ = value_ ^ other.value_;
        return result;
    }

    constexpr UniqueId operator~() const {
        UniqueId result;
        result.value_ = ~value_;
        return result;
    }

    constexpr UniqueId& operator&=(const UniqueId& other) {
        value_ &= other.value_;
        return *this;
    }

    constexpr UniqueId& operator|=(const UniqueId& other) {
        value_ |= other.value_;
        return *this;
    }

    constexpr UniqueId& operator^=(const UniqueId& other) {
        value_ ^= other.value_;
        return *this;
    }

    // Bitwise operations with integral types
    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId operator&(U value) const {
        UniqueId result;
        result.value_ = value_ & value;
        return result;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId operator|(U value) const {
        UniqueId result;
        result.value_ = value_ | value;
        return result;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId operator^(U value) const {
        UniqueId result;
        result.value_ = value_ ^ value;
        return result;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId& operator&=(U value) {
        value_ &= value;
        return *this;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId& operator|=(U value) {
        value_ |= value;
        return *this;
    }

    template<typename U>
        requires std::is_integral_v<U>
    constexpr UniqueId& operator^=(U value) {
        value_ ^= value;
        return *this;
    }

private:
    value_type value_;

    // Generate a random UUID (version 4)
    static UniqueId generate() {
        static thread_local std::random_device rd;
        static thread_local std::mt19937_64 gen(rd());
        static thread_local std::uniform_int_distribution<uint64_t> dis;

        uint64_t high = dis(gen);
        uint64_t low = dis(gen);

        // Set version to 4
        high = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        // Set variant to RFC 4122
        low = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

        return UniqueId(high, low);
    }
};

// Formatter specialization for UUID
template<>
struct std::formatter<UniqueId<uint128_t>> : std::formatter<std::string> {
    template<typename FormatContext>
    auto format(const UniqueId<uint128_t>& id, FormatContext& ctx) {
        return std::formatter<std::string>::format(
            id.format(detail::parse_format_spec(ctx.format_spec())), ctx);
    }
};

// Common type alias
using Uuid = UniqueId<uint128_t>;
