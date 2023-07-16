#include "../inc/bitboardUtil.h"


//TODO:testing if casting to uint8_t makes a difference
const uint8_t long_bit_scan(uint64_t i){
	unsigned long r;
	_BitScanForward64(&r, i);
	return (uint8_t)r;
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


