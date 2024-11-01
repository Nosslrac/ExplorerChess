#include "bitboardUtil.h"

/*
 * Brief: used to find the captured piece's ID in the pieceBoard array
 * Note: Won't find the king, if it returns 0 something has gone wrong elsewhere
 * Default will be white pawns
 */

void initLineBB(bitboard_t (&lineBB)[64][64]) {
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j)
            lineBB[i][j] = pinned_ray(i, j);
    }
}

// Find the ray in which the piece is able to move
bitboard_t pinned_ray(int king, int piece) {
    // On the same rank
    if (king / 8 == piece / 8)
        return ranks[king / 8];
    // On the same file
    if (king % 8 == piece % 8)
        return files[king % 8];
    const int diagonal = king / 8 + king % 8;
    // On the same anti-diagonal
    if (diagonal == piece / 8 + piece % 8)
        return anti_diagonals[diagonal];
    // Otherwise on the same main-diagonal
    if ((king % 8 - king / 8) == (piece % 8 - piece / 8))
        return main_diagonals[7 - king / 8 + king % 8];
    return 0;
}

template uint8_t getPiece<false>(const bitboard_t[], uint8_t);

template uint8_t getPiece<true>(const bitboard_t[], uint8_t);