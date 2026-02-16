#include "DecodedPacket.h"

void DecodedPacket::print() const {
    std::cout << "Packet Name: " << packetName << std::endl;
    std::cout << "Raw Bytes: ";
    for (const auto& byte : rawBytes) {
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;

    print(rootField, 0);
}

void DecodedPacket::print(const std::shared_ptr<InterpretedField>& field, int indent) const {
    std::string indentStr(indent * 3, ' ');

    if (field->type == NodeType::PACKET) {
        auto packetField = std::static_pointer_cast<InterpretedPacket>(field);
        std::cout << indentStr << "Packet: " << packetField->name << std::endl;
        for (const auto& subField : packetField->fields) {
            print(subField, indent + 1);
        }
    } else if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<InterpretedPrimitiveValue>(field);
        if (primField->settings && primField->settings->isHidden) return;
        std::cout << indentStr << primField->name << " = ";
        std::visit([&](auto&& value) {
            using T = std::decay_t<decltype(value)>;

            if constexpr (std::is_same_v<T, bool>) {
                std::cout << (value ? "true" : "false");
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << value;
            } else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>) {
                std::cout << static_cast<int>(value);
            } else if constexpr (std::is_arithmetic_v<T>) {
                std::cout << value;
            } else {
                static_assert(!sizeof(T), "Unhandled variant type");
            }
        }, primField->value);

        if (primField->settings) std::cout << ' ' << primField->settings->units;
    
        std::cout << std::endl;
    } else if (field->type == NodeType::BITFIELD) {
        auto bitField = std::static_pointer_cast<InterpretedBitfield>(field);
        std::cout << indentStr << bitField->name << ":\n";
        for (const auto& subfield : bitField->subfields) {
            print(subfield, indent + 1);
        }
    } else {
        std::cout << indentStr << "Unknown field type for field: " << (int)(field->type) << std::endl;
    }
}