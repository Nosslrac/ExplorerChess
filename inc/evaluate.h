#pragma once
#include <chrono>

#include "bitboardUtil.h"
#include "moveGen.h"
#include "robin_hood.h"
#include "types.h"
/*
class Evaluation
{
public:
  explicit Evaluation(const MoveGen &moveGen);
  ~Evaluation() = default;

  template <bool whiteToMove> int evaluate(const Position &pos) const;

  inline int pieceScoreChange(int8_t from, int8_t to, int8_t mover) const
  {
    return PSTs[mover][to] - PSTs[mover][from];
  }

  inline int pieceScore(int8_t sq, int8_t mover) const
  {
    return PSTs[mover][sq];
  }

  inline int getPieceValue(int8_t index) { return pieceValue[index]; }

  // Material balance
  score_t initMaterialValue(const Position &pos) const;
  score_t staticPieceEvaluation(const bitboard_t piece[10]) const;

private:
  uint16_t pieceValue[5] = {100, 300, 300, 500, 900};

  template <bool whiteToMove> int undefendedPieces(const Position &pos) const;

  // Static pawn structure evaluation, to be stored in a lookup table
  //- Isolated pawns
  //- Passed pawns (+ if it is protected by another pawn)
  template <bool whiteToMove> int passedPawns(const bitboard_t piece[10]) const;

  // Undefended pieces penalty

  // Outpost detection

  // Attacking potential
  template <bool whiteToMove>
  int attackPotential(const bitboard_t piece[10]) const;

  robin_hood::unordered_map<bitboard_t, int> pawnStructure;

  // King safety (use some kind of pext for surrounding squares and then
  // switch case maybe) Also open lines based on oponents sliding pieces
  // potential penalty
  template <bool whiteToMove> int kingSafety(const Position &pos) const;

  // Undefended pieces penalty

  const int8_t whiteKingPST[64] = {
      -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
      -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
      -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
      -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
      0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};

  const int8_t blackKingPST[64] = {
      20,  30,  10,  0,   0,   10,  30,  20,  20,  20,  0,   0,   0,
      0,   20,  20,  -10, -20, -20, -20, -20, -20, -20, -10, -20, -30,
      -30, -40, -40, -30, -30, -20, -30, -40, -40, -50, -50, -40, -40,
      -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50,
      -50, -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30,
  };

  const int8_t whitePawnPST[64] = {
      0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
      10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
      0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
      5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0};

  const int8_t blackPawnPST[64] = {
      0,  0,  0,   0,  0,  0,   0,  0,  5,  10, 10, -20, -20, 10, 10, 5,
      5,  -5, -10, 0,  0,  -10, -5, 5,  0,  0,  0,  20,  20,  0,  0,  0,
      5,  5,  10,  25, 25, 10,  5,  5,  10, 10, 20, 30,  30,  20, 10, 10,
      50, 50, 50,  50, 50, 50,  50, 50, 0,  0,  0,  0,   0,   0,  0,  0};

  const int8_t whiteKnightPST[64] = {
      -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
      0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
      15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
      -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
      5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
  };

  const int8_t blackKnightPST[64] = {
      -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
      0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
      15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
      -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
      5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
  };

  const int8_t whiteBishopPST[64] = {
      -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
      0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
      5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
      -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
      0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
  };

  const int8_t blackBishopPST[64] = {
      -20, -10, -10, -10, -10, -10, -10, -20, -10, 5,   0,   0,   0,
      0,   5,   -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 0,
      10,  10,  10,  10,  0,   -10, -10, 5,   5,   10,  10,  5,   5,
      -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 0,   0,   0,
      0,   0,   0,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
  };

  const int8_t whiteRookPST[64] = {
      0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
      -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
      -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
      -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0};

  const int8_t blackRookPST[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,  -5, 0, 0, 0, 0, 0, 0, -5,
      -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
      -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0, 0, 0, 0, 0, 0, -5,
      5,  10, 10, 10, 10, 10, 10, 5,  0,  0, 0, 0, 0, 0, 0, 0,

  };

  const int8_t *PSTs[12] = {whitePawnPST,   whiteKnightPST, whiteBishopPST,
                            whiteRookPST,   whiteBishopPST, blackPawnPST,
                            blackKnightPST, blackBishopPST, blackRookPST,
                            blackBishopPST, whiteKingPST,   blackKingPST};

  // MoveGen to look up attacks and moves
  MoveGen m_moveGen;
};
*/