#include "evaluate.h"

Evaluation::Evaluation(const MoveGen &moveGen) : m_moveGen(moveGen) {}

template <bool whiteToMove>
int Evaluation::evaluate(const Position &pos) const {
    // const int safety = Evaluation::kingSafety<whiteToMove>(pos);
    // const int passedPawn =
    // Evaluation::passedPawns<whiteToMove>(pos.pieceBoards); const int
    // undefendedScore = Evaluation::undefendedPieces<whiteToMove>(pos);
    constexpr int flip = whiteToMove ? 1 : -1;
    return (pos.st.materialScore + pos.st.materialValue) * flip;
}

/**
 * NOTE: only call on initialization
 */
int Evaluation::staticPieceEvaluation(const bitboard_t pieces[10]) const {
    int materialBalance = 0;
    for (int i = 0; i < 5; ++i) {
        materialBalance +=
            (bitCount(pieces[i]) - bitCount(pieces[i + 5])) * pieceValue[i];
    }
    return materialBalance;
}

template <bool whiteToMove>
int Evaluation::undefendedPieces(const Position &pos) const {
    const bitboard_t defended = m_moveGen.findAttack<!whiteToMove>(
        pos); // Find what the moving side is defending
    constexpr int8_t team     = whiteToMove ? 0 : 5;
    bitboard_t minorNodefense = ~defended & (pos.pieceBoards[Knight + team] |
                                             pos.pieceBoards[Bishop + team]);
    return -5 * bitCount(minorNodefense);
}

/**
 * NOTE: only call on initialization
 */
int Evaluation::initMaterialValue(const Position &pos) const {
    int whiteValue = 0;
    int blackValue = 0;
    for (uint8_t i = 0; i < 64; ++i) {
        if (pos.kings[0] == i) {
            whiteValue += PSTs[10][i];
        } else if (pos.kings[1] == i) {
            blackValue += PSTs[11][i];
        } else if (pos.teamBoards[1] & BB(i)) {
            whiteValue += PSTs[getPiece<true>(pos.pieceBoards, i)][i];
        } else if (pos.teamBoards[2] & BB(i)) {
            blackValue += PSTs[getPiece<false>(pos.pieceBoards, i)][i];
        }
    }
    return whiteValue - blackValue;
}

template <bool whiteToMove>
int Evaluation::kingSafety(const Position &pos) const {
    // Idea: extract surrounding bits for the king if there are a certain amount
    // of sliding attackers Problem not sure what to do what should be
    // considered since opponent has to exploit it See if there are a lot of
    // pins close to king

    constexpr uint8_t kingID = whiteToMove ? 0 : 1;
    int               score  = 0;
    const int         king   = pos.kings[kingID];
    // const bitboard_t bishopKnight = pos.pieceBoards[1 + 5 * kingID] |
    // pos.pieceBoards[2 + 5 * kingID]; //Good pieces for blocking
    const bitboard_t kingSquares = m_moveGen.stepAttackBB<King>(king);
    // const bitboard_t kingShield = forwardSquares<whiteToMove>(king) &
    // kingSquares; //Squares right infront of king

    score -= bitCount(pos.st.enemyAttack & kingSquares) *
             10; // Penalty if enemy is attacking king shield

    // Not sure what to do with the result
    // auto teamShield = pext(bishopKnight, kingShield);
    // auto pawnShield = pext(pos.pieceBoards[5 * kingID], kingShield);
    // std::cout << "King shield: " << teamShield << pawnShield << "\n";

    return score;
}

template <bool whiteToMove>
int Evaluation::passedPawns(const bitboard_t piece[10]) const {
    // Detect the files in front and the adjacent
    constexpr uint8_t team       = whiteToMove ? 0 : 5;
    constexpr uint8_t enemy      = whiteToMove ? 5 : 0;
    bitboard_t        pawns      = piece[team];
    const bitboard_t  enemyPawns = piece[enemy];
    int               score      = 0;
    unsigned long     sq;
    while (pawns) {
        sq = bitScan(pawns);
        const bitboard_t forwardSQ =
            adjacentFiles[sq & 7] & forwardSquares<whiteToMove>(sq);
        if (forwardSQ & enemyPawns) { // Passer
            score += 10;
            const bitboard_t defenders = shift<!whiteToMove, UP_LEFT>(BB(sq)) |
                                         shift<!whiteToMove, UP_RIGHT>(BB(sq));
            if (defenders & piece[team]) { // Protected passer
                score += 20;
            }
        }
        pawns &= pawns - 1;
    }
    return score;
}

template int Evaluation::evaluate<true>(const Position &) const;
template int Evaluation::evaluate<false>(const Position &) const;