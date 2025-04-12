#include "moveOrdering.h"

void MoveOrder::moveSort(MoveList &move_list, const Position &, int)
{
  captureSort(move_list);
}

void MoveOrder::captureSort(MoveList &move_list)
{
  uint32_t move;
  int j;
  for (int i = 1; i < move_list.size(); ++i)
  {
    move = move_list.moves[i];
    j = i - 1;

    while (j >= 0 && captureScore(move, move_list.moves[j]))
    {
      move_list.moves[j + 1] = move_list.moves[j];
      j--;
    }
    move_list.moves[j + 1] = move;
  }
}

inline bool MoveOrder::captureScore(uint32_t move1, uint32_t move2)
{
  const int capture1 = getCaptured(move1);
  const int capture2 = getCaptured(move2);
  const bool bothCapture = (move1 & move2) & CAPTURE;
  const bool onlyFirstCapture =
      ((move1 & CAPTURE) - (move2 & CAPTURE)) == CAPTURE;

  return onlyFirstCapture || (bothCapture && capture1 > capture2);
}
