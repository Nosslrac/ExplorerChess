#include "../inc/Engine.h"


Engine::Engine() {
	std::cout << "ExplorerChess 1.0. Use help for a list of commands" << std::endl;
    Engine::_pos = {};
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



	if (strcmp(command.c_str(), "fen") == 0) {
        //fenInit(_pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        fenInit(_pos, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
	}
    else if (strcmp(command.c_str(), "test") == 0) {
        if (_pos.whiteToMove) {
            moveIntegrity<true>(_pos);
        }
        else {
            moveIntegrity<false>(_pos);
        }
       
    }
    else if (strcmp(command.c_str(), "d") == 0) {
        GUI::print_pieces(_pos);
    }
    else if (strcmp(command.c_str(), "g") == 0) {
        MoveList ml;
        generateAllMoves<true>(_pos, ml);
        GUI::parseMove(ml.moves[std::stoi(arg)]);
    }
    else if (strcmp(command.c_str(), "state") == 0) {
        GUI::print_bit_board(_pos.teamBoards[0]);
        GUI::print_bit_board(_pos.teamBoards[1]);
        GUI::print_bit_board(_pos.teamBoards[2]);
    }
    else if (strcmp(command.c_str(), "make") == 0) {
        MoveList ml;
        if (_pos.whiteToMove) {
            generateAllMoves<true>(_pos, ml);
            const uint32_t move = ml.moves[std::stoi(arg)];
            prevMoves.push(move);
            doMove<true>(_pos, move);
        }
        else {
            generateAllMoves<false>(_pos, ml);
            const uint32_t move = ml.moves[std::stoi(arg)];
            prevMoves.push(move);
            doMove<false>(_pos, move);
        }
        
    }
    else if (strcmp(command.c_str(), "unmake") == 0) {
        uint32_t move  = prevMoves.top();
        prevMoves.pop();
        if (_pos.whiteToMove) {
            undoMove<true>(_pos, move);
        }
        else {
            undoMove<false>(_pos, move);
        }
    }
    else if (strcmp(command.c_str(), "perft") == 0) {
        GUI::print_bit_board(_pos.st.enemyAttack);
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
void Engine::perft(Position& pos, int depth) {
    MoveList move_list;
    const bool castle = generateAllMoves<whiteToMove>(pos, move_list);


    uint64_t numPositions = 0;
    for (int i = 0; i < move_list.size(); ++i) {

        doMove<whiteToMove>(pos, move_list.moves[i]);
        uint64_t part = search<!whiteToMove>(pos, depth - 1);
        numPositions += part;
        GUI::printMove(move_list.moves[i]);
        std::cout << part << std::endl;
        undoMove<!whiteToMove>(pos, move_list.moves[i]);
    }
  
    std::cout << "Total positions: " << numPositions << std::endl;
}

template<bool whiteToMove>
uint64_t Engine::search(Position& pos, int depth) {
    MoveList move_list;
    const bool castle = generateAllMoves<whiteToMove>(pos, move_list);
    if (depth == 0) {
        return 1;
    }

    if (depth == 1)
        return move_list.size();

    uint64_t numPositions = 0;

    for (int i = 0; i < move_list.size(); ++i) {
        doMove<whiteToMove>(pos, move_list.moves[i]);
        numPositions += search<!whiteToMove>(pos, depth - 1);
        undoMove<!whiteToMove>(pos, move_list.moves[i]);
    }

    return numPositions;
}

template<bool whiteToMove>
void Engine::moveIntegrity(Position& pos) {
    MoveList m;
    generateAllMoves<whiteToMove>(pos, m);
    Position copy = pos;
    for (int i = 0; i < m.size(); ++i) {
        doMove<whiteToMove>(pos, m.moves[i]);
        undoMove<!whiteToMove>(pos, m.moves[i]);
        if (!(copy == pos)) {
            std::cout << "Went wrong on move: " << i << std::endl;
            GUI::parseMove(m.moves[i]);
        }
    }
    std::cout << std::endl;
}





template<bool whiteToMove>
void Engine::doMove(Position& pos, uint32_t move) {
    //Get move info
    const uint8_t from = getFrom(move);
    const uint8_t to = getTo(move);
    const uint8_t mover = getMover(move);
    //Captured will be 0 if there is no capture so important to do capture before move
    const uint8_t captured = getCaptured(move);
    const uint32_t flags = getFlags(move);
    const uint64_t fromBB = 1ULL << from;
    const uint64_t toBB = 1ULL << to;

    //Incrementally update position
    if constexpr (whiteToMove) {
        pos.teamBoards[1] ^= fromBB | toBB;
        pos.teamBoards[2] &= ~(toBB);
    }
    else {
        pos.teamBoards[2] ^= fromBB | toBB;
        pos.teamBoards[1] &= ~(toBB);
    }
    
    pos.pieceBoards[captured] &= ~(toBB);
    pos.pieceBoards[mover] ^= fromBB | toBB;

    

    //Save state info

    prevStates.push(pos.st);

    //Update castlingrights based on the move using castlingModifier board

    if (pos.st.castlingRights) {
        //Handles if rook is captured or moves
        pos.st.castlingRights &= castlingModifiers[from];
        pos.st.castlingRights &= castlingModifiers[to];
        if (flags == CASTLE_KING) {
            doCastle<whiteToMove, true>(pos);
        }
        else if (flags == CASTLE_QUEEN) {
            doCastle<whiteToMove, false>(pos);
        }
    }

    //EP: remove pawn if there is a EP capture
    if constexpr (whiteToMove) {
        pos.pieceBoards[7] ^= (flags == EP_CAPTURE) * (1ULL << (to + 8));
        pos.st.enPassant = (flags == DOUBLE_PUSH) * (to + 8);
    }
    else {
        pos.pieceBoards[1] ^= (flags == EP_CAPTURE) * (1ULL << (to - 8));
        pos.st.enPassant = (flags == DOUBLE_PUSH) * (to - 8);
    }
    
    //Restore occupied
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];
    
    //Update state info
    
    pos.whiteToMove = !whiteToMove;


    //Prepare for next movegeneration
    MoveGen::findAttack<!whiteToMove>(pos);
    MoveGen::pinnedBoard<!whiteToMove>(pos);
    MoveGen::checks<!whiteToMove>(pos);
}


template<bool whiteToMove>
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

    const uint64_t fromBB = 1ULL << from;
    const uint64_t toBB = 1ULL << to;
    const uint64_t capMask = toBB * (flags == CAPTURE);

    //Incrementally update position
    if constexpr (whiteToMove) {
        pos.teamBoards[2] ^= fromBB | toBB;
        pos.teamBoards[1] |= capMask;
    }
    else {
        pos.teamBoards[1] ^= fromBB | toBB;
        pos.teamBoards[2] |= capMask;
    }
    pos.pieceBoards[captured] |= capMask;
    pos.pieceBoards[mover] ^= fromBB | toBB;
    
    //EP: add the ep pawn if there was an ep capture
    if constexpr (whiteToMove) {
        pos.pieceBoards[1] ^= (flags == EP_CAPTURE) * (1ULL << (to - 8));
    }
    else {
        pos.pieceBoards[7] ^= (flags == EP_CAPTURE) * (1ULL << (to + 8));
    }
    

    if (pos.st.castlingRights) {
        if (flags == CASTLE_KING) {
            doCastle<!whiteToMove, true>(pos);
        }
        else if (flags == CASTLE_QUEEN) {
            doCastle<!whiteToMove, false>(pos);
        }
    }
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];
    
    pos.whiteToMove = !whiteToMove;
}

template<bool whiteToMove, bool castleKing>
inline void Engine::doCastle(Position& pos) {
    if constexpr (whiteToMove) {
        constexpr uint64_t rookFromTo = castleKing ? 1ULL << 63 | 1ULL << 61 : 1ULL << 56 | 1ULL << 59;
        pos.pieceBoards[4] ^= rookFromTo;
    }
    else {
        constexpr uint64_t rookFromTo = castleKing ? 1ULL << 7 | 1ULL << 5 : 1ULL << 0 | 1ULL << 3;
        pos.pieceBoards[10] ^= rookFromTo;
    }
}







void Engine::fenInit(Position& pos, std::string fen) {
    uint32_t i = 0;
    pos = {};
    for (char f : fen) {
        if (i < 64) {
            switch (f) { 
            case 'K': pos.pieceBoards[0] = 1ULL << i; break;
            case 'P': pos.pieceBoards[1] |= 1ULL << i; break;
            case 'N': pos.pieceBoards[2] |= 1ULL << i; break;
            case 'B': pos.pieceBoards[3] |= 1ULL << i; break;
            case 'R': pos.pieceBoards[4] |= 1ULL << i; break;
            case 'Q': pos.pieceBoards[5] |= 1ULL << i; break;
            case 'k': pos.pieceBoards[6] = 1ULL << i; break;
            case 'p': pos.pieceBoards[7] |= 1ULL << i; break;
            case 'n': pos.pieceBoards[8] |= 1ULL << i; break;
            case 'b': pos.pieceBoards[9] |= 1ULL << i; break;
            case 'r': pos.pieceBoards[10] |= 1ULL << i; break;
            case 'q': pos.pieceBoards[11] |= 1ULL << i; break;
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
            default: break;
            }
        }
    }
    for (int i = 0; i < 6; i++) {
        pos.teamBoards[1] |= pos.pieceBoards[i];
        pos.teamBoards[2] |= pos.pieceBoards[i + 6];
    }
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

    pos.st.enPassant = NoEP;
    if (pos.whiteToMove) {
        MoveGen::pinnedBoard<true>(pos);
        MoveGen::checks<true>(pos);
        MoveGen::findAttack<true>(pos);
    }
    else {
        MoveGen::pinnedBoard<false>(pos);
        MoveGen::checks<false>(pos);
        MoveGen::findAttack<false>(pos);
    }
    
}

