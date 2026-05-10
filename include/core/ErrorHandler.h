#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

#include "lexer.h"

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

struct InterpreterWarning {
    InterpreterWarningCodes warningType;
    std::string errorMessage;
};

class ErrorHandler {
    public:
        static void throwError(const std::string& message, std::vector<Token>& tokens, size_t i, std::string packetName="");
        static InterpreterWarning throwInterpreterWarning(InterpreterWarningCodes code, std::string message);

    private:
        static void printLine(std::vector<Token>& tokens, size_t i, bool includeErrorSquiggle, blockDepth_t targetDepth=0);
        static void printLine(std::vector<Token>& tokens, size_t lineNum);
        static void printBlock(std::vector<Token>& tokens, size_t i);
};

#endif