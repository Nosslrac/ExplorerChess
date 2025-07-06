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
#include "bitboardUtilV2.h"
#include "moveGenV2.h"
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

Engine::Engine() : m_pos{}, m_historyList(std::make_unique<historyList_t>()){};

namespace {
bool validateMove(Move &pseudoLegalMove, const Position &pos)
{
  pseudoLegalMove =
      MoveGen::MoveList<MoveFilter::ALL>(pos).find(pseudoLegalMove);
  return pseudoLegalMove.getData() != 0;
}

bool moveIntegrity(const Position &pos1, const Position &pos2, Move move,
                   int depth)
{
  const bool result = std::memcmp(&pos1, &pos2, sizeof(Position)) == 0;
  const bool stateEq =
      std::memcmp(pos1.st(), pos2.st(), sizeof(StateInfo)) == 0;
  if (!result || !stateEq)
  {
    for (int square = SQ_A8; square <= SQ_H1; square++)
    {
      if (pos1.pieceOn(square) != pos2.pieceOn(square))
      {
        std::cout << "All pieces: " << static_cast<int>(pos1.pieceOn(square))
                  << " - " << static_cast<int>(pos2.pieceOn(square)) << "\n";
      }
    }
    std::cout << "All pieces: " << pos1.pieces<ALL_PIECES>() << " - "
              << pos2.pieces<ALL_PIECES>() << "\n";
    std::cout << TempGUI::makeMoveNotation(move) << ": " << pos1.isWhiteToMove()
              << " - " << depth << "\n";
    std::cout << "All pieces: " << pos1.pieces<ALL_PIECES>() << " - "
              << pos2.pieces<ALL_PIECES>() << "\n";
    PseudoAttacks::print_bit_board(pos1.pieces_s<Side::WHITE>());
    PseudoAttacks::print_bit_board(pos1.pieces_s<Side::BLACK>());
    pos1.printPieces("");
    pos2.printPieces("");
    std::cout << "Black pieces: " << pos1.pieces_s<Side::BLACK>() << " - "
              << pos2.pieces_s<Side::BLACK>() << "\n";
    std::cout << "Piece type: " << pos1.pieces<PAWN>() << " - "
              << pos2.pieces<PAWN>() << "\n";
    std::cout << "Piece type: " << pos1.pieces<KNIGHT>() << " - "
              << pos2.pieces<KNIGHT>() << "\n";
    std::cout << "Piece type: " << pos1.pieces<BISHOP>() << " - "
              << pos2.pieces<BISHOP>() << "\n";
    std::cout << "Piece type: " << pos1.pieces<ROOK>() << " - "
              << pos2.pieces<ROOK>() << "\n";
    std::cout << "Piece type: " << pos1.pieces<QUEEN>() << " - "
              << pos2.pieces<QUEEN>() << "\n";
  }
  return result;
}

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
  constexpr Side enemy = s == Side::WHITE ? Side::BLACK : Side::WHITE;

  for (const auto &move : moveList)
  {
    Position tmp;
    std::memcpy(&tmp, &pos, sizeof(Position));
    pos.doMove<s>(move, newState);
    auto part = bulkCount<enemy>(pos, depth - 1);
    count += part;
    if (part == BitboardUtil::All_SQ)
    {
      std::cout << TempGUI::makeMoveNotation(move) << ": " << part << "\n";
      return BitboardUtil::All_SQ;
    }
    pos.undoMove<enemy>(move);
    if (!moveIntegrity(tmp, pos, move, depth))
    {
      std::cout << TempGUI::makeMoveNotation(move) << ": Integrity failed\n";
      return BitboardUtil::All_SQ;
    }
  }
  return count;
}
} // namespace

void Engine::makeMove(Move move)
{
  if (validateMove(move, m_pos))
  {
    // Insert new History entry for the move to be made
    m_historyList->push_back(History(move, StateInfo()));
    m_pos.doMove(move, m_historyList->back().state);
    std::cout << "Size of history after make: " << m_historyList->size()
              << "\n";
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
    if (part == BitboardUtil::All_SQ)
    {
      std::cout << TempGUI::makeMoveNotation(move) << ": " << part << "\n";
      exit(0);
    }
    pos.undoMove(move);
    count += part;
    std::cout << TempGUI::makeMoveNotation(move) << ": " << part << "\n";
  }

  std::cout << "Total nodes visited: " << count << "\n";
  return count;
}
