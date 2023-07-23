#pragma once
#include <intrin.h>
#include <cinttypes>
#include <iostream>
#include "bitboardUtil.h"

namespace GUI{
	//Fen representation
	constexpr char fenRepresentation[12] = { 'K', 'P', 'N', 'B', 'R', 'Q', 'k', 'p', 'n', 'b', 'r', 'q' };
	constexpr char fenCastle[4] = { 'K', 'Q', 'k', 'q' };
	constexpr char fenMove[2] = { 'w', 'b' };



	//Functions
	/** Prints bitboard b in the console with set bits as x
	* @param b 
	*/
	void print_bit_board(uint64_t b);
	/** Prints board with pieces as their fen representation
	* @param square position info
	*/
	void print_pieces(Position& pos);
	/** Prints the fen for the position
	* @param square position info
	*/
	void getPositionFen(Position& pos);
	/** Helper function to printpieces
	* @param square position info
	* @param pBoard array to store fen representation of the pieces
	*/
	void fillPieceArray(Position& pos, char pBoard[]);

	void parseMove(uint32_t move);

	void printMove(uint32_t move);

	void printState(StateInfo& st);
}











