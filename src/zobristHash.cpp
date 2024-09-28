#include "zobristHash.h"
#include "random"

void ZobristHash::initHash() {
    std::random_device                        os_seed;
    const uint32_t                            seed = os_seed();
    std::mt19937                              generator(seed);
    std::uniform_int_distribution<bitboard_t> distribute(0, UINT64_MAX);

    // Init piece hash
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 64; j++) {
            pieceHash[i][j] = distribute(generator);
        }
    }
    // Init king hash
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 64; j++) {
            kingHash[i][j] = distribute(generator);
        }
    }
    // Init ep hash
    for (int i = 0; i < 8; i++) {
        epHash[i] = distribute(generator);
    }
    // Castle hash and move hash
    for (int i = 0; i < 16; i++) {
        castleHash[i] = distribute(generator);
    }

    whiteToMoveHash = distribute(generator);
    moveHash        = distribute(generator);
}

bitboard_t ZobristHash::hashPosition(const Position &pos) {
    bitboard_t posHash = 0ULL;

    if (pos.whiteToMove) {
        posHash ^= whiteToMoveHash;
    }

    for (int i = 0; i < 10; ++i) {
        bitboard_t pieces = pos.pieceBoards[i];
        while (pieces) {
            int nextSquare = bitScan(pieces);
            posHash ^= pieceHash[i][nextSquare];
            pieces &= pieces - 1;
        }
    }

    posHash ^= kingHash[0][pos.kings[0]];
    posHash ^= kingHash[1][pos.kings[1]];

    posHash ^= castleHash[pos.st.castlingRights];

    if (pos.st.enPassant != 0U) {
        const uint8_t file = pos.st.enPassant & 7;
        posHash ^= epHash[file];
    }
    posHash ^= moveHash * pos.ply;

    return posHash;
}