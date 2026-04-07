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

void Mynic::printPacketField(std::shared_ptr<ASTField> field, blockDepth_t indent) {
    std::string indentStr = std::string(indent * PRINT_INDENT_SIZE, ' ');
    if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<ASTPrimitiveValue>(field);
        std::cout << indentStr << primField->datatype << ' ' << primField->name << " (" << primField->sizeInBits << " bits);" << std::endl;
    } else if (field->type == NodeType::BITFIELD) {
        auto bitField = std::static_pointer_cast<ASTBitfield>(field);
        std::cout << indentStr << "Bitfield " << bitField->name << " {" << std::endl;
        for (const auto& subfield : bitField->subfields) {
            printPacketField(subfield, indent + 1);
        }
        std::cout << indentStr << "}" << std::endl;
    } else if (field->type == NodeType::UNION) {
        auto unionfield = std::static_pointer_cast<ASTUnion>(field);
        std::cout << indentStr << "Union " << unionfield->name << " {" << std::endl;
        for (const auto& subfield : unionfield->subfields) {
            printPacketField(subfield, indent + 1);
        }
        std::cout << indentStr << "}" << std::endl;
    } else if (field->type == NodeType::ARRAY) {
        auto arrayfield = std::static_pointer_cast<ASTArray>(field);
        std::string bitSizeStr = arrayfield->length != 0 ? std::to_string(arrayfield->elementSchema->sizeInBits * arrayfield->length) : "?";
        std::string insideBracketStr = arrayfield->length == 0 ? "" : std::to_string(arrayfield->length);
        std::cout << indentStr << arrayfield->elementSchema->datatype << ' ' << arrayfield->elementSchema->name << "[" << insideBracketStr << "] (" << bitSizeStr << " bits);" << std::endl;
    } else if (field->type == NodeType::SEGMENT) {
        auto segDef = std::static_pointer_cast<ASTPacket>(field);
        std::cout << indentStr << segDef->name << " {" << std::endl;
        for (const auto& subfield : segDef->fields) {
            printPacketField(subfield, indent + 1);
        }
        std::cout << indentStr << "}" << std::endl;
    }
}

void Mynic::printPacket(const std::string& packetName) {
    for (const auto& [name, field] : rootNode->properties) {
        if (field && field->type == NodeType::PACKET && name == packetName) {
            auto packet = std::static_pointer_cast<ASTPacket>(field);
            std::cout << packet->name << ':' << std::endl;
            for (const auto& pktField : packet->fields) {
                printPacketField(pktField, 1);
            }
            return;
        }
    }
    std::cout << "Packet " << packetName << " not found." << std::endl;
}

void Mynic::printSegment(const std::string& segmentName) {
    for (const auto& [name, field] : rootNode->properties) {
        if (field && field->type == NodeType::SEGMENT && name == segmentName) {
            auto packet = std::static_pointer_cast<ASTPacket>(field);
            std::cout << packet->name << ':' << std::endl;
            for (const auto& pktField : packet->fields) {
                printPacketField(pktField, 1);
            }
            return;
        }
    }
    std::cout << "Segment " << segmentName << " not found." << std::endl;
}

void Mynic::printSchema() {
    for (const auto& [name, packet] : rootNode->properties) {
        if (packet->type == NodeType::PACKET) {
            printPacket(name);
            std::cout << std::endl;
        }
    }
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
    DecodedPacket decoded = interpreter.interpretBytes(dataBytes, packetName, rootNode);
    return decoded;
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