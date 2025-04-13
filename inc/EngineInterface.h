#include <memory>

#include "Engine.h"

namespace ExplorerChess {
enum class EngineMode
{
  UCI,
  NONE
};

class EngineParser
{
public:
  EngineParser() : m_engine(), m_mode(EngineMode::NONE) {}
  void runInterface();

private:
  void receiveInput();
  Engine m_engine;
  EngineMode m_mode;
};

class CommandArgs
{
public:
  explicit CommandArgs(const std::string &commandLine, const bool takeArg);
  CommandArgs() = delete;
  ~CommandArgs() = default;
  void print() const;
  std::size_t size() const;
  const std::string &getArg() const { return m_argument; }
  const std::unique_ptr<CommandArgs> &getNext() const { return m_next; };

private:
  std::string m_argument;
  std::unique_ptr<CommandArgs> m_next;
};

namespace UCI {
const std::string ENGINE_ID{"ExplorerChessV1"};
void uciInput();
void runUCI(EngineParser *parser, Engine *engine);
void runUCI(EngineParser *parser, Engine *engine);
} // namespace UCI

} // namespace ExplorerChess