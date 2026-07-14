#include "input_parser.hpp"
#include <iostream>
#include <sstream>
#include <vector>

std::vector<std::string> getTokens(const std::string &inputString){

    std::vector<std::string> tokens;
    std::stringstream ss(inputString);

    std::string token;
    while (ss >> token){
        tokens.push_back(token);
    }

    return tokens;
}

void handleSingleCommand(Command &command,Action action,std::vector<std::string> tokens){
    if (tokens.size() != 1){
        std::cout << "invalid command call\n";
        return;
    }

    command.action = action;
}

Command getNextCommand(){
    Command command;
    command.action = INVALID;

    std::string inputString;

    while (command.action == INVALID){

        std::cout << "\t(debugger)\t";
        std::getline(std::cin,inputString);
        auto tokens = getTokens(inputString);
        if (tokens.size() == 0){
            continue;
        }

        std::string actionStr = tokens[0];
        if (actionStr == "step"){

            if (tokens.size() > 2){
                std::cout << "invalid command call\n";
                continue;
            }

            command.counter = (tokens.size() == 2) ? std::stoi(tokens[1]) : 1;
            command.action = STEP;
        }

        if (actionStr == "ds"){
            if (tokens.size() > 3){
                std::cout << "invalid command call\n";
                continue;
            }

            command.counter = (tokens.size() > 1) ? std::stoi(tokens[1]) : 1;
            command.offset = (tokens.size() == 3) ? std::stoi(tokens[2]) : 0;
            command.action = DISASSEMBLE;
        }

        if (actionStr == "break"){
            if (tokens.size() != 2){
                std::cout << "invalid break call\n";
                continue;
            }

            std::string strOffset = tokens[1];
            command.offset = std::stoi(tokens[1],nullptr,0);
            command.action = BREAK;
        }

        if (actionStr == "quit"){
            handleSingleCommand(command,QUIT,tokens);
        }

        if (actionStr == "start"){
            handleSingleCommand(command,START,tokens);
        }

        if (actionStr == "cpu"){
            handleSingleCommand(command,CPU_INFO,tokens);
        }

        if (actionStr == "run"){
            handleSingleCommand(command,RUN,tokens);
        }



    }

    return command;
}