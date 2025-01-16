#pragma once

#include <bit>
#include <compare>
#include <concepts>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace misc {

  // Convert hex character to integer
  constexpr uint8_t hex_to_int(char low)
  {
    if (low >= '0' && low <= '9')
      return low - '0';
    if (low >= 'A' && low <= 'F')
      return low - 'A' + 10;
    if (low >= 'a' && low <= 'f')
      return low - 'a' + 10;
    return 0x0F;
  }

  constexpr uint8_t hex_to_int(char low, char high)
  {
    return (hex_to_int(high) << 4) | hex_to_int(low);
  }

}  // namespace misc
