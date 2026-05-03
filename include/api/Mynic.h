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

#include "adapters.h"

#define MYNIC_CORE_VERSION "Mynic version 1.0.0"
#define PRINT_INDENT_SIZE 3


class Mynic {
    public:
        Mynic();
        ~Mynic();

        std::vector<std::string> split(const std::string &txt, char ch);

        bool loadFile(std::string& filename);

        void printSchema();
        void printPacket(const std::string& packetName);
        void printSegment(const std::string& segmentName);

        std::string version();

        DecodedPacket decodePacket(const std::string& strBytes, const std::string& packetName, bool asBits=false);
        DecodedPacket decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName);
        std::vector<DecodedPacket> decodeFile(std::string filepath, char sep, std::string& packetName);

        void exportToFile(DecodedPacket& decodedPacket, std::string& filepath, bool appendMode=false);
    private:
        std::shared_ptr<ASTNode> rootNode;
        AST ast;
        Interpreter interpreter;

        std::string collectFileCode(const std::string& FILENAME);
        std::vector<uint8_t> bitsToBytes(const std::string& bitStr);
};

#endif