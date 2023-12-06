#pragma once
#include <string>
#include <stack>
#include "bitboardUtil.h"
#include "zobristHash.h"
#include "GUI.h"
#include "moveGen.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <queue>
#include "evaluate.h"
#include "moveOrdering.h"
#include "robin_hood.h"

class Engine{
public:
	Engine();
	~Engine();

	void run();

private:
	//API
	void runUI();
	
	//Init position
	void fenInit(Position&, std::string);

	//Making moves
	template<bool whiteToMove, bool castle>
	void doMove(Position& pos, uint32_t move);
	template<bool whiteToMove, bool castle>
	void undoMove(Position& pos, uint32_t move);
	template<bool whiteToMove, bool castleKing>
	inline void doCastle(Position& pos);

	//Debugging
	template<bool whiteToMove, bool castle>
	void moveIntegrity(Position& pos);

	

	//Perft
	template<bool whiteToMove>
	uint64_t perft(Position& pos, int depth);
	template<bool whiteToMove, bool castle>
	uint64_t search(Position& pos, int depth);

	//Evaluation search
	template<bool whiteToMove>
	Move analysis(Position& pos, int depth);

	template<bool whiteToMove, bool castle>
	int negaMax(Position& pos, int alpha, int beta, int depth);

	template<bool whiteToMove>
	int qSearch(Position& pos, int alpha, int beta);

	//Build validation
	void tests();
	
	//Position info
	Position _pos;
	MoveGen* _moveGen;
	ZobristHash* _zobristHash;
	uint64_t nodes;
	uint64_t qNodes;
	Evaluation* _evaluation;
	
	

	//History stack
	std::stack<StateInfo> prevStates;
	std::stack<uint32_t> prevMoves;

	//Perft transposition
	//std::unordered_map<uint64_t, uint64_t> _transpositionTable;

	robin_hood::unordered_flat_map<uint64_t, uint64_t> _transpositionTable;

	//Evaluation transposition
	robin_hood::unordered_flat_map<uint64_t, int> _evalTransposition;


	MoveList moveListArena[20];
};