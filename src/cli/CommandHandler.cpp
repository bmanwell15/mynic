#include "CommandHandler.h"

std::shared_ptr<Mynic> CommandHandler::decoder = std::make_shared<Mynic>();

void CommandHandler::runCommand(std::string c) {
    auto command = parseCommand(c);
    if (command.command == "interpret" || command.command == "/i") return CommandHandler::interpret(command);
    if (command.command == "version" || command.command == "/v") return CommandHandler::version();
    if (command.command == "refresh" || command.command == "/r") return CommandHandler::refresh();
    if (command.command == "reset") return CommandHandler::reset();
    if (command.command == "load") return CommandHandler::load(command);

    std::cout << "Command '" + command.command + "' not found.\n";
}

CommandPrompt CommandHandler::parseCommand(std::string& command) {
    auto commandChunks = CommandHandler::decoder->split(command, ' ');
    CommandPrompt prompt;
    prompt.command = commandChunks[0];
    for (int i = 1; i < commandChunks.size();i++) {
        auto chunk = commandChunks[i];
        if (chunk[0] == '-') {
            CommandFlag flag;
            uint8_t doubleDash = 0;
            if (chunk[1] == '-') doubleDash++;
            auto chunkWithoutDash = chunk.substr(1 + doubleDash);
            flag.flagName = chunkWithoutDash;
            auto it = CommandHandler::flagsParamCount.find(chunkWithoutDash);
            if (it == CommandHandler::flagsParamCount.end()) continue;
            for (int j = 0; j < it->second && i + 1 < commandChunks.size(); j++) {
                flag.params.push_back(commandChunks[++i]);
            }
            prompt.flags.push_back(flag);
        } else {
            prompt.commandParams.push_back(chunk);
        }
    }
    return prompt;
}

void CommandHandler::interpret(CommandPrompt& command) {
    if (command.commandParams.size() < 2) {
        std::cout << "Invalid Command: Command missing parameters.\n";
        return;
    }
    std::string byteStr = command.commandParams[0];
    std::string packetName = command.commandParams[1];

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

void CommandHandler::load(CommandPrompt& command) {
    for (const auto& param : command.commandParams) { // commandChunks[0] is 'load'
        if (adapters::file::exists(param) && CommandHandler::decoder->loadFile(param)) {
            std::cout << "Loaded file '" + param + "'...\n";
        } else {
            std::cout << "\nERROR: Tried to load file '" + param + "' but failed...\n\n";
        }
    }
    std::cout << '\n';
}

// void CommandHandler::interpretFile(CommandPrompt& command) {

// }