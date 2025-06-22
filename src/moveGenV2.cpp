#include "moveGenV2.h"

#include "attackPextV2.h"
#include "position.h"
#include "types.h"

#include <iostream>
#include <string>

namespace MoveGen {

template <MoveFilter filter, Side s>
index_t generate(const Position &pos, Move *moveList)
{
  pos.attackOn(0, BB(7U));

  *moveList++ = Move(0 | 213);

  return 0;
}

// Template specializations for the attacks function
template <>
bitboard_t attacks<KING>(const bitboard_t /*occupancy*/, const square_t square)
{
  return PseudoAttacks::KingAttacks[square];
}

template <>
bitboard_t attacks<KNIGHT>(const bitboard_t /*occupancy*/,
                           const square_t square)
{
  return PseudoAttacks::KnightAttacks[square];
}

template <>
bitboard_t attacks<BISHOP>(const bitboard_t occupancy, const square_t square)
{
  return PEXT_ATTACK::ATTACK_MAGICS[square][0].attackBB(occupancy);
}

template <>
bitboard_t attacks<ROOK>(const bitboard_t occupancy, const square_t square)
{
  return PEXT_ATTACK::ATTACK_MAGICS[square][1].attackBB(occupancy);
}

template <>
bitboard_t attacks<QUEEN>(const bitboard_t occupancy, const square_t square)
{
  return attacks<ROOK>(occupancy, square) | attacks<BISHOP>(occupancy, square);
}

template <Side s, PieceType p>
bitboard_t attacks(const bitboard_t /*occupancy*/, const square_t square)

{
  return PseudoAttacks::PawnAttacks[static_cast<int>(s)][square];
}

// Explicit instantiations for pawns
template bitboard_t attacks<Side::WHITE, PAWN>(const bitboard_t,
                                               const square_t);
template bitboard_t attacks<Side::BLACK, PAWN>(const bitboard_t,
                                               const square_t);

} // namespace MoveGen

void PseudoAttacks::print_bit_board(bitboard_t b)
{
  std::string stb = "";
  stb += "--------------------\n";
  for (int i = 0; i < 8; i++)
  {
    stb += "| ";
    for (int j = 0; j < 8; j++)
    {
      stb += (b & 1) == 1 ? "x " : "0 ";
      b >>= 1;
    }
    stb += " |";
    std::cout << stb;
    std::cout << "\n";
    stb = "";
  }
  std::cout << "--------------------\n\n";
}