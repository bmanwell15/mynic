#include "Mynic.h"

Mynic::Mynic() : ast(this) {
    interpreter = Interpreter();
    ast.interpreter = &interpreter;
    interpreter.ast = &ast;
}

bool Mynic::loadFile(std::string& filename) {
    // Implementation for loading a file goes here
    std::string fileContent = collectFileCode(filename);

    Lexer::fileNames.push_back(filename);

    std::vector<Token> tokens = Lexer::tokenize(fileContent);

    // for (int i = 0; i < tokens.size(); i++) { // Debug only
    //     Lexer::printToken(tokens[i]);
    // }

    if (rootNode) {
        std::shared_ptr<ASTNode> importedRootNode = ast.parseTokensToAST(tokens);
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

    if (!file) {throw std::runtime_error("Could not open file: " + FILENAME);}

    while (std::getline(file, line)) {fileContent += line + "\n";}
    file.close();
    return fileContent;
}

void Mynic::printPacket(const std::string& packetName) {
    for (const auto& [name, field] : rootNode->properties) {
        if (field->type == NodeType::PACKET && name == packetName) {
            ASTPacket packet = static_cast<ASTPacket&>(*field);
            std::cout << packet.name << ':' << std::endl;
            for (const auto& pktField : packet.fields) {
                if (pktField->type == NodeType::PRIMITIVE) {
                    ASTPrimitiveValue primField = static_cast<ASTPrimitiveValue&>(*pktField);
                    std::cout << "  " << primField.datatype << ' ' << primField.name << " (" << primField.sizeInBits << " bits);" << std::endl;
                }
            }
            return;
        }
    }
    std::cout << "Packet " << packetName << " not found." << std::endl;
}


DecodedPacket Mynic::decodePacket(const std::string& strBytes, const std::string& packetName) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < strBytes.length(); i += 2) {
        std::string hexByte = strBytes.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(hexByte, nullptr, 16));
        bytes.push_back(byte);
    }
    return decodePacket(bytes, packetName);
}

DecodedPacket Mynic::decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName) {
    DecodedPacket decoded = interpreter.interpretBytes(dataBytes, packetName, rootNode);
    return decoded;
}