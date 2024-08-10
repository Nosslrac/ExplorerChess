#include "attackPext.h"
#include "bitboardUtil.h"
#include <array>
#include <cassert>
#include <cmath>

namespace PEXT_ATTACK {

//////////////////////////////////////////////////////////////////
//////////////////////// Class functions //////////////////////////
//////////////////////////////////////////////////////////////////
PextAttack::PextAttack() {
    initBishop();
    initRook();
}

void PextAttack::initRook() {
    for (int square = 0; square < 64; ++square) {
        uint64_t relBits = relevantBits<Rook>(square);
        rookBits[square] = relBits;
        const int bits   = bitCount(relBits);
        assert(bits < 13);
        int combinations = static_cast<int>(std::pow<int, int>(2, bits));
        for (int i = 0; i < combinations; ++i) {
            uint64_t       occupancy = setOccupancy(relBits, i);
            const uint64_t attack    = rookAttackSlow(occupancy, square);
            rookAttackPtr[square][i] = attack;
            assert(pext(occupancy, relBits) == i);
        }
    }
}

void PextAttack::initBishop() {
    for (int square = 0; square < 64; ++square) {
        uint64_t relBits   = relevantBits<Bishop>(square);
        bishopBits[square] = relBits;
        const int bits     = bitCount(relBits);
        assert(bits < 10); // Max should be 9 bits
        int combinations = static_cast<int>(std::pow<int, int>(2, bits));
        for (int i = 0; i < combinations; ++i) {
            uint64_t       occupancy   = setOccupancy(relBits, i);
            const uint64_t attack      = bishopAttackSlow(occupancy, square);
            bishopAttackPtr[square][i] = attack;
            assert(pext(occupancy, relBits) == i);
        }
    }
}

template <Piece p> uint64_t relevantBits(int square) {
    assert(p == Bishop || p == Rook);

    std::array<int, 4> dirs;
    if constexpr (p == Bishop) {
        dirs = {-7, -9, 7, 9};
    } else {
        dirs = {-1, 1, -8, 8};
    }

    uint64_t board = 0ULL;

    for (int i = 0; i < 4; ++i) {
        int tempSquare = square;
        if (abs((tempSquare & 7) - ((tempSquare + dirs[i]) & 7)) < 2) {
            while (!boardEdge(tempSquare + dirs[i], dirs[i])) {
                tempSquare += dirs[i];
                board |= BB(tempSquare);
            }
        }
    }
    return board;
}

// Return the rooks attacks for the current occupancy and square
uint64_t rookAttackSlow(uint64_t board, int square) {
    const int dirs[4] = {-1, 1, -8, 8};
    uint64_t  attack  = 0ULL;

    for (int i = 0; i < 4; ++i) {
        int tempSquare = square;
        if (abs((tempSquare & 7) - ((tempSquare + dirs[i]) & 7)) < 2) {
            while (!boardEdge(tempSquare, dirs[i]) &&
                   (board & BB(tempSquare)) == 0) {
                tempSquare += dirs[i];
                attack |= BB(tempSquare);
            }
        }
    }
    return attack;
}

uint64_t bishopAttackSlow(uint64_t board, int square) {
    const int dirs[4] = {-7, -9, 7, 9};
    uint64_t  attack  = 0ULL;

    for (int i = 0; i < 4; ++i) {
        int tempSquare = square;
        if (abs((tempSquare & 7) - ((tempSquare + dirs[i]) & 7)) < 2) {
            while (!boardEdge(tempSquare, dirs[i]) &&
                   (board & BB(tempSquare)) == 0) {
                tempSquare += dirs[i];
                attack |= BB(tempSquare);
            }
        }
    }
    return attack;
}

uint64_t setOccupancy(uint64_t relevantBits, int pextIndex) {
    uint64_t occupancy = 0ULL;
    while (pextIndex) {
        int sq = bitScan(relevantBits);
        relevantBits &= relevantBits - 1;
        occupancy |= (pextIndex & 1) * BB(sq);
        pextIndex >>= 1;
    }
    return occupancy;
}

bool boardEdge(int x, int dir) {
    return (x + dir) > 63 || (x + dir) < 0 || abs(x % 8 - (x + dir) % 8) > 1;
}

} // namespace PEXT_ATTACK
