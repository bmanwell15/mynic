#include "DecodedPacket.h"

std::string DecodedPacket::toJson() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "   \"Packet Name\": \"" << packetName << "\",\n";
    ss << "   \"Raw Bytes\": \"";
    for (const auto& byte : rawBytes) {
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
    }
    ss << std::dec << "\",\n";
    toJson(ss, rootField, 1);
    ss << '}';
    return ss.str();
}

void DecodedPacket::print() const {
    std::cout << toJson() << std::endl;
}

void printPrimitive(std::stringstream& ss, std::shared_ptr<InterpretedPrimitiveValue> primField) {
    std::visit([&](auto&& value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, bool>) {
            ss << (value ? "true" : "false");
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (primField->settings && primField->settings->units != "")
                ss << value;
            else
                ss << '\"' << value << '\"';
        } else if constexpr (std::is_same_v<T, uint8_t>) { // byte
            if (primField->settings && primField->settings->units != "")
                ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(value) << std::dec;
            else
                ss << "\"0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(value) << std::dec << '\"';
        } else if constexpr (std::is_same_v<T, int8_t>) {
            ss << static_cast<int>(value);
        } else if constexpr (std::is_arithmetic_v<T>) {
            ss << value;
        } else {
            static_assert(!sizeof(T), "Unhandled variant type");
        }
    }, primField->value);
}

void DecodedPacket::toJson(std::stringstream& ss, const std::shared_ptr<InterpretedField>& field, int indent) const {
    std::string indentStr(indent * 3, ' ');
    if (field->type == NodeType::PACKET || field->type == NodeType::SEGMENT) {
        auto packetField = std::static_pointer_cast<InterpretedPacket>(field);
        ss << indentStr << '\"' << packetField->name << "\": {\n";
        std::vector<std::string> subJsons;
        for (const auto& subField : packetField->fields) {
            std::stringstream subSs;
            toJson(subSs, subField, indent + 1);
            subJsons.push_back(subSs.str());
        }
        for (size_t i = 0; i < subJsons.size(); i++) {
            std::string s = subJsons[i];
            if (!s.empty() && s.back() == '\n') s.pop_back();
            ss << s;
            if (i < subJsons.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << indentStr << "}\n";
    } else if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<InterpretedPrimitiveValue>(field);
        if (primField->settings && primField->settings->isHidden) return;
        ss << indentStr << "\"" << primField->name << "\": ";
        if (primField->settings && primField->settings->units != "") ss << '\"';
        printPrimitive(ss, primField);
        if (primField->settings && primField->settings->units != "") ss << ' ' << primField->settings->units << '\"';
        ss << "\n";
    } else if (field->type == NodeType::BITFIELD) {
        auto bitField = std::static_pointer_cast<InterpretedBitfield>(field);
        ss << indentStr << '\"' << bitField->name << "\": {\n";
        std::vector<std::string> subJsons;
        for (const auto& subfield : bitField->subfields) {
            std::stringstream subSs;
            toJson(subSs, subfield, indent + 1);
            subJsons.push_back(subSs.str());
        }
        for (size_t i = 0; i < subJsons.size(); i++) {
            std::string s = subJsons[i];
            if (!s.empty() && s.back() == '\n') s.pop_back();
            ss << s;
            if (i < subJsons.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << indentStr << "}\n";
    } else if (field->type == NodeType::UNION) {
        auto unionfield = std::static_pointer_cast<InterpretedUnionfield>(field);
        ss << indentStr << '\"' << unionfield->name << "\": {\n";
        std::vector<std::string> subJsons;
        for (const auto& subfield : unionfield->subfields) {
            std::stringstream subSs;
            toJson(subSs, subfield, indent + 1);
            subJsons.push_back(subSs.str());
        }
        for (size_t i = 0; i < subJsons.size(); i++) {
            std::string s = subJsons[i];
            if (!s.empty() && s.back() == '\n') s.pop_back();
            ss << s;
            if (i < subJsons.size() - 1) ss << ",";
            ss << "\n";
        }
        ss << indentStr << "}\n";
    } else if (field->type == NodeType::ARRAY) {
        auto arrayfield = std::static_pointer_cast<InterpretedArray>(field);
        ss << indentStr << '\"' << arrayfield->name << "\": [ ";
        for (size_t i = 0; i < arrayfield->list.size(); i++) {
            auto& listElement = arrayfield->list[i];
            if (listElement->type != NodeType::PRIMITIVE) {throw std::runtime_error("Element not primitive.");}
            auto asPrimitive = std::static_pointer_cast<InterpretedPrimitiveValue>(listElement);
            printPrimitive(ss, asPrimitive);
            if (i != arrayfield->list.size() - 1) ss << ", ";
        }
        ss << " ]\n";
    }
}