#pragma once
#include "bitboardUtilV2.h"
#include "moveGenV2.h"
#include "types.h"
#include <cassert>
#include <string>

struct StateInfo final
{
  // Copied when making a move
  std::uint8_t castlingRights;
  square_t enPassant;
  PieceType capturedPiece;
  score_t materialScore;
  score_t materialValue;

  // Recomputed during doMove
  bitboard_t blockForKing;
  bitboard_t pinnedMask;
  bitboard_t checkers;

  bitboard_t hashKey;
  StateInfo *prevSt;

  constexpr StateInfo()
      : castlingRights(0), enPassant(SQ_NONE), capturedPiece(NO_PIECE)
  {}
};

// Inherits irreversible info from StateInfo
class Position final
{
public:
  explicit Position() = default;

  void init();

  void fenInit(const std::string &fen, StateInfo &st);
  void doMove(Move move, StateInfo &newSt);
  void undoMove(Move move);
  template <Side s> void doMove(Move move, StateInfo &newSt);
  template <Side s> void undoMove(Move move);

  // Fetching pieceBoards and teamBoards (don't use with KING)
  template <Side s> constexpr bitboard_t pieces_s() const;
  template <PieceType... pts> constexpr bitboard_t pieces() const;
  template <Side s, PieceType... pts> constexpr bitboard_t pieces() const;
  template <Side s> constexpr square_t kingSquare() const;

  bitboard_t attackOn(square_t square, bitboard_t board) const;
  template <Side s>
  constexpr bool isSafeSquares(bitboard_t squaresToCheck,
                               bitboard_t board) const;

  template <Side s> bool hasPawnsOnEpRank() const;
  bool isWhiteToMove() const;
  template <Side s> constexpr std::uint8_t castleRights() const;

  Position(const Position &) = delete;
  Position &operator=(const Position &) = delete;

  // Utility and setup
  void placePiece(PieceType piece, square_t square, index_t team);
  void printPieces(const std::string &fen) const;

private:
  // Small inline methods
  template <Side s> bitboard_t EPpawns() const;

  //////////////////
  // Data members //
  //////////////////
  StateInfo *m_st;

  // Kings and bitboards for pieces
  square_t m_kings[NUM_COLORS];
  bitboard_t m_pieceBoards[NUM_TYPES];
  bitboard_t m_teamBoards[NUM_COLORS];
  PieceType m_board[SQ_COUNT];

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

template <Side s> constexpr square_t Position::kingSquare() const
{
  return m_kings[static_cast<index_t>(s)];
}

template <> inline constexpr bitboard_t Position::pieces<KING>() const
{
  return BB(m_kings[0]) | BB(m_kings[1]);
}

template <Side s> inline constexpr bitboard_t Position::pieces_s() const
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
  return pieces_s<s>() & pieces<pts...>();
}

template <Side s> inline bool Position::hasPawnsOnEpRank() const
{
  return EPpawns<s>() != 0;
}

template <Side s> inline constexpr std::uint8_t Position::castleRights() const
{
  return s == Side::WHITE ? m_st->castlingRights : m_st->castlingRights >> 2U;
}

template <Side s>
constexpr bool Position::isSafeSquares(bitboard_t squaresToCheck,
                                       bitboard_t board) const
{
  for (; squaresToCheck != 0; squaresToCheck &= squaresToCheck - 1)
  {
    if (attackOn(BitboardUtil::bitScan(squaresToCheck), board))
    {
      return false;
    }
  }
  return true;
}

inline bool Position::isWhiteToMove() const { return m_whiteToMove; }