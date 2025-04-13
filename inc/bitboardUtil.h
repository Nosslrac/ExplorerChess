#pragma once
#include <immintrin.h>

#include <cstdint>
#include <cstring>

#include "types.h"

#if __has_builtin(__builtin_ia32_pext_di)
#define PEXT
#endif

#define getTo(move) (move & 0xFFU)
#define getFrom(move) ((move >> 8U) & 0xFFU)
#define getFlags(move) (move & 0xFF0000U)
#define getMover(move) ((move >> 24U) & 0xFU)
#define getCaptured(move) (move >> 28U)
#define getPromo(move) ((move >> 16U) & 0x3U)
#define BB(i) (1ULL << i)

enum Piece
{
  King = 10,
  Pawn = 0,
  Knight = 1,
  Bishop = 2,
  Rook = 3,
  Queen = 4,
};

enum Direction
{
  UP,
  DOWN,
  UP_LEFT,
  UP_RIGHT
};

enum Flags
{
  QUIET = 0,
  DOUBLE_PUSH = 0x10000,
  CASTLE_KING = 0x20000,
  CASTLE_QUEEN = 0x30000,
  CAPTURE = 0x40000,
  EP_CAPTURE = 0x50000,
  PROMO_N = 0x80000,
  PROMO_B = 0x90000,
  PROMO_R = 0xA0000,
  PROMO_Q = 0xB0000,
  PROMO_NC = 0xC0000,
  PROMO_BC = 0xD0000,
  PROMO_RC = 0xE0000,
  PROMO_QC = 0xF0000
};

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
constexpr int CHECK_MATE = -0xFFFF;
constexpr int BLACK = 5;
constexpr int WHITE = 0;
constexpr int BOARD_DIMMENSION = 8;
constexpr std::size_t MAX_MOVES = 100;

//---------CASTLING SQUARES--------------------------

constexpr bitboard_t WHITE_QUEEN_PIECES = BB(59U) | BB(58U) | BB(57U);
constexpr bitboard_t WHITE_KING_PIECES = BB(61U) | BB(62U);
constexpr bitboard_t BLACK_QUEEN_PIECES = BB(1U) | BB(2U) | BB(3U);
constexpr bitboard_t BLACK_KING_PIECES = BB(6U) | BB(5U);

// Attacked squares differ from occupied on queenside, also add king square
// since king can't be in check
constexpr bitboard_t WHITE_ATTACK_QUEEN =
    WHITE_QUEEN_PIECES ^ BB(57U) ^ BB(60U);
constexpr bitboard_t BLACK_ATTACK_QUEEN = BLACK_QUEEN_PIECES ^ BB(1U) ^ BB(4U);

constexpr bitboard_t WHITE_ATTACK_KING = WHITE_KING_PIECES ^ BB(60U);
constexpr bitboard_t BLACK_ATTACK_KING = BLACK_KING_PIECES ^ BB(4U);

// TODO: implement 50 move rule
struct StateInfo
{
  bitboard_t blockForKing;
  bitboard_t pinnedMask;
  bitboard_t enemyAttack;
  bitboard_t checkers;

  // uint8_t numCheckers;
  uint8_t castlingRights;
  uint8_t enPassant;

  // Incremental
  bitboard_t hashKey;
  score_t materialScore;
  score_t materialValue;
};

// Inherits irreversible info from StateInfo
struct Position
{
  StateInfo st;
  square_t kings[2];
  bitboard_t pieceBoards[10];
  bitboard_t teamBoards[3];

  // Check boards
  bitboard_t checkSquares[4];

  bool whiteToMove;
  uint16_t ply;
  bool operator==(const Position &pos) const
  {
    return memcmp(this, &pos, sizeof(Position)) == 0;
  }
};

struct MoveList
{
  move_t moves[100] = {};
  uint8_t curr = 0;
  inline void add(uint32_t move) { moves[curr++] = move; }
  inline uint8_t size() const { return curr; }
};

struct Move
{
  int eval;
  uint32_t move;
  bool operator<(const Move &other) const { return other.eval < eval; }
};

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

template <Piece p> inline bitboard_t pieces(const bitboard_t pieces[])
{
  return pieces[p] | pieces[BLACK + p];
}

template <bool white>
inline uint8_t getPiece(const bitboard_t pieces[], const uint8_t sq)
{
  const bitboard_t fromBB = BB(sq);
  constexpr uint8_t last = BLACK + BLACK * !white;
  for (uint8_t i = BLACK * !white; i < last; ++i)
  {
    if (pieces[i] & fromBB)
    {
      return i;
    }
  }
  return white ? King : King + 1;
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

// Return possible ep capturers
template <bool whiteToMove> inline bitboard_t EPpawns(const Position &pos)
{
  if constexpr (whiteToMove)
  {
    return Rank4 & pos.pieceBoards[WHITE];
  }
  return Rank5 & pos.pieceBoards[BLACK];
}

inline bool moreThanOne(bitboard_t b) { return b & (b - 1); }

constexpr uint8_t castlingModifiers[64] = {
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