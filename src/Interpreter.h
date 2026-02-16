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

#include "DecodedPacket.h"
#include "ErrorHandler.h"
#include "BitQueue.h"

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

        std::shared_ptr<InterpretedPacket> interpretPacket(const ASTPacket& packetDef, BitQueue& bitQueue);
        std::shared_ptr<InterpretedField> interpretField(std::shared_ptr<ASTField> field, BitQueue& bitQueue);
        Value interpretValue(ASTPrimitiveValue& field, BitQueue& bitQueue);
};

#endif