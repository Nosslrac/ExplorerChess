#pragma once
#include "bitboardUtil.h"
#include <unordered_map>
#include "moveGen.h"
#include "GUI.h"

class Evaluation {
public:
	Evaluation(MoveGen* moveGen);
	~Evaluation() = default;

	template<bool whiteToMove>
	const int evaluate(const Position& pos) const;
private:
	const uint16_t pieceValue[5] = {100, 300, 300, 500, 900};

	//Material balance
	const int staticPieceEvaluation(const uint64_t piece[10]) const;


	//Static pawn structure evaluation, to be stored in a lookup table
	//- Isolated pawns
	//- Passed pawns (+ if it is protected by another pawn)
	template<bool whiteToMove>
	const int passedPawns(const uint64_t piece[10]) const;

	
	
	//Undefended pieces penalty

	//Outpost detection

	//Attacking potential
	template<bool whiteToMove>
	const int attackPotential(const uint64_t piece[10]) const;

	std::unordered_map<uint64_t, int> pawnStructure;

	//King safety (use some kind of pext for surrounding squares and then switch case maybe)
	//Also open lines based on oponents sliding pieces potential penalty
	template<bool whiteToMove>
	const int kingSafety(const Position& pos) const;

	//Undefended pieces penalty


	//MoveGen to look up attacks and moves
	MoveGen* _moveGen;
};