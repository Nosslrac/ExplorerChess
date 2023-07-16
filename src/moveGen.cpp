#include "../inc/moveGen.h"
#include "../inc/GUI.h"



MoveGen::MoveGen() : magicalBits::MagicalBitboards() {
	initLineBB(LineBB);
}


//--------------------Move generation-----------------------------------------

template<bool whiteToMove>
const void MoveGen::generateAllMoves(const Position& pos, MoveList& move_list) const{
	//TODO: king moves


	const bool pins = pos.st.pinnedMask;
	const bool enPassant = pos.st.enPassant;

	if (pins) {
		generateKnightMoves<whiteToMove, true>(pos, move_list);
		generatePieceMoves<whiteToMove, Bishop, true>(pos, move_list);
		generatePieceMoves<whiteToMove, Rook, true>(pos, move_list);
		generatePieceMoves<whiteToMove, Queen, true>(pos, move_list);
		if (enPassant) {
			generatePawnMoves<whiteToMove, true, true>(pos, move_list);
		}
		else {
			generatePawnMoves<whiteToMove, true, false>(pos, move_list);
		}
	}
	else {
		generateKnightMoves<whiteToMove, false>(pos, move_list);
		generatePieceMoves<whiteToMove, Bishop, false>(pos, move_list);
		generatePieceMoves<whiteToMove, Rook, false>(pos, move_list);
		generatePieceMoves<whiteToMove, Queen, false>(pos, move_list);
		if (enPassant) {
			generatePawnMoves<whiteToMove, false, true>(pos, move_list);
		}
		else {
			generatePawnMoves<whiteToMove, false, false>(pos, move_list);
		}
	}

}



//TODO: maybe inline makePawnMove, it is not that long
template<bool whiteToMove, bool pins, bool enPassant>
const void MoveGen::generatePawnMoves(const Position& pos, MoveList& move_list) const {
	constexpr bool them = !whiteToMove;
	constexpr uint64_t promoRank = whiteToMove ? Rank2 : Rank7;
	constexpr uint64_t doublePotential = whiteToMove ? Rank6 : Rank3;
	constexpr int8_t back = whiteToMove ? 8 : -8;


	uint64_t pawns = getPieces<whiteToMove, Pawn>(pos);
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
	const uint64_t nonOccupied = ~pos.teamBoards[0];
	const uint64_t block = pos.st.blockForKing;

	if constexpr (enPassant) {
		const uint8_t king = pos.kings[whiteToMove];
		const uint8_t EP = pos.st.enPassant;
		uint64_t capturers = pawnAttackBB<!whiteToMove>(1ULL << EP) & pawns;
		//Handle pinned ep cappers differently
		uint64_t pinned = capturers & pos.st.pinnedMask;
		capturers &= ~pinned;

		unsigned long fromSQ;
		while (capturers) {
			bitScan(&fromSQ, capturers);
			capturers &= capturers - 1;
			constructMove<false>(move_list, fromSQ, EP, EP_CAPTURE);

		}

		while (pinned) {
			bitScan(&fromSQ, capturers);
			pinned &= pinned - 1;
			if ((1ULL << fromSQ) & LineBB[fromSQ][king]) {
				constructMove<false>(move_list, fromSQ, EP, EP_CAPTURE);
			}
		}
	}

	//Non promotion push
	const uint64_t nonPromo = pawns & ~promoRank;
	uint64_t push = shift<whiteToMove, UP>(nonPromo) & nonOccupied & block;
	uint64_t doublePush = shift<whiteToMove, UP>(push & doublePotential) & nonOccupied & block;

	makePawnMove<pins, false>(pos, move_list, push, back, NO_FLAG);
	makePawnMove<pins, false>(pos, move_list, doublePush, back * 2, DOUBLE_PUSH);


	//Captures
	uint64_t capLeft = shift<whiteToMove, UP_LEFT>(nonPromo) & enemy & block;
	uint64_t capRight = shift<whiteToMove, UP_RIGHT>(nonPromo) & enemy & block;
	constexpr uint8_t backLeft = whiteToMove ? 9 : -7;
	constexpr uint8_t backRight = whiteToMove ? 7 : -9;

	makePawnMove<pins, false>(pos, move_list, capLeft, backLeft, CAPTURE);
	makePawnMove<pins, false>(pos, move_list, capRight, backRight, CAPTURE);

	//Promotions
	uint64_t promo = pawns & promoRank;
	if (promo) {
		uint64_t push = shift<whiteToMove, UP>(promo) & nonOccupied & block;
		uint64_t capLeft = shift<whiteToMove, UP_LEFT>(promo) & enemy & block;
		uint64_t capRight = shift<whiteToMove, UP_RIGHT>(promo) & enemy & block;

		makePawnMove<pins, true>(pos, move_list, push, back, PROMO_NC);
		makePawnMove<pins, true>(pos, move_list, capLeft, backLeft, PROMO_NC);
		makePawnMove<pins, true>(pos, move_list, capRight, backRight, PROMO_NC);
	}
	

}


template<bool whiteToMove, Piece p, bool pins>
const void MoveGen::generatePieceMoves(const Position& pos, MoveList& move_list) const {
	uint64_t bishops = getPieces<whiteToMove, p>(pos);
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
	
	unsigned long b;
	while (bishops) {
		bitScan(&b, bishops);
		bishops &= bishops - 1;
		uint64_t moves = attackBB<p>(pos.teamBoards[0], b);
		if constexpr (pins) {
			if ((1ULL << b) & pos.st.pinnedMask) {
				moves &= LineBB[b][pos.kings[whiteToMove]];
			}
		}
		makePieceMove(move_list, moves & enemy, b, CAPTURE);
		makePieceMove(move_list, moves & ~enemy, b, NO_FLAG);
	}
}

template<bool whiteToMove, bool pins>
const void MoveGen::generateKnightMoves(const Position& pos, MoveList& move_list) const {
	uint64_t knights = getPieces<whiteToMove, Knight>(pos);
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
	if constexpr (pins) {
		 knights &= ~pos.st.pinnedMask;
	}
	unsigned long kn;
	while (knights) {
		bitScan(&kn, knights);
		knights &= knights - 1;
		const uint64_t moves = stepAttackBB<Knight>(kn);
		makePieceMove(move_list, moves & enemy, kn, CAPTURE);
		makePieceMove(move_list, moves & ~enemy, kn, NO_FLAG);
	}
}

//-----------------------Movemaking helper methods-----------------------------------------------------

template<bool isPromotion>
const inline void MoveGen::constructMove(MoveList& move_list, uint8_t from, uint8_t to, MoveFlags flag) const{
	move_list.add(to | (from << 6) | flag);
	if constexpr (isPromotion) {
		move_list.add(to | (from << 6) | PROMO_B);
		move_list.add(to | (from << 6) | PROMO_R);
		move_list.add(to | (from << 6) | PROMO_Q);
	}
}

template<bool pin, bool isPromotion>
const void MoveGen::makePawnMove(const Position& pos, MoveList& move_list, uint64_t toSQs, uint8_t back, MoveFlags flag) const {

	unsigned long toSQ;
	while (toSQs) {
		bitScan(&toSQ, toSQs);
		toSQs &= toSQs - 1;
		if constexpr (pin) {
			const uint8_t from = toSQ + back;
			//Only add if the pawn isn't pinned or the move moves along the pin
			if (((1ULL << from) & pos.st.pinnedMask) == 0 || ((1ULL << toSQ) & LineBB[from][pos.kings[pos.whiteToMove]])) {
				constructMove<isPromotion>(move_list, from, toSQ, flag);
			}

		}
		else {
			constructMove<isPromotion>(move_list, toSQ + back, toSQ, flag);
		}
	}
}

const inline void MoveGen::makePieceMove(MoveList& move_list, uint64_t toSQs, uint8_t from, MoveFlags flag) const{
	unsigned long toSQ;
	while (toSQs) {
		bitScan(&toSQ, toSQs);
		toSQs &= toSQs - 1;
		constructMove<false>(move_list, from, toSQ, flag);
	}
}




//--------------------Pins and checks-------------------------------------

template<bool whiteToMove>
const void MoveGen::checks(Position& pos){
	uint64_t checkers = 0;
	const uint8_t king = pos.kings[whiteToMove];

	//Opposite color knights
	const uint64_t knightCheck = stepAttackBB<Knight>(king) & getPieces<!whiteToMove, Knight>(pos);
	const uint64_t pawnCheck = pawnAttackBB<Pawn>(1ULL << king) & getPieces<!whiteToMove, Pawn>(pos);

	//Is either a knight or pawn, cannot be both so bool is no problem
	pos.st.numCheckers += (const bool)(knightCheck | pawnCheck);

	pos.st.blockForKing |= knightCheck | pawnCheck;
	//If there is no check then all squares should be available
	const bool isBlock = pos.st.blockForKing;
	pos.st.blockForKing |= !isBlock * All_SQ;
}


/** @brief Finds pinned pieces and their corresponding pinned ray.
*	It also finds checks given by sliders
*
*/
template<bool whiteToMove>
const void MoveGen::pinnedBoard(Position& pos){
	uint64_t pinned = 0;
	uint64_t blockForKing = 0;
	uint8_t numCheckers = 0;
	const uint8_t king = pos.kings[whiteToMove];

	uint64_t snipers = ((emptyAttack<Rook>(king) & (getPieces<!whiteToMove, Rook>(pos) | getPieces<!whiteToMove, Queen>(pos))) 
					  | emptyAttack<Bishop>(king) & (getPieces<!whiteToMove, Bishop>(pos) | getPieces<!whiteToMove, Queen>(pos)));

	const uint64_t occupancy = pos.teamBoards[0];
	const uint64_t team = getTeam<whiteToMove>(pos);
	
	while (snipers) {
		//Pop first sniper
		uint8_t sq = long_bit_scan(snipers);
		snipers &= snipers - 1;

		//Find squares between the pieces and the pieces
		const uint64_t betweenSquares = betweenBB(sq, king);
		const uint64_t betweenPieces = betweenSquares & occupancy;

		//It is a check if there is no piece between
		const bool isCheck = !bitCount(betweenPieces);
		//It is a pin if there is excactly one piece from the kings side between
		const bool isPin = (bitCount(betweenPieces) == 1) && (betweenPieces & team);

		//Update masks
		blockForKing |= isCheck * (betweenSquares | (1ULL << sq));
		pinned |= isPin * (betweenSquares | (1ULL << sq));
		//Update checkers
		numCheckers += isCheck;
	}
	//Update pos
	pos.st.blockForKing = blockForKing;
	pos.st.pinnedMask = pinned;
	pos.st.numCheckers = numCheckers;
}







//Template instanciation (remove later when they are actually called)
template const uint64_t MoveGen::attackBB<Bishop>(uint64_t, uint8_t) const;
template const uint64_t MoveGen::attackBB<Rook>(uint64_t, uint8_t) const;
template const uint64_t MoveGen::attackBB<Queen>(uint64_t, uint8_t) const;

template const void MoveGen::pinnedBoard<true>(Position&);
template const void MoveGen::pinnedBoard<false>(Position&);
template const void MoveGen::checks<true>(Position&);
template const void MoveGen::checks<false>(Position&);

template const void MoveGen::generateAllMoves<true>(const Position&, MoveList&) const;
template const void MoveGen::generateAllMoves<false>(const Position&, MoveList&) const;


template const void MoveGen::generatePawnMoves<true, true, true>(const Position& pos, MoveList& move_list) const;
template const void MoveGen::generatePawnMoves<true, true, false>(const Position& pos, MoveList& move_list) const;
template const void MoveGen::generatePawnMoves<true, false, true>(const Position& pos, MoveList& move_list) const;
template const void MoveGen::generatePawnMoves<true, false, false>(const Position& pos, MoveList& move_list) const;
//template const void MoveGen::generatePawnMoves<true, true, true>(const Position& pos, MoveList& move_list) const;


//---------------------Attack lookups---------------------------------


template<Piece p>
const inline uint64_t MoveGen::stepAttackBB(uint8_t square) const{
	if constexpr (p == Knight) {
		return knightLookUp[square];
	}
	if constexpr (p == King) {
		return kingLookUp[square];
	}
}

template<Piece p>
const inline uint64_t MoveGen::attackBB(uint64_t board, uint8_t square) const {
	if constexpr (p == Bishop) {
		return bishopAttack(board, square);
	}
	if constexpr (p == Rook) {
		return rookAttack(board, square);
	}
	if constexpr (p == Queen) {
		return rookAttack(board, square) | bishopAttack(board, square);
	}
}

const inline uint64_t MoveGen::bishopAttack(uint64_t board, uint8_t square) const{
	board &= m_bishopMasks[square];
	board *= m_bishopMagicBitboard[square];
	board >>= (64 - m_occupacyCountBishop[square]);
	return m_bishopAttacks[square][board];
}

const inline uint64_t MoveGen::rookAttack(uint64_t board, uint8_t square) const{
	board &= m_rookMasks[square];
	board *= m_rookMagicBitboard[square];
	board >>= (64 - m_occupacyCountRook[square]);
	return m_rookAttacks[square][board];
}







