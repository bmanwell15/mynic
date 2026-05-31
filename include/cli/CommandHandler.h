#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>

#include "Mynic.h"
#include "adapters.h"

#define MYNIC_CLI_VERSION "Mynic CLI version 1.0.0"

struct CommandFlag {
    std::string flagName;
    std::vector<std::string> params;
};

struct CommandPrompt {
    std::string command;
    std::vector<std::string> commandParams;
    std::vector<CommandFlag> flags;
};

class CommandHandler {
    public:
        static std::shared_ptr<Mynic> decoder; // The Mynic decoder being used for the CLI

        // Parses and performs the actions for a command.
        static void runCommand(std::string command);

    private:
        inline static const std::unordered_map<std::string, uint8_t> flagsParamCount = {
            {"fileName", 1}
        };

        static CommandPrompt parseCommand(std::string& command);

        // Command to load another file into the AST system.
        static void load(CommandPrompt& command);

        // Interprets a string of bytes using the AST definitions. "interpret 0x022204 as packetName"
        static void interpret(CommandPrompt& command);
        
        // Prints the versions of Mynic Core and Mynic CLI
        static void version();

        // Refreshes and rereads all loaded Mynic files
        static void refresh();

        // Removes all loaded Mynic files
        static void reset();

        // Interprets the file
        // static void interpretFile(CommandPrompt& command);
};

#endif