#pragma once
#include <cstdint>

using move_t     = uint32_t;
using bitboard_t = uint64_t;

enum class MoveType : uint8_t { QUIET = 0x1, CAPTURES = 0x2, CHECKS = 0x4 };
enum class Side : bool { WHITE = true, BLACK = false };