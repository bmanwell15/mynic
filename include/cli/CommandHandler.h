#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include <vector>

#include "Mynic.h"
#include "adapters.h"

#define MYNIC_CLI_VERSION "Mynic CLI version 1.0.0"

class CommandHandler {
    public:
        static std::shared_ptr<Mynic> decoder; // The Mynic decoder being used for the CLI

        /**
         * Parses and performs the actions for a command.
         */
        static void runCommand(std::string command);

    private:
        // Command to load another file into the AST system.
        static void load(std::string command);

        // Interprets a string of bytes using the AST definitions. "interpret 0x022204 as packetName"
        static void interpret(std::string command);
        
        // Prints the versions of Mynic Core and Mynic CLI
        static void version();

        // Refreshes and rereads all loaded Mynic files
        static void refresh();

        // Removes all loaded Mynic files
        static void reset();
};

#endif