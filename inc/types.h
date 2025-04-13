#pragma once
#include <cstdint>

using move_t = std::uint32_t;
using bitboard_t = std::uint64_t;
using square_t = std::uint8_t;
using score_t = std::int16_t;

enum class MoveType : uint8_t
{
  QUIET = 0x1,
  CAPTURES = 0x2,
  CHECKS = 0x4
};
enum class Side : bool
{
  WHITE = true,
  BLACK = false
};