#pragma once
#include "bitboardUtilV2.h"
#include "types.h"
#include <string>

struct StateInfo
{
  bitboard_t blockForKing;
  bitboard_t pinnedMask;
  bitboard_t enemyAttack;
  bitboard_t checkers;

  // uint8_t numCheckers;
  uint8_t castlingRights;
  uint8_t enPassant;

  // Incremental
  bitboard_t hashKey;
  score_t materialScore;
  score_t materialValue;
};

// Inherits irreversible info from StateInfo
class Position
{
public:
  void init();

  void fenInit(const std::string &fen, StateInfo *st);
  void doMove(MoveV2 move);
  void undoMove(MoveV2 move);

  bitboard_t pieces(PieceType pt = ALL_PIECES) const;
  template <typename... PieceTypes>
  bitboard_t pieces(PieceType pt, PieceTypes... pts) const;
  template <Side s> bool hasPawnsOnEpRank() const;

  Position(const Position &) = delete;
  Position &operator=(const Position &) = delete;

private:
  template <Side s> void doMove(MoveV2 move);
  template <Side s> void undoMove(MoveV2 move);

  // Small inline methods
  template <Side s> void doCastle(CastleSide castleSide);
  template <Side s> bitboard_t EPpawns() const;

  //////////////////
  // Data members //
  //////////////////
  StateInfo *m_st;

  // Kings and bitboards for pieces
  square_t m_kings[NUM_COLORS];
  bitboard_t m_pieceBoards[NUM_TYPES];
  bitboard_t m_teamBoards[NUM_COLORS];

  // Check boards
  bitboard_t m_checkSquares[4];

  bool m_whiteToMove;
  uint16_t m_ply;
};

// Return possible ep capturers
template <Side s> inline bitboard_t Position::EPpawns() const
{
  return m_pieceBoards[PAWN] &
         (s == Side::WHITE
              ? (BitboardUtil::Rank4 & m_teamBoards[BitboardUtil::WHITE])
              : (BitboardUtil::Rank5 & m_teamBoards[BitboardUtil::BLACK]));
}

inline bitboard_t Position::pieces(PieceType pt) const
{
  return m_pieceBoards[pt];
}

template <typename... PieceTypes>
inline bitboard_t Position::pieces(PieceType pt, PieceTypes... pts) const
{
  return pieces(pt) | pieces(pts...);
}

template <Side s> inline bool Position::hasPawnsOnEpRank() const
{
  return EPpawns<s>() != 0;
}

/// @note: Does not work for chess960
template <Side s> inline void Position::doCastle(CastleSide castleSide)
{
  constexpr int side =
      s == Side::WHITE ? BitboardUtil::WHITE : BitboardUtil::BLACK;
  constexpr bitboard_t kingCastle = s == Side::WHITE
                                        ? BitboardUtil::WHITE_KING_ROOK_FROM_TO
                                        : BitboardUtil::BLACK_KING_ROOK_FROM_TO;
  constexpr bitboard_t queenCastle =
      s == Side::WHITE ? BitboardUtil::WHITE_QUEEN_ROOK_FROM_TO
                       : BitboardUtil::BLACK_QUEEN_ROOK_FROM_TO;

  const bitboard_t rookFromTo =
      castleSide == CastleSide::KING ? kingCastle : queenCastle;
  m_pieceBoards[ROOK] ^= rookFromTo;
  m_teamBoards[side] ^= rookFromTo;
}
