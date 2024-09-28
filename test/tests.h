#pragma once
#include <gtest/gtest.h>
#include <memory>

#include "Engine.h"


namespace ExplorerChessTest {
using enginePtr = std::unique_ptr<Engine>;

bool testPos(const enginePtr &engine, std::string &&fen, bitboard_t count,
             int depth);


class PerftSuite : public testing::Test {
  protected:
    PerftSuite() = default;

    ~PerftSuite() override = default;

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override { m_engine = std::make_unique<Engine>(); }

    void TearDown() override {}

    std::unique_ptr<Engine> m_engine;
};

} // namespace ExplorerChessTest