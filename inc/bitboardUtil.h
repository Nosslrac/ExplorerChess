#pragma once
#include <intrin.h>
#include <cinttypes>
#include <iostream>

#define bitCount(b) __popcnt64(b)
#define bitScan(i, BB) _BitScanForward64(i, BB)


enum Piece {
	Pawn, 
	Knight,
	Bishop,
	Rook,
	Queen,
	King
};

enum Direction {
	UP,
	DOWN,
	UP_LEFT,
	UP_RIGHT
};

enum MoveFlags {
	NO_FLAG = 0,
	DOUBLE_PUSH = 0x1000,
	CASTLE_KING = 0x2000,
	CASTLE_QUEEN = 0x3000,
	CAPTURE = 0x4000,
	EP_CAPTURE = 0x5000,
	PROMO_N = 0x8000,
	PROMO_B = 0x9000,
	PROMO_R = 0xA000,
	PROMO_Q = 0xB000,
	PROMO_NC = 0xC000,
	PROMO_BC = 0xD000,
	PROMO_RC = 0xE000,
	PROMO_QC = 0xF000
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


constexpr uint8_t NoEP = 0x10;
constexpr uint64_t All_SQ = ~0ULL;

//TODO: implement 50 move rule
struct StateInfo {
	uint64_t blockForKing;
	uint64_t pinnedMask;
	uint64_t enemyAttack;

	uint8_t numCheckers;
	uint8_t castlingRights;
	uint8_t enPassant;
};

//Inherits irreversible info from StateInfo 
struct Position {
	StateInfo st;
	uint8_t kings[2];
	uint64_t pieceBoards[10];
	uint64_t teamBoards[3];

	bool whiteToMove;
};

struct MoveList {
	uint32_t moves[100] = {};
	uint32_t* curr = moves;
	inline void add(uint32_t move){
		*curr++ = move;
	}
};





template<bool whiteToMove, Direction D>
constexpr uint64_t shift(uint64_t b) {
	if constexpr (whiteToMove) {
		return D == UP ? b >> 8 : D == DOWN ? b << 8 :
			D == UP_LEFT ? (b >> 9) & ~FileH : (b >> 7) & ~FileA;
	}
	if constexpr (!whiteToMove) {
		return D == UP ? b << 8 : D == DOWN ? b >> 8 :
			D == UP_LEFT ? (b << 7) & ~FileH : (b << 9) & ~FileA;
	}
}

//Return possible ep capturers
template<bool whiteToMove>
constexpr inline uint64_t EPpawns(const Position& pos) {
	if constexpr (whiteToMove) return Rank4 & pos.pieceBoards[0];
	return Rank5 & pos.pieceBoards[5];
}

constexpr inline uint64_t getFromTo(uint32_t move) {
	return 1ULL << (move & 0x3F) | 1ULL << ((move >> 6) & 0x3F);
}

constexpr inline bool moreThanOne(uint64_t b) {
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
const uint8_t long_bit_scan(uint64_t i);
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