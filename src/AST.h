#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <cctype>
#include <variant>
#include <sstream> 

#include "lexer.h"
#include "ErrorHandler.h"

class Interpreter; // Forward declaration
class Mynic;

using Value = std::variant<bool, char, uint8_t, uint64_t,
                 int8_t, int16_t, int32_t, int64_t,
                 float, double, std::string>;

enum class NodeType {
    ROOT_NODE,
    PRIMITIVE,
    UNION,
    BITFIELD,
    ARRAY,
    BRANCH,
    PACKET,
    SEGMENT,
    ENUM,
    TYPEDEF,
    VARIABLE,
    DEFAULT_BLOCK
};


struct ASTField {
    NodeType type;
};

struct ASTPrimitiveValueSettings {
    bool endianBig = true;
    bool isHidden = false;
    std::string units = "";
};

struct ASTPrimitiveValue : public ASTField {
    std::string name;
    std::string datatype;
    size_t sizeInBits;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

struct ASTTypeDef : public ASTField {
    std::string name;
    std::string newTypeName;
    std::string existingTypeName;
};

struct ASTEnumVariable : public ASTField {
    std::string varName;
    Value varValue;
    std::unordered_map<std::string, Value> enumAttributes;
};

struct ASTEnum : public ASTField {
    std::string datatype;
    std::string name;
    size_t sizeInBits;
    std::vector<std::shared_ptr<ASTEnumVariable>> variables;
};

struct ASTBitfield : public ASTField {
    std::string name;
    std::vector<std::shared_ptr<ASTField>> subfields;
};

struct ASTUnion : public ASTField {
    std::string name;
    std::vector<std::shared_ptr<ASTField>> subfields;
};

struct ASTDefault : public ASTField {
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

struct ASTArray : public ASTField {
    size_t length;
    std::string dynamicLength;
    std::shared_ptr<ASTPrimitiveValue> elementSchema;
};

struct ASTPacket : public ASTField {
    std::string name;
    std::vector<std::shared_ptr<ASTField>> fields;
    std::shared_ptr<ASTPrimitiveValueSettings> defaultSettings;
};

struct ASTNode {
    NodeType type;
    std::unordered_map<std::string, std::shared_ptr<ASTField>> properties; // Holds everything, packets, enums, etc.
};

class AST {
    public:
        explicit AST(Mynic* myn);
        Interpreter* interpreter;
        std::shared_ptr<ASTNode> parseTokensToAST(const std::vector<Token>& inputTokens, bool isMainFile=true);
        std::unordered_map<std::string, size_t> primitiveBitSizes;
        std::unordered_map<std::string, Value> definedVariables; // <varName, varValue> Stored in AST because AST will replace variables with Values during compilation

    private:
        size_t masterIndex;
        std::vector<Token> tokens;
        std::shared_ptr<ASTNode> rootNode;
        Mynic* mynic;
        ASTPacket* currentPacket;

        bool isKnownType(const std::string& type);

        Token eatToken(TokenType expectedType);
        Token eatToken(std::initializer_list<TokenType> types);
        void eatOptionalToken(std::initializer_list<TokenType> types);
        void skipWhiteSpace(bool includeCommas=false, bool includeSemiColons=false);
        size_t parseVariableCall();

        // Parsing functions
        std::shared_ptr<ASTField> parseField();
        std::shared_ptr<ASTPacket> parsePacket();
        std::shared_ptr<ASTField> parseTypeDef();
        std::shared_ptr<ASTField> parsePrimitive();
        std::shared_ptr<ASTPrimitiveValueSettings> parsePrimitiveSettings();
        std::shared_ptr<ASTField> parseImport();
        std::shared_ptr<ASTEnum> parseEnum();
        std::shared_ptr<ASTEnumVariable> parseEnumVarDefinition(ASTEnum* parent=nullptr);
        std::shared_ptr<ASTField> parseDefine();
        std::shared_ptr<ASTDefault> parseDefault();
        std::shared_ptr<ASTBitfield> parseBitfield();
        std::shared_ptr<ASTUnion> parseUnion();
};

#endif