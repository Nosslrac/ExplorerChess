#include "GUI.h"
#include "moveGen.h"
#include <format>
#include <iostream>
#include <string_view>

namespace GUI {
namespace {
constexpr std::string_view PieceIndexes(" PNBRQK pnbrqk");
constexpr std::string_view CastlingIndexes("KQkq");
constexpr std::string_view SideToMove("wb");

} // namespace

// Returns the fen for the current position
void getPositionFen(const Position &pos)
{
  int empty = 0;
  std::string fen = "Fen: ";

  for (std::size_t i = 0;
       i < BitboardUtil::BOARD_DIMMENSION * BitboardUtil::BOARD_DIMMENSION; i++)
  {
    if (i % BitboardUtil::BOARD_DIMMENSION == 0 && i != 0)
    {
      if (empty != 0)
      {
        fen += static_cast<char>('0' + empty);
        empty = 0;
      }
      fen += '/';
    }
    if (pos.pieceOn(square_t(i)) == NO_PIECE)
    {
      empty++;
    }
    else
    {
      if (empty)
      {
        fen += static_cast<char>('0' + empty);
        empty = 0;
      }
      fen += PieceIndexes[pos.pieceOn(square_t(i))];
    }
  }
  const char side = SideToMove[pos.isWhiteToMove() ? 0 : 1];
  std::string castle = getCastleRights(pos);
  std::string enPassant = pos.st()->enPassant == SQ_NONE
                              ? makeSquareNotation(pos.st()->enPassant)
                              : "";

  std::cout << fen << castle << side;
}

square_t makeSquare(char col, char row)
{
  return square_t(col - 'a' + (('8' - row) << 3U));
}

std::string getCastleRights(const Position &pos)
{
  std::string castle;
  const auto castleRights = pos.st()->castlingRights;
  for (std::size_t i = 0; i < 4; i++)
  {
    if (castleRights & BB(i))
    {
      castle += CastlingIndexes[i];
    }
  }
  return castle;
}

Move parseMove(std::string moveNotation)
{
  assert(moveNotation.length() <= 5 && moveNotation.length() >= 4);
  const square_t from = makeSquare(moveNotation.at(0), moveNotation.at(1));
  const square_t to = makeSquare(moveNotation.at(2), moveNotation.at(3));
  if (moveNotation.length() == 5)
  {
    constexpr std::string_view promos = "nbrq";
    if (auto id = promos.find(moveNotation.at(4)); id != std::string::npos)
    {
      return Move::make<PROMOTION>(from, to, PieceType(id));
    }
  }
  return Move::make(from, to);
}

std::string makeSquareNotation(square_t square)
{
  if (!BitboardUtil::isOnBoard(square) || square == SQ_NONE)
  {
    return "-";
  }
  return std::string({static_cast<char>('a' + (square & 7U)),
                      static_cast<char>('8' - (square >> 3U))});
}

std::string makeMoveNotation(Move move)
{
  square_t from = move.getFrom();
  square_t to = move.getTo();
  if (!BitboardUtil::isOnBoard(from) || !BitboardUtil::isOnBoard(to))
  {
    return "(invalid move)";
  }
  const int promo = move.isPromo() ? move.getPromo() + 1 : 0;
  return makeSquareNotation(from) + makeSquareNotation(to) + " nbrq"[promo];
}

} // namespace GUI
