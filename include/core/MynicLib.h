#ifndef MYNICLIB_H
#define MYNICLIB_H

#include <memory>
#include <cctype>
#include <cmath>

#include "AST.h"
#include "Interpreter.h"

struct InterpretedPrimitiveValue;
struct InterpretedPacket;
class MynicLib {
    public:
        static Value mathMax(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathMin(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathPow(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathSqrt(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathLog(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter, uint8_t logBase = 0);
        static Value mathRound(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathSign(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathAbs(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter);
        static Value mathPi();
        static Value mathE();

        static void rewind(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter);
        static void skip(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter);
        static void seek(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter);
        static void terminateIf(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter);
    private:

};

#endif