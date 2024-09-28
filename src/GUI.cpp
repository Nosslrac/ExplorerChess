#include "GUI.h"
namespace GUI {
void print_bit_board(bitboard_t b) {

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

// Prints the pices in a given position
void print_pieces(const Position &pos) {
    char pBoard[64];
    fillPieceArray(pos, pBoard);
    char        rank = '8';
    std::string stb  = "";
    stb += "\n +---+---+---+---+---+---+---+---+\n";
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            stb += " | ";
            stb += pBoard[i * 8 + j];
        }
        stb += " | ";
        stb += rank;
        rank--;
        std::cout << stb;
        std::cout << "\n +---+---+---+---+---+---+---+---+\n";
        stb = "";
    }
    std::cout << "   a   b   c   d   e   f   g   h\n\n";
    getPositionFen(pos);
    std::string checker = "";
    GUI::getCheckers(checker, pos.st.checkers);
    std::cout << "\nHash key: " << pos.st.hashKey << "\nChecker: " << checker
              << "\n\n";
}

// Initiates a piece array for display in cmd
void fillPieceArray(const Position &pos, char pBoard[]) {
    std::fill_n(pBoard, 64, ' ');
    for (int i = 0; i < 10; i++) {
        bitboard_t    pieces = pos.pieceBoards[i];
        unsigned long sq;
        while (pieces) {
            sq         = bitScan(pieces);
            pBoard[sq] = fenRepresentation[i];
            pieces &= pieces - 1;
        }
    }
    pBoard[pos.kings[0]] = 'K';
    pBoard[pos.kings[1]] = 'k';
}


// Returns the fen for the current position
void getPositionFen(const Position &pos) {
    char pBoard[64];
    fillPieceArray(pos, pBoard);

    int         empty = 0;
    std::string fen   = "Fen: ";

    for (int i = 0; i < 64; i++) {
        if (i % 8 == 0 && i != 0) {
            if (empty) {
                fen += '0' + empty;
                empty = 0;
            }
            fen += '/';
        }
        if (pBoard[i] == ' ') {
            empty++;
        } else {
            if (empty) {
                fen += '0' + empty;
                empty = 0;
            }
            fen += pBoard[i];
        }
    }
    char       info[25];
    const char side = fenMove[!pos.whiteToMove];
    char       castle[4];
    uint8_t    legal = pos.st.castlingRights;
    for (int i = 0; i < 4; i++) {
        castle[i] = (legal & 1) == 1 ? fenCastle[i] : '-';
        legal >>= 1;
    }
    char ep[2] = {'-', 0};


    if (pos.st.enPassant != 0) {
        char file = 'a' + pos.st.enPassant % 8;
        char rank = '0' + 8 - pos.st.enPassant / 8;
        ep[0]     = file;
        ep[1]     = rank;
    }
    printf(info, " %c %c%c%c%c %c%c 0 1", side, castle[0], castle[1], castle[2],
           castle[3], ep[0], ep[1]);

    std::cout << fen << info;
}

void parseMove(uint32_t move) {
    char  from  = getFrom(move);
    char  to    = getTo(move);
    Flags flags = (Flags)getFlags(move);
    char  mover = getMover(move);

    char        capped = getCaptured(move);
    std::string fString;
    switch (flags) {
    case CAPTURE:
        fString = "CAPTURE";
        break;
    case QUIET:
        fString = "QUIET";
        break;
    case EP_CAPTURE:
        fString = "EP_CAPTURE";
        break;
    case DOUBLE_PUSH:
        fString = "DOUBLE_PUSH";
        break;
    case CASTLE_KING:
        fString = "CASTLE_KING";
        break;
    case CASTLE_QUEEN:
        fString = "CASTLE_QUEEN";
        break;
    default:
        fString = "PROMO";
        break;
    }
    char buff[44];
    std::snprintf(buff, 44, "From: %d, To: %d, Mover: %d, Captured: %d, ", from,
                  to, mover, capped);
    std::cout << buff << "Flags: " + fString << std::endl;

    GUI::print_bit_board(BB(from) | BB(to));
}

void printMove(uint32_t move) {
    std::string promo = "";
    if (move & PROMO_N) {
        switch (getPromo(move)) {
        case 0:
            promo = "n";
            break;
        case 1:
            promo = "b";
            break;
        case 2:
            promo = "r";
            break;
        default:
            promo = "q";
            break;
        }
    }
    int from = getFrom(move);
    int to   = getTo(move);
    std::cout << (char)(from % 8 + 'a') << (char)('8' - from / 8)
              << (char)('a' + to % 8) << (char)('8' - to / 8) << promo;
}

void printState(StateInfo &st) {
    char buff[150];

    printf(buff,
           "Block: %llu, Pinned: %llu, Attack %llu, Checkers: %llu, Castle: "
           "%d, Enpassant: %d",
           st.blockForKing, st.pinnedMask, st.enemyAttack, st.checkers,
           st.castlingRights, st.enPassant);
    std::cout << buff << std::endl;
}

void getCheckers(std::string &checkerSQ, bitboard_t checkerBB) {
    unsigned long sq;
    while (checkerBB) {
        sq = bitScan(checkerBB);
        checkerSQ += (char)(sq % 8 + 'a');
        checkerSQ += (char)('8' - sq / 8);
        checkerSQ += ' ';
        checkerBB &= checkerBB - 1;
    }
}

uint32_t findMove(MoveList &ml, std::string move) noexcept {
    uint32_t from      = (move.at(0) - 'a') + (8 - move.at(1) + '0') * 8;
    uint32_t to        = (move.at(2) - 'a') + (8 - move.at(3) + '0') * 8;
    uint8_t  promo     = 0;
    bool     skipPromo = true;
    if (move.length() >= 5) {
        switch (move.at(4)) {
        case 'b':
            promo = 1;
            break;
        case 'r':
            promo = 2;
            break;
        case 'q':
            promo = 3;
            break;
        }
        skipPromo = false;
    }
    for (int i = 0; i < ml.size(); ++i) {
        const uint32_t curr = ml.moves[i];
        if (getFrom(curr) == from && getTo(curr) == to) {
            if (skipPromo || getPromo(curr) == promo) {
                return curr;
            }
        }
    }
    return 0;
}

} // namespace GUI
