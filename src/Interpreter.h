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

#include "DecodedPacket.h"
#include "ErrorHandler.h"
#include "BitQueue.h"

#define NS_PER_SECOND       1'000'000'000LL
#define NS_PER_MILLISECOND  1'000'000LL
#define NS_PER_MICROSECOND  1'000LL

class AST; // Forward declaration

class Interpreter {
    public:
        Interpreter();
        AST* ast;
        std::unordered_map<std::string, std::string> defTypeAliases; // {newType, existingType}
        std::vector<std::shared_ptr<ASTEnum>> enums;
        std::shared_ptr<ASTPrimitiveValueSettings> globalSettings;

        DecodedPacket interpretBytes(const std::vector<uint8_t>& dataBytes, const std::string& packetName, const  std::shared_ptr<ASTNode>& rootNode);
    
    private:
        std::shared_ptr<DecodedPacket> decodedPacket;
        std::shared_ptr<ASTNode> astTree;

        std::shared_ptr<InterpretedField> findField(const std::shared_ptr<InterpretedField>& field, const std::string& name);
        std::shared_ptr<InterpretedPacket> interpretPacket(const ASTPacket& packetDef, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode = nullptr);
        std::shared_ptr<InterpretedField> interpretField(std::shared_ptr<ASTField> field, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode = nullptr);
        Value interpretValue(ASTPrimitiveValue& field, BitQueue& bitQueue);
        std::optional<Value> getParsedValue(const std::shared_ptr<InterpretedField>& field, const std::string& varName, BitQueue& bitQueue);
        void enforcePostInterpretationSettings(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode);
        Value evaluateASTExpression(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode);
};

#endif