#ifndef DECODEDPACKET_H
#define DECODEDPACKET_H

#include <string>
#include <vector>
#include <cstdint>
#include <variant>
#include <memory>
#include <iostream>

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
};
                    

class DecodedPacket {
    public:
        std::string packetName;
        std::vector<uint8_t> rawBytes;
        std::shared_ptr<InterpretedField> rootField;
        void print() const;
    
    private:
        void print(const std::shared_ptr<InterpretedField>& field, int indent) const;
        
};

#endif