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

    std::string possibleTag = argv[1];

    if (possibleTag == "-v" || possibleTag == "--version" || possibleTag == "version") {
        CommandHandler::runCommand("version");
        return 0;
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 1; i < argc; i++) {
        std::string fileName = argv[i]; // Convert from char* to string
        CommandHandler::runCommand("load " + fileName);
    }
    auto endTime =  std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Loaded definition file in " << duration << " ms\n" << std::endl;
    CommandHandler::decoder->printSchema();

    while (true) {
        std::cout << "mynic > ";
        std::string inputLine;
        std::getline(std::cin, inputLine);
        if (inputLine == "exit" || inputLine == "quit") break;
        CommandHandler::runCommand(inputLine);
    }
    return 0;
}