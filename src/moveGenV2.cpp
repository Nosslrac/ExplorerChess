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

template <Side s, MoveFilter filter>
Move *generatePawnMoves(const Position &pos, Move *moveList,
                        const bitboard_t targetSQs,
                        const bitboard_t pinnedPieces)
{
  // Useful constants
  constexpr auto masks = BitboardUtil::bitboardMasks<s>();
  constexpr Side enemy = s == Side::WHITE ? Side::BLACK : Side::WHITE;
  const bitboard_t pawns = pos.pieces<s, PAWN>();
  const bitboard_t pinnedPawns = pawns & pinnedPieces;
  const bitboard_t nonPinnedPawms = pawns & ~pinnedPieces;
  const bitboard_t allPieces = pos.pieces<ALL_PIECES>();
  const bitboard_t enemyPieces = pos.pieces_s<enemy>();

  // Generate captures
  if constexpr (filter != MoveFilter::QUIETS)
  {
    // Generate legal non pinned capture moves
    const bitboard_t nonPinnedCaptureRight =
        BitboardUtil::shift<masks->UP_RIGHT>(nonPinnedPawms &
                                             masks->NOT_RIGHT_COL) &
        enemyPieces & targetSQs;
    const bitboard_t nonPinnedCaptureLeft =
        BitboardUtil::shift<masks->UP_LEFT>(nonPinnedPawms &
                                            masks->NOT_LEFT_COL) &
        enemyPieces & targetSQs;

    for (bitboard_t captureRight = nonPinnedCaptureRight; captureRight != 0;
         captureRight &= captureRight - 1)
    {
      const square_t square = BitboardUtil::bitScan(captureRight);
      *moveList++ = Move::make(square + masks->DOWN_LEFT, square);
    }

    for (bitboard_t captureLeft = nonPinnedCaptureLeft; captureLeft != 0;
         captureLeft &= captureLeft - 1)
    {
      const square_t square = BitboardUtil::bitScan(captureLeft);
      *moveList++ = Move::make(square + masks->DOWN_RIGHT, square);
    }

    // Generate legal captures for pinned pawns
    if (pinnedPawns)
    {
      // Generate legal non pinned capture moves
      const bitboard_t pinnedCaptureRight =
          BitboardUtil::shift<masks->UP_RIGHT>(pinnedPawns &
                                               masks->NOT_RIGHT_COL) &
          enemyPieces & targetSQs;
      const bitboard_t pinnedCaptureLeft =
          BitboardUtil::shift<masks->UP_LEFT>(pinnedPawns &
                                              masks->NOT_LEFT_COL) &
          enemyPieces & targetSQs;
      const square_t kingSquare = pos.kingSquare<s>();

      for (bitboard_t captureRight = pinnedCaptureRight; captureRight != 0;
           captureRight &= captureRight - 1)
      {
        const square_t square = BitboardUtil::bitScan(captureRight);
        const square_t from = square + masks->DOWN_LEFT;
        if (RayConstants::RayBB[from][kingSquare] & BB(square))
        {
          *moveList++ = Move::make(from, square);
        }
      }

      for (bitboard_t captureLeft = pinnedCaptureLeft; captureLeft != 0;
           captureLeft &= captureLeft - 1)
      {
        const square_t square = BitboardUtil::bitScan(captureLeft);
        const square_t from = square + masks->DOWN_RIGHT;
        if (RayConstants::RayBB[from][kingSquare] & BB(square))
        {
          *moveList++ = Move::make(from, square);
        }
      }
    }
  }

  if constexpr (filter == MoveFilter::CAPTURES)
  {
    return moveList;
  }

  // Generate one step pushes and double pushes
  // All legal single push moves
  const bitboard_t singlePush =
      BitboardUtil::shift<masks->UP>(nonPinnedPawms) & ~allPieces & targetSQs;
  // All legal double push moves
  const bitboard_t doublePush =
      BitboardUtil::shift<masks->UP>(singlePush) & ~allPieces & targetSQs;

  for (bitboard_t squares = singlePush; squares != 0; squares &= squares - 1)
  {
    const square_t square = BitboardUtil::bitScan(squares);
    *moveList++ = Move::make(square + masks->DOWN, square);
  }

  for (bitboard_t squares = doublePush; squares != 0; squares &= squares - 1)
  {
    const square_t square = BitboardUtil::bitScan(squares);
    *moveList++ = Move::make(square + masks->DOWN + masks->DOWN, square);
  }

  if (pinnedPawns)
  {
    // Generate one step pushes and double pushes for pinned pawns
    // Pseudo legal single pawn pushes (pawn might be pinned)
    const bitboard_t pinnedSinglePush =
        BitboardUtil::shift<masks->UP>(pinnedPawns) & ~allPieces & targetSQs;
    // Pseudo legal double pushes (pawn might be pinned)
    const bitboard_t pinnedDoublePush =
        BitboardUtil::shift<masks->UP>(pinnedSinglePush) & ~allPieces &
        targetSQs;
    const square_t kingSquare = pos.kingSquare<s>();

    for (bitboard_t squares = pinnedSinglePush; squares != 0;
         squares &= squares - 1)
    {
      const square_t square = BitboardUtil::bitScan(squares);
      const square_t from = square + masks->DOWN;
      if (RayConstants::RayBB[from][kingSquare] & BB(square))
      {
        *moveList++ = Move::make(square + masks->DOWN, square);
      }
    }

    for (bitboard_t squares = pinnedDoublePush; squares != 0;
         squares &= squares - 1)
    {
      const square_t square = BitboardUtil::bitScan(squares);
      const square_t from = square + masks->DOWN;
      if (RayConstants::RayBB[from][kingSquare] & BB(square))
      {
        *moveList++ = Move::make(square + masks->DOWN + masks->DOWN, square);
      }
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
  const bitboard_t checkBoard =
      pos.attackOn(kingSquare, allPieces) & enemyPieces;

  if (!BitboardUtil::moreThanOne(checkBoard))
  {
    /// TODO: Get the actually pinned pieces
    const bitboard_t pinned = 0;
    moveList = generatePawnMoves<s, filter>(pos, moveList, targetSQs, pinned);
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
  if (filter == MoveFilter::CAPTURES || checkBoard)
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
  const MoveGen::MoveList<MoveFilter::ALL, s> moveList(pos);
  if (depth == 1)
  {
    return moveList.size();
  }
  // StateInfo newSt;
  std::uint64_t count = 0;
  constexpr Side enemy = s == Side::WHITE ? Side::BLACK : Side::WHITE;

  for (const auto &move : moveList)
  {
    StateInfo newState;
    pos.doMove<s>(move, newState);
    auto part = bulkCount<enemy>(pos, depth - 1);
    count += part;
    pos.undoMove<enemy>(move);
    std::cout << TempGUI::makeMoveNotation(move) << ": " << part << "\n";
  }
  return count;
}

void Perft::perft(Position &pos, int depth)
{
  auto count = pos.isWhiteToMove() ? bulkCount<Side::WHITE>(pos, depth)
                                   : bulkCount<Side::BLACK>(pos, depth);

  std::cout << "Total nodes visited: " << count << "\n";
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

void PseudoAttacks::printMove(Move move)
{
  const auto from = move.getFrom();
  const auto to = move.getTo();

  char rank = '8';
  std::string stb;
  stb += "\n +---+---+---+---+---+---+---+---+\n";
  for (int i = 0; i < BitboardUtil::BOARD_DIMMENSION; i++)
  {
    for (int j = 0; j < BitboardUtil::BOARD_DIMMENSION; j++)
    {
      auto square = i * BitboardUtil::BOARD_DIMMENSION + j;
      stb += " | ";
      stb += square == from ? '&' : square == to ? 'x' : ' ';
    }
    stb += " | ";
    stb += rank;
    rank--;
    std::cout << stb;
    std::cout << "\n +---+---+---+---+---+---+---+---+\n";
    stb = "";
  }
}

void PseudoAttacks::printMoves(square_t from, bitboard_t toSQs)
{
  if (toSQs == 0)
  {
    return;
  }
  char rank = '8';
  std::string stb;
  stb += "\n +---+---+---+---+---+---+---+---+\n";
  for (int i = 0; i < BitboardUtil::BOARD_DIMMENSION; i++)
  {
    for (int j = 0; j < BitboardUtil::BOARD_DIMMENSION; j++)
    {
      auto square = i * BitboardUtil::BOARD_DIMMENSION + j;
      stb += " | ";
      stb += square == from ? '&' : BB(square) & toSQs ? 'x' : ' ';
    }
    stb += " | ";
    stb += rank;
    rank--;
    std::cout << stb;
    std::cout << "\n +---+---+---+---+---+---+---+---+\n";
    stb = "";
  }
}

void PseudoAttacks::printMovelistMoves(Move *move)
{
  bitboard_t toSQs[SQ_COUNT]{};

  while (move->getData() != 0)
  {
    toSQs[move->getFrom()] |= BB(move->getTo());
    move++;
  }

  for (auto i = 0; i < SQ_COUNT; i++)
  {
    if (toSQs[i] != 0)
    {
      printMoves(i, toSQs[i]);
    }
  }
}