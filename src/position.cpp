#include "position.h"
#include "bitboardUtilV2.h"
#include "moveGenV2.h"
#include "types.h"

#include <cctype>
#include <cstring>
#include <ios>
#include <iostream>
#include <sstream>
#include <string_view>

template void Position::doMove<Side::WHITE>(Move, StateInfo &);
template void Position::doMove<Side::BLACK>(Move, StateInfo &);
template void Position::undoMove<Side::WHITE>(Move);
template void Position::undoMove<Side::BLACK>(Move);

namespace {
constexpr std::string_view PieceIndexes(" PNBRQK pnbrqk");
constexpr std::string_view CastlingIndexes("KQkq");
} // namespace

void Position::doMove(Move move, StateInfo &newSt)
{
  if (m_whiteToMove)
  {
    doMove<Side::WHITE>(move, newSt);
  }
  else
  {
    doMove<Side::BLACK>(move, newSt);
  }
}

void Position::undoMove(Move move)
{
  if (m_whiteToMove)
  {
    undoMove<Side::WHITE>(move);
  }
  else
  {
    undoMove<Side::BLACK>(move);
  }
}

template <Side s> void Position::doMove(Move move, StateInfo &newSt)
{
  static_assert(std::is_trivially_copyable_v<StateInfo>,
                "Memcpy cannot be performed safely");
  std::memcpy(
      &newSt, m_st,
      __builtin_offsetof(StateInfo,
                         blockForKing)); // TODO: test if this is slower than
                                         // manually writing the values??
  newSt.prevSt = m_st;
  m_st = &newSt;

  // Get move info
  const square_t from = move.getFrom();
  const square_t to = move.getTo();
  const bitboard_t fromBB = BB(from);
  const bitboard_t toBB = BB(to);

  const PieceType mover = m_board[from];
  const FlagsV2 flags = move.getFlags();

  constexpr auto team = static_cast<index_t>(s);
  constexpr Side enemy = s == Side::WHITE ? Side::BLACK : Side::WHITE;

  constexpr const BitboardUtil::Masks *masks = BitboardUtil::bitboardMasks<s>();

  m_teamBoards[team] ^= fromBB ^ toBB;
  m_board[to] = mover; // Will be overwritten if we have a promotion

  // Remove ep possiblity
  if (m_st->enPassant != SQ_NONE)
  {
    m_st->enPassant = SQ_NONE;
  }

  if (mover == PieceType::KING)
  {
    m_kings[team] = to;
  }
  else
  {
    m_pieceBoards[mover] ^= fromBB ^ toBB;
  }

  if (flags == QUIET_ || flags == DOUBLE_JUMP)
  {
    const PieceType captured = m_board[to];
    if (captured != NO_PIECE)
    {
      m_st->capturedPiece = captured;
      m_pieceBoards[captured] ^= toBB;
      m_teamBoards[team ^ 1U] ^= toBB;
    }
    m_st->enPassant = flags == DOUBLE_JUMP && hasPawnsOnEpRank<enemy>()
                          ? static_cast<square_t>(to + masks->UP)
                          : SQ_NONE;
  }
  else if (flags == CASTLE)
  {
    if (toBB & masks->CASTLE_KING_PIECES)
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_KING_ROOK_FROM_TO;
      m_board[masks->CASTLE_KING_ROOK_SOURCE] = NO_PIECE;
      m_board[masks->CASTLE_KING_ROOK_DEST] = ROOK;
    }
    else
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_QUEEN_ROOK_FROM_TO;
      m_board[masks->CASTLE_QUEEN_ROOK_SOURCE] = NO_PIECE;
      m_board[masks->CASTLE_QUEEN_ROOK_DEST] = ROOK;
    }
  }
  else if (flags == EN_PASSANT)
  {
    const bitboard_t enemyPawnBB =
        (s == Side::WHITE ? toBB << static_cast<index_t>(SOUTH)
                          : toBB >> static_cast<index_t>(SOUTH));
    m_pieceBoards[PAWN] ^= enemyPawnBB;
    m_teamBoards[team ^ 1U] ^= enemyPawnBB;
    m_board[to + masks->DOWN] = NO_PIECE;
  }
  else if (flags == PROMOTION)
  {
    m_pieceBoards[PAWN] ^= toBB; // Remove pawn again
    m_pieceBoards[KNIGHT + move.getPromo()] |= toBB;

    const PieceType captured = m_board[to];
    if (captured != NO_PIECE)
    {
      m_st->capturedPiece = captured;
      m_pieceBoards[captured] ^= toBB;
      m_teamBoards[team ^ 1U] ^= toBB;
    }
  }

  m_st->castlingRights &= BitboardUtil::castlingModifiers[from];
  m_st->castlingRights &= BitboardUtil::castlingModifiers[to];

  // Restore occupied
  m_pieceBoards[ALL_PIECES] =
      m_teamBoards[BitboardUtil::WHITE] | m_teamBoards[BitboardUtil::BLACK];
  m_board[from] = NO_PIECE;

  m_whiteToMove = !m_whiteToMove;
  // newHash ^= pos.ply * m_zobristHash.moveHash;

  m_ply++;

  // newHash ^= pos.ply * m_zobristHash.moveHash;

  // pos.st.hashKey = newHash;
}

template <Side s> void Position::undoMove(Move move)
{
  const square_t from = move.getFrom();
  const square_t to = move.getTo();
  const FlagsV2 flags = move.getFlags();
  const bitboard_t fromBB = BB(from);
  const bitboard_t toBB = BB(to);
  const PieceType mover = m_board[to];
  const PieceType captured = m_st->capturedPiece;

  constexpr auto team = static_cast<index_t>(s);

  constexpr const BitboardUtil::Masks *masks = BitboardUtil::bitboardMasks<s>();

  m_teamBoards[team] ^= fromBB ^ toBB;
  m_board[from] = mover;
  m_board[to] = NO_PIECE; // Will be overwritten if we have a capture

  if (mover == PieceType::KING)
  {
    m_kings[team] = from;
  }
  else
  {
    m_pieceBoards[mover] ^= fromBB ^ toBB;
  }

  if (flags == QUIET_ || flags == DOUBLE_JUMP)
  {}
  else if (flags == CASTLE)
  {
    if (toBB & masks->CASTLE_KING_PIECES)
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_KING_ROOK_FROM_TO;
      m_board[masks->CASTLE_KING_ROOK_SOURCE] = ROOK;
      m_board[masks->CASTLE_KING_ROOK_DEST] = NO_PIECE;
    }
    else
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_QUEEN_ROOK_FROM_TO;
      m_board[masks->CASTLE_QUEEN_ROOK_SOURCE] = ROOK;
      m_board[masks->CASTLE_QUEEN_ROOK_DEST] = NO_PIECE;
    }
  }
  else if (flags == EN_PASSANT)
  {
    m_pieceBoards[PAWN] ^= BB(m_st->enPassant);
    m_teamBoards[team ^ 1U] ^= BB(m_st->enPassant);
    m_board[to + masks->DOWN] = PAWN;
  }
  else
  {                                                  // Promotion
    m_pieceBoards[KNIGHT + move.getPromo()] ^= toBB; // Remove promotion piece
  }

  if (m_st->capturedPiece != NO_PIECE)
  {
    m_board[to] = m_st->capturedPiece;
    m_teamBoards[team ^ 1U] ^= toBB;
    m_pieceBoards[captured] ^= toBB;
  }

  m_st = m_st->prevSt; // Reset state
}

bitboard_t Position::attackOn(square_t square, bitboard_t board) const
{
  return (MoveGen::attacks<Side::WHITE, PAWN>(0, square) &
          pieces<Side::BLACK, PAWN>()) |
         (MoveGen::attacks<Side::BLACK, PAWN>(0, square) &
          pieces<Side::WHITE, PAWN>()) |
         (MoveGen::attacks<ROOK>(board, square) & pieces<ROOK, QUEEN>()) |
         (MoveGen::attacks<BISHOP>(board, square) & pieces<BISHOP, QUEEN>()) |
         (MoveGen::attacks<KNIGHT>(0, square) & pieces<KNIGHT>()) |
         (MoveGen::attacks<KING>(0, square) & pieces<KING>());
}

void Position::placePiece(PieceType piece, square_t square, const index_t team)
{
  assert(piece >= PAWN && piece <= KING);
  assert(BitboardUtil::isOnBoard(square));
  assert(team < 2 && team >= 0);
  m_board[square] = piece;
  m_teamBoards[team] |= BB(square);
  if (piece < KING)
  {
    m_pieceBoards[piece] |= BB(square);
  }
  else
  {
    m_kings[piece - KING + team] = square;
  }
  m_pieceBoards[ALL_PIECES] |= BB(square);
}

/// @brief: Initialize the position according to the fen string
/// @note: Assumes a well formatted fen string, will likely crash on
/// illformatted strings
void Position::fenInit(const std::string &fen, StateInfo &st)
{
  std::memset(this, 0, sizeof(Position));
  m_st = &st;
  std::memset(m_st, 0, sizeof(StateInfo));

  std::istringstream stream(fen);
  stream >> std::noskipws;

  char token = '0';
  square_t square = 0;

  while ((stream >> token) && (std::isspace(token) == 0))
  {
    if (std::isdigit(token) != 0)
    {
      square += (token - '0');
    }
    else if (token == '/')
    {}
    else if (auto id = PieceIndexes.find(token); id != std::string::npos)
    {
      placePiece(PieceType((id % 7U)), square, id / 7U);

      square++;
    }
  }
  assert(square == 64);

  // Side to move
  stream >> token;
  m_whiteToMove = token == 'w';
  stream >> token; // Remove space

  // Castling rights
  while ((stream >> token) && (std::isspace(token) == 0))
  {
    if (auto id = CastlingIndexes.find(token); id != std::string::npos)
    {
      m_st->castlingRights |= BB(id);
    }
  }

  char epCol, epRow;
  if ((stream >> epCol) && (epCol >= 'a' && epCol <= 'h') &&
      (stream >> epRow) && (m_whiteToMove ? '6' : '3') == epRow)
  {
    std::cout << epCol << epRow;
    m_st->enPassant = TempGUI::makeSquare(epCol, epRow);
  }
  if (m_st->enPassant == 0)
  {
    m_st->enPassant = SQ_NONE;
  }
}

void Position::printPieces(const std::string &fen) const
{
  char rank = '8';
  std::string stb;
  stb += "\n +---+---+---+---+---+---+---+---+\n";
  for (int i = 0; i < BitboardUtil::BOARD_DIMMENSION; i++)
  {
    for (int j = 0; j < BitboardUtil::BOARD_DIMMENSION; j++)
    {
      stb += " | ";
      const bool black = BB((i * BitboardUtil::BOARD_DIMMENSION + j)) &
                         m_teamBoards[BitboardUtil::BLACK];
      const auto piece =
          m_board[i * BitboardUtil::BOARD_DIMMENSION + j] + black * 7;
      stb += PieceIndexes.at(piece);
    }
    stb += " | ";
    stb += rank;
    rank--;
    std::cout << stb;
    std::cout << "\n +---+---+---+---+---+---+---+---+\n";
    stb = "";
  }
  std::cout << "   a   b   c   d   e   f   g   h\n\n";
  std::cout << "Fen: " << fen << "\n";
  std::cout << "Side to move: " << (m_whiteToMove ? "WHITE\n" : "BLACK\n");
  std::cout << "Castling rights: "
            << TempGUI::getCastleRights((m_st != nullptr) ? m_st->castlingRights
                                                          : 0,
                                        CastlingIndexes)
            << "\n";
  std::cout << "En passant: "
            << TempGUI::makeSquareNotation((m_st != nullptr) ? m_st->enPassant
                                                             : SQ_NONE)
            << std::endl;
  // GUI::getCheckers(checker, pos.st.checkers);
  // std::cout << "\nHash key: " << pos.st.hashKey << "\nChecker: " << checker
  //           << "\n\n";
}