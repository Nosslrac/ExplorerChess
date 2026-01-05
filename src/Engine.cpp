// clang-format off
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//         ____________                                                                                                                ////
//        /           /_____     _____      _________  ____         ___________    _________    _________    _________                ////
//       /     ______/ \    \   /    /     /  ___   / /   /        /          /   /  ___   /   /  ______/   /  ___   /               ////
//      /     /____     \    \_/    /     /  /  /  / /   /        /   ____   /   /  /  /  /   /  /____     /  /  /  /               ////
//     /     _____/      \         /     /  /__/  / /   /        /   /   /  /   /  /__/ _/   /   ___/     /  /__/ _/               ////
//    /     /______      /    _    \    /    ____/ /   /        /   /___/  /   /   _   /    /   /______  /   _   /                ////
//   /            /     /    / \    \  /    /     /   /______  /          /   /   / \   \  /          / /   / \   \              ////
//  /____________/     /____/   \____\/____/     /__________/ /__________/   /___/   \___\/__________/ /___/   \___\            ////
//                                                                                                                             ////
//        ____________                                                                                                        ////
//       /          /   _____   ____  _________  __________   __________                                                     ////
//      /     _____/   /    /  /   / /  ______/ /   ______/  /   ______/                                                    ////
//     /     /        /    /__/   / /  /____    \__  \       \__  \                                                        //// 
//    /     /        /           / /   ___/        \  \         \  \                                                      ////  
//   /     /______  /    ___    / /   /______   ___/   \     ___/   \                                                    ////
//  /            / /    /  /   / /          /  /       /    /       /                                                   ////
// /____________/ /____/  /___/ /__________/   \______/     \______/                                                   //// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clang-format on

#include "Engine.h"
#include "GUI.h"
#include "bitboardUtil.h"
#include "moveGen.h"
#include "position.h"
#include "types.h"

#include <cstring>
#include <iostream>
#include <memory>

/////////////////////////////////
/// Forward declarations ////////
/////////////////////////////////
namespace Perft {
std::uint64_t perft(Position &pos, int depth);
}

Engine::Engine() : m_pos{}, m_historyList(std::make_unique<historyList_t>()) {};

namespace {
bool validateMove(Move &pseudoLegalMove, const Position &pos)
{
  pseudoLegalMove =
      MoveGen::MoveList<MoveFilter::ALL>(pos).find(pseudoLegalMove);
  return pseudoLegalMove.getData() != 0;
}
/*
void validateBitboard(const bitboard_t b1, const bitboard_t b2,
                      const std::string &name)
{
  if (b1 != b2)
  {
    std::cout << "## Bitboard missmatch: " << name << "\n";
    PseudoAttacks::print_bit_board(b1);
    PseudoAttacks::print_bit_board(b2);
  }
}

bool moveIntegrity(const Position &pos1, const Position &pos2, Move move,
                   int depth)
{
  const bool result = std::memcmp(&pos1, &pos2, sizeof(Position)) == 0;
  const bool stateEq =
      std::memcmp(pos1.st(), pos2.st(), sizeof(StateInfo)) == 0;
  if (!result || !stateEq)
  {
    std::cout << TempGUI::makeMoveNotation(move) << ": at depth " << depth
              << " failed integrity\n";
    for (square_t square = SQ_A8; square <= SQ_H1; square++)
    {
      if (pos1.pieceOn(square) != pos2.pieceOn(square))
      {
        std::cout << "Board miss match: "
                  << static_cast<int>(pos1.pieceOn(square)) << " - "
                  << static_cast<int>(pos2.pieceOn(square)) << "\n";
      }
    }
    validateBitboard(pos1.pieces<ALL_PIECES>(), pos2.pieces<ALL_PIECES>(),
                     "All pieces");
    validateBitboard(pos1.pieces_s<Side::WHITE>(), pos2.pieces_s<Side::WHITE>(),
                     "Side white");
    validateBitboard(pos1.pieces_s<Side::BLACK>(), pos2.pieces_s<Side::BLACK>(),
                     "Side black");
    validateBitboard(pos1.pieces<PAWN>(), pos2.pieces<PAWN>(), "Pawn");
    validateBitboard(pos1.pieces<KNIGHT>(), pos2.pieces<KNIGHT>(), "Knight");
    validateBitboard(pos1.pieces<BISHOP>(), pos2.pieces<BISHOP>(), "Bishop");
    validateBitboard(pos1.pieces<ROOK>(), pos2.pieces<ROOK>(), "Rook");
    validateBitboard(pos1.pieces<QUEEN>(), pos2.pieces<QUEEN>(), "Queen");
  }
  return result;
}
*/

template <Side s> std::uint64_t bulkCount(Position &pos, int depth)
{
  const MoveGen::MoveList<MoveFilter::ALL, s> moveList(pos);
  if (depth == 1)
  {
    return moveList.size();
  }
  // StateInfo newSt;
  std::uint64_t count = 0;
  StateInfo newState;
  constexpr Side enemy = BitboardUtil::opposite<s>();

  for (const auto &move : moveList)
  {
    pos.doMove<s>(move, newState);
    count += bulkCount<enemy>(pos, depth - 1);
    pos.undoMove<enemy>(move);
  }
  return count;
}
} // namespace

void Engine::makeMove(Move move)
{
  if (validateMove(move, m_pos))
  {
    std::cout << GUI::makeMoveNotation(move) << " - Flags: " << move.getFlags()
              << "\n";
    // Insert new History entry for the move to be made
    m_historyList->push_back(History(move, StateInfo()));
    m_pos.doMove(move, m_historyList->back().state);
  }
  else
  {
    std::cout << "Invalid move\n";
  }
}

void Engine::undoMove()
{
  // Slot 0 is reserved for the starting state
  if (m_historyList->size() > 1)
  {
    auto lastEntry = m_historyList->back();
    m_historyList->pop_back();
    m_pos.undoMove(lastEntry.move);
  }
  else
  {
    std::cout << "No move to undo\n";
  }
}
std::uint64_t Engine::runPerft(int depth) { return Perft::perft(m_pos, depth); }

void Engine::initFen(const std::string &fen)
{
  m_historyList->emplace_back(History(Move(), StateInfo()));
  m_pos.fenInit(fen, m_historyList->back().state);
}

void Engine::printPieces() const { m_pos.printPieces(""); }

void Engine::printMoves() const
{
  PseudoAttacks::printMovelistMoves(
      MoveGen::MoveList<MoveFilter::ALL>(m_pos).start());
}

std::uint64_t Perft::perft(Position &pos, const int depth)
{
  StateInfo state;
  std::uint64_t count = 0;
  const bool leaf = depth <= 1;

  for (const auto move : MoveGen::MoveList<MoveFilter::ALL>(pos))
  {
    pos.doMove(move, state);
    auto part = leaf                  ? 1
                : pos.isWhiteToMove() ? bulkCount<Side::WHITE>(pos, depth - 1)
                                      : bulkCount<Side::BLACK>(pos, depth - 1);
    pos.undoMove(move);
    count += part;
    std::cout << GUI::makeMoveNotation(move) << ": " << part << "\n";
  }

  std::cout << "Total nodes visited: " << count << "\n";
  return count;
}
