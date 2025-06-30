#include "moveGenV2.h"

#include "attackPextV2.h"
#include "attackRays.h"
#include "bitboardUtilV2.h"
#include "position.h"
#include "types.h"

#include <iostream>
#include <string>

namespace {

/// @brief Generate all legal moves for a piece rook or queen.
/// @param targetSQs is the bitboard of legal blocking squares when the king is
/// in check or only captures
/// @param pinnedPieces is the bitboard of pieces pinned in any way to the king
template <Side s, PieceType pt>
Move *generatePieceMoves(const Position &pos, Move *moveList,
                         const bitboard_t targetSQs,
                         const bitboard_t pinnedPieces)
{

  static_assert(pt != PAWN && pt != KING,
                "Unsupported piece type in generatePieceMoves()");

  const bitboard_t movers = pos.pieces<s, pt>();
  const bitboard_t pinnedMovers = movers & pinnedPieces;
  const bitboard_t nonPinnedMovers = movers & ~pinnedPieces;
  const bitboard_t allPieces = pos.pieces<ALL_PIECES>();

  assert((pinnedMovers | nonPinnedMovers) == movers);

  // Non pinned pieces
  for (bitboard_t pieces = nonPinnedMovers; pieces != 0; pieces &= pieces - 1)
  {
    const square_t square = BitboardUtil::bitScan(pieces);

    bitboard_t attack = MoveGen::attacks<pt>(allPieces, square) & targetSQs;

    for (; attack != 0; attack &= attack - 1)
    {
      *moveList++ = Move::make(square, BitboardUtil::bitScan(attack));
    }
  }

  // Pinned pieces
  for (bitboard_t pieces = pinnedMovers; pieces != 0; pieces &= pieces - 1)
  {
    const square_t square = BitboardUtil::bitScan(pieces);

    bitboard_t attack = MoveGen::attacks<pt>(allPieces, square) & targetSQs &
                        RayConstants::RayBB[square][pos.kingSquare<s>()];

    for (; attack != 0; attack &= attack - 1)
    {
      *moveList++ = Move::make(square, BitboardUtil::bitScan(attack));
    }
  }
  return moveList;
}

} // namespace

namespace MoveGen {

template <MoveFilter filter> Move *generate(const Position &pos, Move *moveList)
{
  return pos.isWhiteToMove() ? generate<filter, Side::WHITE>(pos, moveList)
                             : generate<filter, Side::BLACK>(pos, moveList);
}

template <MoveFilter filter, Side s>
Move *generate(const Position &pos, Move *moveList)
{
  // Constants
  constexpr Side enemy = s == Side::WHITE ? Side::BLACK : Side::WHITE;
  constexpr BitboardUtil::Masks const *masks = BitboardUtil::bitboardMasks<s>();

  // Useful information for check detection and pin detection
  /// TODO: Remove king when checking where king can go
  const bitboard_t allPieces = pos.pieces<ALL_PIECES>();
  const bitboard_t enemyPieces = pos.pieces_s<enemy>();
  const bitboard_t friendlyPieces = pos.pieces_s<s>();

  /// TODO: Get the correct check evasions squares
  constexpr bitboard_t partialFilter =
      filter == MoveFilter::ALL        ? BitboardUtil::All_SQ
      : filter == MoveFilter::CAPTURES ? enemyPieces
      : filter == MoveFilter::QUIETS   ? ~enemyPieces
                                       : 0;
  const bitboard_t targetSQs =
      partialFilter & ~friendlyPieces; // Can't capture own pieces

  const square_t kingSquare = pos.kingSquare<s>();
  const bitboard_t checkBoard = pos.attackOn(kingSquare, allPieces);
  /// TODO: Get the actually pinned pieces
  const bitboard_t pinned = 0UL;

  if (!BitboardUtil::moreThanOne(checkBoard))
  {
    PseudoAttacks::print_bit_board(targetSQs);
    moveList = generatePieceMoves<s, KNIGHT>(pos, moveList, targetSQs, pinned);
    moveList = generatePieceMoves<s, BISHOP>(pos, moveList, targetSQs, pinned);
    moveList = generatePieceMoves<s, ROOK>(pos, moveList, targetSQs, pinned);
    moveList = generatePieceMoves<s, QUEEN>(pos, moveList, targetSQs, pinned);
  }

  // King moves
  bitboard_t kingStandard = attacks<KING>(0, kingSquare) & targetSQs;
  const bitboard_t boardWithoutKing = allPieces & ~BB(kingSquare);

  for (; kingStandard != 0; kingStandard &= kingStandard - 1)
  {
    square_t square = BitboardUtil::bitScan(kingStandard);
    if ((pos.attackOn(square, boardWithoutKing) & enemyPieces) == 0)
    {
      *moveList++ = Move::make(kingSquare, square);
    }
  }
  if (filter != MoveFilter::CAPTURES || checkBoard)
  {
    return moveList;
  }

  // Castling king moves
  if (((masks->CASTLE_KING_PIECES & allPieces) == 0) &&
      (pos.castleRights<s>() & 1) &&
      (pos.isSafeSquares<s>(masks->CASTLE_KING_ATTACK_SQUARES, allPieces)))
  {
    *moveList++ = Move::make<CASTLE>(kingSquare, kingSquare + 2);
  }

  // Castling queen side
  if (((masks->CASTLE_QUEEN_PIECES & allPieces) == 0) &&
      (pos.castleRights<s>() & 1) &&
      (pos.isSafeSquares<s>(masks->CASTLE_QUEEN_ATTACK_SQUARES, allPieces)))
  {
    *moveList++ = Move::make<CASTLE>(kingSquare, kingSquare - 2);
  }

  return moveList;
}

template Move *generate<MoveFilter::ALL>(const Position &, Move *);

// Template specializations for the attacks function
template <>
bitboard_t attacks<KING>(const bitboard_t /*occupancy*/, const square_t square)
{
  return PseudoAttacks::KingAttacks[square];
}

template <>
bitboard_t attacks<KNIGHT>(const bitboard_t /*occupancy*/,
                           const square_t square)
{
  return PseudoAttacks::KnightAttacks[square];
}

template <>
bitboard_t attacks<BISHOP>(const bitboard_t occupancy, const square_t square)
{
  return PEXT_ATTACK::ATTACK_MAGICS[square][0].attackBB(occupancy);
}

template <>
bitboard_t attacks<ROOK>(const bitboard_t occupancy, const square_t square)
{
  return PEXT_ATTACK::ATTACK_MAGICS[square][1].attackBB(occupancy);
}

template <>
bitboard_t attacks<QUEEN>(const bitboard_t occupancy, const square_t square)
{
  return attacks<ROOK>(occupancy, square) | attacks<BISHOP>(occupancy, square);
}

template <Side s, PieceType p>
bitboard_t attacks(const bitboard_t /*occupancy*/, const square_t square)

{
  return PseudoAttacks::PawnAttacks[static_cast<int>(s)][square];
}

// Explicit instantiations for pawns
template bitboard_t attacks<Side::WHITE, PAWN>(const bitboard_t,
                                               const square_t);
template bitboard_t attacks<Side::BLACK, PAWN>(const bitboard_t,
                                               const square_t);

} // namespace MoveGen

template <Side s> std::uint64_t bulkCount(Position &pos, int depth)
{
  MoveGen::MoveList<MoveFilter::ALL, s> moveList(pos);
  if (depth == 1)
  {
    return moveList.size();
  }
  // StateInfo newSt;
  std::uint64_t count = 0;
  std::cout << "Starting bulk count:" << int(moveList.size()) << "\n";

  for (const auto &move : moveList)
  {
    count++;
    std::cout << TempGUI::makeMoveNotation(move) << "\n";
  }
  return count;
}

void Perft::perft(Position &pos, int depth)
{
  pos.isWhiteToMove() ? bulkCount<Side::WHITE>(pos, depth)
                      : bulkCount<Side::BLACK>(pos, depth);
}

void PseudoAttacks::print_bit_board(bitboard_t b)
{
  std::string stb = "";
  stb += "--------------------\n";
  for (int i = 0; i < 8; i++)
  {
    stb += "| ";
    for (int j = 0; j < 8; j++)
    {
      stb += (b & 1) == 1 ? "x " : "0 ";
      b >>= 1;
    }
    stb += " |";
    std::cout << stb;
    std::cout << "\n";
    stb = "";
  }
  std::cout << "--------------------\n\n";
}