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
		for (int i = 0; i < 10; i++) {
			uint64_t pieces = pos.pieceBoards[i];
			while (pieces) {
				const uint8_t index = long_bit_scan(pieces);
				pBoard[index] = fenRepresentation[i];
				pieces &= pieces - 1;
			}
		}
		pBoard[pos.kings[1]] = 'K';
		pBoard[pos.kings[0]] = 'k';
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
}

