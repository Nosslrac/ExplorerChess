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
  const PieceType captured = m_board[to];
  const FlagsV2 flags = move.getFlags();

  constexpr auto team = static_cast<index_t>(s);
  constexpr Side enemy = BitboardUtil::opposite<s>();

  constexpr const BitboardUtil::Masks *masks = BitboardUtil::bitboardMasks<s>();

  m_teamBoards[team] ^= fromBB ^ toBB;
  m_st->capturedPiece = captured;
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

  if (flags == NO_FLAG)
  {
    if (move.isDoubleJump() && hasPawnsOnEpRank<enemy>())
    {
      m_st->enPassant = static_cast<square_t>(to + masks->DOWN);
    }
  }
  else if (flags == CASTLE)
  {
    if (toBB & masks->CASTLE_KING_PIECES)
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_KING_ROOK_FROM_TO;
      m_teamBoards[team] ^= masks->CASTLE_KING_ROOK_FROM_TO;
      m_board[masks->CASTLE_KING_ROOK_SOURCE] = NO_PIECE;
      m_board[masks->CASTLE_KING_ROOK_DEST] = ROOK;
    }
    else
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_QUEEN_ROOK_FROM_TO;
      m_teamBoards[team] ^= masks->CASTLE_QUEEN_ROOK_FROM_TO;
      m_board[masks->CASTLE_QUEEN_ROOK_SOURCE] = NO_PIECE;
      m_board[masks->CASTLE_QUEEN_ROOK_DEST] = ROOK;
    }
  }
  else if (flags == EN_PASSANT)
  {
    const bitboard_t enemyPawnBB = BitboardUtil::shift<masks->DOWN>(toBB);
    m_pieceBoards[PAWN] ^= enemyPawnBB;
    m_teamBoards[team ^ 1U] ^= enemyPawnBB;
    m_board[to + masks->DOWN] = NO_PIECE;
  }
  else
  { // Promotion
    const auto promoPiece = static_cast<PieceType>(KNIGHT + move.getPromo());
    m_pieceBoards[PAWN] ^= toBB; // Remove pawn
    m_pieceBoards[promoPiece] ^= toBB;
    m_board[to] = promoPiece;
  }

  if (captured != NO_PIECE)
  {
    m_st->capturedPiece = captured;
    m_pieceBoards[captured] ^= toBB;
    m_teamBoards[team ^ 1U] ^= toBB;
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

/// @brief Takes back the move passed as argument.
/// The move is assumed to be the last played move at this point.
/// This function is called with the side that is currently to move,
/// and therefor it is the opposite side move that is retracted.
template <Side s> void Position::undoMove(Move move)
{
  const square_t from = move.getFrom();
  const square_t to = move.getTo();
  const FlagsV2 flags = move.getFlags();
  const bitboard_t fromBB = BB(from);
  const bitboard_t toBB = BB(to);
  const PieceType mover = m_board[to];
  const PieceType captured = m_st->capturedPiece;
  constexpr auto movingTeam = BitboardUtil::opposite<s>();
  constexpr auto teamIndex = static_cast<index_t>(movingTeam);

  constexpr const BitboardUtil::Masks *masks =
      BitboardUtil::bitboardMasks<movingTeam>();

  m_teamBoards[teamIndex] ^= fromBB ^ toBB;
  m_board[from] = mover;
  m_board[to] = NO_PIECE; // Will be overwritten if we have a capture

  if (mover == PieceType::KING)
  {
    m_kings[teamIndex] = from;
  }
  else
  {
    m_pieceBoards[mover] ^= fromBB ^ toBB;
  }

  if (flags == NO_FLAG || move.isDoubleJump())
  {
  }
  else if (flags == CASTLE)
  {
    if (toBB & masks->CASTLE_KING_PIECES)
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_KING_ROOK_FROM_TO;
      m_teamBoards[teamIndex] ^= masks->CASTLE_KING_ROOK_FROM_TO;
      m_board[masks->CASTLE_KING_ROOK_SOURCE] = ROOK;
      m_board[masks->CASTLE_KING_ROOK_DEST] = NO_PIECE;
    }
    else
    {
      m_pieceBoards[ROOK] ^= masks->CASTLE_QUEEN_ROOK_FROM_TO;
      m_teamBoards[teamIndex] ^= masks->CASTLE_QUEEN_ROOK_FROM_TO;
      m_board[masks->CASTLE_QUEEN_ROOK_SOURCE] = ROOK;
      m_board[masks->CASTLE_QUEEN_ROOK_DEST] = NO_PIECE;
    }
  }
  else if (flags == EN_PASSANT)
  {
    const bitboard_t realPawnBB = BB((to + masks->DOWN));
    m_pieceBoards[PAWN] ^= realPawnBB;
    m_teamBoards[teamIndex ^ 1U] ^= realPawnBB;
    m_board[to + masks->DOWN] = PAWN;
  }
  else
  { // Promotion
    // Turn the moved piece back to a pawn
    m_pieceBoards[KNIGHT + move.getPromo()] ^= fromBB;
    m_pieceBoards[PAWN] ^= fromBB;
    m_board[from] = PAWN;
  }

  if (captured != NO_PIECE)
  {
    m_board[to] = captured;
    m_teamBoards[teamIndex ^ 1U] ^= toBB;
    m_pieceBoards[captured] ^= toBB;
  }

  // Restore occupied
  m_pieceBoards[ALL_PIECES] =
      m_teamBoards[BitboardUtil::WHITE] | m_teamBoards[BitboardUtil::BLACK];

  m_whiteToMove = !m_whiteToMove;
  m_ply--;
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

bool Position::isSafeSquares(bitboard_t squaresToCheck, const bitboard_t board,
                             const bitboard_t attackers) const
{
  for (; squaresToCheck != 0; squaresToCheck &= squaresToCheck - 1)
  {
    if ((attackOn(BitboardUtil::bitScan(squaresToCheck), board) & attackers) !=
        0UL)
    {
      return false;
    }
  }
  return true;
}

template <Side s>
bool Position::isSpecialEnPassantKingPin(const bitboard_t epPawn,
                                         const BitboardUtil::Masks *masks) const
{
  constexpr Side enemy = BitboardUtil::opposite<s>();
  const auto kingSq = kingSquare<s>();
  const bitboard_t realPawn =
      BB(static_cast<index_t>(m_st->enPassant + masks->DOWN));
  const bitboard_t snipers = pieces<enemy, ROOK, QUEEN>();
  return (BB(kingSq) & masks->EP_RANK) != 0 &&
         (snipers & masks->EP_RANK) != 0 &&
         (MoveGen::attacks<ROOK>(pieces<ALL_PIECES>() & ~(epPawn | realPawn),
                                 kingSq) &
          snipers) != 0;
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
    {
    }
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

void Position::printState() const
{
  std::cout << "Castling: " << static_cast<int>(m_st->castlingRights) << "\n";
  std::cout << "En passant: " << static_cast<int>(m_st->enPassant) << "\n";
  std::cout << "Captured piece: " << static_cast<int>(m_st->capturedPiece)
            << "\n";
  std::cout << "Block for king: " << static_cast<int>(m_st->blockForKing)
            << "\n";
  std::cout << "Pinned mask: " << static_cast<int>(m_st->pinnedMask) << "\n";
  std::cout << "Checkers: " << static_cast<int>(m_st->checkers) << "\n";
}

template bool Position::isSpecialEnPassantKingPin<Side::WHITE>(
    const bitboard_t epPawns, const BitboardUtil::Masks *masks) const;
template bool Position::isSpecialEnPassantKingPin<Side::BLACK>(
    const bitboard_t epPawns, const BitboardUtil::Masks *masks) const;