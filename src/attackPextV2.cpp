#include "attackPextV2.h"

#include <array>
#include <cassert>
#include <cmath>
#include <iostream>

#include "bitboardUtilV2.h"
#include "types.h"

namespace {

constexpr bool isBoardEdge(square_t start)
{
  return (BB(start) & ~BitboardUtil::NOT_EDGE) != 0;
}

constexpr bool doesClip(square_t start, int dir)
{
  return !BitboardUtil::isOnBoard(start + dir) ||
         (isBoardEdge(start) &&
          std::abs(BitboardUtil::fileOf(start) -
                   BitboardUtil::fileOf(start + dir)) > 1);
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
    if (doesClip(tempSquare, dir))
    {
      continue;
    }

    while (!doesClip(square_t(tempSquare + dir), dir))
    {
      tempSquare += dir;
      board |= BB(tempSquare);
    }
  }
  return board;
}

// Return the rooks attacks for the current occupancy and square
bitboard_t slidingAttackSlow(PieceType pt, bitboard_t occupancy,
                             square_t square)
{
  std::array<int, 4> dirs = pt == BISHOP ? std::array<int, 4>{-7, -9, 7, 9}
                                         : std::array<int, 4>{-1, 1, -8, 8};
  bitboard_t attack = 0ULL;

  for (const auto dir : dirs)
  {
    square_t tempSquare = square;
    if (doesClip(tempSquare, dir))
    {
      continue;
    }

    do
    {
      tempSquare += dir;
      attack |= BB(tempSquare);
    } while (!doesClip(tempSquare, dir) && (occupancy & BB(tempSquare)) == 0);
  }
  return attack;
}

bitboard_t setOccupancy(bitboard_t relevantBits, int pextIndex)
{
  bitboard_t occupancy = 0ULL;
  while (pextIndex != 0)
  {
    const auto sq = BitboardUtil::bitScan(relevantBits);
    relevantBits &= relevantBits - 1;
    occupancy |= (pextIndex & 1) * BB(sq);
    pextIndex >>= 1;
  }
  return occupancy;
}

// Storage area for rook- and bishop attacks
bitboard_t RookArena[102400];
bitboard_t BishopArena[5248];

void init_attack_table(PieceType pt, bitboard_t table[],
                       PEXT_ATTACK::Magic attacks[][2])
{
  int size = 0;
  for (square_t square = SQ_A8; square <= SQ_H1; square++)
  {
    PEXT_ATTACK::Magic &m = attacks[square][pt - BISHOP];

    m.relBits = relevantBits(pt, square);
    const int bits = BitboardUtil::bitCount(m.relBits);
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
      assert(BitboardUtil::pext(occupancy, m.relBits) == i);
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
  init_attack_table(BISHOP, BishopArena, PEXT_ATTACK::ATTACK_MAGICS);
  init_attack_table(ROOK, RookArena, PEXT_ATTACK::ATTACK_MAGICS);
  std::cout << "Attack init complete\n";
}