#include "ErrorHandler.h"

void ErrorHandler::throwError(const std::string& message, std::vector<Token>& tokens, size_t i, std::string packetName) {
    std::cout << "Error in file " << Lexer::fileNames[tokens[i].fileIndex] << ", line " << tokens[i].lineNumber << ":\n";
    std::cout << "\t" << message << "\n\n";

    if (tokens[i].blockDepth > 1) { // If the error is inside a block, print the whole block for context. Note that default block depth is 1 (global scope)
        printBlock(tokens, i);
        std::exit(EXIT_FAILURE);
    }

    std::cout << tokens[i].lineNumber - 2 << " |\t...\n";
    printLine(tokens, tokens[i].lineNumber - 1);
    std::cout << '\n';
    printLine(tokens, i, true);
    std::cout << '\n';
    printLine(tokens, tokens[i].lineNumber + 1);
    std::cout << '\n';
    std::cout << tokens[i].lineNumber + 2 << " |\t...\n";
    
    std::exit(EXIT_FAILURE);
}

void ErrorHandler::printLine(std::vector<Token>& tokens, size_t i, bool includeErrorSquiggle, blockDepth_t targetDepth) {
    int startIndex = i;
    while (startIndex > 0 && tokens[startIndex].lineNumber == tokens[i].lineNumber) {startIndex--;} // Fine the starting token index of the line

    int endIndex = i;
    while (endIndex < tokens.size() - 1 && tokens[endIndex].lineNumber == tokens[i].lineNumber) {endIndex++;} // Find the ending token index of the line

    size_t charsUntilErrorToken = 0;
    size_t charsAfterErrorToken = 0;
    short addSpace = 0;

    std::cout << tokens[i].lineNumber << " |" << std::string(targetDepth*3, ' ');
    for (int j = startIndex + 1; j < endIndex; j++) {
        if (tokens[j].type == NEW_LINE) continue;
        std::cout << tokens[j].value;
        if (tokens[j].type == IDENTIFIER && j + 1 < endIndex && (tokens[j + 1].type != SEMI_COLON && tokens[j + 1].type != COMMA)) {
            std::cout << ' '; // Add space after identifiers unless next token is a semicolon or comma
            addSpace = 1;
        } else {
            addSpace = 0;
        }

        if (j < i)
            charsUntilErrorToken += tokens[j].value.size() + addSpace;
        else if (j > i)
            charsAfterErrorToken += tokens[j].value.size() + addSpace;
    }
    if (!includeErrorSquiggle) return;

    std::cout << "\n    " << std::string(targetDepth*3, ' ')
            << std::string(charsUntilErrorToken, '~')
            << std::string(tokens[i].value.size(), '^') // +1 for the newline
            << std::string(charsAfterErrorToken + 1, '~');
}

void ErrorHandler::printLine(std::vector<Token>& tokens, size_t lineNum) {
    for (size_t i = 0; i < tokens.size(); i++) {
        if (tokens[i].lineNumber == lineNum) {
            printLine(tokens, i, false);
            return;
        }
    }
}

void ErrorHandler::printBlock(std::vector<Token>& tokens, size_t i) {
    blockDepth_t targetDepth = tokens[i].blockDepth;

    int startIndex = i;
    while (startIndex > 0 && tokens[startIndex].blockDepth == targetDepth) {startIndex--;} // Fine the starting token of the block

    int endIndex = i;
    while (endIndex < tokens.size() - 1 && tokens[endIndex].blockDepth == targetDepth) {endIndex++;} // Find the ending token index of the block

    for (int j = startIndex; j < endIndex + 1; j++) {

        if (tokens[j].lineNumber == tokens[i].lineNumber) {
            printLine(tokens, i, true, tokens[j].blockDepth);
            std::cout << '\n';
        } else {
            short w = 0;
            if (tokens[j].type == CLOSE_BRACKET) w = 1;
            printLine(tokens, j, false, tokens[j + w].blockDepth);
            std::cout << '\n';
        }
        size_t currentLine = tokens[j].lineNumber;
        while (tokens[j + 1].type == NEW_LINE || (j + 1 < endIndex && tokens[j + 1].lineNumber == currentLine)) { // Skip to next line
            j++;
        }
    }
}

InterpreterWarning ErrorHandler::throwInterpreterWarning(InterpreterWarningCodes code, std::string message) {
    InterpreterWarning warning;
    warning.warningType = code;
    warning.errorMessage = message;
    return warning;
}