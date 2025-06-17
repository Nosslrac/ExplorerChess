#pragma once

#include "bitboardUtilV2.h"

#ifdef PEXT
#include "attackPextV2.h"
#else
#include "attacks.h"
#endif

// #define getTo(move) (move & 0xFFU)
// #define getFrom(move) ((move >> 8U) & 0xFFU)
// #define getFlags(move) (move & 0xFF0000U)
// #define getMover(move) ((move >> 24U) & 0xFU)
// #define getCaptured(move) (move >> 28U)
// #define getPromo(move) ((move >> 16U) & 0x3U)
//

// clang-format off
struct Move
{
public:
  explicit Move(move_t m) : eval(0), move(m) {}
  [[nodiscard]] constexpr square_t getTo() const { return move & 0xFFU; }
  [[nodiscard]] constexpr square_t getFrom() const { return (move >> 8U) & 0xFFU; }
  [[nodiscard]] constexpr Flags getFlags() const { return static_cast<Flags>(move & 0xFF0000U); }
  [[nodiscard]] constexpr square_t getMover() const { return (move >> 24U) & 0xFU; }
  [[nodiscard]] constexpr square_t getCaptured() const { return move >> 28U; }
  [[nodiscard]] constexpr square_t getPromo() const { return (move >> 16U) & 0x3U; }
  bool operator<(const Move &other) const { return other.eval < eval; }

private:
  int eval;
  uint32_t move;
};
// clang-format on

struct MoveList
{
  move_t moves[100] = {};
  uint8_t curr = 0;
  inline void add(uint32_t move) { moves[curr++] = move; }
  inline uint8_t size() const { return curr; }
};

// TODO: ADD IN POSITION STRUCT

template <Piece p> inline bitboard_t pieces(const bitboard_t pieces[])
{
  return pieces[p] | pieces[BLACK + p];
}

template <bool white>
inline uint8_t getPiece(const bitboard_t pieces[], const uint8_t sq)
{
  const bitboard_t fromBB = BB(sq);
  constexpr uint8_t last = BLACK + BLACK * !white;
  for (uint8_t i = BLACK * !white; i < last; ++i)
  {
    if (pieces[i] & fromBB)
    {
      return i;
    }
  }
  return white ? King : King + 1;
}
