#include "position.h"
#include <cstdint>

template void Position::doMove<Side::WHITE>(MoveV2);
template void Position::doMove<Side::BLACK>(MoveV2);
template void Position::undoMove<Side::WHITE>(MoveV2);
template void Position::undoMove<Side::BLACK>(MoveV2);

void Position::doMove(MoveV2 move)
{
  if (m_whiteToMove)
  {
    doMove<Side::WHITE>(move);
  }
  else
  {
    doMove<Side::BLACK>(move);
  }
}

void Position::undoMove(MoveV2 move)
{
  if (m_whiteToMove)
  {
    undoMove<Side::WHITE>(move);
  }
  else
  {
    undoMove<Side::BLACK>(move);
  }
}

template <Side s> void Position::doMove(MoveV2 move)
{
  // bitboard_t newHash = pos.st.hashKey ^ m_zobristHash.whiteToMoveHash;

  // Get move info
  const uint8_t from = move.getFrom();
  const uint8_t to = move.getTo();
  // const uint8_t mover =
  //     move.getMover(); // TODO: change this to read from 8x8 board
  // Captured will be 0 if there is no capture so important to do capture before
  // move
  const uint8_t captured = move.getCaptured();
  const uint32_t flags = move.getFlags();
  const bitboard_t fromBB = BB(from);
  const bitboard_t toBB = BB(to);

  constexpr uint8_t team = s == Side::WHITE ? 0 : 1;
  constexpr uint8_t enemy = s == Side::WHITE ? 1 : 0;

  // const bitboard_t teamBoard = m_teamBoards[team];

  m_teamBoards[team] ^= fromBB ^ toBB;
  // Increment position value based on prev move
  // pos.st.materialScore +=
  //     scoreFavor * m_evaluation.pieceScoreChange(from, to, mover);

  // Remove old ep hash if there was one
  if (m_st->enPassant != 0)
  {
    // newHash ^= m_zobristHash.epHash[pos.st.enPassant & 7]; // File ep hash
    m_st->enPassant = 0;
  }

  // TODO: fix this to fetch from 8x8 board
  const PieceType movingType = PieceType::PAWN;

  if (movingType == PieceType::KING)
  {
    m_kings[static_cast<std::uint8_t>(s)] = to;
    if (flags == CASTLE_KING)
    {
      doCastle<s>(CastleSide::KING);
      // TODO: update zobrist hash
    }
    else if (flags == CASTLE_QUEEN)
    {
      doCastle<s>(CastleSide::QUEEN);
      // TODO: update zobrist hash
    }
  }
  else
  {
    m_pieceBoards[movingType] ^= fromBB ^ toBB;
  }

  if (flags == CAPTURE)
  {
    m_pieceBoards[captured] ^= toBB;
    m_teamBoards[enemy] ^= toBB;
  }

  if (movingType == PieceType::PAWN)
  {
    constexpr char add = s == Side::WHITE ? 8 : -8;
    if (flags == QUIET)
    {
      // DO nothing
    }
    else if (flags == EP_CAPTURE)
    {
      m_pieceBoards[PAWN] ^= BB((to + add));
      m_teamBoards[enemy] ^= BB((to + add));
      // newHash ^=
      //     m_zobristHash.pieceHash[enemyPawn][to + add]; // Remove pawn from
      //     hash
      // pos.st.materialValue +=
      //     scoreFavor * m_evaluation.getPieceValue(captured + blackOffset);
    }
    else if (flags == DOUBLE_PUSH && hasPawnsOnEpRank<s>())
    {
      m_st->enPassant = static_cast<square_t>(to + add);
      // newHash ^= m_zobristHash.epHash[(to + add) & 7]; // Add new ep hash
    }
    else // Promotion
    {
      m_pieceBoards[PAWN] ^= toBB;
      m_pieceBoards[1 + move.getPromo()] |= toBB;
      // TODO: fix zobrist hash and material

      if (flags & CAPTURE)
      {
        m_pieceBoards[captured] ^= toBB;
        m_teamBoards[enemy] ^= toBB;
        // Fix zobrist and material value
      }
    }
  }

  m_st->castlingRights &= BitboardUtil::castlingModifiers[from];
  m_st->castlingRights &= BitboardUtil::castlingModifiers[to];

  // Restore occupied
  // m_teamBoards[0] = pos.teamBoards[1] | pos.teamBoards[2];

  m_whiteToMove = !m_whiteToMove;
  // newHash ^= pos.ply * m_zobristHash.moveHash;

  m_ply++;

  // // newHash ^= pos.ply * m_zobristHash.moveHash;

  // // pos.st.hashKey = newHash;

  m_st->enemyAttack =
      0; // Find attacks: m_moveGen.findAttack<!whiteToMove>(pos);
  // TODO: update attacks and pins the correct way with new movegen
  // m_moveGen.pinnedBoard<!whiteToMove>(pos);
  // m_moveGen.checks<!whiteToMove>(pos);
  // m_moveGen.setCheckSquares<!whiteToMove>(pos);
}

template <Side s> void Position::undoMove(MoveV2 move)
{
  const square_t from = move.getFrom();
  const square_t to = move.getTo();
  const bitboard_t fromToBB = BB(from) | BB(to);

  constexpr uint8_t team = s == Side::WHITE ? 0 : 1;

  m_teamBoards[team] ^= fromToBB;
}