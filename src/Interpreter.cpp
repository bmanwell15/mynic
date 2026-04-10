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
            decodedPacket->rootField = std::make_shared<InterpretedPacket>();
            decodedPacket->rootField = interpretPacket(*packet, bitQueue, decodedPacket->rootField);
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

std::shared_ptr<InterpretedField> Interpreter::findField(const std::shared_ptr<InterpretedField>& field, const std::string& name) {
    if (field->name == name) {
        return field;
    }
    
    // Recursively search in nested structures
    if (field->type == NodeType::PACKET || field->type == NodeType::SEGMENT) {
        auto packet = std::static_pointer_cast<InterpretedPacket>(field);
        for (const auto& subfield : packet->fields) {
            auto result = findField(subfield, name);
            if (result) return result;
        }
    } else if (field->type == NodeType::BITFIELD) {
        auto bitfield = std::static_pointer_cast<InterpretedBitfield>(field);
        for (const auto& subfield : bitfield->subfields) {
            auto result = findField(subfield, name);
            if (result) return result;
        }
    } else if (field->type == NodeType::UNION) {
        auto unionfield = std::static_pointer_cast<InterpretedUnionfield>(field);
        for (const auto& subfield : unionfield->subfields) {
            auto result = findField(subfield, name);
            if (result) return result;
        }
    } else if (field->type == NodeType::ARRAY) {
        auto array = std::static_pointer_cast<InterpretedArray>(field);
        for (const auto& subfield : array->list) {
            auto result = findField(subfield, name);
            if (result) return result;
        }
    }
    
    return nullptr;
}

std::optional<Value> Interpreter::getParsedValue(const std::shared_ptr<InterpretedField>& field, const std::string& varName, BitQueue& bitQueue) {
    auto foundPeriodIndex = varName.find('.');

    if (AST::MYNIC_KEYWORDS.contains(varName)) {
        if (varName == "TO_END") return (uint64_t)(bitQueue.size() / 8);
    }

    if (foundPeriodIndex != std::string::npos) { // Enum variable lookup
        std::string enumName = varName.substr(0, foundPeriodIndex); // Name of the variable that is assumed to have been previously parsed
        std::string attributeName = varName.substr(foundPeriodIndex + 1); // Attribute of that enum based on the parsed value
        
        auto enumField = findField(field, enumName);
        if (!enumField || enumField->type != NodeType::PRIMITIVE) return std::nullopt;
        
        auto primField = std::static_pointer_cast<InterpretedPrimitiveValue>(enumField);
        if (!std::holds_alternative<std::string>(primField->value)) return std::nullopt;
        
        std::string enumValueName = std::get<std::string>(primField->value);
        
        // Find the enum definition
        for (const auto& enumDef : enums) {
            if (enumDef->name == primField->datatype) {
                for (const auto& var : enumDef->variables) {
                    if (var->varName == enumValueName) {
                        auto it = var->enumAttributes.find(attributeName);
                        if (it != var->enumAttributes.end()) return it->second;
                    }
                }
            }
        }
        return std::nullopt;
    }

    if (field->name == varName) {
        if (field->type == NodeType::PRIMITIVE) {
            auto primValue = std::static_pointer_cast<InterpretedPrimitiveValue>(field);
            return primValue->value;
        }
        throw std::runtime_error("Field '" + varName + "' is not a primitive value.");
    }
    
    // Recursively search in nested structures
    if (field->type == NodeType::PACKET || field->type == NodeType::SEGMENT) {
        auto packet = std::static_pointer_cast<InterpretedPacket>(field);
        for (const auto& subfield : packet->fields) {
            auto result = getParsedValue(subfield, varName, bitQueue);
            if (result.has_value()) return result;
        }
    } else if (field->type == NodeType::BITFIELD) {
        auto bitfield = std::static_pointer_cast<InterpretedBitfield>(field);
        for (const auto& subfield : bitfield->subfields) {
            auto result = getParsedValue(subfield, varName, bitQueue);
            if (result.has_value()) return result;
        }
    } else if (field->type == NodeType::UNION) {
        auto unionfield = std::static_pointer_cast<InterpretedUnionfield>(field);
        for (const auto& subfield : unionfield->subfields) {
            auto result = getParsedValue(subfield, varName, bitQueue);
            if (result.has_value()) return result;
        }
    }
    
    return std::nullopt;
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
        auto pos = field.sizeInBits == 0 ? 1 : field.sizeInBits;
        return Value{pos == std::string::npos ? "0b0" : "0b" + bitStr.substr(bitStr.size() - pos)};
    }

    if (datatype == "byte") {
        return Value{static_cast<uint8_t>(bits)};
    }

    if (datatype.starts_with("bytes")) {
        std::stringstream ss;
        int bytesNum = datatype.size() > 5 ? std::stoi(datatype.substr(5), nullptr, 16) : 1;
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

    if (datatype == "ipv4Address" || datatype == "ipAddress32") {
        uint8_t addressParts[4];
        addressParts[3] = bits & 0xFF;
        addressParts[2] = (bits >> 8) & 0xFF;
        addressParts[1] = (bits >> 16) & 0xFF;
        addressParts[0] = (bits >> 24) & 0xFF;
        std::string asString = std::to_string(addressParts[0]) + "." + std::to_string(addressParts[1]) + "." + std::to_string(addressParts[2]) + "." + std::to_string( addressParts[3]);
        return asString;
    }

    if (datatype == "ipv6Address" || datatype == "ipAddress128") {
        std::stringstream ss;
        const uint8_t bytesNum = 16;
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(bytesNum) << bits << bitQueue.pop(64);
        std::string asString = ss.str();
        for (int i = 4; i < asString.size(); i += 5) {
            asString.insert(i, ":");
        }
        return Value{asString};
    }

    throw std::runtime_error("Unsupported datatype: " + datatype);
}

template<class... T> struct overloaded : T... { using T::operator()...; };
template<class... T> overloaded(T...) -> overloaded<T...>;
Value evaluateBinaryOp(std::string op, Value left, Value right) {
    return std::visit(overloaded{
        [&](int l, int r) -> Value { // 1. Handle pure integer math (to preserve int types)
            if (op == "+") return l + r;
            if (op == "*") return l * r;
            if (op == "-") return l - r;
            if (op == "/") return (r != 0) ? l / r : 0;
            if (op == "%") return l % r;
            return 0;
        },
        [&](double l, double r) -> Value { // 2. Handle pure double math
            if (op == "+") return l + r;
            if (op == "*") return l * r;
            if (op == "-") return l - r;
            if (op == "/") return l / r;
            return 0.0;
        },
        [&](auto l, auto r) -> Value { // 3. Handle mixed or other types safely
            if constexpr (std::is_arithmetic_v<decltype(l)> && std::is_arithmetic_v<decltype(r)>) {
                if (op == "+") return static_cast<double>(l) + static_cast<double>(r);
                if (op == "*") return static_cast<double>(l) * static_cast<double>(r);
                if (op == "-") return static_cast<double>(l) - static_cast<double>(r);
                if (op == "/") return static_cast<double>(l) / static_cast<double>(r);
                if (op == "%") return static_cast<uint64_t>(l) % static_cast<uint64_t>(r); 
            }
            if constexpr (std::is_same_v<decltype(l), std::string> && std::is_same_v<decltype(r), std::string>) {
                if (op == "+") return static_cast<std::string>(l) + static_cast<std::string>(r);
                throw std::runtime_error("strings can only be added together.");
            }
            throw std::runtime_error("Invalid types for binary operator: " + op);
        }
    }, left, right);
}

Value Interpreter::evaluateASTExpression(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode) {
    if (auto n = std::dynamic_pointer_cast<ASTExpressionInt>(node)) {
        return n->value;
    }
    if (auto n = std::dynamic_pointer_cast<ASTExpressionDouble>(node)) {
        return n->value;
    }
    if (auto n = std::dynamic_pointer_cast<ASTExpressionVariable>(node)) {
        if (interpretedPrimitive && n->variableName == interpretedPrimitive->name) 
            return interpretedPrimitive->value;
        
        if (ast->definedVariables.find(n->variableName) != ast->definedVariables.end())
            return ast->definedVariables[n->variableName];
        
        auto possibleVariableCallValue = getParsedValue(rootNode, n->variableName, bitQueue);
        if (!possibleVariableCallValue.has_value()) throw std::runtime_error("Var'" + n->variableName + "' not found in expr.");
        return possibleVariableCallValue.value();
    }
    if (auto b = std::dynamic_pointer_cast<ASTExpressionBinaryOperation>(node)) {
        Value leftVal = evaluateASTExpression(interpretedPrimitive, b->left, bitQueue, rootNode);
        Value rightVal = evaluateASTExpression(interpretedPrimitive, b->right, bitQueue, rootNode);

        return evaluateBinaryOp(b->op, leftVal, rightVal);
    }
    return 0;
}

std::optional<Value> Interpreter::tryEvaluateASTExpression(std::shared_ptr<ASTExpression> node) {
    if (auto n = std::dynamic_pointer_cast<ASTExpressionInt>(node)) {return n->value;}
    if (auto n = std::dynamic_pointer_cast<ASTExpressionDouble>(node)) {return n->value;}
    if (auto n = std::dynamic_pointer_cast<ASTExpressionVariable>(node)) {
        if (ast->definedVariables.find(n->variableName) != ast->definedVariables.end())
            return ast->definedVariables[n->variableName];
    }
    if (auto b = std::dynamic_pointer_cast<ASTExpressionBinaryOperation>(node)) {
        auto left = tryEvaluateASTExpression(b->left);
        if (!left) return std::nullopt;

        auto right = tryEvaluateASTExpression(b->right);
        if (!right) return std::nullopt;

        return evaluateBinaryOp(b->op, *left, *right);
    }
    return std::nullopt;
}

void Interpreter::enforcePostInterpretationSettings(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode) {
    if (!interpretedPrimitive->settings) return;
    if (interpretedPrimitive->settings->exprASTTree) {
        interpretedPrimitive->value = evaluateASTExpression(interpretedPrimitive, interpretedPrimitive->settings->exprASTTree, bitQueue, rootNode);
    }
}


std::shared_ptr<InterpretedPacket> Interpreter::interpretPacket(const ASTPacket& packetDef, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode) {
    auto packet = std::make_shared<InterpretedPacket>();
    // Ensure the interpreted packet has its identifying fields set so
    // nested segments/packets are recognized when printing.
    packet->name = packetDef.name;
    packet->type = packetDef.type;
    for (const auto& fieldPtr : packetDef.fields) {
        auto parsedField = interpretField(fieldPtr, bitQueue, packet);
        if (parsedField->type != NodeType::ROOT_NODE)
            packet->fields.push_back(parsedField);
    }
    return packet;
}

std::shared_ptr<InterpretedField> Interpreter::interpretField(std::shared_ptr<ASTField> field, BitQueue& bitQueue, std::shared_ptr<InterpretedPacket> rootNode) {
    if (field->type == NodeType::PRIMITIVE) {
        auto primField = std::static_pointer_cast<ASTPrimitiveValue>(field);

        if (astTree->properties[primField->datatype]) { // If segment exists
            return interpretField(astTree->properties[primField->datatype], bitQueue, rootNode);
        }

        auto interpretedField = std::make_shared<InterpretedPrimitiveValue>();
        interpretedField->name = primField->name;
        interpretedField->sizeInBits = primField->sizeInBits;
        interpretedField->type = NodeType::PRIMITIVE;
        interpretedField->settings = primField->settings;
        interpretedField->datatype = primField->datatype;
        interpretedField->value = interpretValue(*primField, bitQueue);
        enforcePostInterpretationSettings(interpretedField, bitQueue, rootNode);
        return interpretedField;
    } else if (field->type == NodeType::BITFIELD) {
        auto bitfieldDef = std::static_pointer_cast<ASTBitfield>(field);
        InterpretedBitfield interpretedBitfield;
        interpretedBitfield.name = bitfieldDef->name;
        interpretedBitfield.type = NodeType::BITFIELD;
        for (const auto& subfieldPtr : bitfieldDef->subfields) {
            interpretedBitfield.subfields.push_back(interpretField(subfieldPtr, bitQueue, rootNode));
        }
        return std::make_shared<InterpretedBitfield>(interpretedBitfield);
    } else if (field->type == NodeType::UNION) {
        auto unionfieldDef = std::static_pointer_cast<ASTUnion>(field);
        InterpretedUnionfield interpretedUnionfield;
        interpretedUnionfield.name = unionfieldDef->name;
        interpretedUnionfield.type = NodeType::UNION;
        for (const auto& subfieldPtr : unionfieldDef->subfields) {
            auto interpretedSubfield = interpretField(subfieldPtr, bitQueue, rootNode);
            interpretedUnionfield.subfields.push_back(interpretedSubfield);
            bitQueue.rewind(unionfieldDef->sizeInBits); // Rewind to interpret again
        }
        bitQueue.pop(unionfieldDef->sizeInBits); // Move past union
        return std::make_shared<InterpretedUnionfield>(interpretedUnionfield);
    } else if (field->type == NodeType::SEGMENT || field->type == NodeType::PACKET) {
        auto segment = std::static_pointer_cast<ASTPacket>(field);
        return interpretPacket(*segment, bitQueue, rootNode);
    } else if (field->type == NodeType::ARRAY) {
        auto arrayDef = std::static_pointer_cast<ASTArray>(field);
        InterpretedArray interpretedArray;
        interpretedArray.name = arrayDef->elementSchema->name;
        interpretedArray.type = NodeType::ARRAY;

        if (arrayDef->dynamicLength) {
            auto parsedVal = evaluateASTExpression(nullptr, arrayDef->dynamicLength, bitQueue, rootNode);
            if (std::holds_alternative<uint64_t>(parsedVal)) {
                arrayDef->length = std::get<uint64_t>(parsedVal);
            } else if (std::holds_alternative<int64_t>(parsedVal)) {
                arrayDef->length = static_cast<size_t>(std::get<int64_t>(parsedVal));
            } else if (std::holds_alternative<unsigned long>(parsedVal)) {
                arrayDef->length = std::get<unsigned long>(parsedVal);
            } else if (std::holds_alternative<long>(parsedVal)) {
                arrayDef->length = static_cast<size_t>(std::get<long>(parsedVal));
            } else if (std::holds_alternative<double>(parsedVal)) {
                arrayDef->length = static_cast<size_t>(std::get<double>(parsedVal));
            } else {
                throw std::runtime_error("Dynamic array length must be a numeric value, not " + std::string(parsedVal.index() ? "complex" : "string"));
            }
        }

        for (size_t i = 0; i < arrayDef->length; i++) {
            if ((arrayDef->elementSchema->datatype == "bytes" || arrayDef->elementSchema->datatype == "bits") && interpretedArray.list.size()) {
                auto a = std::static_pointer_cast<InterpretedPrimitiveValue>(interpretField(arrayDef->elementSchema, bitQueue, rootNode));
                auto originalValue = std::static_pointer_cast<InterpretedPrimitiveValue>(interpretedArray.list[0]);
                originalValue->value = std::get<std::string>(originalValue->value) + std::get<std::string>(a->value).substr(2); // .substr(2) to remove 0x prefix
            } else {
                interpretedArray.list.push_back(interpretField(arrayDef->elementSchema, bitQueue, rootNode));
            }
        }
        return std::make_shared<InterpretedArray>(interpretedArray);
    } else if (field->type == NodeType::BRANCH) {
        auto branchDef = std::static_pointer_cast<ASTBranch>(field);
        Value parsedPrimitiveVal = interpretValue(branchDef->parseAs, bitQueue);
        bitQueue.rewind(branchDef->parseAs.sizeInBits);
        for (const auto& nameConditionPair : branchDef->destinationsAndConditions) {
            if (
                nameConditionPair.second->conditionOperator == ConditionOperators::EQUAL && parsedPrimitiveVal == nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::NOT_EQUAL && parsedPrimitiveVal != nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::LESS_THAN && parsedPrimitiveVal < nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::LESS_EQUAL_THAN && parsedPrimitiveVal <= nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::GREATER_THAN && parsedPrimitiveVal > nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::GREATER_EQUAL_THAN && parsedPrimitiveVal >= nameConditionPair.second->parsedCheckValue
            ) {
                return interpretField(nameConditionPair.first, bitQueue, rootNode);
            }
        }

        if (branchDef->destinationDefault) {
            return interpretField(branchDef->destinationDefault, bitQueue, rootNode);
        }
    }  else if (field->type == NodeType::SWITCH) {
        auto switchDef = std::static_pointer_cast<ASTSwitch>(field);
        auto variableValOpt = getParsedValue(rootNode, switchDef->variableName, bitQueue);
        if (!variableValOpt.has_value())
            throw std::runtime_error("Variable in switch not found");
        Value variableVal = variableValOpt.value();
        for (const auto& nameConditionPair : switchDef->destinationsAndConditions) {
            if (
                nameConditionPair.second->conditionOperator == ConditionOperators::EQUAL && variableVal == nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::NOT_EQUAL && variableVal != nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::LESS_THAN && variableVal < nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::LESS_EQUAL_THAN && variableVal <= nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::GREATER_THAN && variableVal > nameConditionPair.second->parsedCheckValue ||
                nameConditionPair.second->conditionOperator == ConditionOperators::GREATER_EQUAL_THAN && variableVal >= nameConditionPair.second->parsedCheckValue
            ) {
                return interpretField(nameConditionPair.first, bitQueue, rootNode);
            }
        }

        if (switchDef->destinationDefault) {
            return interpretField(switchDef->destinationDefault, bitQueue, rootNode);
        }
    }

    return std::make_shared<InterpretedField>();
}