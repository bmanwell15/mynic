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

// Variant type used for all evaluated values in the AST and interpreter.
using Value = std::variant<bool, char, uint8_t, uint64_t,
                 int8_t, int16_t, int32_t, int64_t,
                 float, double, std::string>;

// AST node categories used during parsing and interpretation.
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

// Operators supported by conditional and expression logic.
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

// Base AST field node.
struct ASTField {
    NodeType type;
};

// Base AST expression node.
struct ASTExpression {
    virtual ~ASTExpression() = default;
};

// Integer literal expression.
struct ASTExpressionInt : public ASTExpression {
    uint64_t value;
};

// Double literal expression.
struct ASTExpressionDouble : public ASTExpression {
    double value;
};

// Variable reference expression.
struct ASTExpressionVariable : public ASTExpression {
    std::string variableName;
};

// Binary expression node for arithmetic and logic.
struct ASTExpressionBinaryOperation : public ASTExpression {
    std::string op;
    std::shared_ptr<ASTExpression> left;
    std::shared_ptr<ASTExpression> right;
};

// Function call expression node.
struct ASTExpressionFunctionCall : public ASTExpression {
    std::string className;
    std::string funcName;
    std::vector<std::shared_ptr<ASTExpression>> parameters;
};

// Void function call AST node.
struct ASTFunctionCall : public ASTField { // Void functions not used in expressions, such as REWIND()
    std::string funcName;
    std::vector<std::shared_ptr<ASTExpression>> parameters;
};

// A conditional expression node used by branches and switch cases.
struct ASTCondition : public ASTExpression {
    std::shared_ptr<ASTExpression> left;
    std::shared_ptr<ASTExpression> right;
    std::string op;
};

// Settings associated with primitive values.
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

// Primitive value field in the AST.
struct ASTPrimitiveValue : public ASTField {
    std::string name;
    std::string datatype;
    size_t sizeInBits;
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

// Type alias definition.
struct ASTTypeDef : public ASTField {
    std::string name;
    std::string newTypeName;
    std::string existingTypeName;
};

// Enum value definition with optional metadata.
struct ASTEnumVariable : public ASTField {
    std::string varName;
    Value varValue;
    std::unordered_map<std::string, Value> enumAttributes;
};

// Enum definition node.
struct ASTEnum : public ASTField {
    std::string datatype;
    std::string name;
    size_t sizeInBits;
    std::vector<std::shared_ptr<ASTEnumVariable>> variables;
};

// Bitfield definition node.
struct ASTBitfield : public ASTField {
    std::string name;
    size_t sizeInBits;
    std::vector<std::shared_ptr<ASTField>> subfields;
};

// Union definition node.
struct ASTUnion : public ASTField {
    std::string name;
    size_t sizeInBits;
    std::vector<std::shared_ptr<ASTField>> subfields;
};

// Parsing condition node for conditional field selection.
struct ASTParsingCondition : public ASTField {
    ConditionOperators conditionOperator;
    Value parsedCheckValue;
};

// Branch construct node.
struct ASTBranch : public ASTField {
    ASTPrimitiveValue parseAs;
    std::vector<std::pair<std::shared_ptr<ASTField>, std::shared_ptr<ASTParsingCondition>>> destinationsAndConditions;
    std::shared_ptr<ASTField> destinationDefault;
};

// Switch construct node for conditional parsing.
struct ASTSwitch : public ASTField {
    std::string variableName;
    std::vector<std::pair<std::shared_ptr<ASTField>, std::shared_ptr<ASTParsingCondition>>> destinationsAndConditions;
    std::shared_ptr<ASTField> destinationDefault;
};

// Default settings block node.
struct ASTDefault : public ASTField {
    std::shared_ptr<ASTPrimitiveValueSettings> settings;
};

// Array field node.
struct ASTArray : public ASTField {
    size_t length;
    size_t sizeInBits = 0;
    std::shared_ptr<ASTExpression> dynamicLength;
    std::shared_ptr<ASTPrimitiveValue> elementSchema;
};

// Packet or segment definition node.
struct ASTPacket : public ASTField {
    std::string name;
    size_t sizeInBits = 0;
    std::vector<std::shared_ptr<ASTField>> fields;
    std::shared_ptr<ASTPrimitiveValueSettings> defaultSettings;
};

// Root AST node containing all parsed definitions.
struct ASTNode {
    NodeType type;
    std::unordered_map<std::string, std::shared_ptr<ASTField>> properties; // Holds everything, packets, enums, etc.
};

// Parses Mynic tokens into an AST and calculates field sizes.
class AST {
    public:
        // Constructs the AST parser with a reference to the owning Mynic instance.
        explicit AST(Mynic* myn);

        Interpreter* interpreter;

        // Primitive type bit sizes used during parsing.
        std::unordered_map<std::string, size_t> primitiveBitSizes;

        // Variables defined by the source file.
        std::unordered_map<std::string, Value> definedVariables; // <varName, varValue> Stored in AST because AST will replace variables with Values during compilation

        // Typedef aliases for custom type names.
        std::unordered_map<std::string, std::string> typedefAliases;

        inline static const std::unordered_set<std::string> MYNIC_KEYWORDS = {"TO_END", "END_OF_STREAM", "EOF"};
        inline static const std::unordered_set<std::string> MYNIC_FUNCTIONS = {"TERMINATE_IF"};

        // Parses a list of tokens into the AST root node.
        std::shared_ptr<ASTNode> parseTokensToAST(const std::vector<Token>& inputTokens, bool isMainFile=true);

        // Computes the size in bits for a field or packet.
        size_t getStructureSize(std::shared_ptr<ASTField> field);

        // Computes the bit size of a type from a token.
        size_t getTypeBitSize(const std::string& type, size_t tokenIndex);

    private:
        size_t masterIndex;
        std::vector<Token> tokens;
        std::shared_ptr<ASTNode> rootNode;
        Mynic* mynic;
        ASTPacket* currentPacket;

        // Returns true when the type is known to the parser.
        bool isKnownType(const std::string& type);

        // Token parsing helpers.
        Token eatToken(TokenType expectedType);
        Token eatToken(std::initializer_list<TokenType> types);
        void eatOptionalToken(std::initializer_list<TokenType> types);
        void skipWhiteSpace(bool includeCommas=false, bool includeSemiColons=false);
        Value convertTokenValue(Token token);

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