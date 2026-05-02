#include "CommandHandler.h"

std::shared_ptr<Mynic> CommandHandler::decoder = std::make_shared<Mynic>();
std::string CommandHandler::exportFilePath;

std::vector<std::string> split(const std::string &txt, char ch) {
    size_t pos = txt.find(ch);
    size_t initialPos = 0;
    std::vector<std::string> strs;

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back(txt.substr( initialPos, pos - initialPos ));
        initialPos = pos + 1;
        pos = txt.find(ch, initialPos);
    }

    // Add the last one
    strs.push_back(txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ));
    return strs;
}

void CommandHandler::runCommand(std::string command) {
    auto commandTokens = Lexer::tokenize(command);
    if (commandTokens[0].value == "interpret" || commandTokens[0].type == OPEN_PAREN) return CommandHandler::interpret(commandTokens);
    if (commandTokens[0].value == "version") return CommandHandler::version();
    if (commandTokens[0].value == "refresh") return CommandHandler::refresh();
    if (commandTokens[0].value == "reset") return CommandHandler::reset();
    if (commandTokens[0].value == "load") return CommandHandler::load(command);

    std::cout << "Command '" + commandTokens[0].value + "' not found.\n";
}

void CommandHandler::interpret(std::vector<Token> commandTokens) {
    if (commandTokens.size() < 4) {
        std::cout << "Invalid Command: Command missing parameters.\n";
        return;
    }
    std::string byteStr = commandTokens[1].value;
    if (commandTokens[2].value != "as" && commandTokens[2].type != COMMA) {
        std::cout << "Expected keyword 'as' or comma. Instead recieved '" + commandTokens[2].value + "'\n";
        return;
    }
    
    std::string packetName = commandTokens[3].value;
    auto startTime = std::chrono::high_resolution_clock::now();
    auto decodedPacket = CommandHandler::decoder->decodePacket(byteStr, packetName);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Interpreted Packet in " << duration << " ms\n" << std::endl;
    std::cout << adapters::json::encode(decodedPacket) << std::endl;
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
    auto commandChunks = split(command, ' ');
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