#pragma once
#include "bitboardUtil.h"


namespace MoveOrder{
	void moveSort(MoveList& move_list, const Position& pos, int depth);

	void captureSort(MoveList& move_list);
	inline bool captureScore(uint32_t move1, uint32_t move2);
};
	



	
