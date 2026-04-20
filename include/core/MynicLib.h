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
        static Value mathMax(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathMin(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathPow(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathSqrt(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathLog(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter, uint8_t logBase = 0);
        static Value mathRound(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathSign(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathAbs(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTFunctionCall> functionCall, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode, Interpreter* interpreter);
        static Value mathPi();
        static Value mathE();
    private:

};

#endif