#include "moveGen.h"
#include <cassert>

MoveGen::MoveGen() { initLineBB(LineBB); }

//--------------------Move generation-----------------------------------------

template <bool whiteToMove, bool castling, bool pins, bool onlyCapture>
void MoveGen::generateMoves(const Position &pos, MoveList &move_list) const {
  if (bitCount(pos.st.checkers) < 2) {
    generatePawnMoves<whiteToMove, pins, onlyCapture>(pos, move_list);
    generateKnightMoves<whiteToMove, pins, onlyCapture>(pos, move_list);
    generatePieceMoves<whiteToMove, Bishop, pins, onlyCapture>(pos, move_list);
    generatePieceMoves<whiteToMove, Rook, pins, onlyCapture>(pos, move_list);
    generatePieceMoves<whiteToMove, Queen, pins, onlyCapture>(pos, move_list);
  }
  generateKingMoves<whiteToMove, castling, onlyCapture>(pos, move_list);
}

template <bool whiteToMove, bool onlyCapture>
bool MoveGen::generateAllMoves(const Position &pos, MoveList &move_list) const {

  constexpr uint8_t shift = (2 * !whiteToMove);
  const uint8_t castling =
      static_cast<bool>(0b11 & (pos.st.castlingRights >> shift)) * 0b10;
  const uint8_t pins = static_cast<bool>(pos.st.pinnedMask) * 0b01;

  switch (castling | pins) {
  case 0b00:
    generateMoves<whiteToMove, false, false, onlyCapture>(pos, move_list);
    break;
  case 0b01:
    generateMoves<whiteToMove, false, true, onlyCapture>(pos, move_list);
    break;
  case 0b10:
    generateMoves<whiteToMove, true, false, onlyCapture>(pos, move_list);
    break;
  case 0b11:
    generateMoves<whiteToMove, true, true, onlyCapture>(pos, move_list);
    break;
  }
  return static_cast<bool>(pos.st.castlingRights);
}

template <bool whiteToMove, bool castling, bool onlyCapture>
void MoveGen::generateKingMoves(const Position &pos,
                                MoveList &move_list) const {
  // CHANGE KING, he is not a bitboard any more
  constexpr uint32_t mover = (King + !whiteToMove) << 24;

  const uint8_t king = pos.kings[!whiteToMove];
  const uint64_t moves = stepAttackBB<King>(king) &
                         moveableSquares<whiteToMove>(pos) &
                         ~pos.st.enemyAttack;
  const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];

  makeCaptureMove<whiteToMove>(pos.pieceBoards, move_list, moves & enemy, king,
                               CAPTURE | mover);

  if constexpr (onlyCapture) {
    return;
  }

  // Castling moves
  if constexpr (castling) {
    constexpr uint64_t castleKingAttack =
        whiteToMove ? WHITE_ATTACK_KING : BLACK_ATTACK_KING;
    constexpr uint64_t castleQueenAttack =
        whiteToMove ? WHITE_ATTACK_QUEEN : BLACK_ATTACK_QUEEN;
    constexpr uint64_t castleKingSquares =
        whiteToMove ? WHITE_KING_PIECES : BLACK_KING_PIECES;
    constexpr uint64_t castleQueenSquares =
        whiteToMove ? WHITE_QUEEN_PIECES : BLACK_QUEEN_PIECES;
    constexpr uint8_t castleKing = whiteToMove ? 0b0001 : 0b0100;
    constexpr uint8_t castleQueen = whiteToMove ? 0b0010 : 0b1000;
    const uint64_t board = pos.teamBoards[0];
    const uint64_t attack = pos.st.enemyAttack;

    if ((pos.st.castlingRights & castleKing) &&
        ((board & castleKingSquares) == 0) &&
        ((attack & castleKingAttack) == 0)) {
      constexpr uint32_t to = whiteToMove ? 62 : 6;
      constexpr uint32_t from = whiteToMove ? 60 << 8 : 4 << 8;
      move_list.add((uint32_t)(to | from | CASTLE_KING | mover));
    }
    if ((pos.st.castlingRights & castleQueen) &&
        ((board & castleQueenSquares) == 0) &&
        ((attack & castleQueenAttack) == 0)) {
      constexpr uint32_t to = whiteToMove ? 58 : 2;
      constexpr uint32_t from = whiteToMove ? 60 << 8 : 4 << 8;
      move_list.add((uint32_t)(to | from | CASTLE_QUEEN | mover));
    }
  }

  makePieceMove(move_list, moves & ~enemy, king, QUIET | mover);
}

// TODO: maybe inline makePawnMove, it is not that long
template <bool whiteToMove, bool pins, bool onlyCapture>
void MoveGen::generatePawnMoves(const Position &pos,
                                MoveList &move_list) const {
  constexpr uint64_t promoRank = whiteToMove ? Rank2 : Rank7;
  constexpr uint64_t doublePotential = whiteToMove ? Rank6 : Rank3;
  constexpr int8_t back = whiteToMove ? 8 : -8;
  constexpr uint32_t pID = whiteToMove ? 0 << 24 : 5 << 24;

  uint64_t pawns = getPieces<whiteToMove, Pawn>(pos);
  const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
  const uint64_t nonOccupied = ~pos.teamBoards[0];
  const uint64_t block = pos.st.blockForKing;
  const uint64_t nonPromo = pawns & ~promoRank;

  // Captures
  uint64_t capLeft = shift<whiteToMove, UP_LEFT>(nonPromo) & enemy & block;
  uint64_t capRight = shift<whiteToMove, UP_RIGHT>(nonPromo) & enemy & block;
  constexpr uint8_t backLeft = whiteToMove ? 9 : -7;
  constexpr uint8_t backRight = whiteToMove ? 7 : -9;

  makePawnCapture<whiteToMove, pins, false>(pos, move_list, capLeft, backLeft,
                                            CAPTURE | pID);
  makePawnCapture<whiteToMove, pins, false>(pos, move_list, capRight, backRight,
                                            CAPTURE | pID);

  if (pos.st.enPassant) {
    enPassantMoves<whiteToMove, pins>(pos, move_list, pos.st.enPassant);
  }

  // Promotions
  uint64_t promo = pawns & promoRank;

  if (promo) {
    const uint64_t p_push = shift<whiteToMove, UP>(promo) & nonOccupied & block;
    const uint64_t p_capLeft =
        shift<whiteToMove, UP_LEFT>(promo) & enemy & block;
    const uint64_t p_capRight =
        shift<whiteToMove, UP_RIGHT>(promo) & enemy & block;
    makePawnCapture<whiteToMove, pins, true>(pos, move_list, p_capLeft,
                                             backLeft, PROMO_NC | pID);
    makePawnCapture<whiteToMove, pins, true>(pos, move_list, p_capRight,
                                             backRight, PROMO_NC | pID);
    if constexpr (onlyCapture) {
      return;
    }
    makePawnMove<pins, true>(pos, move_list, p_push, back, PROMO_N | pID);
  }

  if constexpr (onlyCapture) {
    return;
  }

  // Non promotion push

  uint64_t push = shift<whiteToMove, UP>(nonPromo) & nonOccupied;
  uint64_t doublePush =
      shift<whiteToMove, UP>(push & doublePotential) & nonOccupied & block;
  push &= block;

  makePawnMove<pins, false>(pos, move_list, push, back, QUIET | pID);
  makePawnMove<pins, false>(pos, move_list, doublePush, back * 2,
                            DOUBLE_PUSH | pID);
}

template <bool whiteToMove, Piece p, bool pins, bool onlyCapture>
void MoveGen::generatePieceMoves(const Position &pos,
                                 MoveList &move_list) const {
  constexpr uint32_t pID = whiteToMove ? p << 24 : (p + 5) << 24;
  uint64_t bishops = getPieces<whiteToMove, p>(pos);
  const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
  const uint64_t nonTeam = moveableSquares<whiteToMove>(pos);
  const uint64_t block = pos.st.blockForKing;

  unsigned long b;
  while (bishops) {
    b = bitScan(bishops);
    bishops &= bishops - 1;
    uint64_t moves = attackBB<p>(pos.teamBoards[0], b) & nonTeam & block;
    if constexpr (pins) {
      if (BB(b) & pos.st.pinnedMask) {
        moves &= LineBB[b][pos.kings[!whiteToMove]];
      }
    }
    makeCaptureMove<whiteToMove>(pos.pieceBoards, move_list, moves & enemy, b,
                                 CAPTURE | pID);
    if constexpr (!onlyCapture) {
      makePieceMove(move_list, moves & ~enemy, b, QUIET | pID);
    }
  }
}

template <bool whiteToMove, bool pins, bool onlyCapture>
void MoveGen::generateKnightMoves(const Position &pos,
                                  MoveList &move_list) const {
  constexpr uint32_t pID = whiteToMove ? Knight << 24 : (Knight + 5) << 24;
  uint64_t knights = getPieces<whiteToMove, Knight>(pos);
  const uint64_t enemy = pos.teamBoards[2 - !whiteToMove];
  const uint64_t nonTeam = moveableSquares<whiteToMove>(pos);

  if constexpr (pins) {
    knights &= ~pos.st.pinnedMask;
  }

  unsigned long kn;
  while (knights) {
    kn = bitScan(knights);
    knights &= knights - 1;
    const uint64_t moves =
        stepAttackBB<Knight>(kn) & nonTeam & pos.st.blockForKing;
    makeCaptureMove<whiteToMove>(pos.pieceBoards, move_list, moves & enemy, kn,
                                 CAPTURE | pID);
    if constexpr (!onlyCapture) {
      makePieceMove(move_list, moves & ~enemy, kn, QUIET | pID);
    }
  }
}

//-----------------------Movemaking helper
// methods-----------------------------------------------------

template <bool whiteToMove, bool pins>
void MoveGen::enPassantMoves(const Position &pos, MoveList &ml,
                             uint8_t EP) const {
  // TODO: check on rank still allows EP, veri bad
  constexpr uint32_t pID = whiteToMove ? 0 << 24 : 5 << 24;
  constexpr uint64_t EPrank = whiteToMove ? Rank4 : Rank5;

  const uint64_t realPawn = shift<whiteToMove, DOWN>(BB(EP));
  const bool legalWhenCheck = pos.st.blockForKing & realPawn;
  uint64_t capturers =
      pawnAttackBB<!whiteToMove>(BB(EP)) & getPieces<whiteToMove, Pawn>(pos);

  // Special lateral pin case for en passant and check with double push
  const uint64_t rankPin = EPrank & (getPieces<!whiteToMove, Rook>(pos) |
                                     getPieces<!whiteToMove, Queen>(pos));
  const uint8_t king = pos.kings[!whiteToMove];
  const bool kingSameRank = (BB(king) & EPrank) && (EPrank & rankPin);

  const uint64_t boardWithoutEP = (realPawn | capturers) ^ pos.teamBoards[0];

  if (legalWhenCheck && (!kingSameRank || bitCount(capturers) != 1 ||
                         !(attackBB<Rook>(boardWithoutEP, king) & rankPin))) {
    while (capturers) {
      int fromSQ = bitScan(capturers);
      capturers &= capturers - 1;
      // Only add if the pawn isn't pinned or the move moves along the pin
      if constexpr (pins) {
        if ((BB(fromSQ) & pos.st.pinnedMask) == 0 ||
            (BB(EP) & LineBB[fromSQ][king])) {
          constructMove<false>(ml, fromSQ, EP, EP_CAPTURE | pID);
        }
      } else {
        constructMove<false>(ml, fromSQ, EP, EP_CAPTURE | pID);
      }
    }
  }
}

template <bool isPromotion>
inline void MoveGen::constructMove(MoveList &move_list, uint8_t from,
                                   uint8_t to, uint32_t flagAndPieces) const {
  if constexpr (isPromotion) {
    move_list.add(to | (from << 8) | PROMO_Q | flagAndPieces);
    move_list.add(to | (from << 8) | PROMO_R | flagAndPieces);
    move_list.add(to | (from << 8) | PROMO_B | flagAndPieces);
  }
  move_list.add(to | (from << 8) | flagAndPieces);
}

template <bool pin, bool isPromotion>
inline void MoveGen::makePawnMove(const Position &pos, MoveList &move_list,
                                  uint64_t toSQs, int8_t back,
                                  uint32_t flagAndPieces) const {
  while (toSQs) {
    int toSQ = bitScan(toSQs);
    toSQs &= toSQs - 1;
    if constexpr (pin) {
      const uint8_t from = toSQ + back;
      // Only add if the pawn isn't pinned or the move moves along the pin
      if ((BB(from) & pos.st.pinnedMask) == 0 ||
          (BB(toSQ) & LineBB[from][pos.kings[!pos.whiteToMove]])) {
        constructMove<isPromotion>(move_list, from, toSQ, flagAndPieces);
      }

    } else {
      constructMove<isPromotion>(move_list, toSQ + back, toSQ, flagAndPieces);
    }
  }
}

template <bool whiteToMove, bool pin, bool isPromotion>
inline void MoveGen::makePawnCapture(const Position &pos, MoveList &move_list,
                                     uint64_t toSQs, int8_t back,
                                     uint32_t flagAndPieces) const {
  while (toSQs) {
    int toSQ = bitScan(toSQs);
    toSQs &= toSQs - 1;
    const uint32_t flagUpdate =
        flagAndPieces | (getPiece<!whiteToMove>(pos.pieceBoards, toSQ) << 28);
    if constexpr (pin) {
      const uint8_t from = toSQ + back;
      // Only add if the pawn isn't pinned or the move moves along the pin
      if ((BB(from) & pos.st.pinnedMask) == 0 ||
          (BB(toSQ) & LineBB[from][pos.kings[!pos.whiteToMove]])) {
        constructMove<isPromotion>(move_list, from, toSQ, flagUpdate);
      }

    } else {
      constructMove<isPromotion>(move_list, toSQ + back, toSQ, flagUpdate);
    }
  }
}

inline void MoveGen::makePieceMove(MoveList &move_list, uint64_t toSQs,
                                   uint8_t from, uint32_t flagAndPieces) const {
  while (toSQs) {
    int toSQ = bitScan(toSQs);
    toSQs &= toSQs - 1;
    constructMove<false>(move_list, from, toSQ, flagAndPieces);
  }
}

template <bool whiteToMove>
inline void MoveGen::makeCaptureMove(const uint64_t pieceBoards[],
                                     MoveList &move_list, uint64_t toSQs,
                                     uint8_t from,
                                     uint32_t flagAndPiece) const {
  while (toSQs) {
    int toSQ = bitScan(toSQs);
    toSQs &= toSQs - 1;
    const uint32_t flagUpdate =
        flagAndPiece | (getPiece<!whiteToMove>(pieceBoards, toSQ) << 28);
    constructMove<false>(move_list, from, toSQ, flagUpdate);
  }
}

//--------------------Pins and checks-------------------------------------

template <bool whiteToMove> void MoveGen::checks(Position &pos) {
  uint64_t checkers = 0;
  const uint8_t king = pos.kings[!whiteToMove];

  // Opposite color knights
  const uint64_t knightCheck =
      stepAttackBB<Knight>(king) & getPieces<!whiteToMove, Knight>(pos);
  const uint64_t pawnCheck =
      pawnAttackBB<whiteToMove>(BB(king)) & getPieces<!whiteToMove, Pawn>(pos);

  // Is either a knight or pawn, cannot be both so bool is no problem
  checkers |= knightCheck | pawnCheck;

  pos.st.blockForKing |= knightCheck | pawnCheck;
  // If there is no check then all squares should be available
  const bool isBlock = pos.st.blockForKing;
  pos.st.blockForKing |= !isBlock * All_SQ;
  pos.st.checkers |= checkers;
}

template <bool whiteToMove> void MoveGen::setCheckSquares(Position &pos) const {
  const uint8_t enemyKing = pos.kings[!whiteToMove];
  pos.checkSquares[0] = pawnAttackBB<!whiteToMove>(BB(enemyKing));
  pos.checkSquares[1] = knightLookUp[enemyKing];
  pos.checkSquares[2] = attackBB<Bishop>(pos.teamBoards[0], enemyKing);
  pos.checkSquares[3] = attackBB<Rook>(pos.teamBoards[0], enemyKing);
}

/** @brief Finds pinned pieces and their corresponding pinned ray.
 *	It also finds checks given by sliders
 *	TODO: find blocker for king to detect discovered checks
 */
template <bool whiteToMove> void MoveGen::pinnedBoard(Position &pos) {
  uint64_t pinned = 0ULL;
  uint64_t blockForKing = 0ULL;
  uint64_t checkers = 0ULL;
  const uint8_t king = pos.kings[!whiteToMove];

  uint64_t snipers =
      (emptyAttack<Rook>(king) & (getPieces<!whiteToMove, Rook>(pos) |
                                  getPieces<!whiteToMove, Queen>(pos))) |
      (emptyAttack<Bishop>(king) & (getPieces<!whiteToMove, Bishop>(pos) |
                                    getPieces<!whiteToMove, Queen>(pos)));

  const uint64_t occupancy = pos.teamBoards[0];

  while (snipers) {
    // Pop first sniper
    int snip = bitScan(snipers);
    snipers &= snipers - 1;

    // Find squares between the pinner and the king
    const uint64_t betweenSquares = betweenBB(snip, king);
    const uint64_t betweenPieces = betweenSquares & occupancy;

    // Update masks
    const int count = bitCount(betweenPieces);
    if (count == 0) {
      blockForKing |= betweenSquares | BB(snip);
      checkers |= BB(snip);
    } else if (count == 1) {
      pinned |= betweenSquares | BB(snip);
    }
  }
  // Update pos
  pos.st.blockForKing = blockForKing;
  pos.st.pinnedMask = pinned;
  pos.st.checkers = checkers;
}

template <bool whiteToMove>
uint64_t MoveGen::findAttack(const Position &pos) const noexcept {
  uint64_t attack = 0;
  if constexpr (whiteToMove) {
    const uint64_t board_noKing = pos.teamBoards[0] ^ BB(pos.kings[0]);
    attack |= pieceAttack<Bishop>(board_noKing,
                                  pos.pieceBoards[7] | pos.pieceBoards[9]);
    attack |= pieceAttack<Rook>(board_noKing,
                                pos.pieceBoards[8] | pos.pieceBoards[9]);
    attack |= stepAttack<Knight>(pos.pieceBoards[6]);
    attack |= kingLookUp[pos.kings[1]];
    attack |= shift<false, UP_LEFT>(pos.pieceBoards[5]) |
              shift<false, UP_RIGHT>(pos.pieceBoards[5]);
  } else {
    const uint64_t board_noKing = pos.teamBoards[0] ^ BB(pos.kings[1]);
    attack |= pieceAttack<Bishop>(board_noKing,
                                  pos.pieceBoards[2] | pos.pieceBoards[4]);
    attack |= pieceAttack<Rook>(board_noKing,
                                pos.pieceBoards[3] | pos.pieceBoards[4]);
    attack |= stepAttack<Knight>(pos.pieceBoards[1]);
    attack |= kingLookUp[pos.kings[0]];
    attack |= shift<true, UP_LEFT>(pos.pieceBoards[0]) |
              shift<true, UP_RIGHT>(pos.pieceBoards[0]);
  }
  return attack;
}

// Template instanciation (remove later when they are actually called)

template void MoveGen::pinnedBoard<true>(Position &);
template void MoveGen::pinnedBoard<false>(Position &);
template void MoveGen::checks<true>(Position &);
template void MoveGen::checks<false>(Position &);
template uint64_t MoveGen::findAttack<true>(const Position &) const;
template uint64_t MoveGen::findAttack<false>(const Position &) const;
template void MoveGen::setCheckSquares<true>(Position &) const;
template void MoveGen::setCheckSquares<false>(Position &) const;

template bool MoveGen::generateAllMoves<true, true>(const Position &,
                                                    MoveList &) const;
template bool MoveGen::generateAllMoves<true, false>(const Position &,
                                                     MoveList &) const;
template bool MoveGen::generateAllMoves<false, true>(const Position &,
                                                     MoveList &) const;
template bool MoveGen::generateAllMoves<false, false>(const Position &,
                                                      MoveList &) const;

template uint64_t MoveGen::attackBB<Rook>(uint64_t board, uint8_t square) const;
template uint64_t MoveGen::attackBB<Bishop>(uint64_t board,
                                            uint8_t square) const;

//////////////////////////////////////////////////////////////////
////////////////// Attack lookups ////////////////////////////////
//////////////////////////////////////////////////////////////////

template <Piece p>
uint64_t MoveGen::pieceAttack(uint64_t board, uint64_t pieces) const {
  uint64_t attack = 0;
  while (pieces) {
    int fromSQ = bitScan(pieces);
    pieces &= pieces - 1;
    attack |= attackBB<p>(board, fromSQ);
  }
  return attack;
}

template <Piece p> uint64_t MoveGen::stepAttack(uint64_t pieces) const {
  uint64_t attack = 0;
  while (pieces) {
    int fromSQ = bitScan(pieces);
    pieces &= pieces - 1;
    attack |= stepAttackBB<p>(fromSQ);
  }
  return attack;
}

uint64_t MoveGen::attackersTo(uint8_t sq, const Position &pos) const {
  const uint64_t sqBB = BB(sq);
  const uint64_t board = pos.teamBoards[0];
  return (pawnAttackBB<true>(sqBB) & pos.pieceBoards[WHITE]) |
         (pawnAttackBB<false>(sqBB) & pos.pieceBoards[BLACK]) |
         (stepAttackBB<Knight>(sq) & pieces<Knight>(pos.pieceBoards)) |
         (attackBB<Bishop>(board, sq) &
          (pieces<Bishop>(pos.pieceBoards) | pieces<Queen>(pos.pieceBoards))) |
         (attackBB<Rook>(board, sq) &
          (pieces<Rook>(pos.pieceBoards) | pieces<Queen>(pos.pieceBoards)));
}
