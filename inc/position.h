#pragma once
#include "bitboardUtilV2.h"
#include "types.h"
#include <cassert>
#include <string>

struct StateInfo final
{
  // Copied when making a move
  uint8_t castlingRights;
  square_t enPassant;
  score_t materialScore;
  score_t materialValue;

  // Recomputed during doMove
  bitboard_t blockForKing;
  bitboard_t pinnedMask;
  bitboard_t enemyAttack;
  bitboard_t checkers;

  bitboard_t hashKey;
  StateInfo *prevSt;
};

// Inherits irreversible info from StateInfo
class Position final
{
public:
  explicit Position() = default;

  void init();

  void fenInit(const std::string &fen, StateInfo &st);
  void doMove(MoveV2 move, StateInfo &newSt);
  void undoMove(MoveV2 move);

  // Fetching pieceBoards and teamBoards (don't use with KING)
  constexpr bitboard_t pieces(Side s) const;
  template <PieceType... pts> constexpr bitboard_t pieces() const;
  template <Side s, PieceType... pts> constexpr bitboard_t pieces() const;

  bitboard_t attackOn(square_t square, bitboard_t board) const;

  template <Side s> bool hasPawnsOnEpRank() const;

  Position(const Position &) = delete;
  Position &operator=(const Position &) = delete;

  // Utility and setup
  void placePiece(PieceV2 piece, square_t square);
  void printPieces(const std::string &fen) const;

private:
  template <Side s> void doMove(MoveV2 move, StateInfo &newSt);
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
  PieceV2 m_board[SQ_COUNT];

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

template <> inline constexpr bitboard_t Position::pieces<KING>() const
{
  return BB(m_kings[0]) | BB(m_kings[1]);
}

inline constexpr bitboard_t Position::pieces(Side s) const
{
  return m_teamBoards[static_cast<std::uint8_t>(s)];
}

template <PieceType... pts> inline constexpr bitboard_t Position::pieces() const
{
  return (m_pieceBoards[pts] | ...);
}

template <Side s, PieceType... pts>
inline constexpr bitboard_t Position::pieces() const
{
  return pieces(s) & pieces<pts...>();
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

inline void Position::placePiece(PieceV2 piece, square_t square)
{
  assert(piece >= W_PAWN && piece <= B_KING);
  assert(BitboardUtil::isOnBoard(square));
  m_board[square] = piece;
  if (piece < B_PAWN)
  {
    m_teamBoards[BitboardUtil::WHITE] |= BB(square);
    m_pieceBoards[piece] |= BB(square);
  }
  else if (piece <= B_QUEEN)
  {
    m_teamBoards[BitboardUtil::BLACK] |= BB(square);
    m_pieceBoards[piece - BitboardUtil::BLACK_OFFSET] |= BB(square);
  }
  else
  {
    m_kings[piece - W_KING] = square;
  }
  m_pieceBoards[ALL_PIECES] |= BB(square);
}
