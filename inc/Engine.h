#pragma once
#include <string>
#include "bitboardUtil.h"
#include "GUI.h"
#include "moveGen.h"

class Engine : MoveGen{
public:
	Engine();
	~Engine();

	void run();

private:
	void runUI();
	void fenInit(Position&, std::string);

	template<bool whiteToMove, bool castling>
	void doMove(Position& pos, uint32_t move);

	//Position info
	Position _pos;
	std::string _fen;

};