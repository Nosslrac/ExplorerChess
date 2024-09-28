#pragma once
#include "bitboardUtil.h"

namespace PEXT_ATTACK {
// API


template <Piece p> bitboard_t relevantBits(int square);
bitboard_t                    rookAttackSlow(bitboard_t board, int square);
bitboard_t                    bishopAttackSlow(bitboard_t board, int square);

bitboard_t setOccupancy(bitboard_t relevantBits, int pextIndex);
bool       boardEdge(int x, int dir);

class PextAttack {
  protected:
    PextAttack();
    // inline const bitboard_t bishopAttack(bitboard_t board, int square);
    // inline const bitboard_t rookAttack(bitboard_t board, int square);
    bitboard_t rookAttackPtr[64][4096];
    bitboard_t rookBits[64];
    bitboard_t bishopAttackPtr[64][512];
    bitboard_t bishopBits[64];

  private:
    void initRook();
    void initBishop();
};


} // namespace PEXT_ATTACK
