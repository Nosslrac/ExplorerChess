#include "attackPextV2.h"

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>

#include "bitboardUtilV2.h"
#include "types.h"

namespace {

constexpr bool doesClip(square_t start, square_t end)
{
  return std::abs(start % BOARD_DIMMENSION - end % BOARD_DIMMENSION) > 1;
}

constexpr bool isBoardEdge(square_t start, int dir)
{
  return isOnBoard(start) && doesClip(start, square_t(start + dir));
}

bitboard_t relevantBits(PieceType pt, square_t square)
{
  assert(pt == BISHOP || pt == ROOK);

  std::array<int, 4> dirs = pt == BISHOP ? std::array<int, 4>{-7, -9, 7, 9}
                                         : std::array<int, 4>{-1, 1, -8, 8};

  bitboard_t board = 0ULL;

  for (const auto dir : dirs)
  {
    auto tempSquare = square;
    if (abs((tempSquare & 7) - ((tempSquare + dir) & 7)) < 2)
    {
      while (!isBoardEdge(square_t(tempSquare + dir), dir))
      {
        tempSquare += dir;
        board |= BB(tempSquare);
      }
    }
  }
  return board;
}

// Return the rooks attacks for the current occupancy and square
bitboard_t slidingAttackSlow(PieceType pt, bitboard_t board, square_t square)
{
  std::array<int, 4> dirs = pt == BISHOP ? std::array<int, 4>{-7, -9, 7, 9}
                                         : std::array<int, 4>{-1, 1, -8, 8};
  bitboard_t attack = 0ULL;

  for (const auto dir : dirs)
  {
    int tempSquare = square;
    if (abs((tempSquare & 7) - ((tempSquare + dir) & 7)) < 2)
    {
      while (!boardEdge(tempSquare, dir) && (board & BB(tempSquare)) == 0)
      {
        tempSquare += dir;
        attack |= BB(tempSquare);
      }
    }
  }
  return attack;
}

bitboard_t setOccupancy(bitboard_t relevantBits, int pextIndex)
{
  bitboard_t occupancy = 0ULL;
  while (pextIndex != 0)
  {
    const auto sq = bitScan(relevantBits);
    relevantBits &= relevantBits - 1;
    occupancy |= (pextIndex & 1) * BB(sq);
    pextIndex >>= 1;
  }
  return occupancy;
}

template <PieceType p> consteval int getArenaSize()
{
  static_assert(p == ROOK || p == BISHOP, "Invalid pieceType in getArenaSize");

  std::array<int, 4> dirs = p == ROOK ? std::array<int, 4>{1, -1, 8, -8}
                                      : std::array<int, 4>{-7, -9, 7, 9};
  int sum = 0;
  for (const auto dir : dirs)
  {
    sum += dir + 1;
  }
  return sum;
}

// Storage area for rook- and bishop attacks
constexpr int rookSize = getArenaSize<ROOK>();
constexpr int bishopSize = getArenaSize<BISHOP>();
bitboard_t RookArena[rookSize];
bitboard_t BishopArena[bishopSize];

void init_attack_table(PieceType pt, bitboard_t table[],
                       PEXT_ATTACK::Magic attacks[][2])
{
  int size = 0;

  for (square_t square = SQ_A8; square <= SQ_H1; square++)
  {
    PEXT_ATTACK::Magic &m = attacks[square][pt - BISHOP]; // TODO: fix

    m.relBits = relevantBits(pt, square);
    const int bits = bitCount(m.relBits);
    assert(bits < 13);

    m.attacks = square == SQ_A8
                    ? table
                    : attacks[square - 1][pt - BISHOP].attacks + size;
    size = 0; // Reset size for the new square

    int combinations = static_cast<int>(std::pow<int, int>(2, bits));
    for (int i = 0; i < combinations; ++i)
    {
      bitboard_t occupancy = setOccupancy(m.relBits, i);
      const bitboard_t attack = slidingAttackSlow(pt, occupancy, square);
      m.attacks[i] = attack;
      // Verify correct setup
      assert(pext(occupancy, m.relBits) == i);
    }
    size = combinations;
  }
}

} // namespace

namespace PEXT_ATTACK {
alignas(64) Magic ATTACK_MAGICS[SQ_COUNT][2]; // One for bishop, one for rooks

} // namespace PEXT_ATTACK

void ATTACKS::init()
{
  std::cout << "Attack init complete\n";
  init_attack_table(BISHOP, BishopArena, PEXT_ATTACK::ATTACK_MAGICS);
  init_attack_table(ROOK, RookArena, PEXT_ATTACK::ATTACK_MAGICS);
}