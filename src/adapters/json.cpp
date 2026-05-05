#include "json.h"

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
            if (primField->settings && primField->settings->units != "")
                ss << static_cast<int>(value);
            else
                ss << '\'' << static_cast<int>(value) << '\'';
        } else if constexpr (std::is_arithmetic_v<T>) {
            ss << value;
        } else {
            static_assert(!sizeof(T), "Unhandled variant type");
        }
    }, primField->value);
}

void encode(std::stringstream& ss, const std::shared_ptr<InterpretedField>& field, int indent) {
    std::string indentStr(indent * 3, ' ');
    if (field->type == NodeType::PACKET || field->type == NodeType::SEGMENT) {
        auto packetField = std::static_pointer_cast<InterpretedPacket>(field);
        ss << indentStr << '\"' << packetField->name << "\": {\n";
        std::vector<std::string> subJsons;
        for (const auto& subfield : packetField->fields) {
            std::stringstream subSs;
            encode(subSs, subfield, indent + 1);
            subJsons.push_back(subSs.str());
        }
        for (size_t i = 0; i < subJsons.size(); i++) {
            std::string s = subJsons[i];
            if (!s.empty() && s.back() == '\n') s.pop_back();
            ss << s;
            if (i < subJsons.size() - 1) ss << ",";
            ss << "\n";
        }
        std::string addComa = (indent == 1) ? "," : "";
        ss << indentStr << "}" << addComa << "\n";
    } else if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<InterpretedPrimitiveValue>(field);
        if (primField->settings && primField->settings->flags.isHidden) return;
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
            encode(subSs, subfield, indent + 1);
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
            encode(subSs, subfield, indent + 1);
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

void encodeWarnings(std::stringstream& ss, const DecodedPacket& decodedPacket) {
    if (!decodedPacket.warnings.size()) {ss << "   \"Warnings\": []\n";return;}
    
    ss << "   \"Warnings\": [\n";
    for (size_t i = 0; i < decodedPacket.warnings.size(); i++) {
        const auto& warning = decodedPacket.warnings[i];
        ss << "      {\n";
        ss << "         \"Error Code\": " << (int)(warning.warningType) << ",\n";
        ss << "         \"Message\": \"" << warning.errorMessage << "\"\n";
        ss << "      }";
        if (i != decodedPacket.warnings.size() - 1) ss << ',';
        ss << '\n';
    }
    ss << "   ]\n";
}

std::string adapters::json::encode(const DecodedPacket& decodedPacket) {
    std::stringstream ss;
    ss << "{\n";
    if (decodedPacket.rootField->settings->flags.includePacketName)
        ss << "   \"Packet Name\": \"" << decodedPacket.packetName << "\",\n";

    if (decodedPacket.rootField->settings->flags.includeRawBytes) {
        ss << "   \"Raw Bytes\": \"";
        for (const auto& byte : decodedPacket.rawBytes) {
            ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
        }
        ss << std::dec << "\",\n";
    }
    
    if (decodedPacket.rootField->settings->flags.includeTimestamp) {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::floor<std::chrono::milliseconds>(now);
        std::string timestamp = std::format("{:%F %T}", now_ms); // %F = YYYY-MM-DD, %T = HH:MM:SS.mmm
        ss << "   \"Timestamp\": \"" << timestamp << "\",\n";
    }
    encode(ss, decodedPacket.rootField, 1);
    encodeWarnings(ss, decodedPacket);
    ss << '}';
    return ss.str();
}