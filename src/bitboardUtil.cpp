#include "../inc/bitboardUtil.h"


//TODO:testing if casting to uint8_t makes a difference
const uint8_t long_bit_scan(uint64_t i){
	unsigned long r;
	_BitScanForward64(&r, i);
	return (uint8_t)r;
}


/*
* Brief: used to find the captured piece's ID in the pieceBoard array
* Note: Won't find the king, if it returns 0 something has gone wrong elsewhere
* Default will be white pawns
*/
template<bool white>
const inline uint8_t getPiece(const uint64_t pieces[], uint8_t sq) {
	const uint64_t fromBB = BB(sq);
	for (int i = 5 * !white; i < 5 + 5 * !white; ++i) {
		if (pieces[i] & fromBB)
			return i;
	}
	return King;
}





const void initLineBB(uint64_t (&lineBB)[64][64]) {
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j)
			lineBB[i][j] = pinned_ray(i, j);
	}
}




//Find the ray in which the piece is able to move
const uint64_t pinned_ray(int king, int piece) {
	//On the same rank
	if (king / 8 == piece / 8)
		return ranks[king / 8];
	//On the same file
	if (king % 8 == piece % 8)
		return files[king % 8];
	const int diagonal = king / 8 + king % 8;
	//On the same anti-diagonal
	if (diagonal == piece / 8 + piece % 8)
		return anti_diagonals[diagonal];
	//Otherwise on the same main-diagonal
	if ((king % 8 - king / 8) == (piece % 8 - piece / 8))
		return main_diagonals[7 - king / 8 + king % 8];
	return 0;
}


template const uint8_t getPiece<false>(const uint64_t[], uint8_t);

template const uint8_t getPiece<true>(const uint64_t[], uint8_t);