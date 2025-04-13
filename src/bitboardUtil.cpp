#include "bitboardUtil.h"

/*
 * Brief: used to find the captured piece's ID in the pieceBoard array
 * Note: Won't find the king, if it returns 0 something has gone wrong elsewhere
 * Default will be white pawns
 */

void initLineBB(bitboard_t (&lineBB)[64][64])
{
  for (int i = 0; i < 64; ++i)
  {
    for (int j = 0; j < 64; ++j)
      lineBB[i][j] = pinned_ray(i, j);
  }
}

// Find the ray in which the piece is able to move
bitboard_t pinned_ray(int king, int piece)
{
  // On the same rank
  if (king / BOARD_DIMMENSION == piece / BOARD_DIMMENSION)
  {
    return ranks[king / BOARD_DIMMENSION];
  }
  // On the same file
  if (king % BOARD_DIMMENSION == piece % BOARD_DIMMENSION)
  {
    return files[king % BOARD_DIMMENSION];
  }

  const int diagonal = king / BOARD_DIMMENSION + king % BOARD_DIMMENSION;
  // On the same anti-diagonal
  if (diagonal == piece / BOARD_DIMMENSION + piece % BOARD_DIMMENSION)
  {
    return anti_diagonals[diagonal];
  }
  // Otherwise on the same main-diagonal
  if ((king % BOARD_DIMMENSION - king / BOARD_DIMMENSION) ==
      (piece % BOARD_DIMMENSION - piece / BOARD_DIMMENSION))
  {

    return main_diagonals[7 - king / BOARD_DIMMENSION +
                          king % BOARD_DIMMENSION];
  }
  return 0;
}
