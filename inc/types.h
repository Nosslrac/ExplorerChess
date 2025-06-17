#pragma once
#include <cstdint>

using move_t = std::uint32_t;
using bitboard_t = std::uint64_t;
using square_t = std::uint8_t;
using score_t = std::int16_t;

enum class MoveType : std::uint8_t
{
  QUIET = 0x1,
  CAPTURES = 0x2,
  CHECKS = 0x4
};
enum class Side : std::uint8_t
{
  WHITE = 0,
  BLACK = 1
};

enum class CastleSide : std::uint8_t
{
  KING,
  QUEEN,
};

// clang-format off
enum Square : int {
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_NONE,

    SQ_ZERO = 0,
    SQ_COUNT   = 64
};
// clang-format on

enum Flags
{
  QUIET = 0,
  DOUBLE_PUSH = 0x10000,
  CASTLE_KING = 0x20000,
  CASTLE_QUEEN = 0x30000,
  CAPTURE = 0x40000,
  EP_CAPTURE = 0x50000,
  PROMO_N = 0x80000,
  PROMO_B = 0x90000,
  PROMO_R = 0xA0000,
  PROMO_Q = 0xB0000,
  PROMO_NC = 0xC0000,
  PROMO_BC = 0xD0000,
  PROMO_RC = 0xE0000,
  PROMO_QC = 0xF0000
};

enum PieceType
{
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  ALL_PIECES = 0,
  NUM_TYPES = 5,
  NUM_COLORS = 2,
};

// clang-format off
struct MoveV2
{
public:
  explicit MoveV2(move_t m) : eval(0), move(m) {}
  [[nodiscard]] constexpr square_t getTo() const { return move & 0xFFU; }
  [[nodiscard]] constexpr square_t getFrom() const { return (move >> 8U) & 0xFFU; }
  [[nodiscard]] constexpr Flags getFlags() const { return static_cast<Flags>(move & 0xFF0000U); }
  [[nodiscard]] constexpr square_t getMover() const { return (move >> 24U) & 0xFU; }
  [[nodiscard]] constexpr square_t getCaptured() const { return move >> 28U; }
  [[nodiscard]] constexpr square_t getPromo() const { return (move >> 16U) & 0x3U; }
  bool operator<(const MoveV2 &other) const { return other.eval < eval; }

private:
  int eval;
  uint32_t move;
};

// clang-format on