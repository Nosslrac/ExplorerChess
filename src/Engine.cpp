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


	if (strcmp(command.c_str(), "fen") == 0) {
        //fenInit(_pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        fenInit(_pos, "3b4/2P2P2/8/1pp5/3b4/1P1P4/4P3/8 w - - 0 1");
        GUI::print_pieces(_pos);
	}
    else if (strcmp(command.c_str(), "d") == 0) {
        MoveGen::pinnedBoard<false>(_pos);
        MoveGen::checks<false>(_pos);
        MoveList ml;
        MoveGen::generatePawnMoves<true, false, false>(_pos, ml);
        for (int i = 0; i < 20; ++i) {
            const uint32_t move = ml.moves[i];
            GUI::print_bit_board(1ULL << (move & 0x3F) | 1ULL << ((move >> 6) & 0x3F));
        }
        
    }
    else if (strcmp(command.c_str(), "g") == 0) {
        MoveList m;
        MoveGen::generateAllMoves<true>(_pos, m);
    }
}

template<bool whiteToMove, bool castling>
void Engine::doMove(Position& pos, uint32_t move) {
    //Incrementally update position
    const uint64_t fromTo = getFromTo(move);
    pos.teamBoards[0] ^= fromTo;
    pos.teamBoards[2 - whiteToMove] ^= fromTo;
    pos.teamBoards[2 - !whiteToMove] &= ~(1ULL << (move & 0x3F));
    pos.whiteToMove = !pos.whiteToMove;


    //Save state info

    //Update state info
    constexpr int back = whiteToMove ? 8 : -8;
    pos.st.enPassant = (back + (move & 0x3F)) * (move == DOUBLE_PUSH);
    MoveGen::pinnedBoard(pos);
    MoveGen::checks(pos);
    

}



void Engine::fenInit(Position& pos, std::string fen) {
    uint32_t i = 0;
    pos = {};
    for (char f : fen) {
        if (i < 64) {
            switch (f) { 
            case 'K': pos.kings[1] = i; break;
            case 'P': pos.pieceBoards[0] |= 1ULL << i; break;
            case 'N': pos.pieceBoards[1] |= 1ULL << i; break;
            case 'B': pos.pieceBoards[2] |= 1ULL << i; break;
            case 'R': pos.pieceBoards[3] |= 1ULL << i; break;
            case 'Q': pos.pieceBoards[4] |= 1ULL << i; break;
            case 'k': pos.kings[0] = i; break;
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
            default: break;
            }
        }
    }
    for (int i = 0; i < 5; i++) {
        pos.teamBoards[1] |= pos.pieceBoards[i];
        pos.teamBoards[2] |= pos.pieceBoards[i + 5];
    }
    pos.teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];
    pos.st.enPassant = NoEP;

}

