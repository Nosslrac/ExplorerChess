#include "position.h"
#include "bitboardUtilV2.h"
#include "moveGenV2.h"
#include "types.h"

#include <array>
#include <cctype>
#include <cstdint>
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
constexpr std::string_view PieceIndexes(" PNBRQpnbrqKk");
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

  std::memcpy(
      &newSt, m_st,
      __builtin_offsetof(StateInfo,
                         blockForKing)); // TODO: test if this is slower than
                                         // manually writing the values??
  newSt.prevSt = m_st;
  m_st = &newSt;

  // bitboard_t newHash = pos.st.hashKey ^ m_zobristHash.whiteToMoveHash;

  // Get move info
  const square_t from = move.getFrom();
  const square_t to = move.getTo();
  const bitboard_t fromBB = BB(from);
  const bitboard_t toBB = BB(to);

  const PieceV2 mover = m_board[from];
  const uint32_t flags = move.getFlags();

  constexpr index_t team = s == Side::WHITE ? 0 : 1;
  constexpr uint8_t enemy = s == Side::WHITE ? 1 : 0;

  // const bitboard_t teamBoard = m_teamBoards[team];

  m_teamBoards[team] ^= fromBB ^ toBB;
  // Increment position value based on prev move
  // pos.st.materialScore +=
  //     scoreFavor * m_evaluation.pieceScoreChange(from, to, mover);

  // Remove old ep hash if there was one
  if (m_st->enPassant != 0)
  {
    // newHash ^= m_zobristHash.epHash[pos.st.enPassant & 7]; // File ep hash
    m_st->enPassant = 0;
  }

  // TODO: fix this to fetch from 8x8 board
  const PieceType movingType = PieceType::PAWN;

  if (movingType == PieceType::KING)
  {
    m_kings[static_cast<std::uint8_t>(s)] = to;
    if (flags == CASTLE_KING)
    {
      doCastle<s>(CastleSide::KING);
      // TODO: update zobrist hash
    }
    else if (flags == CASTLE_QUEEN)
    {
      doCastle<s>(CastleSide::QUEEN);
      // TODO: update zobrist hash
    }
  }
  else
  {
    m_pieceBoards[movingType] ^= fromBB ^ toBB;
  }

  if (flags == CAPTURE)
  {
    // m_board will be overwritten later when moving the piece
    constexpr index_t capturedOffset =
        s == Side::WHITE ? BitboardUtil::BLACK_OFFSET : 0;
    const PieceV2 captured = m_board[to];
    m_pieceBoards[captured - capturedOffset] ^= toBB;
    m_teamBoards[enemy] ^= toBB;
  }

  if (movingType == PieceType::PAWN)
  {
    constexpr char add = s == Side::WHITE ? 8 : -8;
    if (flags == QUIET)
    {
      // DO nothing
    }
    else if (flags == EP_CAPTURE)
    {
      m_pieceBoards[PAWN] ^= BB((to + add));
      m_teamBoards[enemy] ^= BB((to + add));
      // newHash ^=
      //     m_zobristHash.pieceHash[enemyPawn][to + add]; // Remove pawn from
      //     hash
      // pos.st.materialValue +=
      //     scoreFavor * m_evaluation.getPieceValue(captured + blackOffset);
    }
    else if (flags == DOUBLE_PUSH && hasPawnsOnEpRank<s>())
    {
      m_st->enPassant = static_cast<square_t>(to + add);
      // newHash ^= m_zobristHash.epHash[(to + add) & 7]; // Add new ep hash
    }
    else // Promotion
    {
      m_pieceBoards[PAWN] ^= toBB;
      m_pieceBoards[1 + move.getPromo()] |= toBB;
      // TODO: fix zobrist hash and material

      if (flags & CAPTURE)
      {
        // m_board will be overwritten later when moving the piece
        constexpr index_t capturedOffset =
            s == Side::WHITE ? BitboardUtil::BLACK_OFFSET : 0;
        const PieceV2 captured = m_board[to];
        m_pieceBoards[captured - capturedOffset] ^= toBB;
        m_teamBoards[enemy] ^= toBB;
        // Fix zobrist and material value
      }
    }
  }

  m_st->castlingRights &= BitboardUtil::castlingModifiers[from];
  m_st->castlingRights &= BitboardUtil::castlingModifiers[to];

  // Restore occupied
  m_pieceBoards[ALL_PIECES] =
      m_teamBoards[BitboardUtil::WHITE] | m_teamBoards[BitboardUtil::BLACK];
  m_board[from] = NO_PIECE;
  m_board[to] = mover;

  m_whiteToMove = !m_whiteToMove;
  // newHash ^= pos.ply * m_zobristHash.moveHash;

  m_ply++;

  // // newHash ^= pos.ply * m_zobristHash.moveHash;

  // // pos.st.hashKey = newHash;

  m_st->enemyAttack =
      0; // Find attacks: m_moveGen.findAttack<!whiteToMove>(pos);
  // TODO: update attacks and pins the correct way with new movegen
  // m_moveGen.pinnedBoard<!whiteToMove>(pos);
  // m_moveGen.checks<!whiteToMove>(pos);
  // m_moveGen.setCheckSquares<!whiteToMove>(pos);
}

template <Side s> void Position::undoMove(Move move)
{
  const square_t from = move.getFrom();
  const square_t to = move.getTo();
  const bitboard_t fromToBB = BB(from) | BB(to);

  constexpr uint8_t team = s == Side::WHITE ? 0 : 1;

  m_teamBoards[team] ^= fromToBB;
}

bitboard_t Position::attackOn(square_t square, bitboard_t board) const
{
  PseudoAttacks::print_bit_board(pieces<KING>());
  return (MoveGen::attacks<Side::WHITE, PAWN>(0, square) &
          pieces<Side::BLACK, PAWN>()) |
         (MoveGen::attacks<Side::BLACK, PAWN>(0, square) &
          pieces<Side::WHITE, PAWN>()) |
         (MoveGen::attacks<ROOK>(board, square) & pieces<ROOK, QUEEN>()) |
         (MoveGen::attacks<BISHOP>(board, square) & pieces<BISHOP, QUEEN>()) |
         (MoveGen::attacks<KNIGHT>(0, square) & pieces<KNIGHT>()) |
         (MoveGen::attacks<KING>(0, square) & pieces<KING>());
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
  std::size_t id = 0;
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
    else if ((id = PieceIndexes.find(token)) != std::string::npos)
    {
      placePiece(PieceV2(id), square);

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
    if ((id = CastlingIndexes.find(token)) != std::string::npos)
    {
      m_st->castlingRights |= BB(id);
    }
  }

  char epCol, epRow;
  if ((stream >> epCol) && (epCol >= 'a' && epCol <= 'h') &&
      (stream >> epRow) && (m_whiteToMove ? '6' : '3') == epRow)
  {
    std::cout << epCol << epRow;
    m_st->enPassant = TempGUI::makeSquare(std::array<char, 2>{epCol, epRow});
  }
  if (m_st->enPassant == 0)
  {
    m_st->enPassant = SQ_NONE;
  }
  printPieces(fen);
}

void Position::printPieces(const std::string &fen) const
{
  char rank = '8';
  std::string stb;
  stb += "\n +---+---+---+---+---+---+---+---+\n";
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      stb += " | ";
      stb += PieceIndexes.at(m_board[i * 8 + j]);
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
            << TempGUI::getCastleRights(m_st->castlingRights, CastlingIndexes)
            << "\n";
  std::cout << "En passant: " << TempGUI::makeSquareNotation(m_st->enPassant)
            << std::endl;
  // GUI::getCheckers(checker, pos.st.checkers);
  // std::cout << "\nHash key: " << pos.st.hashKey << "\nChecker: " << checker
  //           << "\n\n";
}