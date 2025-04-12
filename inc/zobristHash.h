#pragma once
#include "bitboardUtil.h"

class ZobristHash
{
public:
  ZobristHash() { initHash(); }
  ~ZobristHash() = default;
  bitboard_t hashPosition(const Position &pos);

  bitboard_t pieceHash[10][64];
  bitboard_t kingHash[2][64];
  bitboard_t epHash[8];
  bitboard_t castleHash[16];
  bitboard_t whiteToMoveHash;
  bitboard_t moveHash;

private:
  void initHash();
};
