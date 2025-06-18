// #include "EngineInterface.h"
#include "attackPextV2.h"
#include "bitboardUtilV2.h"
#include "moveGenV2.h"
#include "types.h"
#include <cstdio>

int main()
{
  ATTACKS::init();
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