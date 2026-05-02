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

#define MYNIC_CORE_VERSION "Mynic Core version 1.0.0"
#define PRINT_INDENT_SIZE 3


class Mynic {
    public:
        Mynic();
        ~Mynic();

        bool loadFile(std::string& filename);

        void printSchema();
        void printPacket(const std::string& packetName);
        void printSegment(const std::string& segmentName);

        std::string version();

        DecodedPacket decodePacket(const std::string& strBytes, const std::string& packetName, bool asBits=false);
        DecodedPacket decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName);

    private:
        std::shared_ptr<ASTNode> rootNode;
        AST ast;
        Interpreter interpreter;

        std::string collectFileCode(const std::string& FILENAME);
        void printPacketField(std::shared_ptr<ASTField> field, blockDepth_t indent);
        std::vector<uint8_t> bitsToBytes(const std::string& bitStr);
};

#endif // MYNIC_H