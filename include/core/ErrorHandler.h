#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include "lexer.h"

// Warning codes used by the Mynic interpreter.
enum class InterpreterWarningCodes {
    UNKNOWN,
    VALIDATION_FAIL,
    PACKET_NOT_FOUND,
    EXPR_VAR_NOT_PRIMITIVE,
    ENUM_VAL_NOT_FOUND,
    ENUM_NOT_FOUND,
    ENUM_TYPE_NOT_INT,
    UNKNOWN_DATATYPE,
    INVALID_STRING_OPERATION,
    INVALID_BINARY_OPERATOR_TYPE,
    FUNCTION_NOT_FOUND,
    VARIABLE_NOT_FOUND_IN_EXPR,
    VARIABLE_NOT_FOUND_IN_SWITCH,
    ARRAY_LENGTH_NOT_INT,
    WRONG_NUMBER_OF_PARAMETERS,
    VARIABLE_NOT_ARITHMETIC,
    BIT_QUEUE_INDEX_OUT_OF_BOUNDS,
    BIT_QUEUE_EMPTY
};

// Represents an interpreter warning and its message.
struct InterpreterWarning {
    InterpreterWarningCodes warningType;
    std::string errorMessage;
};

// Handles parser errors and interpreter warnings.
class ErrorHandler {
    public:
        // Throws a syntax or parsing error including token context.
        static void throwError(const std::string& message, std::vector<Token>& tokens, size_t i, std::string packetName="");

        // Creates an interpreter warning object.
        static InterpreterWarning throwInterpreterWarning(InterpreterWarningCodes code, std::string message);

    private:
        // Prints a token line with optional error highlighting.
        static void printLine(std::vector<Token>& tokens, size_t i, bool includeErrorSquiggle, blockDepth_t targetDepth=0);

        // Prints a specific line from token stream.
        static void printLine(std::vector<Token>& tokens, size_t lineNum);

        // Prints the token block around an error.
        static void printBlock(std::vector<Token>& tokens, size_t i);
};

#endif