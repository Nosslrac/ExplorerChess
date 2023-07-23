#include "../inc/GUI.h"

namespace GUI {
	void print_bit_board(uint64_t b) {

		std::string stb = "";
		stb += "--------------------\n";
		for (int i = 0; i < 8; i++) {
			stb += "| ";
			for (int j = 0; j < 8; j++) {
				stb += (b & 1) == 1 ? "x " : "0 ";
				b >>= 1;
			}
			stb += " |";
			std::cout << stb;
			std::cout << "\n";
			stb = "";
		}
		std::cout << "--------------------\n\n";
	}

	//Prints the pices in a given position
	void print_pieces(Position& pos) {
		char pBoard[64];
		fillPieceArray(pos, pBoard);
		std::string stb = "";
		stb += "--------------------\n";
		for (int i = 0; i < 8; i++) {
			stb += "| ";
			for (int j = 0; j < 8; j++) {
				stb += pBoard[i * 8 + j];
				stb += ' ';
			}
			stb += " |";
			std::cout << stb;
			std::cout << "\n";
			stb = "";
		}
		std::cout << "--------------------\n";
		getPositionFen(pos);
	}

	//Initiates a piece array for display in cmd
	void fillPieceArray(Position& pos, char pBoard[]) {
		std::fill_n(pBoard, 64, '-');
		for (int i = 0; i < 12; i++) {
			uint64_t pieces = pos.pieceBoards[i];
			unsigned long sq;
			while (pieces) {
				bitScan(&sq, pieces);
				pBoard[sq] = fenRepresentation[i];
				pieces &= pieces - 1;
			}
		}
	}



	//Returns the fen for the current position
	void getPositionFen(Position& pos) {
		char pBoard[64];
		fillPieceArray(pos, pBoard);

		int empty = 0;
		std::string fen = "Fen: ";

		for (int i = 0; i < 64; i++) {
			if (i % 8 == 0 && i != 0) {
				if (empty) {
					fen += '0' + empty;
					empty = 0;
				}
				fen += '/';
			}
			if (pBoard[i] == '-') {
				empty++;
			}
			else {
				if (empty) {
					fen += '0' + empty;
					empty = 0;
				}
				fen += pBoard[i];
			}
		}
		char info[20];
		const char side = fenMove[!pos.whiteToMove];
		char castle[4];
		uint8_t legal = pos.st.castlingRights;
		for (int i = 0; i < 4; i++) {
			castle[i] = (legal & 1) == 1 ? fenCastle[i] : 0;
			legal >>= 1;
		}
		sprintf_s(info, " %c %c%c%c%c - 0 1", side, castle[0], castle[1], castle[2], castle[3]);

		std::cout << fen << info << "\n\n";
	}

	void parseMove(uint32_t move) {
		char from = getFrom(move);
		char to = getTo(move);
		MoveFlags flags = (MoveFlags)getFlags(move);
		char mover = getMover(move);
		char capped = getCaptured(move);
		std::string fString;
		switch (flags)
		{
		case CAPTURE: fString = "CAPTURE"; break;
		case NO_FLAG: fString = "NO_FLAG"; break;
		case EP_CAPTURE: fString = "EP_CAPTURE"; break;
		case DOUBLE_PUSH: fString = "DOUBLE_PUSH"; break;

		default: fString = "PROMO";
			break;
		}
		char buff[100];
		sprintf_s(buff, "From: %d, To: %d, Mover: %d, Captured: %d, ", from, to, mover, capped);
		std::cout << buff << "Flags: " + fString << std::endl;

		GUI::print_bit_board(1ULL << from | 1ULL << to);
	}

	void printMove(uint32_t move) {
		int from = getFrom(move);
		int to = getTo(move);
		std::cout << (char)(from % 8 + 'a') << (char)('8' - from / 8)
			<< (char)('a' + to % 8) << (char)('8' - to / 8) << ": ";
	}

	void printState(StateInfo& st) {
		char buff[150];
		sprintf_s(buff, "Block: %llu, Pinned: %llu, Attack %llu, Checkers: %d, Castle: %d, Enpassant: %d", st.blockForKing, st.pinnedMask, st.enemyAttack, st.numCheckers, st.castlingRights, st.enPassant);
		std::cout << buff << std::endl;
	}
}

