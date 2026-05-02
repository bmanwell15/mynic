#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>

#include "Mynic.h"
#include "adapters.h"

#define MYNIC_CLI_VERSION "Mynic CLI version 1.0.0"

class CommandHandler {
    public:
        static std::shared_ptr<Mynic> decoder;
        static std::string exportFilePath;

        static void runCommand(std::string command);

    private:
        static void load(std::string command);
        static void interpret(std::vector<Token> commandTokens);
        // static void exportTo(std::vector<Token> commandTokens);
        // static void help(std::vector<Token> commandTokens);
        static void version();
        static void refresh();
        static void reset();
};

#endif