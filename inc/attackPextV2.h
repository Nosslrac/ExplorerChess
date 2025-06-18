#pragma once
#include "bitboardUtilV2.h"
#include "types.h"

namespace ATTACKS {
void init();
}

namespace PEXT_ATTACK {

// Contains all necessary information
struct Magic
{
  bitboard_t relBits;
  bitboard_t *attacks;

  int index(bitboard_t occupied) const
  {
    return BitboardUtil::pext(occupied, relBits);
  }
};

extern Magic ATTACK_MAGICS[SQ_COUNT][2];

} // namespace PEXT_ATTACK
