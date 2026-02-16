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
            decodedPacket->rootField = interpretPacket(packet, bitQueue);
            decodedPacket->rootField->name = packet.name;
            decodedPacket->rootField->type = NodeType::PACKET;
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


Value Interpreter::interpretValue(ASTPrimitiveValue& field, BitQueue& bitQueue) {
    uint64_t bits = bitQueue.pop(field.sizeInBits);
    std::string datatype;

    if (defTypeAliases.find(field.datatype) != defTypeAliases.end()) // If using deftype alias
        datatype = defTypeAliases[field.datatype];
    else
        datatype = field.datatype;
    
    bits = enforceEndian(bits, field.sizeInBits, field.settings && field.settings->endianBig == false);

    for (const auto& enumDef : enums) {
        if (datatype == enumDef->name) {
            if (enumDef->datatype.substr(0, 4) != "uint") throw std::runtime_error("Enum must have uint type.");
            for (const auto enumValue : enumDef->variables) {
                if (std::get<uint64_t>(enumValue->varValue) == bits) {
                    return Value{enumValue->varName};
                }
            }
            throw std::runtime_error("Enum value not set"); // May change into a warning?
        }
    }

    if (datatype == "bit" || datatype == "bits") {
        auto bitStr = std::bitset<64>(bits).to_string();
        auto pos = bitStr.find_first_not_of('0');
        return Value{pos == std::string::npos ? "0b0" : "0b" + bitStr.substr(pos)};
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


std::shared_ptr<InterpretedPacket> Interpreter::interpretPacket(const ASTPacket& packetDef, BitQueue& bitQueue) {
    auto packet = std::make_shared<InterpretedPacket>();
    for (const auto& fieldPtr : packetDef.fields) {
        auto parsedField = interpretField(fieldPtr, bitQueue);
        if (parsedField->type != NodeType::ROOT_NODE)
            packet->fields.push_back(parsedField);
    }
    return packet;
}

std::shared_ptr<InterpretedField> Interpreter::interpretField(std::shared_ptr<ASTField> field, BitQueue& bitQueue) {
    if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<ASTPrimitiveValue>(field);

        InterpretedPrimitiveValue interpretedField;
        interpretedField.name = primField->name;
        interpretedField.sizeInBytes = primField->sizeInBits / 8;
        interpretedField.type = NodeType::PRIMITIVE;
        interpretedField.settings = primField->settings;
        interpretedField.value = interpretValue(*primField, bitQueue);
        return std::make_shared<InterpretedPrimitiveValue>(interpretedField);
    } else if (field->type == NodeType::BITFIELD) {
        auto bitfieldDef = std::static_pointer_cast<ASTBitfield>(field);
        InterpretedBitfield interpretedBitfield;
        interpretedBitfield.name = bitfieldDef->name;
        interpretedBitfield.type = NodeType::BITFIELD;
        for (const auto& subfieldPtr : bitfieldDef->subfields) {
            interpretedBitfield.subfields.push_back(interpretField(subfieldPtr, bitQueue));
        }
        return std::make_shared<InterpretedBitfield>(interpretedBitfield);
    }
    return std::make_shared<InterpretedBitfield>();
}