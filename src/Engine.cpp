// clang-format off
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//         ____________                                                                                                       ////
//        /           /_____     _____      _________  ____         ___________    _________    _________    _________       ////
//       /     ______/ \    \   /    /     /  ___   / /   /        /          /   /  ___   /   /  ______/   /  ___   /      // //
//      /     /____     \    \_/    /     /  /  /  / /   /        /   ____   /   /  /  /  /   /  /____     /  /  /  /      //  //
//     /     _____/      \         /     /  /__/  / /   /        /   /   /  /   /  /__/ _/   /   ___/     /  /__/ _/      //   //
//    /     /______      /    _    \    /    ____/ /   /        /   /___/  /   /   _   /    /   /______  /   _   /       //    //
//   /            /     /    / \    \  /    /     /   /______  /          /   /   / \   \  /          / /   / \   \     //     //
//  /____________/     /____/   \____\/____/     /__________/ /__________/   /___/   \___\/__________/ /___/   \___\   //      //
//                                                                                                                    //       //
//        ____________                                                                                               //        //
//       /          /   _____   ____  _________  __________   __________                                            //         //
//      /     _____/   /    /  /   / /  ______/ /   ______/  /   ______/                                           //          //
//     /     /        /    /__/   / /  /____    \__  \       \__  \                                               //           //
//    /     /        /           / /   ___/        \  \         \  \                                             //            //
//   /     /______  /    ___    / /   /______   ___/   \     ___/   \                                           //             //
//  /            / /    /  /   / /          /  /       /    /       /                                          //              //
// /____________/ /____/  /___/ /__________/   \______/     \______/                                          //               //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clang-format 

#include "Engine.h"
#include "moveOrdering.h"
#include "GUI.h"

// Libs
#include <iostream>
#include <cmath>
#include <cstdint>


Engine::Engine()
    : m_pos{}, m_evaluation(Evaluation(m_moveGen)), m_nodes(0),
      m_captureOnlyNodes(0) {}

Engine::~Engine() = default;

void Engine::run() {
  std::cout << "ExplorerChess 1.0. Use help for a list of commands"
            << std::endl;
  while (true) {
    runUI();
  }
}

void Engine::runUI() {
  std::string userInput;
  std::getline(std::cin, userInput);

  size_t commandEnd1 = userInput.find(' ');
  size_t commandEnd2 = userInput.find(' ', commandEnd1 + 1);
  commandEnd2 = commandEnd2 > userInput.size() ? userInput.size() : commandEnd2;

  std::string command = userInput.substr(0, commandEnd1);
  std::string arg =
      userInput.substr(commandEnd1 + 1, commandEnd2 - commandEnd1 - 1);

  if (strcmp(command.c_str(), "testpos") == 0) {
#ifdef PEXT
    std::cout << "PEXT\n";
#else
    std::cout << "MAGIC\n";
#endif
    fenInit(m_pos, "8/8/p1p5/1p5p/1P5p/8/PPP2K1p/4R1rk w - - 0 1");
    // fenInit(m_pos, "r1b1kb1r/pppp1ppp/5q2/4n3/3KP3/2N3PN/PPP4P/R1BQ1B1R b kq
    // - 0 1");
  }

  else if (strcmp(command.c_str(), "position") == 0) {

    if (strcmp(arg.c_str(), "startpos") == 0) {
      fenInit(m_pos,
              "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else if (strcmp(arg.c_str(), "fen") == 0) {
      std::string arg2 = userInput.substr(commandEnd2 + 1);
      fenInit(m_pos, arg2);
    } else {
      fenInit(m_pos, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/"
                     "R4RK1 w - - 0 10");
    }

  } else if (strcmp(command.c_str(), "analyse") == 0) {

    Move move{};
    const int depth = std::stoi(arg);
    if (m_pos.whiteToMove) {
      move = analysis<true>(m_pos, depth);
    } else {
      move = analysis<false>(m_pos, depth);
    }
    GUI::printMove(move.move);

    std::cout << "\nNodes searched: " << m_nodes
              << "\nQnodes searched: " << m_captureOnlyNodes
              << "\nEval: " << move.eval << std::endl;
  } else if (strcmp(command.c_str(), "hashTest") == 0) {
    MoveList ml;
    if (m_pos.whiteToMove)
      m_moveGen.generateAllMoves<true, false>(m_pos, ml);
    else
      m_moveGen.generateAllMoves<false, false>(m_pos, ml);

    for (int i = 0; i < ml.size(); ++i) {
      if (m_pos.whiteToMove) {
        prevMoves.push(ml.moves[i]);
        doMove<true, true>(m_pos, ml.moves[i]);
        const uint64_t hash = m_zobristHash.hashPosition(m_pos);
        if (m_pos.st.hashKey != hash) {
          std::cout << "Hash fail on move: ";
          GUI::parseMove(ml.moves[i]);
        }
        undoMove<false, true>(m_pos, ml.moves[i]);
      } else {
        prevMoves.push(ml.moves[i]);
        doMove<false, true>(m_pos, ml.moves[i]);
        const uint64_t hash = m_zobristHash.hashPosition(m_pos);
        if (m_pos.st.hashKey != hash) {
          std::cout << "Hash fail on move: ";
          GUI::parseMove(ml.moves[i]);
        }
        undoMove<true, true>(m_pos, ml.moves[i]);
      }
    }
  } else if (strcmp(command.c_str(), "d") == 0) {
    GUI::print_pieces(m_pos);
  } else if (strcmp(command.c_str(), "g") == 0) {
    moveIntegrity<true, true>(m_pos);
  } else if (strcmp(command.c_str(), "bishop") == 0) {
    uint64_t bs = m_pos.pieceBoards[2];
    while (bs) {
      int x = bitScan(bs);
      bs &= bs - 1;
      uint64_t attack = m_moveGen.attackBB<Bishop>(m_pos.teamBoards[0], x);
      GUI::print_bit_board(attack);
      GUI::print_pieces(m_pos);
    }
  } else if (strcmp(command.c_str(), "state") == 0) {
    const int score = m_pos.whiteToMove ? m_evaluation.evaluate<true>(m_pos)
                                        : m_evaluation.evaluate<false>(m_pos);
    std::cout << "Eval: " << score << " Score: " << m_pos.st.materialScore
              << " Value: " << m_pos.st.materialValue << "\n";
  } else if (strcmp(command.c_str(), "make") == 0) {
    MoveList ml;
    if (m_pos.whiteToMove) {
      m_moveGen.generateAllMoves<true, false>(m_pos, ml);
      const uint32_t move = GUI::findMove(ml, arg);
      if (move != 0) {
        prevMoves.push(move);
        doMove<true, true>(m_pos, move);
      }
    } else {
      m_moveGen.generateAllMoves<false, false>(m_pos, ml);
      const uint32_t move = GUI::findMove(ml, arg);
      if (move != 0) {
        prevMoves.push(move);
        doMove<false, true>(m_pos, move);
      }
    }

  } else if (strcmp(command.c_str(), "unmake") == 0) {
    if (!prevMoves.empty()) {
      uint32_t move = prevMoves.top();
      prevMoves.pop();
      if (m_pos.whiteToMove) {
        undoMove<true, true>(m_pos, move);
      } else {
        undoMove<false, true>(m_pos, move);
      }
    }
  } else if (strcmp(command.c_str(), "perft") == 0) {
    const uint32_t depth = std::stoi(arg);
    auto start = std::chrono::system_clock::now();
    if (m_pos.whiteToMove) {
      perft<true>(m_pos, depth);
    } else {
      perft<false>(m_pos, depth);
    }
    auto end = std::chrono::system_clock::now();
    auto duration =
        duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Execution time: " << duration << " ms\n";
  } else if (strcmp(command.c_str(), "help") == 0) {
    std::cout
        << "Available commands:\n"
        << "position: Set position. Usage: position startpos or position fen "
           "<fenstring>.\n"
        << "perft: Count number of possible positions at certain given depth "
           "for a position.\n"
        << "make: Makes specified move. Does nothing if move is illegal.\n"
        << "unmake: Unmakes last made move.\n"
        << "d: display current position.\n"
        << "analyse: Analyse position at certain given depth\n";
  } else {

    std::cout << "Command not found: Try help for a list of commands\n";
  }
}

//---------------------------------------------------------------------
//---------------------- Perft ----------------------------------------
//---------------------------------------------------------------------

template <bool whiteToMove>
uint64_t Engine::perft(Position &pos, uint32_t depth) {
  MoveList move_list;
  const bool castle =
      m_moveGen.generateAllMoves<whiteToMove, false>(pos, move_list);
  m_nodes = 0;

#ifndef SHALLOW_SEARCH
  if (depth < 2) {
    for (int i = 0; i < move_list.size(); ++i) {
      GUI::printMove(move_list.moves[i]);
      std::cout << ": " << 1 << std::endl;
    }
    std::cout << "Total positions: " << (int)move_list.size() << std::endl;
    return move_list.size();
  }
#endif
  uint64_t numPositions = 0;

  if (castle) {
    for (int i = 0; i < move_list.size(); ++i) {

      doMove<whiteToMove, true>(pos, move_list.moves[i]);
      const uint64_t part = search<!whiteToMove, true>(pos, depth - 1);
      numPositions += part;
#ifdef PRINT_OUT
      GUI::printMove(move_list.moves[i]);
      std::cout << ": " << part << std::endl;
#endif
      undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
    }
  } else {
    for (int i = 0; i < move_list.size(); ++i) {

      doMove<whiteToMove, false>(pos, move_list.moves[i]);
      const uint64_t part = search<!whiteToMove, false>(pos, depth - 1);
      numPositions += part;
#ifdef PRINT_OUT
      GUI::printMove(move_list.moves[i]);
      std::cout << ": " << part << std::endl;
#endif
      undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
    }
  }

#ifdef PRINT_OUT
  std::cout << "Total positions: " << numPositions
            << "\nNodes searched: " << m_nodes << std::endl;
#endif
  //_transpositionTable.clear();
  return numPositions;
}

template <bool whiteToMove, bool castle>
uint64_t Engine::search(Position &pos, int depth) {
  m_nodes++;
  // if (_transpositionTable.contains(pos.st.hashKey)) {
  //   return _transpositionTable.at(pos.st.hashKey);
  //}

#ifdef SHALLOW_SEARCH
  MoveList move_list;
  const bool castleAllowed =
      m_moveGen.generateAllMoves<whiteToMove, false>(pos, move_list);
  if (depth > 1) {
    uint64_t numPositions = 0;
    if constexpr (castle) {
      if (castleAllowed) {
        for (int i = 0; i < move_list.size(); ++i) {
          doMove<whiteToMove, true>(pos, move_list.moves[i]);
          numPositions += search<!whiteToMove, true>(pos, depth - 1);
          undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
        }
        return numPositions;
      }
    }
    for (int i = 0; i < move_list.size(); ++i) {
      doMove<whiteToMove, false>(pos, move_list.moves[i]);
      numPositions += search<!whiteToMove, false>(pos, depth - 1);
      undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
    }
    //_transpositionTable[pos.st.hashKey] = numPositions;
    return numPositions;
  }
  return move_list.size();
#else
  if (depth == 0)
    return 1;

  MoveList move_list;
  const bool castleAllowed =
      m_moveGen.generateAllMoves<whiteToMove, false>(pos, move_list);
  uint64_t numPositions = 0;
  for (int i = 0; i < move_list.size(); ++i) {
    doMove<whiteToMove, true>(pos, move_list.moves[i]);
    numPositions += search<!whiteToMove, true>(pos, depth - 1);
    undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
  }
  return numPositions;
#endif
}

//---------------------------------------------------------------------
//---------------------- Evaluation search ----------------------------
//---------------------------------------------------------------------

template <bool whiteToMove> Move Engine::analysis(Position &pos, int depth) {
  _evalTransposition.clear();

  MoveList move_list;
  const bool castle =
      m_moveGen.generateAllMoves<whiteToMove, false>(pos, move_list);
  MoveOrder::moveSort(move_list, pos, depth);
  m_nodes = 0;
  m_captureOnlyNodes = 0;
  Move bestMove = {
      CHECK_MATE,
      move_list.moves[0]}; // Initiate first move with worst possible eval

  if (castle) {
    for (int i = 0; i < move_list.size(); ++i) {
      doMove<whiteToMove, true>(pos, move_list.moves[i]);
      const int eval = -negaMax<!whiteToMove, true>(
          pos, CHECK_MATE - depth, -CHECK_MATE + depth, depth - 1);
      GUI::printMove(move_list.moves[i]);
      std::cout << " => Eval: " << eval << std::endl;
      if (eval > bestMove.eval) {
        bestMove = {eval, move_list.moves[i]};
      }
      undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
    }
  } else {
    for (int i = 0; i < move_list.size(); ++i) {
      doMove<whiteToMove, false>(pos, move_list.moves[i]);
      const int eval = -negaMax<!whiteToMove, false>(
          pos, CHECK_MATE - depth, -CHECK_MATE + depth, depth - 1);
      GUI::printMove(move_list.moves[i]);
      std::cout << " => Eval: " << eval << std::endl;
      if (eval > bestMove.eval) {
        bestMove = {eval, move_list.moves[i]};
      }
      undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
    }
  }
  return bestMove;
}

template <bool whiteToMove, bool castle>
int Engine::negaMax(Position &pos, int alpha, int beta, int depth) {
  // Negamax with alpha beta pruning
  m_nodes++;
  if (depth == 0) {
    return qSearch<whiteToMove>(pos, alpha, beta);
    // return m_evaluation.evaluate<whiteToMove>(pos);
  }

  if (_evalTransposition.contains(pos.st.hashKey)) {
    return _evalTransposition.at(pos.st.hashKey);
  }

  MoveList move_list;

  const bool castleAllowed =
      m_moveGen.generateAllMoves<whiteToMove, false>(pos, move_list);

  if (move_list.size() == 0) {
    if (pos.st.checkers) {
      return CHECK_MATE - depth; // depth makes sure we get the fastest
                                 // checkmate as the best one
    }
    return 0; // Stale mate
  }
  // Do some move ordering (maybe do move ordering already in the generation
  MoveOrder::moveSort(move_list, pos, depth);

  if constexpr (castle) {
    if (castleAllowed) {
      for (int i = 0; i < move_list.size(); ++i) {
        doMove<whiteToMove, true>(pos, move_list.moves[i]);
        const int eval =
            -negaMax<!whiteToMove, true>(pos, -beta, -alpha, depth - 1);
        undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
        if (eval >= beta) {
          return beta;
        }
        alpha = std::max(eval, alpha);
      }
      return alpha;
    }
  }
  for (int i = 0; i < move_list.size(); ++i) {
    doMove<whiteToMove, false>(pos, move_list.moves[i]);
    const int eval =
        -negaMax<!whiteToMove, false>(pos, -beta, -alpha, depth - 1);
    undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
    if (eval >= beta) {
      return beta;
    }
    alpha = std::max(alpha, eval);
  }
  _evalTransposition[pos.st.hashKey] = alpha;
  return alpha;
}

template <bool whiteToMove>
int Engine::qSearch(Position &pos, int alpha, int beta) {
  m_captureOnlyNodes++;
  int stand_pat = m_evaluation.evaluate<whiteToMove>(pos);
  if (stand_pat >= beta)
    return beta;
  if (alpha < stand_pat)
    alpha = stand_pat;

  MoveList moveList;
  m_moveGen.generateAllMoves<whiteToMove, true>(pos, moveList);
  if (moveList.size() == 0) {
    return stand_pat;
  }

  for (int i = 0; i < moveList.size(); ++i) {
    doMove<whiteToMove, false>(pos, moveList.moves[i]);
    const int eval = -qSearch<!whiteToMove>(pos, -beta, -alpha);
    undoMove<!whiteToMove, false>(pos, moveList.moves[i]);
    if (eval >= beta) {
      return beta;
    }
    alpha = std::max(alpha, eval);
  }
  return alpha;
}

template <bool whiteToMove, bool castle>
void Engine::moveIntegrity(Position &pos) {
  MoveList m;
  m_moveGen.generateAllMoves<whiteToMove, false>(pos, m);
  for (int i = 0; i < m.size(); ++i) {
    const int materialValue = pos.st.materialValue;
    const int materialScore = pos.st.materialScore;
    doMove<whiteToMove, castle>(pos, m.moves[i]);
    undoMove<!whiteToMove, castle>(pos, m.moves[i]);

    if (pos.st.materialScore != materialScore ||
        pos.st.materialValue != materialValue) {
      GUI::parseMove(m.moves[i]);
      std::cout << "Material: " << materialValue
                << ", After: " << pos.st.materialValue << std::endl;
    }
  }
}

/*Hash check list
 * Flip move hash
 * xor out the moving piece from hash (hashKey ^= hashFrom ^ hashTo;
 * Capture xor out hash for the captured piece
 * Ep capture xor out pawn and the corresponding file enPassanthash
 * Castling, move the rook hash, update castling rights hash
 * Movecount hash (not needed in evaluation, only in perft)
 */

template <bool whiteToMove, bool castle>
void Engine::doMove(Position &pos, uint32_t move) {
  // Save state info
  prevStates.push(pos.st);
  uint64_t newHash = pos.st.hashKey ^ m_zobristHash.whiteToMoveHash;

  // Get move info
  const uint8_t from = getFrom(move);
  const uint8_t to = getTo(move);
  const uint8_t mover = getMover(move);
  // Captured will be 0 if there is no capture so important to do capture before
  // move
  const uint8_t captured = getCaptured(move);
  const uint32_t flags = getFlags(move);
  const uint64_t fromBB = BB(from);
  const uint64_t toBB = BB(to);

  constexpr uint8_t team = whiteToMove ? 1 : 2;
  constexpr uint8_t enemy = whiteToMove ? 2 : 1;
  constexpr char add = whiteToMove ? 8 : -8;
  constexpr uint8_t enemyPawn = whiteToMove ? 5 : 0;
  constexpr uint8_t teamPawn = whiteToMove ? 0 : 5;
  constexpr int8_t scoreFavor = whiteToMove ? 1 : -1;
  constexpr int8_t blackOffset = whiteToMove ? -5 : 0;

  pos.teamBoards[team] ^= fromBB ^ toBB;
  // Increment position value based on prev move
  pos.st.materialScore +=
      scoreFavor * m_evaluation.pieceScoreChange(from, to, mover);

  // Remove old ep hash if there was one
  if (pos.st.enPassant != 0) {
    newHash ^= m_zobristHash.epHash[pos.st.enPassant & 7]; // File ep hash
    pos.st.enPassant = 0;
  }

  if (mover > King - 1) {
    if constexpr (whiteToMove) {
      pos.kings[0] = to;
      newHash ^=
          m_zobristHash.kingHash[0][from] ^ m_zobristHash.kingHash[0][to];
    } else {
      pos.kings[1] = to;
      newHash ^=
          m_zobristHash.kingHash[1][from] ^ m_zobristHash.kingHash[1][to];
    }

    if (flags & CAPTURE) {
      pos.pieceBoards[captured] ^= toBB;
      pos.teamBoards[enemy] ^= toBB;
      newHash ^= m_zobristHash.pieceHash[captured][to];
      pos.st.materialValue +=
          scoreFavor * m_evaluation.getPieceValue(captured + blackOffset);
    }

    if constexpr (castle) {
      constexpr int8_t rook = whiteToMove ? 3 : 8;
      if (flags == CASTLE_KING) {
        doCastle<whiteToMove, true>(pos);
        constexpr uint8_t rookFrom = whiteToMove ? 63 : 7;
        constexpr uint8_t rookTo = whiteToMove ? 61 : 5;

        newHash ^= m_zobristHash.pieceHash[3][rookFrom] ^
                   m_zobristHash.pieceHash[3][rookTo];
        pos.st.materialScore +=
            scoreFavor * m_evaluation.pieceScoreChange(rookFrom, rookTo, rook);
      } else if (flags == Flags::CASTLE_QUEEN) {
        doCastle<whiteToMove, false>(pos);
        constexpr uint8_t rookFrom = whiteToMove ? 56 : 0;
        constexpr uint8_t rookTo = whiteToMove ? 59 : 3;
        newHash ^= m_zobristHash.pieceHash[3][rookFrom] ^
                   m_zobristHash.pieceHash[3][rookTo];
        pos.st.materialScore +=
            scoreFavor * m_evaluation.pieceScoreChange(rookFrom, rookTo, rook);
      }
    }
  } else {
    pos.pieceBoards[mover] ^= fromBB ^ toBB;
    // Mover hash
    newHash ^= m_zobristHash.pieceHash[mover][from] ^
               m_zobristHash.pieceHash[mover][to];
    switch (flags) {
    case QUIET:
      break;
    case CAPTURE:
      pos.pieceBoards[captured] ^= toBB;
      pos.teamBoards[enemy] ^= toBB;
      newHash ^= m_zobristHash.pieceHash[captured][to];
      pos.st.materialValue +=
          scoreFavor * m_evaluation.getPieceValue(captured + blackOffset);
      break;
    case DOUBLE_PUSH:
      pos.st.enPassant = to + add;
      newHash ^= m_zobristHash.epHash[(to + add) & 7]; // Add new ep hash
      break;
    case EP_CAPTURE:
      pos.pieceBoards[enemyPawn] ^= BB((to + add));
      pos.teamBoards[enemy] ^= BB((to + add));
      newHash ^=
          m_zobristHash.pieceHash[enemyPawn][to + add]; // Remove pawn from hash
      pos.st.materialValue +=
          scoreFavor * m_evaluation.getPieceValue(captured + blackOffset);
      break;
    default: // Promotion
      pos.pieceBoards[teamPawn] ^= toBB;
      newHash ^= m_zobristHash.pieceHash[teamPawn][to];
      const uint8_t pID = 1 + 5 * !whiteToMove + getPromo(move);
      pos.pieceBoards[pID] |= toBB;
      newHash ^= m_zobristHash.pieceHash[pID][to];
      // Remove pawn score and add new piece value
      pos.st.materialValue +=
          scoreFavor *
          (m_evaluation.getPieceValue(pID + (blackOffset ^ -5)) - 100);
      // Pawn score will already have been removed with pieceScoreChange
      pos.st.materialScore += scoreFavor * m_evaluation.pieceScore(to, pID);
      if (flags & CAPTURE) {
        pos.pieceBoards[captured] ^= toBB;
        pos.teamBoards[enemy] ^= toBB;
        newHash ^= m_zobristHash.pieceHash[captured][to];
        pos.st.materialValue +=
            scoreFavor * m_evaluation.getPieceValue(captured + blackOffset);
      }
      break;
    }
  }

  if constexpr (castle) {

    // Solve with pext and then switch case on the number maybe
    newHash ^= m_zobristHash.castleHash[pos.st.castlingRights];
    // Handles if rook is captured or moves
    pos.st.castlingRights &= castlingModifiers[from];
    pos.st.castlingRights &= castlingModifiers[to];

    newHash ^= m_zobristHash.castleHash[pos.st.castlingRights];
  }

  // Restore occupied
  pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

  pos.whiteToMove = !whiteToMove;
  newHash ^= pos.ply * m_zobristHash.moveHash;

  pos.ply++;

  newHash ^= pos.ply * m_zobristHash.moveHash;

  pos.st.hashKey = newHash;
  // Prepare for next movegeneration
  pos.st.enemyAttack = m_moveGen.findAttack<!whiteToMove>(pos);
  m_moveGen.pinnedBoard<!whiteToMove>(pos);
  m_moveGen.checks<!whiteToMove>(pos);
  m_moveGen.setCheckSquares<!whiteToMove>(pos);
}

template <bool whiteToMove, bool castle>
void Engine::undoMove(Position &pos, uint32_t move) {
  pos.st = prevStates.top();
  prevStates.pop();

  // Get move info
  const uint8_t from = getFrom(move);
  const uint8_t to = getTo(move);
  const uint8_t mover = getMover(move);
  // Captured will be 0 if there is no capture so important to do capture before
  // move
  const uint8_t captured = getCaptured(move);
  const uint32_t flags = getFlags(move);

  const uint64_t fromBB = BB(from);
  const uint64_t toBB = BB(to);
  const bool isCapture =
      (((flags & CAPTURE) == CAPTURE) && flags != EP_CAPTURE);
  const uint64_t capMask = toBB * isCapture;

  // Incrementally update position
  if constexpr (whiteToMove) {
    pos.teamBoards[2] ^= fromBB ^ toBB;
    pos.teamBoards[1] ^= capMask;
  } else {
    pos.teamBoards[1] ^= fromBB ^ toBB;
    pos.teamBoards[2] ^= capMask;
  }

  pos.pieceBoards[captured] ^= capMask;

  if (mover > King - 1) {
    pos.kings[whiteToMove] = from;

    if constexpr (castle) {
      if (flags == CASTLE_KING) {
        doCastle<!whiteToMove, true>(pos);
      } else if (flags == CASTLE_QUEEN) {
        doCastle<!whiteToMove, false>(pos);
      }
    }

  } else {
    pos.pieceBoards[mover] ^= fromBB ^ toBB;

    // EP: add the ep pawn if there was an ep capture
    if constexpr (whiteToMove) {
      const uint64_t ep_cap = (flags == EP_CAPTURE) * BB((to - 8));
      pos.pieceBoards[0] ^= ep_cap;
      pos.teamBoards[1] ^= ep_cap;
    } else {
      const uint64_t ep_cap = (flags == EP_CAPTURE) * BB((to + 8));
      pos.pieceBoards[5] ^= ep_cap;
      pos.teamBoards[2] ^= ep_cap;
    }

    // Pawn promotion
    if ((flags & PROMO_N) != 0) {
      if constexpr (whiteToMove) {
        pos.pieceBoards[5] ^= toBB;
        const int8_t pID = 6 + getPromo(move);
        pos.pieceBoards[pID] ^= toBB;
      } else {
        pos.pieceBoards[0] ^= toBB;
        const int8_t pID = 1 + getPromo(move);
        pos.pieceBoards[pID] ^= toBB;
      }
    }
  }

  pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

  pos.whiteToMove = !whiteToMove;
  pos.ply--;
}

template <bool whiteToMove, bool castleKing>
inline void Engine::doCastle(Position &pos) {
  if constexpr (whiteToMove) {
    constexpr uint64_t rookFromTo =
        castleKing ? BB(63) | BB(61) : BB(56) | BB(59);
    pos.pieceBoards[3] ^= rookFromTo;
    pos.teamBoards[1] ^= rookFromTo;
  } else {
    constexpr uint64_t rookFromTo = castleKing ? BB(7) | BB(5) : BB(0) | BB(3);
    pos.pieceBoards[8] ^= rookFromTo;
    pos.teamBoards[2] ^= rookFromTo;
  }
}

void Engine::fenInit(Position &pos, std::string fen) {
  uint32_t sq{0};

  char eP[2] = {};
  int epID = 0;

  pos = {};
  for (char f : fen) {
    if (sq < 64) {
      switch (f) {
      case 'K':
        pos.kings[0] = sq;
        break;
      case 'P':
        pos.pieceBoards[0] |= 1ULL << sq;
        break;
      case 'N':
        pos.pieceBoards[1] |= 1ULL << sq;
        break;
      case 'B':
        pos.pieceBoards[2] |= 1ULL << sq;
        break;
      case 'R':
        pos.pieceBoards[3] |= 1ULL << sq;
        break;
      case 'Q':
        pos.pieceBoards[4] |= 1ULL << sq;
        break;
      case 'k':
        pos.kings[1] = sq;
        break;
      case 'p':
        pos.pieceBoards[5] |= 1ULL << sq;
        break;
      case 'n':
        pos.pieceBoards[6] |= 1ULL << sq;
        break;
      case 'b':
        pos.pieceBoards[7] |= 1ULL << sq;
        break;
      case 'r':
        pos.pieceBoards[8] |= 1ULL << sq;
        break;
      case 'q':
        pos.pieceBoards[9] |= 1ULL << sq;
        break;
      case '/':
        sq--;
        break;
      default:
        sq += f - 49;
        break;
      }
      sq++;
    } else {
      switch (f) {
      case 'w':
        pos.whiteToMove = true;
        break;
      case 'b':
        pos.whiteToMove = false;
        break;
      case 'K':
        pos.st.castlingRights |= 0b0001;
        break; // White king side castling
      case 'Q':
        pos.st.castlingRights |= 0b0010;
        break; // White queen side castling
      case 'k':
        pos.st.castlingRights |= 0b0100;
        break; // Black king side castling
      case 'q':
        pos.st.castlingRights |= 0b1000;
        break; // Black queen side castling
      case '-':
        break;
      case ' ':
        break;
      case '/':
        break;
      default:
        if (epID < 2)
          eP[epID++] = f;
        break;
      }
    }
  }
  for (int i{0}; i < 5; i++) {
    pos.teamBoards[1] |= pos.pieceBoards[i];
    pos.teamBoards[2] |= pos.pieceBoards[i + 5];
  }
  pos.teamBoards[1] |= BB(pos.kings[0]);
  pos.teamBoards[2] |= BB(pos.kings[1]);

  pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

  if (!isdigit(eP[0])) {
    int file = eP[0] - 'a';
    int rank = 8 - eP[1] - '0';
    pos.st.enPassant = file + 8 * rank;
  } else {
    pos.st.enPassant = NoEP;
  }

  if (pos.whiteToMove) {
    m_moveGen.pinnedBoard<true>(pos);
    m_moveGen.checks<true>(pos);
    pos.st.enemyAttack = m_moveGen.findAttack<true>(pos);
  } else {
    m_moveGen.pinnedBoard<false>(pos);
    m_moveGen.checks<false>(pos);
    pos.st.enemyAttack = m_moveGen.findAttack<false>(pos);
  }
  pos.ply = 0;
  pos.st.hashKey = m_zobristHash.hashPosition(pos);
  pos.st.materialScore = m_evaluation.initMaterialValue(pos);
  pos.st.materialValue = m_evaluation.staticPieceEvaluation(pos.pieceBoards);
}
