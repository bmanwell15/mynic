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
        // Constructs a new Mynic parser/interpreter instance.
        Mynic();

        // Cleans up the Mynic instance.
        ~Mynic();

        // Splits a string by a single character delimiter.
        std::vector<std::string> split(const std::string &txt, char ch);

        // Loads a Mynic definition file into the AST.
        bool loadFile(std::string& filename);

        // Prints the entire loaded schema to the console.
        void printSchema();

        // Prints the specified packet definition.
        void printPacket(const std::string& packetName);

        // Prints the specified segment definition.
        void printSegment(const std::string& segmentName);

        // Returns the library version string.
        std::string version();

        // Decodes a packet from a hex string or bit string.
        DecodedPacket decodePacket(const std::string& strBytes, const std::string& packetName, bool asBits=false);

        // Decodes a packet from a vector of bytes.
        DecodedPacket decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName);

        // Decodes multiple packets from a file separated by the given character.
        std::vector<DecodedPacket> decodeFile(std::string filepath, char sep, std::string& packetName);

        // Writes decoded packet JSON to a file.
        void exportToFile(DecodedPacket& decodedPacket, std::string& filepath, bool appendMode=false);
    private:
        std::shared_ptr<ASTNode> rootNode;
        AST ast;
        Interpreter interpreter;

        // Reads the source code from a file.
        std::string collectFileCode(const std::string& FILENAME);

        // Converts a string of bits to bytes.
        std::vector<uint8_t> bitsToBytes(const std::string& bitStr);
};

#endif