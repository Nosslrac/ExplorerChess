#include "../inc/moveGen.h"
#include "../inc/GUI.h"



MoveGen::MoveGen() : magicalBits::MagicalBitboards() {
	initLineBB(LineBB);
}


//--------------------Move generation-----------------------------------------

template<bool whiteToMove, bool castling, bool pins, bool enPassant>
const void MoveGen::generateMoves(const Position& pos, MoveList& move_list) const{

	if (pos.st.numCheckers < 2) {
		generatePawnMoves<whiteToMove, pins, enPassant>(pos, move_list);
		generateKnightMoves<whiteToMove, pins>(pos, move_list);
		generatePieceMoves<whiteToMove, Bishop, pins>(pos, move_list);
		generatePieceMoves<whiteToMove, Rook, pins>(pos, move_list);
		generatePieceMoves<whiteToMove, Queen, pins>(pos, move_list);
	}

	//TODO: make kingmoves generator
	generateKingMoves<whiteToMove, castling>(pos, move_list);

}


template<bool whiteToMove>
const bool MoveGen::generateAllMoves(const Position& pos, MoveList& move_list) const{

	constexpr uint8_t shift = (2 * !whiteToMove);
	const uint8_t castling = (bool)(0b11 & (pos.st.castlingRights >> shift)) * 0b100;
	const uint8_t pins = (bool)(pos.st.pinnedMask) * 0b010;
	const uint8_t enPassant = (bool)(pos.st.enPassant) * 0b001;

	switch (castling | pins | enPassant) {
	case 0b000: generateMoves<whiteToMove, false, false, false>(pos, move_list); break;
	case 0b001: generateMoves<whiteToMove, false, false, true>(pos, move_list); break;
	case 0b010: generateMoves<whiteToMove, false, true, false>(pos, move_list); break;
	case 0b011: generateMoves<whiteToMove, false, true, true>(pos, move_list); break;
	case 0b100: generateMoves<whiteToMove, true, false, false>(pos, move_list); break;
	case 0b101: generateMoves<whiteToMove, true, false, true>(pos, move_list); break;
	case 0b110: generateMoves<whiteToMove, true, true, false>(pos, move_list); break;
	default: generateMoves<whiteToMove, true, true, true>(pos, move_list);
	}
	return castling;
}


template<bool whiteToMove, bool castling>
const void MoveGen::generateKingMoves(const Position& pos, MoveList& move_list) const {
	//CHANGE KING, he is not a bitboard any more
	constexpr uint32_t mover = King << 24;

	if constexpr (castling) {
		const uint64_t board = pos.teamBoards[0];
		const uint64_t attack = pos.st.enemyAttack;
		if constexpr (whiteToMove) {
			const bool kingSide = !(board & WHITE_KING_PIECES) && !(attack & WHITE_ATTACK_KING) && (pos.st.castlingRights & 0b0001);
			const bool queenSide = !(board & WHITE_QUEEN_PIECES) && !(attack & WHITE_ATTACK_QUEEN) && (pos.st.castlingRights & 0b0010);
			move_list.moves[move_list.curr] = (uint32_t)(62 | (60 << 8) | CASTLE_KING | mover) * kingSide;
			move_list.curr += kingSide;
			move_list.moves[move_list.curr] = (uint32_t)(58 | (60 << 8) | CASTLE_QUEEN | mover) * queenSide;
			move_list.curr += queenSide;
		}
		else {
			const bool kingSide = !(board & BLACK_KING_PIECES) && !(attack & BLACK_ATTACK_KING) && (pos.st.castlingRights & 0b0100);
			const bool queenSide = !(board & BLACK_QUEEN_PIECES) && !(attack & BLACK_ATTACK_QUEEN) && (pos.st.castlingRights & 0b1000);
			move_list.moves[move_list.curr] = (uint32_t)(6 | (4 << 8) | CASTLE_KING | mover) * kingSide;
			move_list.curr += kingSide;
			move_list.moves[move_list.curr] = (uint32_t)(2 | (4 << 8) | CASTLE_QUEEN | mover) * queenSide;
			move_list.curr += queenSide;
		}
	}
	const uint8_t king = pos.kings[!whiteToMove];
	const uint64_t moves = kingLookUp[king] & moveableSquares<whiteToMove>(pos) & ~pos.st.enemyAttack;
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];

	makeCaptureMove<whiteToMove>(pos.pieceBoards, move_list, moves & enemy, king, CAPTURE | mover);
	makePieceMove(move_list, moves & ~enemy, king, NO_FLAG | mover);

	
}



//TODO: maybe inline makePawnMove, it is not that long
template<bool whiteToMove, bool pins, bool enPassant>
const void MoveGen::generatePawnMoves(const Position& pos, MoveList& move_list) const {
	constexpr bool them = !whiteToMove;
	constexpr uint64_t promoRank = whiteToMove ? Rank2 : Rank7;
	constexpr uint64_t doublePotential = whiteToMove ? Rank6 : Rank3;
	constexpr int8_t back = whiteToMove ? 8 : -8;
	constexpr uint32_t pID = whiteToMove ? 0 << 24: 5 << 24;



	uint64_t pawns = getPieces<whiteToMove, Pawn>(pos);
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
	const uint64_t nonOccupied = ~pos.teamBoards[0];
	const uint64_t block = pos.st.blockForKing;

	//Non promotion push
	//TODO: when incheck the double push does't work
	const uint64_t nonPromo = pawns & ~promoRank;
	uint64_t push = shift<whiteToMove, UP>(nonPromo) & nonOccupied;
	uint64_t doublePush = shift<whiteToMove, UP>(push & doublePotential) & nonOccupied & block;
	push &= block;

	makePawnMove<pins, false>(pos, move_list, push, back, NO_FLAG | pID);
	makePawnMove<pins, false>(pos, move_list, doublePush, back * 2, DOUBLE_PUSH | pID);


	//Captures
	uint64_t capLeft = shift<whiteToMove, UP_LEFT>(nonPromo) & enemy & block;
	uint64_t capRight = shift<whiteToMove, UP_RIGHT>(nonPromo) & enemy & block;
	constexpr uint8_t backLeft = whiteToMove ? 9 : -7;
	constexpr uint8_t backRight = whiteToMove ? 7 : -9;

	makePawnCapture<whiteToMove, pins, false>(pos, move_list, capLeft, backLeft, CAPTURE | pID);
	makePawnCapture<whiteToMove, pins, false>(pos, move_list, capRight, backRight, CAPTURE | pID);

	//Promotions
	uint64_t promo = pawns & promoRank;
	if (promo) {
		uint64_t push = shift<whiteToMove, UP>(promo) & nonOccupied & block;
		uint64_t capLeft = shift<whiteToMove, UP_LEFT>(promo) & enemy & block;
		uint64_t capRight = shift<whiteToMove, UP_RIGHT>(promo) & enemy & block;

		makePawnMove<pins, true>(pos, move_list, push, back, PROMO_N | pID);
		makePawnCapture<whiteToMove, pins, true>(pos, move_list, capLeft, backLeft, PROMO_NC | pID);
		makePawnCapture<whiteToMove, pins, true>(pos, move_list, capRight, backRight, PROMO_NC | pID);
	}

	if constexpr (enPassant) {
		enPassantMoves<whiteToMove, pins>(pos, move_list, pos.st.enPassant);
	}
}


template<bool whiteToMove, Piece p, bool pins>
const void MoveGen::generatePieceMoves(const Position& pos, MoveList& move_list) const {
	constexpr uint32_t pID = whiteToMove ? p << 24 : (p + 5) << 24;
	uint64_t bishops = getPieces<whiteToMove, p>(pos);
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
	const uint64_t nonTeam = moveableSquares<whiteToMove>(pos);
	const uint64_t block = pos.st.blockForKing;
	
	unsigned long b;
	while (bishops) {
		bitScan(&b, bishops);
		bishops &= bishops - 1;
		uint64_t moves = attackBB<p>(pos.teamBoards[0], b) & nonTeam & block;
		if constexpr (pins) {
			if ((1ULL << b) & pos.st.pinnedMask) {
				moves &= LineBB[b][pos.kings[!whiteToMove]];
			}
		}
		makeCaptureMove<whiteToMove>(pos.pieceBoards, move_list, moves & enemy, b, CAPTURE | pID);
		makePieceMove(move_list, moves & ~enemy, b, NO_FLAG | pID);
	}
}

template<bool whiteToMove, bool pins>
const void MoveGen::generateKnightMoves(const Position& pos, MoveList& move_list) const {
	constexpr uint32_t pID = whiteToMove ? Knight << 24 : Knight + 5 << 24;
	uint64_t knights = getPieces<whiteToMove, Knight>(pos);
	const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
	const uint64_t nonTeam = moveableSquares<whiteToMove>(pos);

	if constexpr (pins) {
		 knights &= ~pos.st.pinnedMask;
	}

	unsigned long kn;
	while (knights) {
		bitScan(&kn, knights);
		knights &= knights - 1;
		const uint64_t moves = stepAttackBB<Knight>(kn) & nonTeam & pos.st.blockForKing;
		makeCaptureMove<whiteToMove>(pos.pieceBoards, move_list, moves & enemy, kn, CAPTURE | pID);
		makePieceMove(move_list, moves & ~enemy, kn, NO_FLAG | pID);
	}
}

//-----------------------Movemaking helper methods-----------------------------------------------------


template<bool whiteToMove, bool pins>
const void MoveGen::enPassantMoves(const Position& pos, MoveList& ml, uint8_t EP) const{
	//TODO: check on rank still allows EP, veri bad
	constexpr uint32_t pID = whiteToMove ? 0 << 24 : 5 << 24;
	constexpr uint64_t EPrank = whiteToMove ? Rank4 : Rank5;
	
	const uint64_t realPawn = shift<whiteToMove, DOWN>(BB(EP));
	const bool legalWhenCheck = pos.st.blockForKing & realPawn;
	uint64_t capturers = pawnAttackBB<!whiteToMove>(BB(EP)) & getPieces<whiteToMove, Pawn>(pos);

	//Special lateral pin case for en passant and check with double push
	const uint64_t rankPin = EPrank & (getPieces<!whiteToMove, Rook>(pos) | getPieces<!whiteToMove, Queen>(pos));
	const uint8_t king = pos.kings[!whiteToMove];
	const bool kingSameRank = (BB(king) & EPrank) && (EPrank & rankPin);
	
	const uint64_t boardWithoutEP = (realPawn | capturers) ^ pos.teamBoards[0];

	if (legalWhenCheck && !(kingSameRank && bitCount(capturers) == 1 && rookAttack(boardWithoutEP, king) & rankPin)) {
		unsigned long fromSQ;
		while (capturers) {
			bitScan(&fromSQ, capturers);
			capturers &= capturers - 1;
			//Only add if the pawn isn't pinned or the move moves along the pin
			if constexpr (pins) {
				if ((BB(fromSQ) & pos.st.pinnedMask) == 0 || (BB(EP) & LineBB[fromSQ][king])) {
					constructMove<false>(ml, fromSQ, EP, EP_CAPTURE | pID);
				}
			}
			else {
				constructMove<false>(ml, fromSQ, EP, EP_CAPTURE | pID);
			}
		}
	}
	
}




template<bool isPromotion>
const inline void MoveGen::constructMove(MoveList& move_list, uint8_t from, uint8_t to, uint32_t flagAndPieces) const{
	move_list.add(to | (from << 8) | flagAndPieces);
	if constexpr (isPromotion) {
		move_list.add(to | (from << 8) | PROMO_B | flagAndPieces);
		move_list.add(to | (from << 8) | PROMO_R | flagAndPieces);
		move_list.add(to | (from << 8) | PROMO_Q | flagAndPieces);
	}
}

template<bool pin, bool isPromotion>
const inline void MoveGen::makePawnMove(const Position& pos, MoveList& move_list, uint64_t toSQs, int8_t back, uint32_t flagAndPieces) const {
	unsigned long toSQ;
	while (toSQs) {
		bitScan(&toSQ, toSQs);
		toSQs &= toSQs - 1;
		if constexpr (pin) {
			const uint8_t from = toSQ + back;
			//Only add if the pawn isn't pinned or the move moves along the pin
			if (((1ULL << from) & pos.st.pinnedMask) == 0 || ((1ULL << toSQ) & LineBB[from][pos.kings[!pos.whiteToMove]])) {
				constructMove<isPromotion>(move_list, from, toSQ, flagAndPieces);
			}

		}
		else {
			constructMove<isPromotion>(move_list, toSQ + back, toSQ, flagAndPieces);
		}
	}
}

template<bool whiteToMove, bool pin, bool isPromotion>
const inline void MoveGen::makePawnCapture(const Position& pos, MoveList& move_list, uint64_t toSQs, int8_t back, uint32_t flagAndPieces) const{
	unsigned long toSQ;
	while (toSQs) {
		bitScan(&toSQ, toSQs);
		toSQs &= toSQs - 1;
		const uint32_t flagUpdate = flagAndPieces | (getPiece<!whiteToMove>(pos.pieceBoards, toSQ) << 28);
		if constexpr (pin) {
			const uint8_t from = toSQ + back;
			//Only add if the pawn isn't pinned or the move moves along the pin
			if (((1ULL << from) & pos.st.pinnedMask) == 0 || ((1ULL << toSQ) & LineBB[from][pos.kings[!pos.whiteToMove]])) {
				constructMove<isPromotion>(move_list, from, toSQ, flagUpdate);
			}

		}
		else {
			constructMove<isPromotion>(move_list, toSQ + back, toSQ, flagUpdate);
		}
	}
}


const inline void MoveGen::makePieceMove(MoveList& move_list, uint64_t toSQs, uint8_t from, uint32_t flagAndPieces) const {
	unsigned long toSQ;
	while (toSQs) {
		bitScan(&toSQ, toSQs);
		toSQs &= toSQs - 1;
		constructMove<false>(move_list, from, toSQ, flagAndPieces);
	}
}

template<bool whiteToMove>
const inline void MoveGen::makeCaptureMove(const uint64_t pieceBoards[], MoveList& move_list, uint64_t toSQs, uint8_t from, uint32_t flagAndPiece) const {
	unsigned long toSQ;
	while (toSQs) {
		bitScan(&toSQ, toSQs);
		toSQs &= toSQs - 1;
		const uint32_t flagUpdate = flagAndPiece | (getPiece<!whiteToMove>(pieceBoards, toSQ) << 28);
		constructMove<false>(move_list, from, toSQ, flagUpdate);
	}
}




//--------------------Pins and checks-------------------------------------

template<bool whiteToMove>
const void MoveGen::checks(Position& pos){
	uint64_t checkers = 0;
	const uint8_t king = pos.kings[!whiteToMove];

	//Opposite color knights
	const uint64_t knightCheck = stepAttackBB<Knight>(king) & getPieces<!whiteToMove, Knight>(pos);
	const uint64_t pawnCheck = pawnAttackBB<whiteToMove>(1ULL << king) & getPieces<!whiteToMove, Pawn>(pos);

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
	const uint8_t king = pos.kings[!whiteToMove];

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

template<bool whiteToMove>
const void MoveGen::findAttack(Position& pos) {
	uint64_t attack = 0;
	if constexpr (whiteToMove) {
		const uint64_t board_noKing = pos.teamBoards[0] ^ BB(pos.kings[0]);
		attack |= pieceAttack<Bishop>(board_noKing, pos.pieceBoards[7] | pos.pieceBoards[9]);
		attack |= pieceAttack<Rook>(board_noKing, pos.pieceBoards[8] | pos.pieceBoards[9]);
		attack |= stepAttack<Knight>(pos.pieceBoards[6]);
		attack |= kingLookUp[pos.kings[1]];
		attack |= shift<false, UP_LEFT>(pos.pieceBoards[5]) | shift<false, UP_RIGHT>(pos.pieceBoards[5]);
	}
	else {
		const uint64_t board_noKing = pos.teamBoards[0] ^ BB(pos.kings[1]);
		attack |= pieceAttack<Bishop>(board_noKing, pos.pieceBoards[2] | pos.pieceBoards[4]);
		attack |= pieceAttack<Rook>(board_noKing, pos.pieceBoards[3] | pos.pieceBoards[4]);
		attack |= stepAttack<Knight>(pos.pieceBoards[1]);
		attack |= kingLookUp[pos.kings[0]];
		attack |= shift<true, UP_LEFT>(pos.pieceBoards[0]) | shift<true, UP_RIGHT>(pos.pieceBoards[0]);
	}
	pos.st.enemyAttack = attack;
}







//Template instanciation (remove later when they are actually called)
template const uint64_t MoveGen::attackBB<Bishop>(uint64_t, uint8_t) const;
template const uint64_t MoveGen::attackBB<Rook>(uint64_t, uint8_t) const;
template const uint64_t MoveGen::attackBB<Queen>(uint64_t, uint8_t) const;

template const void MoveGen::pinnedBoard<true>(Position&);
template const void MoveGen::pinnedBoard<false>(Position&);
template const void MoveGen::checks<true>(Position&);
template const void MoveGen::checks<false>(Position&);
template const void MoveGen::findAttack<true>(Position&);
template const void MoveGen::findAttack<false>(Position&);

template const bool MoveGen::generateAllMoves<true>(const Position&, MoveList&) const;
template const bool MoveGen::generateAllMoves<false>(const Position&, MoveList&) const;


template const void MoveGen::generatePawnMoves<true, true, true>(const Position& pos, MoveList& move_list) const;
template const void MoveGen::generatePawnMoves<true, true, false>(const Position& pos, MoveList& move_list) const;
template const void MoveGen::generatePawnMoves<true, false, true>(const Position& pos, MoveList& move_list) const;
template const void MoveGen::generatePawnMoves<true, false, false>(const Position& pos, MoveList& move_list) const;


//---------------------Attack lookups---------------------------------
template<Piece p>
const uint64_t MoveGen::pieceAttack(uint64_t board, uint64_t pieces) const{
	uint64_t attack = 0;
	unsigned long fromSQ;
	while (pieces) {
		bitScan(&fromSQ, pieces);
		pieces &= pieces - 1;
		attack |= attackBB<p>(board, fromSQ);
	}
	return attack;
}

template<Piece p>
const uint64_t MoveGen::stepAttack(uint64_t pieces) const {
	uint64_t attack = 0;
	unsigned long fromSQ;
	while (pieces) {
		bitScan(&fromSQ, pieces);
		pieces &= pieces - 1;
		attack |= stepAttackBB<p>(fromSQ);
	}
	return attack;
}






template<Piece p>
const inline uint64_t MoveGen::stepAttackBB(uint8_t square) const{
	static_assert(p == Knight || p == King);
	if constexpr (p == Knight) {
		return knightLookUp[square];
	}
	if constexpr (p == King) {
		return kingLookUp[square];
	}
}

template<Piece p>
const inline uint64_t MoveGen::attackBB(uint64_t board, uint8_t square) const {
	static_assert(p > 1);
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







