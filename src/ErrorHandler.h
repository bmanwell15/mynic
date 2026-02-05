#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <vector>
#include <string>
#include <iostream>

#include "lexer.h"

class ErrorHandler {
    public:
        static void throwError(const std::string& message, std::vector<Token>& tokens, size_t i, std::string packetName="");

    private:
        static void printLine(std::vector<Token>& tokens, size_t i, bool includeErrorSquiggle, blockDepth_t targetDepth=0);
        static void printLine(std::vector<Token>& tokens, size_t lineNum);
        static void printBlock(std::vector<Token>& tokens, size_t i);
};

#endif