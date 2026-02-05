#include "Interpreter.h"

Interpreter::Interpreter() {
    defTypeAliases = {};
    decodedPacket = nullptr;
    globalSettings = std::make_shared<ASTPrimitiveValueSettings>(ASTPrimitiveValueSettings{});
}

DecodedPacket Interpreter::interpretBytes(const std::vector<uint8_t>& dataBytes, const std::string& packetName, const std::shared_ptr<ASTNode>& rootNode) {
    decodedPacket = std::make_shared<DecodedPacket>();
    decodedPacket->rawBytes = dataBytes;
    decodedPacket->packetName = packetName;

    BitQueue bitQueue(dataBytes);

    for (const auto& [name, field] : rootNode->properties) {
        if (field->type == NodeType::PACKET && name == packetName) { // Found the packet definition in the AST
            ASTPacket packet = static_cast<ASTPacket&>(*field);
            decodedPacket->rootField = std::make_shared<InterpretedPacket>();
            decodedPacket->rootField->name = packet.name;
            decodedPacket->rootField->type = NodeType::PACKET;
            interpretPacket(packet, decodedPacket, bitQueue);
            return *decodedPacket;
        }
    }

    throw std::runtime_error("Packet definition for " + packetName + " not found in AST.");
}


template<typename T>
Value readValue(BitQueue& bitQueue) {
    constexpr size_t bits = sizeof(T) * 8;

    if constexpr (std::is_floating_point_v<T>) {
        uint64_t raw = bitQueue.pop(bits);

        T value;
        std::memcpy(&value, &raw, sizeof(T));
        return value;
    } else {
        T value = static_cast<T>(bitQueue.pop(bits));
        return value;
    }
}

uint64_t enforceEndian(uint64_t value, uint8_t  bitSize, bool isLittle){
    if (!isLittle || bitSize == 8)
        return value;

    const uint8_t byteCount = bitSize / 8;
    uint64_t result = 0;

    for (uint8_t i = 0; i < byteCount; ++i) {
        uint64_t byte = (value >> (i * 8)) & 0xFFULL;
        result |= byte << ((byteCount - 1 - i) * 8);
    }
    return result;
}


Value Interpreter::interpretValue(const std::string& datatype, BitQueue& bitQueue, std::shared_ptr<ASTPrimitiveValueSettings> settings) {
    uint64_t bits = bitQueue.pop(ast->primitiveBitSizes[datatype]);
    bits = enforceEndian(bits, ast->primitiveBitSizes[datatype], settings && settings->endianBig == false);

    for (const auto& enumDef : enums) {
        if (datatype == enumDef->name) {
            if (enumDef->datatype.substr(0, 4) != "uint") throw std::runtime_error("Enum must have uint type.");
            for (const auto enumValue : enumDef->variables) {
                if (std::get<uint64_t>(enumValue->varValue) == bits) {
                    return Value{enumValue->varName};
                }
            }
        }
    }

    if (datatype == "bool") {
        return Value{static_cast<bool>(bits)};
    }

    if (datatype == "char") {
        return Value{std::string(1, static_cast<char>(bits & 0xFF))};
    }

    // Arbitrary uintX
    if (datatype.starts_with("uint")) {
        return Value{bits};
    }

    // Optional: arbitrary intX later
    if (datatype.starts_with("int")) {
        int bitsNum = std::stoi(datatype.substr(3));

        // sign extend
        if (bits & (1ULL << (bitsNum - 1))) {
            bits |= (~0ULL << bitsNum);
        }

        return Value{static_cast<int64_t>(bits)};
    }

    // floats
    if (datatype == "float") {
        float f;
        std::memcpy(&f, &bits, sizeof(float));
        return f;
    }

    if (datatype == "double") {
        double d;
        std::memcpy(&d, &bits, sizeof(double));
        return d;
    }

    if (datatype == "byte") {
        return Value{static_cast<uint8_t>(bits)};
    }

    throw std::runtime_error("Unsupported datatype: " + datatype);
}


void Interpreter::interpretPacket(const ASTPacket& packetDef, const std::shared_ptr<DecodedPacket>& decodedPacket, BitQueue& bitQueue) {
    
    for (const auto& fieldPtr : packetDef.fields) {
        if (fieldPtr->type == NodeType::PRIMITIVE) {
                ASTPrimitiveValue primField = static_cast<ASTPrimitiveValue&>(*fieldPtr);
                size_t fieldSizeBits = primField.sizeInBits;

                InterpretedPrimitiveValue interpretedField;
                interpretedField.name = primField.name;
                interpretedField.sizeInBytes = (fieldSizeBits + 7) / 8;
                interpretedField.type = NodeType::PRIMITIVE;
                interpretedField.settings = primField.settings;

            if (defTypeAliases.find(primField.datatype) != defTypeAliases.end()) // If using deftype alias
                interpretedField.value = interpretValue(defTypeAliases[primField.datatype], bitQueue, primField.settings);
            else
                interpretedField.value = interpretValue(primField.datatype, bitQueue, primField.settings);

            static_cast<InterpretedPacket*>(decodedPacket->rootField.get())->fields.push_back(std::make_shared<InterpretedPrimitiveValue>(interpretedField));
        }
    }

}