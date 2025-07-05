// #include "EngineInterface.h"
#include "attackPextV2.h"
#include "bitboardUtilV2.h"
#include "moveGenV2.h"
#include "position.h"
#include "types.h"
#include <cstdio>

int main()
{
  ATTACKS::init();
  Position pos{};
  StateInfo st{};

  // pos.fenInit("4k3/4b3/8/r7/8/4B3/2R5/4K3 b - - 0 1", st);
  pos.fenInit("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq 0 1", st);
  // auto attackers = pos.attackOn(53, pos.pieces<ALL_PIECES>());
  // PseudoAttacks::print_bit_board(attackers);

  Perft::perft(pos, 2);
  // for (int i = 0; i < 64; i++)
  // {
  //   if (i % 3 == 0)
  //   {
  //     printf("\n");
  //   }
  //   bitboard_t tmp = ((BB(i) << 7) & ~BitboardUtil::FileH) |
  //                    ((BB(i) << 9) & ~BitboardUtil::FileA);

  //   printf("0x%016lxUL,", tmp);
  // }
  // ExplorerChess::EngineParser parser;
  // parser.runInterface();
  return 0;
}