#include <gtest/gtest.h>


#include "Engine.h"
#include "tests.h"

namespace ExplorerChessTest {


bool testPos(const enginePtr &engine, std::string &&fen, const bitboard_t count,
             const int depth) {
    Position pos;
    engine->fenInit(pos, fen);

    if (pos.whiteToMove) {
        return engine->perft<true>(pos, depth) == count;
    }
    return engine->perft<false>(pos, depth) == count;
}

// clang-format off
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////// TEST SUITES /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
// clang-format on

TEST_F(PerftSuite, startpos) {
    EXPECT_TRUE(testPos(
        m_engine, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        119060324ULL, 6));
}

TEST_F(PerftSuite, KiwipeteDoubleCheckPosition) {
    EXPECT_TRUE(testPos(
        m_engine,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
        193690690ULL, 5));
}

TEST_F(PerftSuite, SpecialEnpassantPin) {
    EXPECT_TRUE(testPos(m_engine, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ",
                        11030083ULL, 6));
}

TEST_F(PerftSuite, WhiteInCheckBlackCantCastleButCanPromote) {
    EXPECT_TRUE(testPos(
        m_engine,
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        706045033ULL, 6));
}

TEST_F(PerftSuite, KnightOnf2WhitePromoteWithPin) {
    EXPECT_TRUE(testPos(
        m_engine, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        3048196529ULL, 6));
}

TEST_F(PerftSuite, BishopPinPawnEnPassantNotPossible) {
    EXPECT_TRUE(testPos(m_engine, "8/8/4k3/8/2p5/8/B2P2K1/8 w - - 0 1",
                        102503850ULL, 8));
}

TEST_F(PerftSuite, AllCastleAllowedAllRooksCanBeTaken) {
    EXPECT_TRUE(testPos(m_engine, "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1",
                        1509218880ULL, 6));
}

TEST_F(PerftSuite, CastleRightsAllowedNonePossible) {
    EXPECT_TRUE(testPos(m_engine, "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",
                        2010267707ULL, 6));
}

} // namespace ExplorerChessTest


//     // "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1", 2010267707ULL, 6));
//     // REQUIRE(testPos(engine, "2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1",
//     // 905613447ULL, 8)); REQUIRE(testPos(engine,
//     "8/8/1P2K3/8/2n5/1q6/8/5k2 b -
//     // - 0 1", 197013195ULL, 7)); REQUIRE(testPos(engine,
//     "4k3/1P6/8/8/8/8/K7/8
//     // w - - 0 1", 397481663ULL, 9)); REQUIRE(testPos(engine,
//     // "8/P1k5/K7/8/8/8/8/8 w - - 0 1", 153850274ULL, 9));
//     // REQUIRE(testPos(engine, "K1k5/8/P7/8/8/8/8/8 w - - 0 1",
//     85822924ULL,
//     // 11)); REQUIRE(testPos(engine, "8/k1P5/8/1K6/8/8/8/8 w - - 0 1",
//     // 173596091ULL, 10)); REQUIRE(testPos(engine,
//     "8/8/2k5/5q2/5n2/8/5K2/8 b
//     -
//     // - 0 1", 104644508ULL, 7));
// }