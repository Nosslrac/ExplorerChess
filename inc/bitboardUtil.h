#pragma once
#include <x86intrin.h>
#include <cinttypes>
#include <iostream>
#include <string.h>

#define bitCount(b) __builtin_popcountll(b)
#define bitScan(BB) __builtin_ctzll(BB)
#define pext(BB, mask) _pext_u64(BB, mask)

#define getTo(move) (move & 0xFF)
#define getFrom(move) ((move >> 8) & 0xFF)
#define getFlags(move) (move & 0xFF0000)
#define getMover(move) ((move >> 24) & 0xF)
#define getCaptured(move) (move >> 28)
#define getPromo(move) ((move >> 16) & 0x3)
#define BB(i) (1ULL << i)




enum Piece {
	King = 10,
	Pawn = 0, 
	Knight = 1,
	Bishop = 2,
	Rook = 3,
	Queen = 4,
};

enum Direction {
	UP,
	DOWN,
	UP_LEFT,
	UP_RIGHT
};

enum Flags {
	QUIET = 0,
	DOUBLE_PUSH = 0x10000,
	CASTLE_KING = 0x20000,
	CASTLE_QUEEN = 0x30000,
	CAPTURE = 0x40000,
	EP_CAPTURE = 0x50000,
	PROMO_N = 0x80000,
	PROMO_B = 0x90000,
	PROMO_R = 0xA0000,
	PROMO_Q = 0xB0000,
	PROMO_NC = 0xC0000,
	PROMO_BC = 0xD0000,
	PROMO_RC = 0xE0000,
	PROMO_QC = 0xF0000
};

constexpr uint64_t FileA = 0x0101010101010101ULL;
constexpr uint64_t FileB = FileA << 1;
constexpr uint64_t FileC = FileA << 2;
constexpr uint64_t FileD = FileA << 3;
constexpr uint64_t FileE = FileA << 4;
constexpr uint64_t FileF = FileA << 5;
constexpr uint64_t FileG = FileA << 6;
constexpr uint64_t FileH = FileA << 7;

constexpr uint64_t Rank1 = 0xFF;
constexpr uint64_t Rank2 = Rank1 << (8 * 1);
constexpr uint64_t Rank3 = Rank1 << (8 * 2);
constexpr uint64_t Rank4 = Rank1 << (8 * 3);
constexpr uint64_t Rank5 = Rank1 << (8 * 4);
constexpr uint64_t Rank6 = Rank1 << (8 * 5);
constexpr uint64_t Rank7 = Rank1 << (8 * 6);
constexpr uint64_t Rank8 = Rank1 << (8 * 7);


constexpr uint8_t NoEP = 0;
constexpr uint64_t All_SQ = ~0ULL;
constexpr int CHECK_MATE = -0xFFFF;
constexpr uint32_t CHECK_FLAG = 0x80;

//---------CASTLING SQUARES--------------------------

constexpr uint64_t WHITE_QUEEN_PIECES = BB(59) | BB(58) | BB(57);
constexpr uint64_t WHITE_KING_PIECES = BB(61) | BB(62);
constexpr uint64_t BLACK_QUEEN_PIECES = BB(1) | BB(2) | BB(3);
constexpr uint64_t BLACK_KING_PIECES = BB(6) | BB(5);

//Attacked squares differ from occupied on queenside, also add king square since king can't be in check
constexpr uint64_t WHITE_ATTACK_QUEEN = WHITE_QUEEN_PIECES ^ BB(57) ^ BB(60);
constexpr uint64_t BLACK_ATTACK_QUEEN = BLACK_QUEEN_PIECES ^ BB(1) ^ BB(4);

constexpr uint64_t WHITE_ATTACK_KING = WHITE_KING_PIECES ^ BB(60);
constexpr uint64_t BLACK_ATTACK_KING = BLACK_KING_PIECES ^ BB(4);


//TODO: implement 50 move rule
struct StateInfo {
	uint64_t blockForKing;
	uint64_t pinnedMask;
	uint64_t enemyAttack;
	uint64_t checkers;

	//uint8_t numCheckers;
	uint8_t castlingRights;
	uint8_t enPassant;

	//Incremental
	uint64_t hashKey;
};

//Inherits irreversible info from StateInfo 
struct Position {
	StateInfo st;
	uint8_t kings[2];
	uint64_t pieceBoards[10];
	uint64_t teamBoards[3];

	//Check boards
	uint64_t checkSquares[4];


	bool whiteToMove;
	uint16_t ply;
	int materialValue;

	bool operator ==(const Position& pos) const{
		return memcmp(this, &pos, sizeof(Position)) == 0;
	}
};

struct MoveList {
	uint32_t moves[100] = {};
	uint8_t curr = 0;
	inline void add(uint32_t move){
		moves[curr++] = move;
	}
	inline uint8_t size() {
		return curr;
	}
};

struct Move {
	int eval;
	uint32_t move;
	bool operator<(const Move& move) const{
		return move.eval < eval;
	}
};


template<bool white>
const inline uint8_t getPiece(const uint64_t pieces[], uint8_t sq);


template<bool whiteToMove, Direction D>
uint64_t shift(uint64_t b) {
	if constexpr (whiteToMove) {
		return D == UP ? b >> 8 : D == DOWN ? b << 8 :
			D == UP_LEFT ? (b >> 9) & ~FileH : (b >> 7) & ~FileA;
	}
	if constexpr (!whiteToMove) {
		return D == UP ? b << 8 : D == DOWN ? b >> 8 :
			D == UP_LEFT ? (b << 7) & ~FileH : (b << 9) & ~FileA;
	}
}
//Squares infront of pawn
template<bool white>
inline uint64_t forwardSquares(uint8_t sq) {
	if constexpr (white) {
		return All_SQ >> ((8 - sq / 8) * 8);
	}
	else {
		return All_SQ << ((sq / 8) * 8 + 8);
	}
}






//Castling is blocked
template<bool white, bool kingSide>
bool isOccupied(uint64_t board) {
	if constexpr (white) {
		return kingSide ? WHITE_KING_PIECES & board : WHITE_QUEEN_PIECES & board;
	}
	else {
		return kingSide ? BLACK_KING_PIECES & board : BLACK_QUEEN_PIECES & board;
	}
}


template<bool white, bool kingSide>
bool isAttacked(uint64_t attack) {
	if constexpr (white) {
		return kingSide ? WHITE_KING_PIECES & attack : WHITE_ATTACK_QUEEN & attack;
	}
	else {
		return kingSide ? BLACK_KING_PIECES & attack : BLACK_ATTACK_QUEEN & attack;
	}
}


//Return possible ep capturers
template<bool whiteToMove>
inline uint64_t EPpawns(const Position& pos) {
	if constexpr (whiteToMove) return Rank4 & pos.pieceBoards[0];
	return Rank5 & pos.pieceBoards[5];
}


inline bool moreThanOne(uint64_t b) {
	return b & (b - 1);
}





constexpr uint8_t castlingModifiers[64] = {
	0b0111, 0b1111, 0b1111, 0b1111, 0b0011, 0b1111, 0b1111, 0b1011,
	0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
	0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
	0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
	0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
	0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
	0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
	0b1101, 0b1111, 0b1111, 0b1111, 0b1100, 0b1111, 0b1111, 0b1110
};



//---------------Bitboard functions------------------
   
const void initLineBB(uint64_t (& lineBB)[64][64]);
const uint64_t pinned_ray(int, int);

//------------------Files for init----------------------



constexpr uint64_t files[8] = { 0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
	0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL, 0x4040404040404040ULL,
	0x8080808080808080ULL };
constexpr uint64_t ranks[8] = { 0xFFLL, 0xFF00LL, 0xFF0000LL, 0xFF000000LL, 0xFF00000000LL, 0xFF0000000000LL,
		0xFF000000000000LL, 0xFF00000000000000LL };
constexpr uint64_t main_diagonals[15] = { 0x0100000000000000ULL, 0x0201000000000000ULL, 0x0402010000000000ULL, 0x0804020100000000ULL,
		0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
		0x80402010080402ULL, 0x804020100804ULL, 0x8040201008ULL, 0x80402010ULL, 0x804020ULL, 0x8040ULL, 0x80ULL };
constexpr uint64_t anti_diagonals[15] = { 0x1ULL, 0x0102ULL, 0x010204ULL, 0x01020408ULL, 0x0102040810ULL, 0x010204081020ULL, 0x01020408102040ULL,
		0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL, 0x1020408000000000ULL,
		0x2040800000000000ULL, 0x4080000000000000ULL, 0x8000000000000000ULL };

constexpr uint64_t adjacentFiles[8] = {files[0] | files[1], files[0] | files[1] | files[2], files[1] | files[2] | files[3], 
										files[2] | files[3] | files[4], files[3] | files[4] | files[5], files[4] | files[5] | files[6], 
										files[5] | files[6] | files[7], files[6] | files[7]};