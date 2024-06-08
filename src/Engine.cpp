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
#include "../inc/Engine.h"
#include "../inc/attackPext.h"
#include <cmath>

//#define PRINT_OUT

//#define SHALLOW_SEARCH


Engine::Engine() {
    _pos = {};
    nodes = 0;
    _moveGen = new MoveGen();
    _zobristHash = new ZobristHash();
    _evaluation = new Evaluation(_moveGen);
}

Engine::~Engine() {
    delete(_moveGen);
    delete(_zobristHash);
    delete(_evaluation);
}

void Engine::run() {
    std::cout << "ExplorerChess 1.0. Use help for a list of commands" << std::endl;
    while (true) {
        runUI();
    }
}



void Engine::runUI() {
	std::string userInput;
	std::getline(std::cin, userInput);

	size_t commandEnd1 = userInput.find(' ');
    size_t commandEnd2 = userInput.find(' ', commandEnd1 + 1);
    commandEnd2 = commandEnd2 > userInput.size() ? userInput.size()  : commandEnd2;

	std::string command = userInput.substr(0, commandEnd1);
    std::string arg = userInput.substr(commandEnd1 + 1, commandEnd2 - commandEnd1 - 1);
   
	if (strcmp(command.c_str(), "testpos") == 0){
        fenInit(_pos, "8/8/p1p5/1p5p/1P5p/8/PPP2K1p/4R1rk w - - 0 1");
        //fenInit(_pos, "r1b1kb1r/pppp1ppp/5q2/4n3/3KP3/2N3PN/PPP4P/R1BQ1B1R b kq - 0 1");
    }

	else if (strcmp(command.c_str(), "position") == 0){
        
        if (strcmp(arg.c_str(), "startpos") == 0) {
            fenInit(_pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if (strcmp(arg.c_str(), "fen") == 0) {
            std::string arg2 = userInput.substr(commandEnd2 + 1);
            fenInit(_pos, arg2);
        }
        else {
            fenInit(_pos, "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");
        }

	}
    else if (strcmp(command.c_str(), "test") == 0) {
        tests();
    }
    else if (strcmp(command.c_str(), "analyse") == 0) {
        /*int eval = 0;
        if (_pos.whiteToMove) eval = _evaluation->evaluate<true>(_pos);
        else eval = _evaluation->evaluate<false>(_pos);
        
            
        std::cout << "Score: " << eval << std::endl;*/

        
        Move move;
        const int depth = std::stoi(arg);
        if (_pos.whiteToMove) {
            move = analysis<true>(_pos, depth);
        }
        else {
            move = analysis<false>(_pos, depth);
        }
        GUI::printMove(move.move);

        std::cout << "\nNodes searched: " << nodes << "\nQnodes searched: " << qNodes << "\nEval: " << move.eval << std::endl;
    }
    else if (strcmp(command.c_str(), "hashTest") == 0) {
        MoveList ml;
        if (_pos.whiteToMove)
            _moveGen->generateAllMoves<true>(_pos, ml, false);
        else
            _moveGen->generateAllMoves<false>(_pos, ml, false);

        for (int i = 0; i < ml.size(); ++i) {
            if (_pos.whiteToMove) {
                prevMoves.push(ml.moves[i]);
                doMove<true, true>(_pos, ml.moves[i]);
                const uint64_t hash = _zobristHash->hashPosition(_pos);
                if (_pos.st.hashKey != hash) {
                    std::cout << "Hash fail on move: ";
                    GUI::parseMove(ml.moves[i]);
                }
                undoMove<false, true>(_pos, ml.moves[i]);
            }
            else {
                prevMoves.push(ml.moves[i]);
                doMove<false, true>(_pos, ml.moves[i]);
                const uint64_t hash = _zobristHash->hashPosition(_pos);
                if (_pos.st.hashKey != hash) {
                    std::cout << "Hash fail on move: ";
                    GUI::parseMove(ml.moves[i]);
                }
                undoMove<true, true>(_pos, ml.moves[i]);
            }
        }
    }
    else if (strcmp(command.c_str(), "d") == 0) {
        GUI::print_pieces(_pos);
    }
    else if (strcmp(command.c_str(), "g") == 0) {
        moveIntegrity<true, true>(_pos);
    }
    else if(strcmp(command.c_str(), "bishop") == 0){
        uint64_t bs = _pos.pieceBoards[2];
        while(bs){
            int x = bitScan(bs);
            bs &= bs - 1;
            uint64_t attack = _moveGen->attackBB<Bishop>(_pos.teamBoards[0], x);
            GUI::print_bit_board(attack);
            GUI::print_pieces(_pos);
        }
    }
    else if (strcmp(command.c_str(), "state") == 0) {
        const int score = _pos.whiteToMove ? _evaluation->evaluate<true>(_pos) : _evaluation->evaluate<false>(_pos);
        std::cout << "Eval: " << score << " Score: " << _pos.st.materialScore << " Value: " << _pos.st.materialValue << "\n";
    }
    else if (strcmp(command.c_str(), "make") == 0) {
        MoveList ml;
        if (_pos.whiteToMove) {
            _moveGen->generateAllMoves<true>(_pos, ml, false);
            const uint32_t move = GUI::findMove(ml, arg);
            if (move != 0) {
                prevMoves.push(move);
                doMove<true, true>(_pos, move);
            }
        }
        else {
            _moveGen->generateAllMoves<false>(_pos, ml, false);
            const uint32_t move = GUI::findMove(ml, arg);
            if (move != 0) {
                prevMoves.push(move);
                doMove<false, true>(_pos, move);
            }
        }
        
    }
    else if (strcmp(command.c_str(), "unmake") == 0) {
        if (!prevMoves.empty()) {
            uint32_t move = prevMoves.top();
            prevMoves.pop();
            if (_pos.whiteToMove) {
                undoMove<true, true>(_pos, move);
            }
            else {
                undoMove<false, true>(_pos, move);
            }
        }
    }
    else if (strcmp(command.c_str(), "perft") == 0) {
        const uint32_t depth = std::stoi(arg);
        auto start = std::chrono::system_clock::now();
        if (_pos.whiteToMove) {
            perft<true>(_pos, depth);
        }
        else {
            perft<false>(_pos, depth);
        }
        auto end = std::chrono::system_clock::now();
        auto duration = duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Execution time: " << duration << " ms\n";
    }
    else if (strcmp(command.c_str(), "help") == 0) {
        std::cout << "Available commands:\n"
            << "position: Set position. Usage: position startpos or position fen <fenstring>.\n"
            << "perft: Count number of possible positions at certain given depth for a position.\n"
            << "make: Makes specified move. Does nothing if move is illegal.\n"
            << "unmake: Unmakes last made move.\n"
            << "d: display current position.\n"
            << "analyse: Analyse position at certain given depth\n";
    }
    else {
        
        std::cout << "Command not found: Try help for a list of commands\n";
        
    }
}




//---------------------------------------------------------------------
//---------------------- Perft ----------------------------------------
//---------------------------------------------------------------------

template<bool whiteToMove>
uint64_t Engine::perft(Position& pos, int depth) {
    MoveList move_list;
    const bool castle = _moveGen->generateAllMoves<whiteToMove>(pos, move_list, false);
    nodes = 0;

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
    }
    else {
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
    std::cout << "Total positions: " << numPositions << "\nNodes searched: " << nodes << std::endl;
#endif
    //_transpositionTable.clear();
    return numPositions;
}

template<bool whiteToMove, bool castle>
uint64_t Engine::search(Position& pos, int depth) {
    nodes++;
    //if (_transpositionTable.contains(pos.st.hashKey)) {
      //  return _transpositionTable.at(pos.st.hashKey);
    //}

#ifdef SHALLOW_SEARCH
    MoveList move_list;
    const bool castleAllowed = _moveGen->generateAllMoves<whiteToMove>(pos, move_list, false);
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
    const bool castleAllowed = _moveGen->generateAllMoves<whiteToMove>(pos, move_list, false);
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

template<bool whiteToMove>
Move Engine::analysis(Position& pos, int depth) {
    _evalTransposition.clear();

    MoveList move_list;
    const bool castle = _moveGen->generateAllMoves<whiteToMove>(pos, move_list, false);
    MoveOrder::moveSort(move_list, pos, depth);
    nodes = 0;
    qNodes = 0;
    Move bestMove = { CHECK_MATE, move_list.moves[0]}; //Initiate first move with worst possible eval
   
    if (castle) {
        for (int i = 0; i < move_list.size(); ++i) {
            doMove<whiteToMove, true>(pos, move_list.moves[i]);
            const int eval = -negaMax<!whiteToMove, true>(pos, CHECK_MATE - depth, -CHECK_MATE + depth, depth - 1);
            GUI::printMove(move_list.moves[i]);
            std::cout << " => Eval: " << eval << std::endl;
            if (eval > bestMove.eval) {
                bestMove = { eval, move_list.moves[i] };
            }
            undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
        }
    }
    else {
        for (int i = 0; i < move_list.size(); ++i) {
            doMove<whiteToMove, false>(pos, move_list.moves[i]);
            const int eval = -negaMax<!whiteToMove, false>(pos, CHECK_MATE - depth, -CHECK_MATE + depth, depth - 1);
            GUI::printMove(move_list.moves[i]);
            std::cout << " => Eval: " << eval << std::endl;
            if (eval > bestMove.eval) {
                bestMove = { eval, move_list.moves[i] };
            }
            undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
        }
    }
    return bestMove;
}

template<bool whiteToMove, bool castle>
int Engine::negaMax(Position& pos, int alpha, int beta, int depth) {
    //Negamax with alpha beta pruning
    nodes++;
    if (depth == 0) {
        return qSearch<whiteToMove>(pos, alpha, beta);
        //return _evaluation->evaluate<whiteToMove>(pos);
    }

    if (_evalTransposition.contains(pos.st.hashKey)) {
       return _evalTransposition.at(pos.st.hashKey);
    }
    
    MoveList move_list;

    const bool castleAllowed = _moveGen->generateAllMoves<whiteToMove>(pos, move_list, false);

    if (move_list.size() == 0) {
        if (pos.st.checkers) {
            return CHECK_MATE - depth; //depth makes sure we get the fastest checkmate as the best one
        }
        return 0; //Stale mate
    }
    //Do some move ordering (maybe do move ordering already in the generation
    MoveOrder::moveSort(move_list, pos, depth);

    if constexpr (castle) {
        if (castleAllowed) {
            for (int i = 0; i < move_list.size(); ++i) {
                doMove<whiteToMove, true>(pos, move_list.moves[i]);
                const int eval = -negaMax<!whiteToMove, true>(pos, -beta, -alpha, depth - 1);
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
        const int eval = -negaMax<!whiteToMove, false>(pos, -beta, -alpha, depth - 1);
        undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
        if (eval >= beta) {
            return beta;
        }
        alpha = std::max(alpha, eval);
    }
    _evalTransposition[pos.st.hashKey] = alpha;
    return alpha;
}

template<bool whiteToMove>
int Engine::qSearch(Position& pos, int alpha, int beta) {
    qNodes++;
    int stand_pat = _evaluation->evaluate<whiteToMove>(pos);
    if(stand_pat >= beta)
        return beta;
    if(alpha < stand_pat)
        alpha = stand_pat;

    MoveList moveList;
    _moveGen->generateAllMoves<whiteToMove>(pos, moveList, true);
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


template<bool whiteToMove, bool castle>
void Engine::moveIntegrity(Position& pos) {
    MoveList m;
    _moveGen->generateAllMoves<whiteToMove>(pos, m, false);
    for (int i = 0; i < m.size(); ++i) {
        const int materialValue = pos.st.materialValue;
        const int materialScore = pos.st.materialScore;
        doMove<whiteToMove, castle>(pos, m.moves[i]);
        undoMove<!whiteToMove, castle>(pos, m.moves[i]);
        
        if (pos.st.materialScore != materialScore || pos.st.materialValue != materialValue) {
            GUI::parseMove(m.moves[i]);
            std::cout << "Material: " << materialValue << ", After: " << pos.st.materialValue << std::endl;            
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

template<bool whiteToMove, bool castle>
void Engine::doMove(Position& pos, uint32_t move) {
    //Save state info
    prevStates.push(pos.st);
    uint64_t newHash = pos.st.hashKey ^ _zobristHash->whiteToMoveHash;


    //Get move info
    const uint8_t from = getFrom(move);
    const uint8_t to = getTo(move);
    const uint8_t mover = getMover(move);
    //Captured will be 0 if there is no capture so important to do capture before move
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
    //Increment position value based on prev move
    pos.st.materialScore += scoreFavor * _evaluation->pieceScoreChange(from, to, mover);
    
    //Remove old ep hash if there was one
    if (pos.st.enPassant != 0) {
        newHash ^= _zobristHash->epHash[pos.st.enPassant & 7]; // File ep hash
        pos.st.enPassant = 0;
    }
 
    if (mover > King - 1) {
        if constexpr (whiteToMove) {
            pos.kings[0] = to;
            newHash ^= _zobristHash->kingHash[0][from] ^ _zobristHash->kingHash[0][to];
        }
        else {
            pos.kings[1] = to;
            newHash ^= _zobristHash->kingHash[1][from] ^ _zobristHash->kingHash[1][to];
        }

        if (flags & CAPTURE) {
            pos.pieceBoards[captured] ^= toBB;
            pos.teamBoards[enemy] ^= toBB;
            newHash ^= _zobristHash->pieceHash[captured][to];
            pos.st.materialValue += scoreFavor * _evaluation->getPieceValue(captured + blackOffset);
        }

        if constexpr (castle) {
            constexpr int8_t rook = whiteToMove ? 3 : 8;
            if (flags == CASTLE_KING) {
                doCastle<whiteToMove, true>(pos);
                constexpr uint8_t rookFrom = whiteToMove ? 63 : 7;
                constexpr uint8_t rookTo = whiteToMove ? 61 : 5;
                 
                newHash ^= _zobristHash->pieceHash[3][rookFrom] ^ _zobristHash->pieceHash[3][rookTo];
                pos.st.materialScore += scoreFavor * _evaluation->pieceScoreChange(rookFrom, rookTo, rook);
            }
            else if (flags == Flags::CASTLE_QUEEN) {
                doCastle<whiteToMove, false>(pos);
                constexpr uint8_t rookFrom = whiteToMove ? 56 : 0;
                constexpr uint8_t rookTo = whiteToMove ? 59 : 3;
                newHash ^= _zobristHash->pieceHash[3][rookFrom] ^ _zobristHash->pieceHash[3][rookTo];
                pos.st.materialScore += scoreFavor * _evaluation->pieceScoreChange(rookFrom, rookTo, rook);
            }
        }
    }
    else {
        pos.pieceBoards[mover] ^= fromBB ^ toBB;
        //Mover hash
        newHash ^= _zobristHash->pieceHash[mover][from] ^ _zobristHash->pieceHash[mover][to];
        switch (flags) {
        case QUIET: break;
        case CAPTURE:
            pos.pieceBoards[captured] ^= toBB;
            pos.teamBoards[enemy] ^= toBB;
            newHash ^= _zobristHash->pieceHash[captured][to];
            pos.st.materialValue += scoreFavor * _evaluation->getPieceValue(captured + blackOffset);
            break;
        case DOUBLE_PUSH:
            pos.st.enPassant = to + add;
            newHash ^= _zobristHash->epHash[(to + add) & 7]; //Add new ep hash
            break;
        case EP_CAPTURE:
            pos.pieceBoards[enemyPawn] ^= BB((to + add));
            pos.teamBoards[enemy] ^= BB((to + add));
            newHash ^= _zobristHash->pieceHash[enemyPawn][to + add]; // Remove pawn from hash
            pos.st.materialValue += scoreFavor * _evaluation->getPieceValue(captured + blackOffset);
            break;
        default: //Promotion
            pos.pieceBoards[teamPawn] ^= toBB;
            newHash ^= _zobristHash->pieceHash[teamPawn][to];
            const uint8_t pID = 1 + 5 * !whiteToMove + getPromo(move);
            pos.pieceBoards[pID] |= toBB;
            newHash ^= _zobristHash->pieceHash[pID][to];
            //Remove pawn score and add new piece value
            pos.st.materialValue += scoreFavor * (_evaluation->getPieceValue(pID + (blackOffset ^ -5)) - 100);
            //Pawn score will already have been removed with pieceScoreChange
            pos.st.materialScore += scoreFavor * _evaluation->pieceScore(to, pID); 
            if (flags & CAPTURE) { 
                pos.pieceBoards[captured] ^= toBB;
                pos.teamBoards[enemy] ^= toBB;
                newHash ^= _zobristHash->pieceHash[captured][to];
                pos.st.materialValue += scoreFavor * _evaluation->getPieceValue(captured + blackOffset);
            }
            break;
        }
    }

    if constexpr (castle) {

        //Solve with pext and then switch case on the number maybe
        newHash ^= _zobristHash->castleHash[pos.st.castlingRights];
        //Handles if rook is captured or moves
        pos.st.castlingRights &= castlingModifiers[from];
        pos.st.castlingRights &= castlingModifiers[to];

        newHash ^= _zobristHash->castleHash[pos.st.castlingRights];
    }


    //Restore occupied
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

    pos.whiteToMove = !whiteToMove;
    newHash ^= pos.ply * _zobristHash->moveHash;

    pos.ply++;

    newHash ^= pos.ply * _zobristHash->moveHash;

    pos.st.hashKey = newHash;
    //Prepare for next movegeneration
    pos.st.enemyAttack = _moveGen->findAttack<!whiteToMove>(pos);
    _moveGen->pinnedBoard<!whiteToMove>(pos);
    _moveGen->checks<!whiteToMove>(pos);
    _moveGen->setCheckSquares<!whiteToMove>(pos);
}


template<bool whiteToMove, bool castle>
void Engine::undoMove(Position& pos, uint32_t move) {
    pos.st = prevStates.top();
    prevStates.pop();

    //Get move info
    const uint8_t from = getFrom(move);
    const uint8_t to = getTo(move);
    const uint8_t mover = getMover(move);
    //Captured will be 0 if there is no capture so important to do capture before move
    const uint8_t captured = getCaptured(move);
    const uint32_t flags = getFlags(move);

    const uint64_t fromBB = BB(from);
    const uint64_t toBB = BB(to);
    const bool isCapture = (((flags & CAPTURE) == CAPTURE) && flags != EP_CAPTURE);
    const uint64_t capMask = toBB * isCapture;

    //Incrementally update position
    if constexpr (whiteToMove) {
        pos.teamBoards[2] ^= fromBB ^ toBB;
        pos.teamBoards[1] ^= capMask;
    }
    else {
        pos.teamBoards[1] ^= fromBB ^ toBB;
        pos.teamBoards[2] ^= capMask;
    }

    pos.pieceBoards[captured] ^= capMask;

    if (mover > King - 1) {
        pos.kings[whiteToMove] = from;


        if constexpr (castle) {
            if (flags == CASTLE_KING) {
                doCastle<!whiteToMove, true>(pos);
            }
            else if (flags == CASTLE_QUEEN) {
                doCastle<!whiteToMove, false>(pos);
            }
        }
        
    }
    else {
        pos.pieceBoards[mover] ^= fromBB ^ toBB;

        //EP: add the ep pawn if there was an ep capture
        if constexpr (whiteToMove) {
            const uint64_t ep_cap = (flags == EP_CAPTURE) * BB((to - 8));
            pos.pieceBoards[0] ^= ep_cap;
            pos.teamBoards[1] ^= ep_cap;
        }
        else {
            const uint64_t ep_cap = (flags == EP_CAPTURE) * BB((to + 8));
            pos.pieceBoards[5] ^= ep_cap;
            pos.teamBoards[2] ^= ep_cap;
        }

        //Pawn promotion
        if ((flags & PROMO_N) != 0) {
            if constexpr (whiteToMove) {
                pos.pieceBoards[5] ^= toBB;
                const int8_t pID = 6 + getPromo(move);
                pos.pieceBoards[pID] ^= toBB;
            }
            else {
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

/*template<bool whiteToMove, bool castle>
void Engine::undoMove(Position& pos, uint32_t move) {
    pos.st = prevStates.top();
    prevStates.pop();

    //Get move info
    const uint8_t from = getFrom(move);
    const uint8_t to = getTo(move);
    const uint8_t mover = getMover(move);
    //Captured will be 0 if there is no capture so important to do capture before move
    const uint8_t captured = getCaptured(move);
    const uint32_t flags = getFlags(move);

    const uint64_t fromBB = BB(from);
    const uint64_t toBB = BB(to);
    const uint64_t capMask = toBB * (((flags & CAPTURE) == CAPTURE) && flags != EP_CAPTURE);

    //Incrementally update position
    if constexpr (whiteToMove) {
        pos.teamBoards[2] ^= fromBB ^ toBB;
        pos.teamBoards[1] ^= capMask;
    }
    else {
        pos.teamBoards[1] ^= fromBB ^ toBB;
        pos.teamBoards[2] ^= capMask;
    }

    pos.pieceBoards[captured] ^= capMask;

    if (mover == King) {
        pos.kings[whiteToMove] = from;


        if constexpr (castle) {
            if (flags == CASTLE_KING) {
                doCastle<!whiteToMove, true>(pos);
            }
            else if (flags == CASTLE_QUEEN) {
                doCastle<!whiteToMove, false>(pos);
            }
        }
        
    }
    else {
        pos.pieceBoards[mover] ^= fromBB ^ toBB;

        //EP: add the ep pawn if there was an ep capture
        if constexpr (whiteToMove) {
            const uint64_t ep_cap = (flags == EP_CAPTURE) * BB(to - 8);
            pos.pieceBoards[0] ^= ep_cap;
            pos.teamBoards[1] ^= ep_cap;
        }
        else {
            const uint64_t ep_cap = (flags == EP_CAPTURE) * BB(to + 8);
            pos.pieceBoards[5] ^= ep_cap;
            pos.teamBoards[2] ^= ep_cap;
        }

        //Pawn promotion
        if ((flags & PROMO_N) != 0) {
            if constexpr (whiteToMove) {
                pos.pieceBoards[5] ^= toBB;
                pos.pieceBoards[6 + getPromo(move)] ^= toBB;
            }
            else {
                pos.pieceBoards[0] ^= toBB;
                pos.pieceBoards[1 + getPromo(move)] ^= toBB;
            }
        }

    }
    
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];
    
    pos.whiteToMove = !whiteToMove;
}*/


template<bool whiteToMove, bool castleKing>
inline void Engine::doCastle(Position& pos) {
    if constexpr (whiteToMove) {
        constexpr uint64_t rookFromTo = castleKing ? BB(63) | BB(61) : BB(56) | BB(59);
        pos.pieceBoards[3] ^= rookFromTo;
        pos.teamBoards[1] ^= rookFromTo;
    }
    else {
        constexpr uint64_t rookFromTo = castleKing ? BB(7) | BB(5) : BB(0) | BB(3);
        pos.pieceBoards[8] ^= rookFromTo;
        pos.teamBoards[2] ^= rookFromTo;
    }
}







void Engine::fenInit(Position& pos, std::string fen) {
    uint32_t i = 0;

    char eP[2] = {};
    int epID = 0;

    pos = {};
    for (char f : fen) {
        if (i < 64) {
            switch (f) { 
            case 'K': pos.kings[0] = i; break;
            case 'P': pos.pieceBoards[0] |= 1ULL << i; break;
            case 'N': pos.pieceBoards[1] |= 1ULL << i; break;
            case 'B': pos.pieceBoards[2] |= 1ULL << i; break;
            case 'R': pos.pieceBoards[3] |= 1ULL << i; break;
            case 'Q': pos.pieceBoards[4] |= 1ULL << i; break;
            case 'k': pos.kings[1] = i; break;
            case 'p': pos.pieceBoards[5] |= 1ULL << i; break;
            case 'n': pos.pieceBoards[6] |= 1ULL << i; break;
            case 'b': pos.pieceBoards[7] |= 1ULL << i; break;
            case 'r': pos.pieceBoards[8] |= 1ULL << i; break;
            case 'q': pos.pieceBoards[9] |= 1ULL << i; break;
            case '/': i--; break;
            default: i += f - 49; break;
            }
            i++;
        }
        else {
            switch (f) {
            case 'w': pos.whiteToMove = true; break;
            case 'b': pos.whiteToMove = false; break;
            case 'K': pos.st.castlingRights |= 0b0001; break; //White king side castling
            case 'Q': pos.st.castlingRights |= 0b0010; break; //White queen side castling
            case 'k': pos.st.castlingRights |= 0b0100; break; //Black king side castling
            case 'q': pos.st.castlingRights |= 0b1000; break; //Black queen side castling
            case '-': break;
            case ' ': break;
            case '/': break;
            default: if(epID < 2) eP[epID++] = f; break;
            }
        }
    }
    for (int i = 0; i < 5; i++) {
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
    }
    else {
        pos.st.enPassant = NoEP;
    }


    if (pos.whiteToMove) {
        _moveGen->pinnedBoard<true>(pos);
        _moveGen->checks<true>(pos);
        pos.st.enemyAttack = _moveGen->findAttack<true>(pos);
    }
    else {
        _moveGen->pinnedBoard<false>(pos);
        _moveGen->checks<false>(pos);
        pos.st.enemyAttack = _moveGen->findAttack<false>(pos);
    }
    pos.ply = 0;
    pos.st.hashKey = _zobristHash->hashPosition(pos);
    pos.st.materialScore = _evaluation->initMaterialValue(pos);
    pos.st.materialValue = _evaluation->staticPieceEvaluation(pos.pieceBoards);
}



void Engine::tests()
{
    bool fail = false;
    Position pos;
    int testNr = 1;
    double totalTime = 0.0;
    
    auto test = [&](const char* str, uint64_t count, uint8_t perftLevel = 6) -> double {
        using namespace std;

        cout << "\nRunning test: " << testNr << endl;
        
        auto start = chrono::system_clock::now();
        uint64_t perftCount;
        if(pos.whiteToMove){
            perftCount = Engine::perft<true>(pos, perftLevel);
        }
        else{
            perftCount = Engine::perft<false>(pos, perftLevel);    
        } 
        auto end = chrono::system_clock::now();
        chrono::duration<double> elapsed_time = end - start;
        double kN = static_cast<double>(perftCount) / elapsed_time.count() / 1000;
        totalTime += elapsed_time.count();
        cout << "Time: " << elapsed_time.count() << "s" << endl;
        cout << "KN/s: "
            << static_cast<double>(perftCount) / elapsed_time.count() / 1000
            << endl;
        cout << "Perft depth: " << (int)perftLevel << endl;

        if (count != perftCount)
        {
            cout << "Test " << testNr
                << " failed, expected:" << count
                << " got: " << perftCount << endl
                << "Fen: " << str << endl;
            fail = true;
        }
        else
        {
            cout << "Test " << testNr << " success!" << endl;
        }
        testNr++;
        return kN;
    };

            
    auto geometric_mean = [&](std::vector<double> const& data) -> double
    {
        double m = 1.0;
        long long ex = 0;
        double invN = 1.0 / data.size();

        for (double x : data)
        {
            int i;
            double f1 = std::frexp(x, &i);
            m *= f1;
            ex += i;
        }

        return std::pow(std::numeric_limits<double>::radix, ex * invN) * std::pow(m, invN);
    };


    std::vector<double> results;




    const double geo_mean = geometric_mean(results);
    std::cout << "Average kN/s: " << geo_mean << std::endl;
    if (!fail)
    {
        std::cout << "Current build cleared all tests in " << totalTime << " s" << std::endl; 
    }
}
