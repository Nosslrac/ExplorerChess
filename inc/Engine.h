#pragma once
#include "bitboardUtil.h"
#include "evaluate.h"
#include "moveGen.h"
#include "robin_hood.h"
#include "zobristHash.h"
#include <cassert>
#include <stack>
#include <string>

class Engine {
  public:
    Engine();
    Engine(const Engine &)                       = delete;
    Engine(Engine &&)                            = delete;
    Engine const &operator=(const Engine &other) = delete;
    Engine       &operator=(Engine &&other)      = delete;
    ~Engine();

    void run();

    template <bool whiteToMove> uint64_t perft(Position &pos, uint32_t depth);
    // Init position
    void fenInit(Position &, std::string);

  private:
    // API
    void runUI();

    // Making moves
    template <bool whiteToMove, bool castle>
    void doMove(Position &pos, uint32_t move);
    template <bool whiteToMove, bool castle>
    void undoMove(Position &pos, uint32_t move);
    template <bool whiteToMove, bool castleKing>
    inline void doCastle(Position &pos);

    // Debugging
    template <bool whiteToMove, bool castle> void moveIntegrity(Position &pos);

    // Perft
    template <bool whiteToMove, bool castle>
    uint64_t search(Position &pos, int depth);

    // Evaluation search
    template <bool whiteToMove> Move analysis(Position &pos, int depth);

    template <bool whiteToMove, bool castle>
    int negaMax(Position &pos, int alpha, int beta, int depth);

    template <bool whiteToMove, bool castle>
    const Move traverseMoves(Position &pos, int depth, int alpha, int beta);

    template <bool whiteToMove> int qSearch(Position &pos, int alpha, int beta);

    // Position info
    Position    m_pos;
    MoveGen     m_moveGen;
    Evaluation  m_evaluation;
    ZobristHash m_zobristHash;
    uint64_t    m_nodes;
    uint64_t    m_captureOnlyNodes;

    // History stack
    std::stack<StateInfo> prevStates;
    std::stack<uint32_t>  prevMoves;

    // Perft transposition
    // std::unordered_map<uint64_t, uint64_t> _transpositionTable;

    robin_hood::unordered_flat_map<uint64_t, uint64_t> _transpositionTable;

    // Evaluation transposition
    robin_hood::unordered_flat_map<uint64_t, int> _evalTransposition;
};