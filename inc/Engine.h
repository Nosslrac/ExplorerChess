#pragma once
#include "moveGenV2.h"
#include "position.h"
#include <deque>
#include <memory>

struct History final
{
  constexpr explicit History(const Move m, const StateInfo &st)
      : move(m), state(st){};
  Move move;
  StateInfo state;
};

using historyList_t = std::deque<History>;
using historyListPtr_t = std::unique_ptr<historyList_t>;

class Engine
{
public:
  explicit Engine();
  Engine(const Engine &) = delete;
  Engine(Engine &&) = delete;
  Engine const &operator=(const Engine &other) = delete;
  Engine &operator=(Engine &&other) = delete;
  ~Engine() = default;

  void makeMove(Move move);
  void undoMove();
  std::uint64_t runPerft(int depth);
  void initFen(const std::string &fen);
  void printPieces() const;
  void printMoves() const;

private:
  Position m_pos;
  historyListPtr_t m_historyList;
};