#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <variant>
#include <cstring>
#include <iostream>
#include <bits/stdc++.h>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <typeinfo>

#include "ErrorHandler.h"
#include "BitQueue.h"
#include "AST.h"
#include "MynicLib.h"

#define NS_PER_SECOND       1'000'000'000LL
#define NS_PER_MILLISECOND  1'000'000LL
#define NS_PER_MICROSECOND  1'000LL

class AST; // Forward declaration

// Common interpreted field properties.
struct InterpretedField {
    std::string name;
    size_t sizeInBits;
    NodeType type;
};

// Parsed packet/segment representation.
struct InterpretedPacket : public InterpretedField {
    std::vector<std::shared_ptr<InterpretedField>> fields;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

// Parsed primitive value representation.
struct InterpretedPrimitiveValue : public InterpretedField {
    Value value;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
    std::string datatype;
};

// Parsed bitfield container.
struct InterpretedBitfield : public InterpretedField {
    std::string name;
    std::vector<std::shared_ptr<InterpretedField>> subfields;
};

// Parsed union field container.
struct InterpretedUnionfield : public InterpretedField {
    std::string name;
    std::vector<std::shared_ptr<InterpretedField>> subfields;
};

// Parsed array container.
struct InterpretedArray : public InterpretedField {
    std::vector<std::shared_ptr<InterpretedField>> list;
};

// Result structure returned by packet decoding.
struct DecodedPacket {
    std::string packetName;
    std::vector<uint8_t> rawBytes;
    std::shared_ptr<InterpretedPacket> rootField;
    std::string timestamp;
    std::vector<InterpreterWarning> warnings;
};

// Interpreter that executes packet parsing against AST definitions.
class Interpreter {
    public:
        Interpreter();

        AST* ast;
        BitQueue bitQueue;
        std::shared_ptr<InterpretedPacket> rootNode;
        std::vector<std::shared_ptr<ASTEnum>> enums;
        std::shared_ptr<ASTPrimitiveValueSettings> globalSettings;
        bool terminateSignal;
        bool isEndOfStream;

        // Interprets byte data using the named packet definition.
        DecodedPacket interpretBytes(const std::vector<uint8_t>& dataBytes, const std::string& packetName, const  std::shared_ptr<ASTNode>& pAstTree);

        // Attempts to evaluate an AST expression, returning optional value.
        std::optional<Value> tryEvaluateASTExpression(std::shared_ptr<ASTExpression> node);

        // Evaluates an AST expression to a concrete value.
        Value evaluateASTExpression(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node);

        // Evaluates a boolean condition expression.
        bool evaluateASTCondition(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node);

        // Records a runtime warning from the interpreter.
        void throwWarning(InterpreterWarningCodes code, std::string message);

        // Pops bits from the interpreter's bit queue.
        uint64_t popBits(size_t bitNum);
    
    private:
        std::shared_ptr<DecodedPacket> decodedPacket;
        std::shared_ptr<ASTNode> astTree;
        std::shared_ptr<InterpretedPacket> currentPacket;  // Track current parsing context

        std::shared_ptr<InterpretedField> findField(const std::shared_ptr<InterpretedField>& field, const std::string& name);
        std::shared_ptr<InterpretedPacket> interpretPacket(const ASTPacket& packetDef, std::shared_ptr<InterpretedPacket> rootNode=nullptr);
        std::shared_ptr<InterpretedField> interpretField(std::shared_ptr<ASTField> field);
        Value interpretValue(ASTPrimitiveValue& field);
        std::optional<Value> getParsedValue(const std::shared_ptr<InterpretedField>& field, const std::string& varName);
        void enforcePostInterpretationSettings(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive);
        Value evaluateASTExpressionFunctionCall(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall);
        Value evaluateBinaryOp(std::string op, Value left, Value right);
};

#endif