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
#include <unordered_set>

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
    DEFAULT_BLOCK,
    SWITCH,
    FUNCTION_CALL
};

enum class ConditionOperators {
    EQUAL,
    NOT_EQUAL,
    LESS_THAN,
    GREATER_THAN,
    LESS_EQUAL_THAN,
    GREATER_EQUAL_THAN,
    AND,
    OR,
    NOT
};

struct ASTField {
    NodeType type;
};

struct ASTExpression {
    virtual ~ASTExpression() = default;
};

struct ASTExpressionInt : public ASTExpression {
    uint64_t value;
};

struct ASTExpressionDouble : public ASTExpression {
    double value;
};

struct ASTExpressionVariable : public ASTExpression {
    std::string variableName;
};

struct ASTExpressionBinaryOperation : public ASTExpression {
    std::string op;
    std::shared_ptr<ASTExpression> left;
    std::shared_ptr<ASTExpression> right;
};

struct ASTExpressionFunctionCall : public ASTExpression {
    std::string className;
    std::string funcName;
    std::vector<std::shared_ptr<ASTExpression>> parameters;
};

struct ASTFunctionCall : public ASTField { // Void functions not used in expressions, such as REWIND()
    std::string funcName;
    std::vector<std::shared_ptr<ASTExpression>> parameters;
};

struct ASTCondition : public ASTExpression {
    std::shared_ptr<ASTExpression> left;
    std::shared_ptr<ASTExpression> right;
    std::string op;
};

struct ASTPrimitiveValueSettings {
    std::string units = "";
    std::shared_ptr<ASTExpression> exprASTTree;
    struct Flags {
        bool endianBig : 1;
        bool isHidden : 1;
        bool includeTimestamp : 1;
        bool includeRawBytes : 1;
        bool includePacketName : 1;
        bool packetShouldReturn : 1;
        Flags() : 
            endianBig(true), 
            isHidden(false), 
            includeTimestamp(true), 
            includeRawBytes(true), 
            includePacketName(true),
            packetShouldReturn(false) {}
    };
    Flags flags;
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
    size_t sizeInBits;
    std::vector<std::shared_ptr<ASTField>> subfields;
};

struct ASTUnion : public ASTField {
    std::string name;
    size_t sizeInBits;
    std::vector<std::shared_ptr<ASTField>> subfields;
};

struct ASTParsingCondition : public ASTField {
    ConditionOperators conditionOperator;
    Value parsedCheckValue;
};

struct ASTBranch : public ASTField {
    ASTPrimitiveValue parseAs;
    std::vector<std::pair<std::shared_ptr<ASTField>, std::shared_ptr<ASTParsingCondition>>> destinationsAndConditions;
    std::shared_ptr<ASTField> destinationDefault;
};

struct ASTSwitch : public ASTField {
    std::string variableName;
    std::vector<std::pair<std::shared_ptr<ASTField>, std::shared_ptr<ASTParsingCondition>>> destinationsAndConditions;
    std::shared_ptr<ASTField> destinationDefault;
};

struct ASTDefault : public ASTField {
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

struct ASTArray : public ASTField {
    size_t length;
    size_t sizeInBits = 0;
    std::shared_ptr<ASTExpression> dynamicLength;
    std::shared_ptr<ASTPrimitiveValue> elementSchema;
};

struct ASTPacket : public ASTField {
    std::string name;
    size_t sizeInBits = 0;
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
        std::unordered_map<std::string, size_t> primitiveBitSizes;
        std::unordered_map<std::string, Value> definedVariables; // <varName, varValue> Stored in AST because AST will replace variables with Values during compilation
        inline static const std::unordered_set<std::string> MYNIC_KEYWORDS = {"TO_END", "END_OF_STREAM", "EOF"};
        inline static const std::unordered_set<std::string> MYNIC_FUNCTIONS = {"TERMINATE_IF"};

        std::shared_ptr<ASTNode> parseTokensToAST(const std::vector<Token>& inputTokens, bool isMainFile=true);
        size_t getStructureSize(std::shared_ptr<ASTField> field);

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
        std::shared_ptr<ASTBranch> parseBranch();
        std::shared_ptr<ASTSwitch> parseSwitch();

        std::shared_ptr<ASTExpression> parseFactor();
        std::shared_ptr<ASTExpression> parseTerm();
        std::shared_ptr<ASTExpression> parseExpression();
        std::shared_ptr<ASTExpression> parseFunctionCall(bool hasClassName, Token token);
        std::shared_ptr<ASTExpression> parseComparison();
        std::shared_ptr<ASTExpression> parseLogicalAnd();
        std::shared_ptr<ASTExpression> parseLogicalOr();

        std::shared_ptr<ASTFunctionCall> parseVoidFunctionCall();
};

#endif