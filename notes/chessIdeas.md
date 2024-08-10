# Ideas and notes for Explorer Chess
All ideas or problems for Explorer chess will be stored here.
## Quiescence search
- Maybe all promotions should be considered not only captures.

## Pins: LineBB
- LineBB is currently 64x64 uint64_t which is memory costly since many pairings are 0.
Consider compressing with PEXT similar to attacks.

## Move list: improvement
- Pass the position in MoveList construction along with movegen type.