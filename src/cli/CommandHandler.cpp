#include "CommandHandler.h"

std::shared_ptr<Mynic> CommandHandler::decoder = std::make_shared<Mynic>();
std::string CommandHandler::exportFilePath;

void CommandHandler::runCommand(std::string command) {
    auto commandTokens = Lexer::tokenize(command);
    if (commandTokens[0].value == "interpret") return CommandHandler::interpret(command);
    if (commandTokens[0].value == "version") return CommandHandler::version();
    if (commandTokens[0].value == "refresh") return CommandHandler::refresh();
    if (commandTokens[0].value == "reset") return CommandHandler::reset();
    if (commandTokens[0].value == "load") return CommandHandler::load(command);

    std::cout << "Command '" + commandTokens[0].value + "' not found.\n";
}

void CommandHandler::interpret(std::string command) {
    auto commandChunks = CommandHandler::decoder->split(command, ' ');
    if (commandChunks.size() < 4) {
        std::cout << "Invalid Command: Command missing parameters.\n";
        return;
    }
    std::string byteStr = commandChunks[1];
    if (commandChunks[2] != "as") {
        std::cout << "Expected keyword 'as' or comma. Instead recieved '" + commandChunks[2] + "'\n";
        return;
    }
    
    std::string packetName = commandChunks[3];
    auto startTime = std::chrono::high_resolution_clock::now();
    auto decodedPacket = CommandHandler::decoder->decodePacket(byteStr, packetName);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << adapters::json::encode(decodedPacket);
    std::cout << "\n\nInterpreted Packet in " << duration << " ms\n";
}

void CommandHandler::version() {
    std::cout << MYNIC_CLI_VERSION << std::endl;
    std::cout << MYNIC_CORE_VERSION << std::endl;
    std::cout << "\nCreated by Benjamin Manwell.\nhttps://github.com/bmanwell15\n";
}

void CommandHandler::refresh() {
    auto fileNames = Lexer::fileNames;

    Lexer::fileNames.clear();
    CommandHandler::decoder = std::make_shared<Mynic>();

    for (auto& fileName : fileNames) {
        CommandHandler::decoder->loadFile(fileName);
    }
    CommandHandler::decoder->printSchema();
}

void CommandHandler::reset() {
    std::cout << "Are you sure you want to clear all loaded packets? (yes/no) ";
    std::string inputLine;
    std::getline(std::cin, inputLine);
    if (inputLine != "yes") return;
    Lexer::fileNames.clear();
    CommandHandler::decoder = std::make_shared<Mynic>();
    std::cout << "All files and packets cleared.\n";
}

void CommandHandler::load(std::string command) {
    auto commandChunks = CommandHandler::decoder->split(command, ' ');
    for (size_t i = 1; i < commandChunks.size(); i++) { // commandChunks[0] is 'load'
        if (CommandHandler::decoder->loadFile(commandChunks[i])) {
            std::cout << "Loaded file '" + commandChunks[i] + "'...\n";
        } else {
            std::cout << "\nERROR: Tried to load file '" + commandChunks[i] + "' but failed...\n\n";
        }
    }
    std::cout << '\n';
    CommandHandler::decoder->printSchema();
}