#include "Mynic.h"

std::vector<std::string> Mynic::split(const std::string &txt, char ch) {
    std::string current = "";
    std::vector<std::string> strs;

    for (size_t i = 0; i < txt.size(); i++) {
        if (txt[i] == ch) {
            strs.push_back(current);
            current.clear();
            continue;
        }
        if (txt[i] == '\"') {
            i++; // Move past "
            while (txt[i] != '\"') {
                current += txt[i++];
            }
            i++; // Move past "
            continue;
        }
        current += txt[i];
    }
    strs.push_back(current);
    return strs;
}

Mynic::Mynic() : ast(this) {
    interpreter = Interpreter();
    ast.interpreter = &interpreter;
    interpreter.ast = &ast;
}

Mynic::~Mynic() = default;

std::string Mynic::version() {
    return MYNIC_CORE_VERSION;
}

bool Mynic::loadFile(std::string& filename) {
    // If filename already in Lexer, don't parse again
    if (std::find(Lexer::fileNames.begin(), Lexer::fileNames.end(), filename) != Lexer::fileNames.end()) return true;

    std::string fileContent = collectFileCode(filename);
    if (fileContent == "") return false;

    Lexer::fileNames.push_back(filename);
    std::vector<Token> tokens = Lexer::tokenize(fileContent);

    // for (int i = 0; i < tokens.size(); i++) { // Debug only
    //     Lexer::printToken(tokens[i]);
    // }

    if (rootNode) {
        std::shared_ptr<ASTNode> importedRootNode = ast.parseTokensToAST(tokens, false);
        for (const auto& [name, property] : importedRootNode->properties) {
            rootNode->properties[name] = property;
        }
        return true;
    }

    rootNode = ast.parseTokensToAST(tokens);
    return true;
}

std::string Mynic::collectFileCode(const std::string& FILENAME) {
    std::string line; // Will hold a single line from the file
    std::string fileContent;
    std::ifstream file(FILENAME);

    if (!file) {return "";}

    while (std::getline(file, line)) {fileContent += line + "\n";}
    file.close();
    return fileContent;
}

DecodedPacket Mynic::decodePacket(const std::string& strBytes, const std::string& packetName, bool asBits) {
    if (asBits)
        return decodePacket(bitsToBytes(strBytes), packetName);
    
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < strBytes.length(); i += 2) {
        std::string hexByte = strBytes.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(hexByte, nullptr, 16));
        bytes.push_back(byte);
    }
    return decodePacket(bytes, packetName);
}

DecodedPacket Mynic::decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName) {
    return interpreter.interpretBytes(dataBytes, packetName, rootNode);
}

std::vector<uint8_t> Mynic::bitsToBytes(const std::string& bitStr) {
    std::vector<uint8_t> asBytes;
    std::string formattedBitStr;

    if (bitStr.size() % 8 != 0)
        formattedBitStr = std::string(bitStr.size() % 8, '0') + bitStr;

    for (size_t i = 0; i < bitStr.size(); i++) {
        if (i % 8 == 0) asBytes.push_back(0);
        if (bitStr[i] == '1') {
            asBytes[asBytes.size() - 1] += (uint8_t)(pow(2, 7 - (i % 8)));
        } else if (bitStr[i] != '0') {
            throw std::runtime_error("String of bytes caught unrecognisable character '" + std::to_string(bitStr[i]) + "'.");
        }
    }
    return asBytes;
}

void Mynic::printSchema() {
    if (rootNode)
        adapters::objects::printSchema(rootNode);
}

void Mynic::printPacket(const std::string& packetName) {
    for (const auto& [name, field] : rootNode->properties) {
        if (field && field->type == NodeType::PACKET && name == packetName) {
            auto packet = std::static_pointer_cast<ASTPacket>(field);
            adapters::objects::printPacket(packet);
        }
    }
}

void Mynic::printSegment(const std::string& segmentName) {
    for (const auto& [name, field] : rootNode->properties) {
        if (field && field->type == NodeType::SEGMENT && name == segmentName) {
            auto packet = std::static_pointer_cast<ASTPacket>(field);
            adapters::objects::printPacket(packet);
        }
    }
}

void Mynic::exportToFile(DecodedPacket& decodedPacket, std::string& filepath, bool appendMode) {
    if (appendMode)
        adapters::file::append(filepath, adapters::json::encode(decodedPacket));
    else
        adapters::file::write(filepath, adapters::json::encode(decodedPacket));
}

std::vector<DecodedPacket> Mynic::decodeFile(std::string filepath, char sep, std::string& packetName) {
    std::string allPacketStr = adapters::file::read(filepath);
    auto packets = split(allPacketStr, sep);
    std::vector<DecodedPacket> parsedPackets;
    for (const auto& packet : packets) {
        parsedPackets.push_back(decodePacket(packet, packetName));
    }
    return parsedPackets;
}