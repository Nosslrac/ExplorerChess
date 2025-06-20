#pragma once
#include <array>
#include <cassert>
#include <immintrin.h>

#include <cstddef>
#include <cstdint>
#include <string>

#include "types.h"

#if __has_builtin(__builtin_ia32_pext_di)
#define PEXT
#endif

// #define getTo(move) (move & 0xFFU)
// #define getFrom(move) ((move >> 8U) & 0xFFU)
// #define getFlags(move) (move & 0xFF0000U)
// #define getMover(move) ((move >> 24U) & 0xFU)
// #define getCaptured(move) (move >> 28U)
// #define getPromo(move) ((move >> 16U) & 0x3U)
#define BB(i) (1UL << i)

enum Direction
{
  UP,
  DOWN,
  UP_LEFT,
  UP_RIGHT
};

namespace BitboardUtil {

constexpr bitboard_t FileA = 0x0101010101010101ULL;
constexpr bitboard_t FileB = FileA << 1U;
constexpr bitboard_t FileC = FileA << 2U;
constexpr bitboard_t FileD = FileA << 3U;
constexpr bitboard_t FileE = FileA << 4U;
constexpr bitboard_t FileF = FileA << 5U;
constexpr bitboard_t FileG = FileA << 6U;
constexpr bitboard_t FileH = FileA << 7U;

constexpr bitboard_t Rank1 = 0xFF;
constexpr bitboard_t Rank2 = Rank1 << (8 * 1U);
constexpr bitboard_t Rank3 = Rank1 << (8 * 2U);
constexpr bitboard_t Rank4 = Rank1 << (8 * 3U);
constexpr bitboard_t Rank5 = Rank1 << (8 * 4U);
constexpr bitboard_t Rank6 = Rank1 << (8 * 5U);
constexpr bitboard_t Rank7 = Rank1 << (8 * 6U);
constexpr bitboard_t Rank8 = Rank1 << (8 * 7U);

//---------- Usful constants------------------------
constexpr uint8_t NoEP = 0;
constexpr bitboard_t All_SQ = ~0ULL;
constexpr bitboard_t NOT_EDGE =
    (Rank2 | Rank3 | Rank4 | Rank5 | Rank6 | Rank7) & ~FileA & ~FileH;
constexpr int CHECK_MATE = -0xFFFF;
constexpr int BLACK = 1;
constexpr int WHITE = 0;
constexpr int BLACK_OFFSET = 5;
constexpr int BOARD_DIMMENSION = 8;
constexpr std::size_t MAX_MOVES = 100;

//---------CASTLING SQUARES--------------------------

constexpr bitboard_t WHITE_QUEEN_PIECES = BB(59U) | BB(58U) | BB(57U);
constexpr bitboard_t WHITE_KING_PIECES = BB(61U) | BB(62U);
constexpr bitboard_t WHITE_KING_ROOK_FROM_TO = BB(63U) | BB(61U);
constexpr bitboard_t WHITE_QUEEN_ROOK_FROM_TO = BB(56U) | BB(59U);

constexpr bitboard_t BLACK_QUEEN_PIECES = BB(1U) | BB(2U) | BB(3U);
constexpr bitboard_t BLACK_KING_PIECES = BB(6U) | BB(5U);
constexpr bitboard_t BLACK_KING_ROOK_FROM_TO = BB(7U) | BB(5U);
constexpr bitboard_t BLACK_QUEEN_ROOK_FROM_TO = BB(0U) | BB(3U);

// Attacked squares differ from occupied on queenside, also add king square
// since king can't be in check
constexpr bitboard_t WHITE_ATTACK_QUEEN =
    WHITE_QUEEN_PIECES ^ BB(57U) ^ BB(60U);
constexpr bitboard_t BLACK_ATTACK_QUEEN = BLACK_QUEEN_PIECES ^ BB(1U) ^ BB(4U);

constexpr bitboard_t WHITE_ATTACK_KING = WHITE_KING_PIECES ^ BB(60U);
constexpr bitboard_t BLACK_ATTACK_KING = BLACK_KING_PIECES ^ BB(4U);

inline int pext(bitboard_t BB, bitboard_t mask)
{
#ifdef PEXT
  return static_cast<int>(__builtin_ia32_pext_di(BB, mask));
#else
  return 0; // Impl needed
#endif
}

inline uint8_t bitCount(bitboard_t BB)
{
#if __has_builtin(__builtin_popcountll)
  return static_cast<uint8_t>(__builtin_popcountll(BB));
#else
  return 0; // Impl needed
#endif
}

inline uint8_t bitScan(bitboard_t BB)
{
#if __has_builtin(__builtin_ctzll)
  return static_cast<uint8_t>(__builtin_ctzll(BB));
#else
  return 0; // Impl needed
#endif
}

template <PieceV2 p> inline bitboard_t pieces(const bitboard_t pieces[])
{
  return pieces[p] | pieces[BLACK + p];
}

template <bool whiteToMove, Direction D> bitboard_t shift(bitboard_t b)
{
  if constexpr (whiteToMove)
  {
    return D == UP        ? b >> 8
           : D == DOWN    ? b << 8
           : D == UP_LEFT ? (b >> 9) & ~FileH
                          : (b >> 7) & ~FileA;
  }
  if constexpr (!whiteToMove)
  {
    return D == UP        ? b << 8
           : D == DOWN    ? b >> 8
           : D == UP_LEFT ? (b << 7) & ~FileH
                          : (b << 9) & ~FileA;
  }
}
// Squares infront of pawn
template <bool white> inline bitboard_t forwardSquares(square_t sq)
{
  if constexpr (white)
  {
    return All_SQ >>
           ((BOARD_DIMMENSION - sq / BOARD_DIMMENSION) * BOARD_DIMMENSION);
  }
  else
  {
    return All_SQ << ((sq / BOARD_DIMMENSION) * BOARD_DIMMENSION +
                      BOARD_DIMMENSION);
  }
}

// Castling is blocked
template <bool white, bool kingSide> bool isOccupied(bitboard_t board)
{
  if constexpr (white)
  {
    return kingSide ? WHITE_KING_PIECES & board : WHITE_QUEEN_PIECES & board;
  }
  else
  {
    return kingSide ? BLACK_KING_PIECES & board : BLACK_QUEEN_PIECES & board;
  }
}

template <bool white, bool kingSide> bool isAttacked(bitboard_t attack)
{
  if constexpr (white)
  {
    return kingSide ? WHITE_KING_PIECES & attack : WHITE_ATTACK_QUEEN & attack;
  }
  else
  {
    return kingSide ? BLACK_KING_PIECES & attack : BLACK_ATTACK_QUEEN & attack;
  }
}

inline bool moreThanOne(bitboard_t b) { return b & (b - 1); }

constexpr bool isOnBoard(square_t square)
{
  return square >= SQ_A8 && square <= SQ_H1;
}

constexpr std::uint8_t fileOf(square_t square) { return square & 7U; }

namespace TempGUI {

inline constexpr square_t makeSquare(const std::array<char, 2> &move)
{
  assert(move.size() == 2);

  return square_t(move.at(0) - 'a' + (('8' - move.at(1)) << 3U));
}

inline std::string getCastleRights(std::uint8_t castleRights,
                                   std::string_view castleLetters)
{
  std::string castle;
  for (auto i = 0; i < 4; i++)
  {
    if (castleRights & BB(i))
    {
      castle += castleLetters[i];
    }
  }
  return castle;
}

inline std::string makeSquareNotation(square_t square)
{
  if (!isOnBoard(square) || square == SQ_NONE)
  {
    return "-";
  }
  return std::string({static_cast<char>('a' + (square & 7U)),
                      static_cast<char>('8' - (square >> 3U))});
}

inline std::string makeMoveNotation(MoveV2 move)
{
  square_t from = move.getFrom();
  square_t to = move.getTo();
  if (!isOnBoard(from) || !isOnBoard(to))
  {
    return "(invalid move)";
  }
  return makeSquareNotation(from) + makeSquareNotation(to) +
         " nbrq"[move.getPromo()];
}

} // namespace TempGUI

constexpr uint8_t castlingModifiers[SQ_COUNT] = {
    0b0111, 0b1111, 0b1111, 0b1111, 0b0011, 0b1111, 0b1111, 0b1011,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111, 0b1111,
    0b1101, 0b1111, 0b1111, 0b1111, 0b1100, 0b1111, 0b1111, 0b1110};

//---------------Bitboard functions------------------

void initLineBB(bitboard_t (&lineBB)[64][64]);
bitboard_t pinned_ray(int, int);

//------------------Files for init----------------------

constexpr bitboard_t files[8] = {0x0101010101010101ULL, 0x0202020202020202ULL,
                                 0x0404040404040404ULL, 0x0808080808080808ULL,
                                 0x1010101010101010ULL, 0x2020202020202020ULL,
                                 0x4040404040404040ULL, 0x8080808080808080ULL};
constexpr bitboard_t ranks[8] = {
    0xFFLL,         0xFF00LL,         0xFF0000LL,         0xFF000000LL,
    0xFF00000000LL, 0xFF0000000000LL, 0xFF000000000000LL, 0xFF00000000000000LL};
constexpr bitboard_t main_diagonals[15] = {0x0100000000000000ULL,
                                           0x0201000000000000ULL,
                                           0x0402010000000000ULL,
                                           0x0804020100000000ULL,
                                           0x1008040201000000ULL,
                                           0x2010080402010000ULL,
                                           0x4020100804020100ULL,
                                           0x8040201008040201ULL,
                                           0x80402010080402ULL,
                                           0x804020100804ULL,
                                           0x8040201008ULL,
                                           0x80402010ULL,
                                           0x804020ULL,
                                           0x8040ULL,
                                           0x80ULL};
constexpr bitboard_t anti_diagonals[15] = {0x1ULL,
                                           0x0102ULL,
                                           0x010204ULL,
                                           0x01020408ULL,
                                           0x0102040810ULL,
                                           0x010204081020ULL,
                                           0x01020408102040ULL,
                                           0x0102040810204080ULL,
                                           0x0204081020408000ULL,
                                           0x0408102040800000ULL,
                                           0x0810204080000000ULL,
                                           0x1020408000000000ULL,
                                           0x2040800000000000ULL,
                                           0x4080000000000000ULL,
                                           0x8000000000000000ULL};

constexpr bitboard_t adjacentFiles[8] = {files[0] | files[1],
                                         files[0] | files[1] | files[2],
                                         files[1] | files[2] | files[3],
                                         files[2] | files[3] | files[4],
                                         files[3] | files[4] | files[5],
                                         files[4] | files[5] | files[6],
                                         files[5] | files[6] | files[7],
                                         files[6] | files[7]};

} // namespace BitboardUtil