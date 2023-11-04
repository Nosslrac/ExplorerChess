#include "../inc/Engine.h"

//#define PRINT_OUT

#define SHALLOW_SEARCH


Engine::Engine() {
	std::cout << "ExplorerChess 1.0. Use help for a list of commands" << std::endl;
    _pos = {};
    _moveGen = new MoveGen();
    _zobristHash = new ZobristHash();
}

Engine::~Engine() {
    delete(&_pos);
}

void Engine::run() {
    while (true) {
        runUI();
    }
}





void Engine::runUI() {
	std::string userInput;
	std::getline(std::cin, userInput);

	size_t commandEnd = userInput.find(' ');

	std::string command = userInput.substr(0, commandEnd);
    std::string arg = userInput.substr(commandEnd + 1, userInput.size());



	if (strcmp(command.c_str(), "fen") == 0){
        //fenInit(_pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        fenInit(_pos, "4k3/1P6/8/8/8/8/K7/8 w - - 0 1");
        //fenInit(_pos, "r1n1kn1r/4pp2/6N1/1NPp2P1/2p3p1/8/3PPP2/R3K2R w KQkq d6 0 1");
	}
    else if (strcmp(command.c_str(), "test") == 0) {
        tests();
    }
    else if (strcmp(command.c_str(), "hashTest") == 0) {
        MoveList ml;
        if (_pos.whiteToMove)
            _moveGen->generateAllMoves<true>(_pos, ml);
        else
            _moveGen->generateAllMoves<false>(_pos, ml);

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
        MoveList ml;
        _moveGen->generateAllMoves<true>(_pos, ml);
        GUI::parseMove(ml.moves[std::stoi(arg)]);
    }
    else if (strcmp(command.c_str(), "state") == 0) {
        GUI::printState(_pos.st);
    }
    else if (strcmp(command.c_str(), "make") == 0) {
        MoveList ml;
        if (_pos.whiteToMove) {
            _moveGen->generateAllMoves<true>(_pos, ml);
            const uint32_t move = GUI::findMove(ml, arg);
            prevMoves.push(move);
            doMove<true, true>(_pos, move);
        }
        else {
            _moveGen->generateAllMoves<false>(_pos, ml);
            const uint32_t move = GUI::findMove(ml, arg);
            prevMoves.push(move);
            doMove<false, true>(_pos, move);
        }
        
    }
    else if (strcmp(command.c_str(), "unmake") == 0) {
        uint32_t move  = prevMoves.top();
        prevMoves.pop();
        if (_pos.whiteToMove) {
            undoMove<true, true>(_pos, move);
        }
        else {
            undoMove<false, true>(_pos, move);
        }
    }
    else if (strcmp(command.c_str(), "perft") == 0) {
        const uint32_t depth = std::stoi(arg);
        if (_pos.whiteToMove) {
            perft<true>(_pos, depth);
        }
        else {
            perft<false>(_pos, depth);
        } 
    }
}


template<bool whiteToMove>
uint64_t Engine::perft(Position& pos, int depth) {
    hashHits = 0;
    MoveList move_list;
    const bool castle = _moveGen->generateAllMoves<whiteToMove>(pos, move_list);

#ifndef SHALLOW_SEARCH
    if (depth < 2) {
        for (int i = 0; i < move_list.size(); ++i) {
            GUI::printMove(move_list.moves[i]);
            std::cout << 1 << std::endl;
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
            std::cout << part << std::endl;
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
            std::cout << part << std::endl;
#endif  
            undoMove<!whiteToMove, false>(pos, move_list.moves[i]);
        }
    }
    
  
    std::cout << "Total positions: " << numPositions << "\nHash hits: " << hashHits << std::endl;
    _transpositionTable.clear();
    return numPositions;
}

template<bool whiteToMove, bool castle>
uint64_t Engine::search(Position& pos, int depth) {

    if (_transpositionTable.find(pos.st.hashKey) != _transpositionTable.end()) {
        hashHits++;
        return _transpositionTable.at(pos.st.hashKey);
    }

#ifdef SHALLOW_SEARCH
    MoveList move_list;
    const bool castleAllowed = _moveGen->generateAllMoves<whiteToMove>(pos, move_list);

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
        _transpositionTable[pos.st.hashKey] = numPositions;
        return numPositions;
    }
    return move_list.size();
#else
    if (depth == 0)
        return 1;
    
    MoveList move_list;
    const bool castleAllowed = _moveGen->generateAllMoves<whiteToMove>(pos, move_list);
    uint64_t numPositions = 0;
    for (int i = 0; i < move_list.size(); ++i) {
        doMove<whiteToMove, true>(pos, move_list.moves[i]);
        numPositions += search<!whiteToMove, true>(pos, depth - 1);
        undoMove<!whiteToMove, true>(pos, move_list.moves[i]);
    }
    return numPositions;
#endif

}


template<bool whiteToMove, bool castle>
void Engine::moveIntegrity(Position& pos) {
    MoveList m;
    _moveGen->generateAllMoves<whiteToMove>(pos, m);
    Position copy = pos;
    for (int i = 0; i < m.size(); ++i) {
        doMove<whiteToMove, castle>(pos, m.moves[i]);
        undoMove<!whiteToMove, castle>(pos, m.moves[i]);
        if (!(copy == pos)) {
            GUI::print_pieces(pos);
            GUI::parseMove(m.moves[i]);
            std::cout << "Team boards: " << std::endl;
            std::cout << "Copy: " << std::endl;
            GUI::print_bit_board(copy.teamBoards[2]);
            std::cout << "Current: " << std::endl;
            GUI::print_bit_board(pos.teamBoards[2]);            
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

    pos.teamBoards[team] ^= fromBB ^ toBB;
    
    //Remove old ep hash if there was one
    if (pos.st.enPassant != 0) {
        newHash ^= _zobristHash->epHash[pos.st.enPassant & 7]; // File ep hash
        pos.st.enPassant = 0;
    }
 


    if (mover == King) {
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
        }

        if constexpr (castle) {
            if (flags == CASTLE_KING) {
                doCastle<whiteToMove, true>(pos);
                constexpr uint8_t rookFrom = whiteToMove ? 63 : 7;
                constexpr uint8_t rookTo = whiteToMove ? 61 : 5;
                newHash ^= _zobristHash->pieceHash[3][rookFrom] ^ _zobristHash->pieceHash[3][rookTo];
            }
            else if (flags == CASTLE_QUEEN) {
                doCastle<whiteToMove, false>(pos);
                constexpr uint8_t rookFrom = whiteToMove ? 56 : 0;
                constexpr uint8_t rookTo = whiteToMove ? 59 : 3;
                newHash ^= _zobristHash->pieceHash[3][rookFrom] ^ _zobristHash->pieceHash[3][rookTo];
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
            break;
        case DOUBLE_PUSH:
            pos.st.enPassant = to + add;
            newHash ^= _zobristHash->epHash[(to + add) & 7]; //Add new ep hash
            break;
        case EP_CAPTURE:
            pos.pieceBoards[enemyPawn] ^= BB(to + add);
            pos.teamBoards[enemy] ^= BB(to + add);
            newHash ^= _zobristHash->pieceHash[enemyPawn][to + add]; // Remove pawn from hash
            break;
        default: //Promotion
            pos.pieceBoards[teamPawn] ^= toBB;
            newHash ^= _zobristHash->pieceHash[teamPawn][to];
            const uint8_t pID = 1 + 5 * !whiteToMove + getPromo(move);
            pos.pieceBoards[pID] |= toBB;
            newHash ^= _zobristHash->pieceHash[pID][to];
            if (flags & CAPTURE) {
                pos.pieceBoards[captured] ^= toBB;
                pos.teamBoards[enemy] ^= toBB;
                newHash ^= _zobristHash->pieceHash[captured][to];
            }
            break;
        }
    }

    if constexpr (castle) {

        //Solve with pext and then switch case on the number maybe
        const uint8_t beforeCnt = pos.st.castlingRights;
        //Handles if rook is captured or moves
        pos.st.castlingRights &= castlingModifiers[from];
        pos.st.castlingRights &= castlingModifiers[to];

        //TODO: capture of rook needs to update castle hash
        //Idea xor before and after => changed bits will be set to 1
        switch (beforeCnt ^ pos.st.castlingRights) {
        case 0b1100: newHash ^= _zobristHash->castleHash[3] ^ _zobristHash->castleHash[2]; break; //black king move
        case 0b1000: newHash ^= _zobristHash->castleHash[3]; break; //black queen rook move
        case 0b0100: newHash ^= _zobristHash->castleHash[2]; break; //black king rook move
        case 0b0011: newHash ^= _zobristHash->castleHash[0] ^ _zobristHash->castleHash[1]; break; //white king move
        case 0b0010: newHash ^= _zobristHash->castleHash[1]; break; //white queen rook move
        case 0b0001: newHash ^= _zobristHash->castleHash[0]; break; //white king rook move
        case 0b1010: newHash ^= _zobristHash->castleHash[1] ^ _zobristHash->castleHash[3]; break; //queen rooks capture other either way
        case 0b0101: newHash ^= _zobristHash->castleHash[0] ^ _zobristHash->castleHash[2]; break; //king rooks capture other either way
        }
    }


    //Restore occupied
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

    pos.whiteToMove = !whiteToMove;
    newHash ^= pos.ply * _zobristHash->moveHash;

    pos.ply++;

    newHash ^= pos.ply * _zobristHash->moveHash;

    pos.st.hashKey = newHash;
    //Prepare for next movegeneration
    _moveGen->findAttack<!whiteToMove>(pos);
    _moveGen->pinnedBoard<!whiteToMove>(pos);
    _moveGen->checks<!whiteToMove>(pos);
}

//Check list for undo move
// - Reset state
// - Move back piece
// - If capture of normal or promotion add back piece to current side
// - If ep capture add back pawn of current side
// - If promotion remove piece and add back pawn






/*
template<bool whiteToMove, bool castle>
void Engine::doMove(Position& pos, uint32_t move) {
    //Save state info
    prevStates.push(pos.st);


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
        pos.teamBoards[1] ^= fromBB ^ toBB;
        pos.teamBoards[2] ^= capMask;
        pos.st.enPassant = (flags == DOUBLE_PUSH) * (to + 8);
    }
    else {
        pos.teamBoards[2] ^= fromBB ^ toBB;
        pos.teamBoards[1] ^= capMask;
        pos.st.enPassant = (flags == DOUBLE_PUSH) * (to - 8);
    }
    pos.pieceBoards[captured] ^= capMask;


    if (mover == King) {
        if constexpr (whiteToMove) {
            pos.kings[0] = to;
        }
        else {
            pos.kings[1] = to;
        }
        if constexpr (castle) {
            if (flags == CASTLE_KING) {
                doCastle<whiteToMove, true>(pos);
            }
            else if (flags == CASTLE_QUEEN) {
                doCastle<whiteToMove, false>(pos);
            }
        }
    }
    else {
        pos.pieceBoards[mover] ^= fromBB ^ toBB;

        //EP: remove pawn if there is a EP capture and update ep if there is a double push
        //And promotions

        if constexpr (whiteToMove) {
            const uint64_t ep_cap = (flags == EP_CAPTURE) * BB(to + 8);
            pos.pieceBoards[5] ^= ep_cap;
            pos.teamBoards[2] ^= ep_cap;

        }
        else {
            const uint64_t ep_cap = (flags == EP_CAPTURE) * BB(to - 8);
            pos.pieceBoards[0] ^= ep_cap;
            pos.teamBoards[1] ^= ep_cap;

        }



        if ((flags & PROMO_N) != 0) {
            if constexpr (whiteToMove) {
                pos.pieceBoards[0] ^= toBB;
                pos.pieceBoards[1 + getPromo(move)] |= toBB;
            }
            else {
                pos.pieceBoards[5] ^= toBB;
                pos.pieceBoards[6 + getPromo(move)] |= toBB;
            }
        }
    }
    if constexpr (castle) {
        //Handles if rook is captured or moves
        pos.st.castlingRights &= castlingModifiers[from];
        pos.st.castlingRights &= castlingModifiers[to];
        //TODO: capture of rook needs to update castle hash
    }


    //Restore occupied
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

    pos.whiteToMove = !whiteToMove;

    //Prepare for next movegeneration
    _moveGen->findAttack<!whiteToMove>(pos);
    _moveGen->pinnedBoard<!whiteToMove>(pos);
    _moveGen->checks<!whiteToMove>(pos);
}
*/

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
        _moveGen->findAttack<true>(pos);
    }
    else {
        _moveGen->pinnedBoard<false>(pos);
        _moveGen->checks<false>(pos);
        _moveGen->findAttack<false>(pos);
    }
    pos.ply = 0;
    pos.st.hashKey = _zobristHash->hashPosition(pos);
}


void Engine::tests()
{
    bool fail = false;
    Position pos;
    int testNr = 1;
    auto test = [&](const char* str,
        uint64_t count, uint8_t perftLevel = 6) -> double {
            using namespace std;

            cout << "\nRunning test: " << testNr << endl;
            fenInit(pos, str);

            auto start = chrono::system_clock::now();
            uint64_t perftCount = pos.whiteToMove ? Engine::perft<true>(pos, perftLevel) : Engine::perft<false>(pos, perftLevel);
            auto end = chrono::system_clock::now();
            chrono::duration<double> elapsed_time = end - start;
            double kN = static_cast<double>(perftCount) / elapsed_time.count() / 1000;
            cout << "Time: " << elapsed_time.count() << "s" << endl;
            cout << "KN/s: "
                << static_cast<double>(perftCount) / elapsed_time.count() / 1000
                << endl;

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

    results.push_back(test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 119060324ULL, 6));

    
    results.push_back(test("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 193690690ULL, 5));
    results.push_back(test("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 11030083ULL, 6));
    results.push_back(test("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 706045033ULL, 6));
    results.push_back(test("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3048196529ULL, 6));

    //Veri long test results.push_back(test("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ", 6923051137ULL, 6));
    results.push_back(test("8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1", 102503850ULL, 8));
    results.push_back(test("3k4/3p4/8/K1P4r/8/8/8/8 b - - 0 1", 130459988ULL, 8));
    //results.push_back(test("8/8/1k6/2b5/2pP4/8/5K2/8 b - d3 0 1", 1440467ULL, 6));
    results.push_back(test("5k2/8/8/8/8/8/8/4K2R w K - 0 1", 73450134ULL, 8));
    results.push_back(test("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1", 91628014ULL, 8));

    results.push_back(test("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1", 1509218880ULL, 6));
    results.push_back(test("r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 2010267707ULL, 6));
    results.push_back(test("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1", 905613447ULL, 8));
    results.push_back(test("8/8/1P2K3/8/2n5/1q6/8/5k2 b - - 0 1", 197013195ULL, 7));
    results.push_back(test("4k3/1P6/8/8/8/8/K7/8 w - - 0 1", 397481663ULL, 9));

    results.push_back(test("8/P1k5/K7/8/8/8/8/8 w - - 0 1", 153850274ULL, 9));
    results.push_back(test("K1k5/8/P7/8/8/8/8/8 w - - 0 1", 85822924ULL, 11));
    results.push_back(test("8/k1P5/8/1K6/8/8/8/8 w - - 0 1", 173596091ULL, 10));
    results.push_back(test("8/8/2k5/5q2/5n2/8/5K2/8 b - - 0 1", 104644508ULL, 7));

    const double geo_mean = geometric_mean(results);
    std::cout << "Average kN/s: " << geo_mean << std::endl;
    if (!fail)
    {
        std::cout << "Current build cleared all tests" << std::endl; 
    }
}

