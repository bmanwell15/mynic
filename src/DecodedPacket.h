#ifndef DECODEDPACKET_H
#define DECODEDPACKET_H

#include <string>
#include <vector>
#include <cstdint>
#include <variant>
#include <memory>
#include <iostream>
#include <iomanip>

#include "AST.h"


struct InterpretedField {
    std::string name;
    size_t sizeInBytes;
    NodeType type;
};

struct InterpretedPacket : public InterpretedField {
    std::vector<std::shared_ptr<InterpretedField>> fields;
};

struct InterpretedPrimitiveValue : public InterpretedField {
    Value value;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
    std::string datatype;
};

struct InterpretedBitfield : public InterpretedField {
    std::string name;
    std::vector<std::shared_ptr<InterpretedField>> subfields;
};

struct InterpretedUnionfield : public InterpretedField {
    std::string name;
    std::vector<std::shared_ptr<InterpretedField>> subfields;
};

struct InterpretedArray : public InterpretedField {
    std::vector<std::shared_ptr<InterpretedField>> list;
};


class DecodedPacket {
    public:
        std::string packetName;
        std::vector<uint8_t> rawBytes;
        std::shared_ptr<InterpretedPacket> rootField;
        void print() const;
        std::string toJson() const;
    
    private:
        void toJson(std::stringstream& ss, const std::shared_ptr<InterpretedField>& field, int indent) const;
        
};

#endif