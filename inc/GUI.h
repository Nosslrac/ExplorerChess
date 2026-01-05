#pragma once
#include <string>
#include <x86intrin.h>

#include "bitboardUtil.h"
#include "moveGen.h"

namespace GUI {

// Functions
/** Prints bitboard b in the console with set bits as x
 * @param b
 */
void print_bit_board(bitboard_t b);
/** Prints board with pieces as their fen representation
 * @param square position info
 */
void print_pieces(const Position &pos);
/** Prints the fen for the position
 * @param square position info
 */
void getPositionFen(const Position &pos);
/** Helper function to printpieces
 * @param square position info
 * @param pBoard array to store fen representation of the pieces
 */

std::string makeMoveNotation(Move move);
std::string makeSquareNotation(square_t square);
Move parseMove(std::string moveNotation);
std::string getCastleRights(const Position &pos);
square_t makeSquare(char col, char row);

} // namespace GUI
