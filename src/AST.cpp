#include "AST.h"
#include "Interpreter.h"
#include "Mynic.h"

AST::AST(Mynic* myn) {
    masterIndex = 0;
    tokens = {};
    rootNode = std::make_shared<ASTNode>(ASTNode{NodeType::ROOT_NODE, {}});
    primitiveBitSizes = {
        {"bit", 0},
        {"bits", 0},
        {"bool", 8},
        {"short", 16},
        {"ushort", 16},
        {"int", 32},
        {"uint", 32},
        {"float", 32},
        {"float32", 32},
        {"double", 64},
        {"long", 64},
        {"ulong", 64}
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

Token AST::eatToken(TokenType expectedType) {
    return eatToken({expectedType});
}

void AST::skipWhiteSpace(bool includeCommas, bool includeSemiColons) {
    while (masterIndex < tokens.size() && (tokens[masterIndex].type == NEW_LINE || (tokens[masterIndex].type == COMMA && includeCommas) || (tokens[masterIndex].type == SEMI_COLON && includeSemiColons)))
        masterIndex++;
}

// Returns true if string is in format intX or uintX where X is an integer
bool isIntX(const std::string& s) {
    if (s.size() <= 3) return false;          // must have "int" + at least 1 digit
    if (s.compare(0, 3, "int") != 0 && s.compare(0, 4, "uint") != 0) return false;
    size_t i = std::isdigit(s[3]) ? 3 : 4; // set to 3 if int, 4 if uint

    for (; i < s.size(); i++) 
        if (!std::isdigit(s[i])) return false;

    return true;
}

std::string removeQuotes(std::string& str) {
    if (str[0] == '\"')
        return str.substr(1, str.size() - 2);
    return str;
}


bool AST::isKnownType(const std::string& type) {
    return primitiveBitSizes.count(type) || isIntX(type);
}

std::shared_ptr<ASTField> AST::parseField() {
    const Token& token = tokens[masterIndex];

    if (token.type == MULTI_LINED_COMMENT) {
        masterIndex++;
        while (masterIndex < tokens.size() && tokens[masterIndex].type != MULTI_LINED_COMMENT) {masterIndex++;}
        return std::make_shared<ASTField>(ASTField{});
    }
    
    if (token.type == IDENTIFIER && token.value == "packet") {
        std::shared_ptr<ASTPacket> packet = parsePacket();
        rootNode->properties[packet->name] = packet;
        return packet;
    }

    if (token.type == IDENTIFIER && token.value == "typedef") {
        return parseTypeDef();
    }

    if (token.type == IDENTIFIER && token.value == "enum") {
        std::shared_ptr<ASTEnum> enumDef = parseEnum();
        interpreter->enums.push_back(enumDef);
        return enumDef;
    }

    if (token.type == IDENTIFIER && token.value == "bitfield") {
        return parseBitfield();
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
    return std::make_shared<ASTField>(ASTField{});
}

std::shared_ptr<ASTPacket> AST::parsePacket() {
    ASTPacket packetNode;
    packetNode.type = NodeType::PACKET;
    packetNode.defaultSettings = nullptr;
    currentPacket = &packetNode;

    eatToken(IDENTIFIER); // Consume 'packet' token
    packetNode.name = eatToken(IDENTIFIER).value;

    eatToken(OPEN_BRACKET);
    blockDepth_t packetDepth = tokens[masterIndex].blockDepth;

    while (masterIndex < tokens.size() && tokens[masterIndex].blockDepth >= packetDepth) {
        std::shared_ptr<ASTField> field = parseField();

        if (field->type == NodeType::DEFAULT_BLOCK) {
            packetNode.defaultSettings = std::static_pointer_cast<ASTDefault>(field)->settings;
        } else {
            packetNode.fields.push_back(field);
        }
        masterIndex++;
    }
    eatToken(CLOSE_BRACKET);
    currentPacket = nullptr;

    return std::make_shared<ASTPacket>(packetNode);
}

std::shared_ptr<ASTField> AST::parseTypeDef() {
    eatToken(IDENTIFIER); // Consume 'typedef' token
    ASTTypeDef typeDef;
    typeDef.type = NodeType::TYPEDEF;
    typeDef.existingTypeName = eatToken(IDENTIFIER).value;
    typeDef.newTypeName = eatToken(IDENTIFIER).value;

    if (isIntX(typeDef.existingTypeName)) {
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
    ASTPrimitiveValue field;
    field.datatype = eatToken(IDENTIFIER).value;

    if (isIntX(field.datatype)) {
        size_t pos = field.datatype.find_first_of("0123456789");
        if (pos != std::string::npos) {
            size_t bits = std::stoi(field.datatype.substr(pos));
            if (bits == 0 || bits > 64) {
                ErrorHandler::throwError("Integer type width must be between 1 and 64 bits: " + field.datatype, tokens, masterIndex - 1);
            }
            primitiveBitSizes[field.datatype] = bits;
        }
    }

    if (!isKnownType(field.datatype)) {
        ErrorHandler::throwError("Unknown primitive datatype: " + field.datatype, tokens, masterIndex - 1);
    }

    field.name = eatToken(IDENTIFIER).value;
    field.type = NodeType::PRIMITIVE;

    if (Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == COLON) {
        eatToken(COLON);
        field.sizeInBits = std::stoul(eatToken(INT_LITERAL).value);
        field.settings = parsePrimitiveSettings();
        return std::make_shared<ASTPrimitiveValue>(field);
    }

    field.sizeInBits = primitiveBitSizes[field.datatype];
    field.settings = parsePrimitiveSettings();
    return std::make_shared<ASTPrimitiveValue>(field);
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
        } else {
            ErrorHandler::throwError("Unknown primitive setting: " + settingToken.value, tokens, masterIndex - 1);
        }

        Token nextToken = eatToken({COMMA, NEW_LINE, CLOSE_BRACKET});
        if (nextToken.type == CLOSE_BRACKET || Lexer::nextNonWhiteSpaceToken(tokens, masterIndex).type == CLOSE_BRACKET) {
            break;
        }
    }

    std::cout << interpreter->globalSettings->units << std::endl;

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

std::shared_ptr<ASTVariable> AST::parseVarDefinition(ASTEnum* enumVar) {
    ASTVariable var;
    var.varName = eatToken(IDENTIFIER).value;
    Token possibleEqualSign = eatToken({COMMA, EQUALS, CLOSE_BRACKET, NEW_LINE});
    if (possibleEqualSign.type != EQUALS) { // If var defined like VAR_NAME, (Implicit value of 0 or next in Enum)
        if (!enumVar) ErrorHandler::throwError("Implicit decloration must be inside of an enum", tokens, masterIndex);

        if (enumVar->variables.size())
            var.varValue = std::get<uint64_t>(enumVar->variables.back()->varValue) + 1;
        else
            var.varValue = 0ULL;
        // if (tokens[masterIndex + 1].type == CLOSE_BRACKET) eatToken(CLOSE_BRACKET);
        return std::make_shared<ASTVariable>(var);
    }
    // Assumed explicit decloration here (VAR_NAME = VAL)
    Token varValToken = eatToken({INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL, BOOL_LITERAL});
    var.varValue = convertTokenValue(varValToken);
    eatToken({COMMA, CLOSE_BRACKET, NEW_LINE});
    return std::make_shared<ASTVariable>(var);
}

std::shared_ptr<ASTEnum> AST::parseEnum() {
    ASTEnum enumVar;
    eatToken(IDENTIFIER); // Parse enum token
    enumVar.datatype = eatToken(IDENTIFIER).value;

    if (enumVar.datatype.substr(0, 4) != "uint")
        ErrorHandler::throwError("Enums must have an uint type, got " + enumVar.datatype + " instead.", tokens, masterIndex);

    enumVar.name = eatToken(IDENTIFIER).value;
    eatToken(OPEN_BRACKET);

    blockDepth_t currentBlockDepth = tokens[masterIndex].blockDepth;
    while (masterIndex < tokens.size() && tokens[masterIndex].blockDepth >= currentBlockDepth) {
        std::shared_ptr<ASTVariable> var = parseVarDefinition(&enumVar);
        enumVar.variables.push_back(var);
    }

    primitiveBitSizes[enumVar.name] = primitiveBitSizes[enumVar.datatype];
    return std::make_shared<ASTEnum>(enumVar);
}

std::shared_ptr<ASTField> AST::parseDefine() {
    eatToken(IDENTIFIER); // Eat define token
    std::string varName = eatToken(IDENTIFIER).value;
    Value varValue = convertTokenValue(eatToken({INT_LITERAL, DOUBLE_LITERAL, STRING_LITERAL, BOOL_LITERAL}));
    definedVariables[varName] = varValue;
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
        std::shared_ptr<ASTField> var = parsePrimitive();
        bitfield.subfields.push_back(var);
        eatToken({SEMI_COLON, NEW_LINE});
    }
    return std::make_shared<ASTBitfield>(bitfield);
}