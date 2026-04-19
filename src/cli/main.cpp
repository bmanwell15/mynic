#include <string>
#include <vector>
#include <cctype>
#include <chrono>
#include <iostream>

#include "Mynic.h"
#include "adapters.h"

#define VERSION "Alpha 0.0.1"
#define PACKET_NAME "BasicPacket"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "mynic <filename>" << std::endl;
        return 1;
    }

    std::string possibleFileName = argv[1];

    if (possibleFileName == "--version" || possibleFileName == "-v") {
        std::cout << "Version " << VERSION << std::endl;
        return 0;
    }

    // if (possibleFileName == "--help" || possibleFileName == "-h") {
    //     std::cout << "Help Screen" << std::endl;
    //     return 0;
    // }

    Mynic decoder;
    auto startTime = std::chrono::high_resolution_clock::now();
    decoder.loadFile(possibleFileName);
    auto endTime =  std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Loaded definition file in " << duration << " ms\n" << std::endl;

    decoder.printSchema();

    while (true) {
        std::cout << "mynic > ";
        std::string inputLine;
        std::getline(std::cin, inputLine);
        if (inputLine == "exit" || inputLine == "quit") break;

        DecodedPacket result = decoder.decodePacket(inputLine, PACKET_NAME);
        std::cout << adapters::json::encode(result) << std::endl;
    }
    std::cout << "bye\n";
    return 0;
}