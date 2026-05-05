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

#include "ErrorHandler.h"
#include "BitQueue.h"
#include "AST.h"
#include "MynicLib.h"

#define NS_PER_SECOND       1'000'000'000LL
#define NS_PER_MILLISECOND  1'000'000LL
#define NS_PER_MICROSECOND  1'000LL

class AST; // Forward declaration

struct InterpretedField {
    std::string name;
    size_t sizeInBits;
    NodeType type;
};

struct InterpretedPacket : public InterpretedField {
    std::vector<std::shared_ptr<InterpretedField>> fields;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

struct InterpretedPrimitiveValue : public InterpretedField {
    Value value;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
    std::string datatype;
};

struct InterpretedBitfield : public InterpretedField {
    std::string name;
    std::vector<std::shared_ptr<InterpretedField>> subfields;
};

struct InterpretedUnionfield : public InterpretedField {
    std::string name;
    std::vector<std::shared_ptr<InterpretedField>> subfields;
};

struct InterpretedArray : public InterpretedField {
    std::vector<std::shared_ptr<InterpretedField>> list;
};

struct DecodedPacket {
    std::string packetName;
    std::vector<uint8_t> rawBytes;
    std::shared_ptr<InterpretedPacket> rootField;
    std::string timestamp;
    std::vector<InterpreterWarning> warnings;
};

class Interpreter {
    public:
        Interpreter();
        AST* ast;
        BitQueue bitQueue;
        std::shared_ptr<InterpretedPacket> rootNode;
        // std::unordered_map<std::string, std::string> defTypeAliases; // {newType, existingType}
        std::vector<std::shared_ptr<ASTEnum>> enums;
        std::shared_ptr<ASTPrimitiveValueSettings> globalSettings;
        bool terminateSignal;
        bool isEndOfStream;

        DecodedPacket interpretBytes(const std::vector<uint8_t>& dataBytes, const std::string& packetName, const  std::shared_ptr<ASTNode>& pAstTree);
        std::optional<Value> tryEvaluateASTExpression(std::shared_ptr<ASTExpression> node);
        Value evaluateASTExpression(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node);
        bool evaluateASTCondition(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node);
        void throwWarning(InterpreterWarningCodes code, std::string message);
    
    private:
        std::shared_ptr<DecodedPacket> decodedPacket;
        std::shared_ptr<ASTNode> astTree;

        std::shared_ptr<InterpretedField> findField(const std::shared_ptr<InterpretedField>& field, const std::string& name);
        std::shared_ptr<InterpretedPacket> interpretPacket(const ASTPacket& packetDef);
        std::shared_ptr<InterpretedField> interpretField(std::shared_ptr<ASTField> field);
        Value interpretValue(ASTPrimitiveValue& field);
        std::optional<Value> getParsedValue(const std::shared_ptr<InterpretedField>& field, const std::string& varName);
        void enforcePostInterpretationSettings(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive);
        Value evaluateASTExpressionFunctionCall(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall);
        Value evaluateBinaryOp(std::string op, Value left, Value right);
};

#endif