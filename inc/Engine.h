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

class Engine{
public:
	Engine();
	~Engine();

	void run();

private:
	void runUI();
	void fenInit(Position&, std::string);

	template<bool whiteToMove, bool castle>
	void doMove(Position& pos, uint32_t move);
	template<bool whiteToMove, bool castle>
	void undoMove(Position& pos, uint32_t move);


	template<bool whiteToMove, bool castle>
	void moveIntegrity(Position& pos);

	template<bool whiteToMove, bool castleKing>
	inline void doCastle(Position& pos);

	template<bool whiteToMove>
	uint64_t perft(Position& pos, int depth);

	template<bool whiteToMove, bool castle>
	uint64_t search(Position& pos, int depth);

	void tests();
	//Position info
	Position _pos;
	MoveGen* _moveGen;
	ZobristHash* _zobristHash;
	uint64_t hashHits;
	

	//History stack
	std::stack<StateInfo> prevStates;
	std::stack<uint32_t> prevMoves;

	std::unordered_map<uint64_t, uint64_t> _transpositionTable;
};