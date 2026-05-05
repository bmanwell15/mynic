#include <string>
#include <vector>
#include <cctype>
#include <chrono>
#include <iostream>

#include "CommandHandler.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "mynic <filename>" << std::endl;
        return 1;
    }

    std::string possibleFileName = argv[1];

    auto startTime = std::chrono::high_resolution_clock::now();
    CommandHandler::runCommand("load " + possibleFileName);
    auto endTime =  std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Loaded definition file in " << duration << " ms\n" << std::endl;

    while (true) {
        std::cout << "mynic > ";
        std::string inputLine;
        std::getline(std::cin, inputLine);
        if (inputLine == "exit" || inputLine == "quit") break;
        CommandHandler::runCommand(inputLine);
    }
    return 0;
}