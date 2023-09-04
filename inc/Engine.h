#pragma once
#include <string>
#include <stack>
#include "bitboardUtil.h"
#include "GUI.h"
#include "moveGen.h"
#include <cassert>

class Engine : MoveGen{
public:
	Engine();
	~Engine();

	void run();

private:
	void runUI();
	void fenInit(Position&, std::string);

	template<bool whiteToMove>
	void doMove(Position& pos, uint32_t move);
	template<bool whiteToMove>
	void undoMove(Position& pos, uint32_t move);


	template<bool whiteToMove>
	void moveIntegrity(Position& pos);

	template<bool whiteToMove, bool castleKing>
	inline void doCastle(Position& pos);

	template<bool whiteToMove>
	void perft(Position& pos, int depth);

	template<bool whiteToMove>
	uint64_t search(Position& pos, int depth);

	//Position info
	Position _pos;
	

	//History stack
	std::stack<StateInfo> prevStates;
	std::stack<uint32_t> prevMoves;
};