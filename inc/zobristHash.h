#pragma once
#include "bitboardUtil.h"


class ZobristHash {
  public:
    ZobristHash() { initHash(); }
    ~ZobristHash() = default;
    uint64_t hashPosition(const Position &pos);

    uint64_t pieceHash[10][64];
    uint64_t kingHash[2][64];
    uint64_t epHash[8];
    uint64_t castleHash[16];
    uint64_t whiteToMoveHash;
    uint64_t moveHash;

  private:
    void initHash();
};
