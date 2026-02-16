#ifndef MYNIC_H
#define MYNIC_H

#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <memory>

#include "lexer.h"
#include "AST.h"
#include "Interpreter.h"
#include "DecodedPacket.h"

#define PRINT_INDENT_SIZE 3


class Mynic {
    public:
        Mynic();

        bool loadFile(std::string& filename);

        void printPacket(const std::string& packetName);

        DecodedPacket decodePacket(const std::string& strBytes, const std::string& packetName);
        DecodedPacket decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName);

    private:
        std::shared_ptr<ASTNode> rootNode;
        std::unordered_map<std::string, ASTPacket> packets;
        AST ast;
        Interpreter interpreter;

        std::string collectFileCode(const std::string& FILENAME);
        void printPacketField(std::shared_ptr<ASTField> field, blockDepth_t indent);

};

#endif // MYNIC_H