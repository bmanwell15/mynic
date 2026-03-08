#include "Interpreter.h"

Interpreter::Interpreter() {
    defTypeAliases = {};
    decodedPacket = nullptr;
    astTree = nullptr;
    globalSettings = std::make_shared<ASTPrimitiveValueSettings>(ASTPrimitiveValueSettings{});
}

DecodedPacket Interpreter::interpretBytes(const std::vector<uint8_t>& dataBytes, const std::string& packetName, const std::shared_ptr<ASTNode>& rootNode) {
    astTree = rootNode;
    decodedPacket = std::make_shared<DecodedPacket>();
    decodedPacket->rawBytes = dataBytes;
    decodedPacket->packetName = packetName;

    BitQueue bitQueue(dataBytes);

    for (const auto& [name, field] : rootNode->properties) {
        if (field && field->type == NodeType::PACKET && name == packetName) { // Found the packet definition in the AST
            auto packet = std::static_pointer_cast<ASTPacket>(field);
            decodedPacket->rootField = interpretPacket(*packet, bitQueue);
            decodedPacket->rootField->name = packet->name;
            decodedPacket->rootField->type = NodeType::PACKET;
            return *decodedPacket;
        }
    }

    throw std::runtime_error("Packet definition for " + packetName + " not found in AST.");
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

std::string epochToDatetimeString(int64_t epoch_ns) {
    using namespace std::chrono;

    sys_time<nanoseconds> tp{nanoseconds{epoch_ns}};

    auto days_part = floor<days>(tp);
    year_month_day ymd{days_part};

    auto time_part = tp - days_part;
    auto h = duration_cast<hours>(time_part);
    auto m = duration_cast<minutes>(time_part - h);
    auto s = duration_cast<seconds>(time_part - h - m);
    auto ns = duration_cast<nanoseconds>(time_part - h - m - s).count();

    std::ostringstream out;

    out << int(ymd.year()) << "-"
        << std::setw(2) << std::setfill('0') << unsigned(ymd.month()) << "-"
        << std::setw(2) << unsigned(ymd.day()) << " ";

    out << std::setw(2) << h.count() << ":"
        << std::setw(2) << m.count() << ":"
        << std::setw(2) << s.count();

    if (ns != 0){
        std::ostringstream frac;
        frac << std::setw(9) << std::setfill('0') << ns;

        std::string f = frac.str();
        while (!f.empty() && f.back() == '0')
            f.pop_back();

        out << "." << f;
    }

    return out.str();
}

std::string formatDuration(int64_t ns) {
    // Handle negative durations
    bool negative = ns < 0;
    uint64_t abs_ns = negative ? -ns : ns;

    constexpr uint64_t NS_PER_MINUTE = 60ULL * NS_PER_SECOND;
    constexpr uint64_t NS_PER_HOUR   = 60ULL * NS_PER_MINUTE;
    constexpr uint64_t NS_PER_DAY    = 24ULL * NS_PER_HOUR;
    constexpr uint64_t NS_PER_WEEK   = 7ULL * NS_PER_DAY;
    constexpr double NS_PER_YEAR    = 365.2425 * NS_PER_DAY; // approximate
    constexpr double NS_PER_MONTH   = 30.44 * NS_PER_DAY;    // approximate

    std::ostringstream out;

    if (negative) out << "-";

    // Years
    int64_t years = abs_ns / NS_PER_YEAR;
    abs_ns -= static_cast<uint64_t>(years * NS_PER_YEAR);
    if (years) out << years << "y ";

    // Months
    int64_t months = abs_ns / NS_PER_MONTH;
    abs_ns -= static_cast<uint64_t>(months * NS_PER_MONTH);
    if (months) out << months << "mo ";

    // Weeks
    int64_t weeks = abs_ns / NS_PER_WEEK;
    abs_ns -= weeks * NS_PER_WEEK;
    if (weeks) out << weeks << "w ";

    // Days
    int64_t days = abs_ns / NS_PER_DAY;
    abs_ns -= days * NS_PER_DAY;
    if (days) out << days << "d ";

    // Hours
    int64_t hours = abs_ns / NS_PER_HOUR;
    abs_ns -= hours * NS_PER_HOUR;
    if (hours) out << hours << "h ";

    // Minutes
    int64_t minutes = abs_ns / NS_PER_MINUTE;
    abs_ns -= minutes * NS_PER_MINUTE;
    if (minutes) out << minutes << "m ";

    // Seconds with fraction
    double seconds = static_cast<double>(abs_ns) / NS_PER_SECOND;
    std::ostringstream secStream;
    secStream << std::fixed << std::setprecision(9) << seconds;
    std::string secStr = secStream.str();
    
    // Remove trailing zeros
    while (!secStr.empty() && secStr.back() == '0')
        secStr.pop_back();
    
    // Remove trailing decimal point if no fractional part
    if (!secStr.empty() && secStr.back() == '.')
        secStr.pop_back();
    
    out << secStr << "s";

    return out.str();
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

    if (datatype.starts_with("bits")) { // && != "bits" is assumed -> Arbitrary bitsX
        auto bitStr = std::bitset<64>(bits).to_string();
        auto pos = field.sizeInBits;
        return Value{pos == std::string::npos ? "0b0" : "0b" + bitStr.substr(bitStr.size() - pos)};
    }

    if (datatype == "byte") {
        return Value{static_cast<uint8_t>(bits)};
    }

    if (datatype.starts_with("bytes")) {
        std::stringstream ss;
        int bytesNum = std::stoi(datatype.substr(5));
        ss << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(bytesNum * 2) << bits;
        return Value{ss.str()};
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

    if (datatype == "datetime32s") {
        int64_t seconds = static_cast<int64_t>(bits);
        return epochToDatetimeString(seconds * NS_PER_SECOND);
    }

    if (datatype.starts_with("datetime64")) {
        int64_t rawTime = static_cast<int64_t>(bits);

        if (datatype.ends_with("ms")) {
            return epochToDatetimeString(rawTime * NS_PER_MILLISECOND);
        } else if (datatype.ends_with("us")) {
            return epochToDatetimeString(rawTime * NS_PER_MICROSECOND);
        } else if (datatype.ends_with("ns")) {
            return epochToDatetimeString(rawTime);
        }

        return epochToDatetimeString(rawTime * NS_PER_SECOND);
    }

    if (datatype == "timespan32s") {
        int64_t seconds = static_cast<int64_t>(bits);
        return formatDuration(seconds * NS_PER_SECOND);
    }

    if (datatype.starts_with("timespan64")) {
        int64_t rawTime = static_cast<int64_t>(bits);

        if (datatype.ends_with("ms")) {
            return formatDuration(rawTime * NS_PER_MILLISECOND);
        } else if (datatype.ends_with("us")) {
            return formatDuration(rawTime * NS_PER_MICROSECOND);
        } else if (datatype.ends_with("ns")) {
            return formatDuration(rawTime);
        }

        return formatDuration(rawTime * NS_PER_SECOND);
    }

    throw std::runtime_error("Unsupported datatype: " + datatype);
}


std::shared_ptr<InterpretedPacket> Interpreter::interpretPacket(const ASTPacket& packetDef, BitQueue& bitQueue) {
    auto packet = std::make_shared<InterpretedPacket>();
    // Ensure the interpreted packet has its identifying fields set so
    // nested segments/packets are recognized when printing.
    packet->name = packetDef.name;
    packet->type = packetDef.type;
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

        if (astTree->properties[primField->datatype]) {
            return interpretField(astTree->properties[primField->datatype], bitQueue);
        }

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
    } else if (field->type == NodeType::UNION) {
        auto unionfieldDef = std::static_pointer_cast<ASTUnion>(field);
        InterpretedUnionfield interpretedUnionfield;
        interpretedUnionfield.name = unionfieldDef->name;
        interpretedUnionfield.type = NodeType::UNION;
        size_t unionSize = 0;
        for (const auto& subfieldPtr : unionfieldDef->subfields) {
            auto interpretedSubfield = interpretField(subfieldPtr, bitQueue);
            interpretedUnionfield.subfields.push_back(interpretedSubfield);
            if (interpretedSubfield->type == NodeType::PRIMITIVE) {
                auto subfieldPrim = std::static_pointer_cast<InterpretedUnionfield>(interpretedSubfield);
                if (unionSize == 0) unionSize = subfieldPrim->sizeInBytes * 8;
                bitQueue.rewind(unionSize); // Rewind to interpret again
            } else {
                throw std::runtime_error("Only primitives can be inside of a union field.");
            }
        }
        bitQueue.pop(unionSize); // Move past union
        return std::make_shared<InterpretedUnionfield>(interpretedUnionfield);
    } else if (field->type == NodeType::SEGMENT || field->type == NodeType::PACKET) {
        auto segment = std::static_pointer_cast<ASTPacket>(field);
        return interpretPacket(*segment, bitQueue);
    }

    return std::make_shared<InterpretedField>();
}