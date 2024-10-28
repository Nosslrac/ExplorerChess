#include <cstdlib>
#include <iostream>
#include <memory>
#include <string.h>

#include "EngineInterface.h"
#include "Engine.h"
#include "GUI.h"

namespace ExplorerChess{
    namespace UCI{
        
        inline void runGo(const std::unique_ptr<CommandArgs>& args, Engine& engine){
            const std::string& secondArg = args->getArg();
            if(secondArg == "perft"){
                int depth = std::stoi(args->getNext()->getArg());
                auto start = std::chrono::system_clock::now();
                engine.goPerft(static_cast<uint32_t>(depth));
                auto end = std::chrono::system_clock::now();
                auto duration =
                    duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << "Execution time: " << duration << " ms\n";
            }
            // Do other go commands
        }

        inline void uciInput(){
            std::cout << "id name " << ENGINE_ID << '\n';
            std::cout << "id author " << "Nosslrac\n";
            std::cout << "uciok\n";
        }

        
       
    }

    inline void setPosition(const CommandArgs& args, Engine& engine){
        const std::string& secondArg = args.getArg();
        if(secondArg == "startpos"){
            engine.position("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if(secondArg == "fen"){
            engine.position(args.getNext()->getArg());
        }
        else{
            std::cout << "Unknown position command\n";
        }
    }

    inline void makeMove(const CommandArgs& args, Engine& engine){
        engine.makeMove(args.getArg());
    }


    void EngineParser::runInterface(){
        std::cout << "Welcome to ExplorerChess v1.0\n";
        while(true){
            receiveInput();
        }

    }


    CommandArgs::CommandArgs(const std::string& commandLine, const bool takeArg = false){
        size_t nextSpace = commandLine.find(' ');
        if(takeArg || nextSpace > commandLine.size()){
            m_argument = commandLine;
            m_next = nullptr;
            return;
        }
        m_argument = commandLine.substr(0, nextSpace);
        const std::string& nextArg = commandLine.substr(nextSpace + 1);

        if(m_argument == "fen"){
            m_next = std::make_unique<CommandArgs>(nextArg, true);
            return;
        }
        m_next = std::make_unique<CommandArgs>(nextArg);
    }

    void CommandArgs::print() const{
        std::cout << m_argument << '\n';
        if(m_next){
            m_next->print();
        }
    }

    size_t CommandArgs::size() const{
        if(!m_next){
            return 1;
        }
        return 1 + m_next->size();
    }



    void EngineParser::receiveInput(){
        std::string userInput;
        std::getline(std::cin, userInput);
        CommandArgs args(userInput);

        if(m_mode == EngineMode::UCI){
            // Do uci stuff
            if(args.getArg() == "go"){
                UCI::runGo(args.getNext(), m_engine);
            }
            else if(args.getArg() == "d"){
                GUI::print_pieces(m_engine.getPos());
            }
            
        }
        if(args.getArg() == "position"){
            setPosition(*args.getNext(), m_engine);
        }
        else if(args.getArg() == "make"){
            makeMove(*args.getNext(), m_engine);
        }
        else if(args.getArg() == "uci"){
            m_mode = EngineMode::UCI;
            UCI::uciInput();
        }
        else{
            std::cout << "Unknown command\n";
        }
    }
}