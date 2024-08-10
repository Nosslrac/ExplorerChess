#pragma once
#include "bitboardUtil.h"

namespace PEXT_ATTACK{
    //API
   


    template<Piece p>
    uint64_t relevantBits(int square);
    uint64_t rookAttackSlow(uint64_t board, int square);
    uint64_t bishopAttackSlow(uint64_t board, int square);

    uint64_t setOccupancy(uint64_t relevantBits, int pextIndex);
    bool boardEdge(int x, int dir);

    class PextAttack{
    protected:
    PextAttack();
    // inline const uint64_t bishopAttack(uint64_t board, int square);
    // inline const uint64_t rookAttack(uint64_t board, int square);
    uint64_t rookAttackPtr[64][4096];
    uint64_t rookBits[64];
    uint64_t bishopAttackPtr[64][512];
    uint64_t bishopBits[64];
    private:
    void initRook();
    void initBishop();


    };

    
}

