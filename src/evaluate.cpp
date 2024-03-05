#include "../inc/evaluate.h"


Evaluation::Evaluation(MoveGen* moveGen) {
	//Init all stuff
	_moveGen = moveGen;
}



template<bool whiteToMove>
const int Evaluation::evaluate(const Position& pos) const {
	const int safety = Evaluation::kingSafety<whiteToMove>(pos);
	const int passedPawn = Evaluation::passedPawns<whiteToMove>(pos.pieceBoards);
	constexpr int flip = whiteToMove ? 1 : -1;
	return (pos.st.materialScore + pos.st.materialValue) * flip;
}



const int Evaluation::staticPieceEvaluation(const uint64_t pieces[10]) const{
	int materialBalance = 0;
	for (int i = 0; i < 5; ++i) {
		materialBalance += (bitCount(pieces[i]) - bitCount(pieces[i + 5])) * pieceValue[i];
	}
	return materialBalance;
}


const int Evaluation::materialValue(const Position& pos) const{
	int whiteValue = 0;
	int blackValue = 0;
	for(uint8_t i = 0; i < 64; ++i){
		if(pos.kings[0] == i){
			whiteValue += PSTs[10][i];
		}
		else if(pos.kings[1] == i){
			blackValue += PSTs[11][i];
		}
		else if(pos.teamBoards[1] & BB(i)){
			whiteValue += PSTs[getPiece<true>(pos.pieceBoards, i)][i];
		}
		else if(pos.teamBoards[2] & BB(i)){
			blackValue += PSTs[getPiece<false>(pos.pieceBoards, i)][i];
		}
 	}
	return whiteValue - blackValue;
}





template<bool whiteToMove>
const int Evaluation::kingSafety(const Position& pos) const {
	//Idea: extract surrounding bits for the king if there are a certain amount of sliding attackers
	//Problem not sure what to do what should be considered since opponent has to exploit it
	//See if there are a lot of pins close to king

	constexpr uint8_t kingID = whiteToMove ? 0 : 1;
	int score = 0;
	const int king = pos.kings[kingID];
	const uint64_t bishopKnight = pos.pieceBoards[1 + 5 * kingID] | pos.pieceBoards[2 + 5 * kingID]; //Good pieces for blocking
	const uint64_t kingSquares = _moveGen->stepAttackBB<King>(king);
	const uint64_t kingShield = forwardSquares<whiteToMove>(king) & kingSquares; //Squares right infront of king

	score -= bitCount(pos.st.enemyAttack & kingSquares) * 10; //Penalty if enemy is attacking king shield

	//Not sure what to do with the result
	//auto teamShield = pext(bishopKnight, kingShield);
	//auto pawnShield = pext(pos.pieceBoards[5 * kingID], kingShield);
	//std::cout << "King shield: " << teamShield << pawnShield << "\n";
	
	return score;
}

template<bool whiteToMove>
const int Evaluation::passedPawns(const uint64_t piece[10]) const {
	//Detect the files in front and the adjacent
	constexpr uint8_t team = whiteToMove ? 0 : 5;
	constexpr uint8_t enemy = whiteToMove ? 5 : 0;
	uint64_t pawns = piece[team];
	const uint64_t enemyPawns = piece[enemy];
	int score = 0;
	unsigned long sq;
	while (pawns) {
		sq = bitScan(pawns);
		const uint64_t forwardSQ = adjacentFiles[sq & 7] & forwardSquares<whiteToMove>(sq);
		if (forwardSQ & enemyPawns) { //Passer
			score += 10;
			const uint64_t defenders = shift<!whiteToMove, UP_LEFT>(BB(sq)) | shift<!whiteToMove, UP_RIGHT>(BB(sq));
			if (defenders & piece[team]) { //Protected passer
				score += 20;
			}
		}
		pawns &= pawns - 1;
	}
	return score;
}




template const int Evaluation::evaluate<true>(const Position&) const;
template const int Evaluation::evaluate<false>(const Position&) const;