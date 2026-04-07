#include "AST.h"
#include "Interpreter.h"
#include "Mynic.h"

AST::AST(Mynic* myn) {
    masterIndex = 0;
    tokens = {};
    rootNode = std::make_shared<ASTNode>(ASTNode{NodeType::ROOT_NODE, {}});
    toEndFlagUsed = false;
    primitiveBitSizes = {
        {"bit", 1},
        {"bits", 1},
        {"byte", 8},
        {"bytes", 8},
        {"bool", 8},
        {"short", 16},
        {"ushort", 16},
        {"int", 32},
        {"uint", 32},
        {"float", 32},
        {"float32", 32},
        {"double", 64},
        {"long", 64},
        {"ulong", 64},

        // datetimes
        {"datetime32s", 32},
        {"datetime64s", 64},
        {"datetime64ms", 64},
        {"datetime64us", 64},
        {"datetime64ns", 64},

        // timespans
        {"timespan32s", 32},
        {"timespan64s", 64},
        {"timespan64ms", 64},
        {"timespan64us", 64},
        {"timespan64ns", 64},

        // IPs
        {"ipv4Address", 32},
        {"ipAddress32", 32},
        {"ipv6Address", 64}, // Really 128, but cannot take more than 64 bits as of now
        {"ipAddress128", 64}
    };
    mynic = myn;
    currentPacket = nullptr;
}

std::shared_ptr<ASTNode> AST::parseTokensToAST(const std::vector<Token>& inputTokens, bool isMainFile) {
    if (isMainFile) {
        rootNode->type = NodeType::ROOT_NODE;
        this->tokens = inputTokens;
        rootNode = std::make_shared<ASTNode>(ASTNode{NodeType::ROOT_NODE, {}});
    }

    for (masterIndex = 0; masterIndex < this->tokens.size(); masterIndex++) {
        parseField();
    }

    return rootNode;
}

Token AST::eatToken(std::initializer_list<TokenType> types) {
    if (!(std::find(types.begin(), types.end(), NEW_LINE) != types.end()))
        skipWhiteSpace();

    if (masterIndex >= tokens.size() - 1)
        throw std::runtime_error("Unexpected end of token stream.");

    for (const TokenType& expectedType : types) {
        if (tokens[masterIndex].type == expectedType) {
            return tokens[masterIndex++];
        }
    }

    std::string strMsg = "Unexpected token type encountered. Expected {";
    std::stringstream msg;
    msg << "Unexpected token type encountered. Expected {";
    for (const auto& type : types) {
        msg << type << ", ";
    }
    msg << "} but got " << tokens[masterIndex].value;

    ErrorHandler::throwError(msg.str(), tokens, masterIndex);
    throw std::runtime_error("Unreachable code in AST::eatToken");
}

void AST::eatOptionalToken(std::initializer_list<TokenType> types) {
    if (!(std::find(types.begin(), types.end(), NEW_LINE) != types.end()))
        skipWhiteSpace();

    if (masterIndex >= tokens.size() - 1)
        throw std::runtime_error("Unexpected end of token stream.");

    for (const TokenType& expectedType : types) {
        if (tokens[masterIndex].type == expectedType) {
            masterIndex++;
            return;
        }
    }
}

Token AST::eatToken(TokenType expectedType) {
    return eatToken({expectedType});
}

void AST::skipWhiteSpace(bool includeCommas, bool includeSemiColons) {
    while (masterIndex < tokens.size() && (tokens[masterIndex].type == NEW_LINE || (tokens[masterIndex].type == COMMA && includeCommas) || (tokens[masterIndex].type == SEMI_COLON && includeSemiColons)))
        masterIndex++;
}

// Returns true if string is in format intX or uintX where X is an integer
bool isDynamicSizeType(const std::string& s) {
    const std::vector<std::string> prefixes = {"int", "uint", "bits", "bytes"};
    for (const auto& p : prefixes) {
        if (p == "bytes" && s.size() <= p.size()) continue; // must have at least one digit after prefix
        if (s.compare(0, p.size(), p) != 0) continue;

        for (size_t i = p.size(); i < s.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(s[i]);
            if (!std::isdigit(c)) return false;
        }
        return true;
    }

    return false;
}

std::string removeQuotes(std::string& str) {
    if (str[0] == '\"')
        return str.substr(1, str.size() - 2);
    return str;
}


bool AST::isKnownType(const std::string& type) {
    return primitiveBitSizes.count(type) || isDynamicSizeType(type) || rootNode->properties[type];
}

size_t AST::getStructureSize(std::shared_ptr<ASTField> field) {
    if (field->type == NodeType::PRIMITIVE) {
        auto asPrim = std::static_pointer_cast<ASTPrimitiveValue>(field);
        return asPrim->sizeInBits;
    }
    if (field->type == NodeType::PACKET || field->type == NodeType::SEGMENT) {
        auto packetField = std::static_pointer_cast<ASTPacket>(field);
        size_t packetSize = 0;
        for (const auto& subField : packetField->fields) {
            packetSize += getStructureSize(subField);
        }
        packetField->sizeInBits = packetSize;
        return packetSize;
    }
    if (field->type == NodeType::BITFIELD) {
        auto bitfield = std::static_pointer_cast<ASTBitfield>(field);
        size_t bitfieldSize = 0;
        for (const auto& subfield : bitfield->subfields) {
            bitfieldSize += getStructureSize(subfield);
        }
        bitfield->sizeInBits = bitfieldSize;
        return bitfieldSize;
    }
    if (field->type == NodeType::UNION) {
        auto unionField = std::static_pointer_cast<ASTUnion>(field);
        return unionField->sizeInBits; // Field is already set when validating that all union subfields have the same size
    }
    if (field->type == NodeType::ARRAY) {
        auto arrayField = std::static_pointer_cast<ASTArray>(field);
        arrayField->sizeInBits = getStructureSize(arrayField->elementSchema);
        return arrayField->sizeInBits;
    }
    return 0;
}

size_t AST::parseVariableCall() {
    Token possibleVar = Lexer::nextNonWhiteSpaceToken(tokens, masterIndex);
    if (possibleVar.type == INT_LITERAL) return std::stoul(eatToken(INT_LITERAL).value);

    auto varIndex = definedVariables.find(possibleVar.value);
    if (varIndex != definedVariables.end()) {
        eatToken(IDENTIFIER);
        return std::get<uint64_t>(definedVariables[possibleVar.value]);
    }
    return 0;
}

std::shared_ptr<ASTField> AST::parseField() {
    const Token& token = tokens[masterIndex];

    if (token.type == MULTI_LINED_COMMENT) {
        masterIndex++;
        while (masterIndex < tokens.size() && tokens[masterIndex].type != MULTI_LINED_COMMENT) {masterIndex++;}
        return std::make_shared<ASTField>();
    }

    if (token.type == IDENTIFIER && MYNIC_KEYWORDS.contains(token.value)) {

    }
    
    if (token.type == IDENTIFIER && token.value == "packet") {
        std::shared_ptr<ASTPacket> packet = parsePacket();
        rootNode->properties[packet->name] = packet;
        return packet;
    }

    if (token.type == IDENTIFIER && token.value == "segment") {
        bool isLambdaSegment = (currentPacket != nullptr);
        std::shared_ptr<ASTPacket> segment = parsePacket();
        segment->type = NodeType::SEGMENT;
        if (!isLambdaSegment)
            rootNode->properties[segment->name] = segment;
        return segment;
    }

    if (token.type == IDENTIFIER && token.value == "typedef") {
        return parseTypeDef();
    }

    if (token.type == IDENTIFIER && token.value == "enum") {
        std::shared_ptr<ASTEnum> enumDef = parseEnum();
        interpreter->enums.push_back(enumDef);
        return enumDef;
    }

    if (token.type == IDENTIFIER && token.value == "union") {
        return parseUnion();
    }

    if (token.type == IDENTIFIER && token.value == "bitfield") {
        return parseBitfield();
    }

    if (token.type == IDENTIFIER && token.value == "branch") {
        return parseBranch();
    }

    if (token.type == IDENTIFIER && token.value == "define") {
        return parseDefine();
    }

    if (token.type == IDENTIFIER && token.value == "import") {
        return parseImport();
    }

    if (token.type == IDENTIFIER && token.value == "default") {
        return parseDefault();
    }

    if (token.type == IDENTIFIER) { // Assume primitive for now
        return parsePrimitive();
    }
    return std::make_shared<ASTField>();
}

std::shared_ptr<ASTPacket> AST::parsePacket() {
    auto packetNode = std::make_shared<ASTPacket>();
    packetNode->type = NodeType::PACKET;
    packetNode->defaultSettings = nullptr;
    bool isLambdaSegment = (eatToken(IDENTIFIER).value == "segment" && currentPacket); // Consumed 'packet'/'segment' token
    currentPacket = &(*packetNode);
    packetNode->name = eatToken(IDENTIFIER).value;

    eatToken(OPEN_BRACKET);
    blockDepth_t packetDepth = tokens[masterIndex].blockDepth;

    while (masterIndex < tokens.size() && tokens[masterIndex].blockDepth >= packetDepth) {
        std::shared_ptr<ASTField> field = parseField();

        if (field->type == NodeType::DEFAULT_BLOCK) {
            packetNode->defaultSettings = std::static_pointer_cast<ASTDefault>(field)->settings;
        } else {
            packetNode->fields.push_back(field);
        }
        masterIndex++;
    }
    eatToken(CLOSE_BRACKET);
    packetNode->sizeInBits = getStructureSize(packetNode);
    if (!isLambdaSegment) {
        currentPacket = nullptr;
        primitiveBitSizes[packetNode->name] = packetNode->sizeInBits;
    }
    return packetNode;
}

std::shared_ptr<ASTField> AST::parseTypeDef() {
    eatToken(IDENTIFIER); // Consume 'typedef' token
    ASTTypeDef typeDef;
    typeDef.type = NodeType::TYPEDEF;
    typeDef.existingTypeName = eatToken(IDENTIFIER).value;
    typeDef.newTypeName = eatToken(IDENTIFIER).value;

    if (isDynamicSizeType(typeDef.existingTypeName)) {
        size_t pos = typeDef.existingTypeName.find_first_of("0123456789");
        if (pos != std::string::npos) {
            size_t bits = std::stoi(typeDef.existingTypeName.substr(pos));
            if (bits == 0 || bits > 64) {
                ErrorHandler::throwError("Integer type width must be between 1 and 64 bits: " + typeDef.existingTypeName, tokens, masterIndex - 1);
            }
            primitiveBitSizes[typeDef.existingTypeName] = bits;
        }
    }

    if (!isKnownType(typeDef.existingTypeName)) {
        ErrorHandler::throwError("Unknown datatype in typedef: " + typeDef.existingTypeName, tokens, masterIndex - 1);
    }

    interpreter->defTypeAliases[typeDef.newTypeName] = typeDef.existingTypeName;

    eatToken({SEMI_COLON, NEW_LINE});
    rootNode->properties[typeDef.newTypeName] = std::make_shared<ASTTypeDef>(typeDef);
    primitiveBitSizes[typeDef.newTypeName] = primitiveBitSizes[typeDef.existingTypeName];
    return rootNode->properties[typeDef.newTypeName];
}

std::shared_ptr<ASTField> AST::parsePrimitive() {
    if (toEndFlagUsed) ErrorHandler::throwError("Another field cannot be used after 'TO_END' value is called.", tokens, masterIndex);
    auto field = std::make_shared<ASTPrimitiveValue>();
    field->datatype = eatToken(IDENTIFIER).value;

    if (isDynamicSizeType(field->datatype)) {
        size_t pos = field->datatype.find_first_of("0123456789");
        if (pos != std::string::npos) {
            size_t bits = std::stoi(field->datatype.substr(pos));
            if (field->datatype.starts_with("bytes")) bits *= 8;
            if (bits == 0 || bits > 64) {
                ErrorHandler::throwError("Integer type width must be between 1 and 64 bits: " + field->datatype, tokens, masterIndex - 1);
            }
            primitiveBitSizes[field->datatype] = bits;
        }
    }

    if (!isKnownType(field->datatype)) {
        ErrorHandler::throwError("Unknown primitive datatype: " + field->datatype, tokens, masterIndex - 1);
    }

    field->name = eatToken(IDENTIFIER).value;
    field->type = NodeType::PRIMITIVE;

    if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == OPEN_SQUARE_BRACKET) { // Means array
        eatToken(OPEN_SQUARE_BRACKET);
        auto arr = std::make_shared<ASTArray>();
        arr->elementSchema = field;
        arr->type = NodeType::ARRAY;
        arr->dynamicLength = parseExpression();
        auto tryEval = interpreter->tryEvaluateASTExpression(arr->dynamicLength);
        if (tryEval.has_value()) {
            if (std::holds_alternative<uint64_t>(tryEval.value())) {
                arr->length = std::get<uint64_t>(tryEval.value());
            } else if (std::holds_alternative<int64_t>(tryEval.value())) {
                arr->length = static_cast<size_t>(std::get<int64_t>(tryEval.value()));
            } else if (std::holds_alternative<unsigned long>(tryEval.value())) {
                arr->length = std::get<unsigned long>(tryEval.value());
            } else if (std::holds_alternative<long>(tryEval.value())) {
                arr->length = static_cast<size_t>(std::get<long>(tryEval.value()));
            } else if (std::holds_alternative<double>(tryEval.value())) {
                arr->length = static_cast<size_t>(std::get<double>(tryEval.value()));
            }
        }
        eatToken(CLOSE_SQUARE_BRACKET);
        field->sizeInBits = primitiveBitSizes[field->datatype];
        field->settings = parsePrimitiveSettings();
        return arr;
    }

    if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == COLON) {
        eatToken(COLON);
        field->sizeInBits = std::stoul(eatToken(INT_LITERAL).value);
        field->settings = parsePrimitiveSettings();
        return field;
    }

    field->sizeInBits = primitiveBitSizes[field->datatype];
    field->settings = parsePrimitiveSettings();
    return field;
}

std::shared_ptr<ASTPrimitiveValueSettings> AST::parsePrimitiveSettings() {
    std::shared_ptr<ASTPrimitiveValueSettings> settings;

    if (currentPacket && currentPacket->defaultSettings) {
        settings = std::make_shared<ASTPrimitiveValueSettings>(*(currentPacket->defaultSettings));
    } else {
        settings = std::make_shared<ASTPrimitiveValueSettings>(*(interpreter->globalSettings));
    }

    if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type != OPEN_BRACKET) // If custom settings are not being used
        return settings;

    eatToken(OPEN_BRACKET);

    while (true) {
        Token settingToken = eatToken(IDENTIFIER);
        eatToken(COLON);
        if (settingToken.value == "endianness") {
            Token endianValueToken = eatToken(STRING_LITERAL);
            if (endianValueToken.value == "\"big\"") {
                settings->endianBig = true;
            } else if (endianValueToken.value == "\"little\"") {
                settings->endianBig = false;
            } else {
                ErrorHandler::throwError("Invalid endian setting: " + endianValueToken.value, tokens, masterIndex - 1);
            }
        } else if (settingToken.value == "hidden") {
            Token toHide = eatToken(BOOL_LITERAL);
            settings->isHidden = (toHide.value == "true");
        } else if (settingToken.value == "units") {
            Token unitVal = eatToken(STRING_LITERAL);
            settings->units = removeQuotes(unitVal.value);
        } else if (settingToken.value == "expr") {
            settings->exprASTTree = parseExpression();
        } else {
            ErrorHandler::throwError("Unknown primitive setting: " + settingToken.value, tokens, masterIndex - 1);
        }

        Token nextToken = eatToken({COMMA, NEW_LINE, CLOSE_BRACKET});
        if (nextToken.type == CLOSE_BRACKET || Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == CLOSE_BRACKET) {
            break;
        }
    }
    return settings;
}

std::shared_ptr<ASTField> AST::parseImport() {
    eatToken(IDENTIFIER);
    Token fileName = eatToken(STRING_LITERAL);
    std::string str = removeQuotes(fileName.value);
    std::shared_ptr<std::vector<Token>> mainFileTokens = std::make_shared<std::vector<Token>>(tokens);
    size_t mainMasterIndex = masterIndex;
    mynic->loadFile(str);
    tokens = *mainFileTokens;
    masterIndex = mainMasterIndex;
    return std::make_shared<ASTField>(ASTField{});
}

Value convertTokenValue(Token token) {
    switch (token.type) {
        case INT_LITERAL:
            return (uint64_t)(std::stoull(token.value));
        case DOUBLE_LITERAL:
            return std::stod(token.value); // floats and doubles
        case STRING_LITERAL:
            return token.value;
        case BOOL_LITERAL:
            return (token.value == "true");
    }
    throw std::runtime_error("Invalid type in convertTokenValue()");
}

std::shared_ptr<ASTEnumVariable> AST::parseEnumVarDefinition(ASTEnum* enumVar) {
    ASTEnumVariable var;
    var.varName = eatToken(IDENTIFIER).value;
    Token nextToken = eatToken({COMMA, EQUALS, CLOSE_BRACKET, NEW_LINE, OPEN_BRACKET});

    if (nextToken.type == EQUALS) { // Explicit decloration
        Token varValToken = eatToken({BOOL_LITERAL, INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL});
        var.varValue = convertTokenValue(varValToken);
    } else {
        if (enumVar->variables.size())
            var.varValue = std::get<uint64_t>(enumVar->variables.back()->varValue) + 1;
        else
            var.varValue = 0ULL;
    }

    // Complex enum parsing
    if (nextToken.type == OPEN_BRACKET || Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == OPEN_BRACKET) { 
        eatOptionalToken({OPEN_BRACKET}); // Will not exist if nextToken.type == OPEN_BRACKET
        while (true) {
            std::string attributeName = eatToken(IDENTIFIER).value;
            eatToken(COLON);
            Value attributeValue = convertTokenValue(eatToken({BOOL_LITERAL, INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL}));

            var.enumAttributes[attributeName] = attributeValue;

            Token nextToken = eatToken({COMMA, NEW_LINE, CLOSE_BRACKET});
            if (nextToken.type == CLOSE_BRACKET || Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == CLOSE_BRACKET) {
                break;
            }
        }
    }

    eatOptionalToken({COMMA, CLOSE_BRACKET});
    return std::make_shared<ASTEnumVariable>(var);
}

std::shared_ptr<ASTEnum> AST::parseEnum() {
    ASTEnum enumVar;
    eatToken(IDENTIFIER); // Parse enum token
    enumVar.datatype = eatToken(IDENTIFIER).value;

    if (enumVar.datatype.substr(0, 4) != "uint")
        ErrorHandler::throwError("Enums must have an uint type, got " + enumVar.datatype + " instead.", tokens, masterIndex);

    size_t bitSize = primitiveBitSizes[enumVar.datatype];
    if (enumVar.datatype.size() >= 4) {
        bitSize = std::stoul(enumVar.datatype.substr(4));
    }
    enumVar.sizeInBits = bitSize;
    enumVar.name = eatToken(IDENTIFIER).value;
    eatToken(OPEN_BRACKET);

    blockDepth_t currentBlockDepth = tokens[masterIndex].blockDepth;
    while (masterIndex < tokens.size() && tokens[masterIndex].blockDepth >= currentBlockDepth) {
        std::shared_ptr<ASTEnumVariable> var = parseEnumVarDefinition(&enumVar);
        enumVar.variables.push_back(var);
    }

    primitiveBitSizes[enumVar.name] = enumVar.sizeInBits;
    return std::make_shared<ASTEnum>(enumVar);
}

std::shared_ptr<ASTField> AST::parseDefine() {
    eatToken(IDENTIFIER); // Eat define token
    std::string varName = eatToken(IDENTIFIER).value;
    auto varExpression = parseExpression();
    auto varValueOption = interpreter->tryEvaluateASTExpression(varExpression);
    if (varValueOption.has_value()) {
        definedVariables[varName] = varValueOption.value();
    } else {
        ErrorHandler::throwError("defines cannot contain variables in expression.", tokens, masterIndex);
    }
    return std::make_shared<ASTField>();
}

std::shared_ptr<ASTDefault> AST::parseDefault() {
    std::shared_ptr<ASTDefault> defaultBlock = std::make_shared<ASTDefault>();
    defaultBlock->type = NodeType::DEFAULT_BLOCK;
    Token defaultToken = eatToken(IDENTIFIER); // Eat the default token
    defaultBlock->settings = parsePrimitiveSettings();
    if (!defaultToken.blockDepth) { // Global level default
        interpreter->globalSettings = defaultBlock->settings;
    }
    return defaultBlock;
}

std::shared_ptr<ASTBitfield> AST::parseBitfield() {
    eatToken(IDENTIFIER); // Eat bitfield token
    ASTBitfield bitfield;
    bitfield.type = NodeType::BITFIELD;
    bitfield.name = eatToken(IDENTIFIER).value;
    eatToken(OPEN_BRACKET);
    blockDepth_t currentBlockDepth = tokens[masterIndex].blockDepth;
    while (masterIndex < tokens.size() && tokens[masterIndex + 1].blockDepth >= currentBlockDepth) {
        std::shared_ptr<ASTField> var = parseField();
        if (var->type != NodeType::ROOT_NODE)
            bitfield.subfields.push_back(var);

        if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == CLOSE_BRACKET) break;
        eatToken({SEMI_COLON, NEW_LINE});
    }

    size_t bitSizeOfField = 0;
    for (const auto& subfield : bitfield.subfields) { // Collect bit size to check if multiple of 8
        if (subfield->type == NodeType::PRIMITIVE) {
            bitSizeOfField += std::static_pointer_cast<ASTPrimitiveValue>(subfield)->sizeInBits;
        }
    }

    if (bitSizeOfField % 8 != 0) { // If the bitfield does not end on a byte, add a hidden field to make it
        auto primitiveSettings = std::make_shared<ASTPrimitiveValueSettings>();
        primitiveSettings->isHidden = true;
        auto remainderField = std::make_shared<ASTPrimitiveValue>();
        remainderField->datatype = "bits";
        remainderField->name = "_remainder";
        remainderField->sizeInBits = (((bitSizeOfField + 7) / 8) * 8) - (bitSizeOfField % 8); // ceil to nearest 8
        remainderField->type = NodeType::PRIMITIVE;
        remainderField->settings = primitiveSettings;
        bitfield.subfields.push_back(remainderField);
    }

    return std::make_shared<ASTBitfield>(bitfield);
}

std::shared_ptr<ASTUnion> AST::parseUnion() {
    eatToken(IDENTIFIER); // Eat union token
    ASTUnion unionfield;
    unionfield.type = NodeType::UNION;
    unionfield.name = eatToken(IDENTIFIER).value;
    eatToken(OPEN_BRACKET);
    blockDepth_t currentBlockDepth = tokens[masterIndex].blockDepth;
    while (masterIndex < tokens.size() && tokens[masterIndex + 1].blockDepth >= currentBlockDepth) {
        std::shared_ptr<ASTField> var = parseField();
        if (var->type != NodeType::ROOT_NODE)
            unionfield.subfields.push_back(var);
        // if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == CLOSE_BRACKET) break;
        eatOptionalToken({SEMI_COLON, NEW_LINE, CLOSE_BRACKET});
    }

    if (unionfield.subfields.size() == 0)
        ErrorHandler::throwError("Union '" + unionfield.name + "' must have at least one subvalue.", tokens, masterIndex);

    size_t bitSizeOfField = getStructureSize(unionfield.subfields[0]);
    for (const auto& subfield : unionfield.subfields) { // Collect bit size to check if they are consistant
        if (subfield->type == NodeType::PRIMITIVE && bitSizeOfField != getStructureSize(subfield)) {
            ErrorHandler::throwError("Union '" + unionfield.name + "' must have values of the same bit size.", tokens, masterIndex);
        }
    }
    unionfield.sizeInBits = bitSizeOfField;
    return std::make_shared<ASTUnion>(unionfield);
}

std::shared_ptr<ASTBranch> AST::parseBranch() {
    eatToken(IDENTIFIER); // Eat branch Token
    ASTBranch branch;
    ASTPrimitiveValue parseAs;
    if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type != OPEN_BRACKET) { // If branch type explicitly defined
        parseAs.datatype = eatToken(IDENTIFIER).value;
    } else {
        parseAs.datatype = "uint8";
    }
    parseAs.sizeInBits = primitiveBitSizes[parseAs.datatype];
    parseAs.type = NodeType::PRIMITIVE;
    branch.type = NodeType::BRANCH;
    branch.parseAs = parseAs;
    eatToken(OPEN_BRACKET);
    blockDepth_t currentBlockDepth = tokens[masterIndex].blockDepth;
    while (masterIndex < tokens.size() && tokens[masterIndex + 1].blockDepth >= currentBlockDepth) {
        std::string destination = eatToken(IDENTIFIER).value; // Destination
        std::string ifOrDefaultsKeyword = eatToken(IDENTIFIER).value;
        if (ifOrDefaultsKeyword == "defaults") {
            branch.destinationDefault = destination;
            eatOptionalToken({SEMI_COLON, NEW_LINE});
            continue;
        } else if (ifOrDefaultsKeyword != "if") {
            ErrorHandler::throwError("Expected 'if' after destination in branch.", tokens, masterIndex);
        }

        std::shared_ptr<ASTCondition> condition = std::make_shared<ASTCondition>();
        if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == CONDITION_OPERATOR) {
            Token conditionalOperator = eatToken(CONDITION_OPERATOR);
            if (conditionalOperator.value == "==") condition->conditionOperator = ConditionOperators::EQUAL; else
            if (conditionalOperator.value == "!=") condition->conditionOperator = ConditionOperators::NOT_EQUAL; else
            if (conditionalOperator.value == ">") condition->conditionOperator = ConditionOperators::GREATER_THAN; else
            if (conditionalOperator.value == "<") condition->conditionOperator = ConditionOperators::LESS_THAN; else
            if (conditionalOperator.value == ">=") condition->conditionOperator = ConditionOperators::GREATER_EQUAL_THAN; else
            if (conditionalOperator.value == "<=") condition->conditionOperator = ConditionOperators::LESS_EQUAL_THAN;
        } else {
            condition->conditionOperator = ConditionOperators::EQUAL;
        }
        condition->parsedCheckValue = convertTokenValue(eatToken({BOOL_LITERAL, INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL}));
        branch.destinationsAndConditions.push_back(std::make_pair(destination, condition));
        eatOptionalToken({SEMI_COLON, NEW_LINE});
    }
    return std::make_shared<ASTBranch>(branch);
}


// 1. Factors: Numbers, Variables, or ( Expressions )
std::shared_ptr<ASTExpression> AST::parseFactor() {
    Token token = eatToken({INT_LITERAL, DOUBLE_LITERAL, OPEN_PAREN, IDENTIFIER});

    if (token.type == INT_LITERAL) {
        auto intLiteral = std::make_shared<ASTExpressionInt>();
        intLiteral->value = std::stoull(token.value);
        return intLiteral;
    }

    if (token.type == DOUBLE_LITERAL) {
        auto doubleLiteral = std::make_shared<ASTExpressionDouble>();
        doubleLiteral->value = std::stod(token.value);
        return doubleLiteral;
    }

    if (token.type == IDENTIFIER) {
        auto variableCall = std::make_shared<ASTExpressionVariable>();
        variableCall->variableName = token.value;
        if (variableCall->variableName == "TO_END") toEndFlagUsed = true;
        if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == PERIOD) { // if variable takes the form (enum.attribute)
            variableCall->variableName += eatToken(PERIOD).value;
            variableCall->variableName += eatToken(IDENTIFIER).value;
        }
        return variableCall;
    }

    if (token.type == OPEN_PAREN) {
        auto node = parseExpression();
        eatToken(CLOSE_PAREN);
        return node;
    }
    ErrorHandler::throwError("Unexpected token in factor", tokens, masterIndex);
    return std::make_shared<ASTExpression>();
};

// 2. Terms: Handles Multiplication and Division
std::shared_ptr<ASTExpression> AST::parseTerm() {
    auto node = parseFactor();

    Token nextToken = Lexer::nextNonWhiteSpaceToken(tokens, masterIndex);
    while (nextToken.type == BINARY_OPERATOR && (nextToken.value == "*" || nextToken.value == "/")) {
        std::string operation = eatToken(BINARY_OPERATOR).value;
        auto right = parseFactor();
        auto binOpNode = std::make_shared<ASTExpressionBinaryOperation>();
        binOpNode->op = operation;
        binOpNode->left = node;
        binOpNode->right = right;
        node = binOpNode;
        nextToken = Lexer::nextNonWhiteSpaceToken(tokens, masterIndex);
    }
    return node;
};


// 3. Expressions: Handles Addition and Subtraction
std::shared_ptr<ASTExpression> AST::parseExpression() {
    auto node = parseTerm();

    Token nextToken = Lexer::nextNonWhiteSpaceToken(tokens, masterIndex);
    while (nextToken.type == BINARY_OPERATOR && (nextToken.value == "+" || nextToken.value == "-" || nextToken.value == "%")) {
        std::string operation = eatToken(BINARY_OPERATOR).value;
        auto right = parseTerm();
        auto binOpNode = std::make_shared<ASTExpressionBinaryOperation>();
        binOpNode->op = operation;
        binOpNode->left = node;
        binOpNode->right = right;
        node = binOpNode;
        nextToken = Lexer::nextNonWhiteSpaceToken(tokens, masterIndex);
    }
    return node;
}