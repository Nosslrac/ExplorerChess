#pragma once
#include "attacks.h"
#include "bitboardUtil.h"



class MoveGen : private magicalBits::MagicalBitboards{
public:
	MoveGen();
	template<bool whiteToMove>
	const bool generateAllMoves(const Position& pos, MoveList& move_list, const bool onlyCapture) const;

	template<bool whiteToMove, bool castling, bool pins, bool enPassant>
	const void generateMoves(const Position& pos, MoveList& move_list, const bool onlyCapture) const;

	template<bool whiteToMove, bool castling>
	const void generateKingMoves(const Position& pos, MoveList& move_list, const bool onlyCapture) const;

	template<bool whiteToMove, bool pins, bool enPassent>
	const void generatePawnMoves(const Position& pos, MoveList& move_list, const bool onlyCapture) const;

	template<bool whiteToMove, bool pins>
	const void generateKnightMoves(const Position& pos, MoveList& move_list, const bool onlyCapture) const;
	
	template<bool whiteToMove, Piece p, bool pins>
	const void generatePieceMoves(const Position& pos, MoveList& move_list, const bool onlyCapture) const;

	//Move to private when working
	template<Piece p>
	const inline uint64_t stepAttackBB(uint8_t square) const;
	template<Piece p>
	const inline uint64_t attackBB(const uint64_t board, const uint8_t square) const;
	
	//Checks and pins
	template<bool whiteToMove>
	const void pinnedBoard(Position& pos);
	template<bool whiteToMove>
	const void checks(Position& pos);
	template<bool whiteToMove>
	const void findAttack(Position& pos);
	template<bool whiteToMove>
	const void setCheckSquares(Position& pos) const;

private:

	template<Piece p>
	const uint64_t pieceAttack(uint64_t board, uint64_t pieces) const;
	template<Piece p>
	const uint64_t stepAttack(uint64_t pieces) const;

	const inline uint64_t bishopAttack(uint64_t board, uint8_t square) const;
	const inline uint64_t rookAttack(uint64_t board, uint8_t square) const;

	template<bool isPromotion>
	const inline void constructMove(MoveList& move_list, uint8_t from, uint8_t to, uint32_t flagAndPiece) const;

	template<bool whiteToMove, bool enPassent>
	const void pinnedPawns(const Position& pos, MoveList& move_list, uint64_t pinned) const;
	

	template<bool whiteToMove, bool pins>
	const void enPassantMoves(const Position& pos, MoveList& ml, uint8_t EP) const;


	template<bool pin, bool isPromotion>
	const void makePawnMove(const Position& pos, MoveList& move_list, uint64_t toSQs, int8_t back, uint32_t flagAndPiece) const;

	template<bool whiteToMove, bool pin, bool isPromotion>
	const inline void makePawnCapture(const Position& pos, MoveList& move_list, uint64_t toSQs, int8_t back, uint32_t flagAndPieces) const;
	
	const inline void makePieceMove(MoveList& move_list, uint64_t toSQs, uint8_t back, uint32_t flagAndPiece) const;

	template<bool whiteToMove>
	const inline void makeCaptureMove(const uint64_t pieceBoards[], MoveList& move_list, uint64_t toSQs, uint8_t back, uint32_t flagAndPiece) const;


	//------------------Helper lookups------------------------------

	template<bool white>
	constexpr inline uint64_t pawnAttackBB(uint64_t squareBB) const {
		return shift<white, UP_LEFT>(squareBB) | shift<white, UP_RIGHT>(squareBB);
	}

	template<bool white>
	constexpr inline uint64_t getTeam(const Position& pos) const {
		if constexpr (white) return pos.teamBoards[1];
		return pos.teamBoards[2];
	}

	template<bool whiteToMove>
	constexpr inline uint64_t moveableSquares(const Position& pos) const
	{
		if constexpr (whiteToMove) return ~pos.teamBoards[1];
		return ~pos.teamBoards[2];
	}

	template<bool white>
	constexpr inline uint64_t getStraigthSliders(const Position& pos) const{ 
		if constexpr (white) {
						return pos.pieceBoards[3] | pos.pieceBoards[4];
		}
		else {
			return pos.pieceBoards[8] | pos.pieceBoards[9];
		}
	}


	template<bool white, Piece p>
	constexpr inline uint64_t getPieces(const Position& pos) const {
		if constexpr (white) {
			return pos.pieceBoards[p];
		}
		else {
			return pos.pieceBoards[p + 5];
		}
		
	}


	template<Piece p>
	const inline uint64_t emptyAttack(uint8_t sq) const{
		if constexpr (p == Bishop) {
			return m_bishopAttacks[sq][0];
		}
		if constexpr (p == Rook) {
			return m_rookAttacks[sq][0];
		}
	}

	const inline uint64_t betweenBB(uint8_t sq1, uint8_t sq2) const{
		uint64_t b = LineBB[sq1][sq2] & ((All_SQ << sq1) ^ (All_SQ << sq2));
		return b & (b - 1); //exclude lsb
	}



	//Lookups for knight and king
	static constexpr uint64_t kingLookUp[64] = {
		770ULL,
		1797ULL,
		3594ULL,
		7188ULL,
		14376ULL,
		28752ULL,
		57504ULL,
		49216ULL,
		197123ULL,
		460039ULL,
		920078ULL,
		1840156ULL,
		3680312ULL,
		7360624ULL,
		14721248ULL,
		12599488ULL,
		50463488ULL,
		117769984ULL,
		235539968ULL,
		471079936ULL,
		942159872ULL,
		1884319744ULL,
		3768639488ULL,
		3225468928ULL,
		12918652928ULL,
		30149115904ULL,
		60298231808ULL,
		120596463616ULL,
		241192927232ULL,
		482385854464ULL,
		964771708928ULL,
		825720045568ULL,
		3307175149568ULL,
		7718173671424ULL,
		15436347342848ULL,
		30872694685696ULL,
		61745389371392ULL,
		123490778742784ULL,
		246981557485568ULL,
		211384331665408ULL,
		846636838289408ULL,
		1975852459884544ULL,
		3951704919769088ULL,
		7903409839538176ULL,
		15806819679076352ULL,
		31613639358152704ULL,
		63227278716305408ULL,
		54114388906344448ULL,
		216739030602088448ULL,
		505818229730443264ULL,
		1011636459460886528ULL,
		2023272918921773056ULL,
		4046545837843546112ULL,
		8093091675687092224ULL,
		16186183351374184448ULL,
		13853283560024178688ULL,
		144959613005987840ULL,
		362258295026614272ULL,
		724516590053228544ULL,
		1449033180106457088ULL,
		2898066360212914176ULL,
		5796132720425828352ULL,
		11592265440851656704ULL,
		4665729213955833856ULL
	};
	
	static constexpr uint64_t knightLookUp[64] = {
		132096ULL,
		329728ULL,
		659712ULL,
		1319424ULL,
		2638848ULL,
		5277696ULL,
		10489856ULL,
		4202496ULL,
		33816580ULL,
		84410376ULL,
		168886289ULL,
		337772578ULL,
		675545156ULL,
		1351090312ULL,
		2685403152ULL,
		1075839008ULL,
		8657044482ULL,
		21609056261ULL,
		43234889994ULL,
		86469779988ULL,
		172939559976ULL,
		345879119952ULL,
		687463207072ULL,
		275414786112ULL,
		2216203387392ULL,
		5531918402816ULL,
		11068131838464ULL,
		22136263676928ULL,
		44272527353856ULL,
		88545054707712ULL,
		175990581010432ULL,
		70506185244672ULL,
		567348067172352ULL,
		1416171111120896ULL,
		2833441750646784ULL,
		5666883501293568ULL,
		11333767002587136ULL,
		22667534005174272ULL,
		45053588738670592ULL,
		18049583422636032ULL,
		145241105196122112ULL,
		362539804446949376ULL,
		725361088165576704ULL,
		1450722176331153408ULL,
		2901444352662306816ULL,
		5802888705324613632ULL,
		11533718717099671552ULL,
		4620693356194824192ULL,
		288234782788157440ULL,
		576469569871282176ULL,
		1224997833292120064ULL,
		2449995666584240128ULL,
		4899991333168480256ULL,
		9799982666336960512ULL,
		1152939783987658752ULL,
		2305878468463689728ULL,
		1128098930098176ULL,
		2257297371824128ULL,
		4796069720358912ULL,
		9592139440717824ULL,
		19184278881435648ULL,
		38368557762871296ULL,
		4679521487814656ULL,
		9077567998918656ULL
	};

	uint64_t LineBB[64][64] = {};

};


