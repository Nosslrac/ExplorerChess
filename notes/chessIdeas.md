# Ideas and notes for Explorer Chess
All ideas or problems for Explorer chess will be stored here.
## Quiescence search
- Maybe all promotions should be considered not only captures.

## Pins: LineBB
- LineBB is currently 64x64 bitboard_t which is memory costly since many pairings are 0.
Consider compressing with PEXT similar to attacks.
- **REJECTED** since you want to return a 0 when not aligned, difficult to match 60% to same 0.

## Move list: improvement
- Pass the position in MoveList construction along with movegen type.