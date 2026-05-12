# Mynic Header Files Documentation

This document provides concise documentation for all classes, structs, functions, and enums defined in the Mynic header files.

## adapters/adapters.h

**Purpose**: Header file that includes all adapter modules for external interfaces.

## adapters/FileHandler.h

**Namespace**: `adapters::file`

### Functions

- `std::string read(std::string filepath)` - Reads entire file content into a string
- `bool write(std::string filepath, std::string content)` - Writes string content to file (overwrites)
- `bool append(std::string filepath, std::string content)` - Appends string content to existing file

## adapters/json.h

**Namespace**: `adapters::json`

### Functions

- `std::string encode(const DecodedPacket& decodedPacket)` - Converts decoded packet to JSON string format

## adapters/objects.h

**Namespace**: `adapters::objects`

### Functions

- `void printSchema(std::shared_ptr<ASTNode>& rootNode)` - Prints complete schema of loaded packet definitions
- `void printPacket(std::shared_ptr<ASTPacket>& packet)` - Prints detailed structure of a packet
- `void printSegment(std::shared_ptr<ASTPacket>& packet)` - Prints detailed structure of a segment

## api/Mynic.h

### Class: Mynic

Main API class for packet parsing and decoding operations.

#### Public Methods

- `Mynic()` - Constructor, initializes AST and interpreter
- `~Mynic()` - Destructor, cleans up resources
- `std::vector<std::string> split(const std::string &txt, char ch)` - Splits string by delimiter character
- `bool loadFile(std::string& filename)` - Loads and parses .myn definition file
- `void printSchema()` - Prints all loaded packet definitions
- `void printPacket(const std::string& packetName)` - Prints specific packet structure
- `void printSegment(const std::string& segmentName)` - Prints specific segment structure
- `std::string version()` - Returns Mynic version string
- `DecodedPacket decodePacket(const std::string& strBytes, const std::string& packetName, bool asBits=false)` - Decodes hex/bit string using packet definition
- `DecodedPacket decodePacket(const std::vector<uint8_t>& dataBytes, const std::string& packetName)` - Decodes byte array using packet definition
- `std::vector<DecodedPacket> decodeFile(std::string filepath, char sep, std::string& packetName)` - Decodes multiple packets from file
- `void exportToFile(DecodedPacket& decodedPacket, std::string& filepath, bool appendMode=false)` - Exports decoded packet to JSON file

#### Private Methods

- `std::string collectFileCode(const std::string& FILENAME)` - Reads file content from disk
- `std::vector<uint8_t> bitsToBytes(const std::string& bitStr)` - Converts bit string to byte array

## cli/CommandHandler.h

### Class: CommandHandler

Handles CLI command parsing and execution.

#### Static Members

- `static std::shared_ptr<Mynic> decoder` - Shared Mynic instance for CLI operations
- `static std::string exportFilePath` - Path for export operations

#### Public Static Methods

- `static void runCommand(std::string command)` - Parses and executes CLI command

#### Private Static Methods

- `static void load(std::string command)` - Handles 'load' command for file loading
- `static void interpret(std::string command)` - Handles 'interpret' command for packet decoding
- `static void version()` - Handles 'version' command
- `static void refresh()` - Handles 'refresh' command to reload files
- `static void reset()` - Handles 'reset' command to clear all data

## core/AST.h

### Type Aliases

- `using Value = std::variant<bool, char, uint8_t, uint64_t, int8_t, int16_t, int32_t, int64_t, float, double, std::string>` - Union type for all possible Mynic values

### Enums

- `enum class NodeType` - AST node types (ROOT_NODE, PRIMITIVE, UNION, BITFIELD, ARRAY, BRANCH, PACKET, SEGMENT, ENUM, TYPEDEF, VARIABLE, DEFAULT_BLOCK, SWITCH, FUNCTION_CALL)
- `enum class ConditionOperators` - Comparison operators (EQUAL, NOT_EQUAL, LESS_THAN, GREATER_THAN, LESS_EQUAL_THAN, GREATER_EQUAL_THAN, AND, OR, NOT)

### Structs

#### ASTField
Base struct for all AST fields.
- `NodeType type` - Type of AST field

#### ASTExpression (Abstract Base)
Base class for expression nodes.
- `virtual ~ASTExpression() = default` - Virtual destructor

#### ASTExpressionInt : ASTExpression
Integer literal expression.
- `uint64_t value` - Integer value

#### ASTExpressionDouble : ASTExpression
Floating-point literal expression.
- `double value` - Double value

#### ASTExpressionVariable : ASTExpression
Variable reference expression.
- `std::string variableName` - Name of referenced variable

#### ASTExpressionBinaryOperation : ASTExpression
Binary operation expression.
- `std::string op` - Operator (+, -, *, /, etc.)
- `std::shared_ptr<ASTExpression> left` - Left operand
- `std::shared_ptr<ASTExpression> right` - Right operand

#### ASTExpressionFunctionCall : ASTExpression
Function call expression.
- `std::string className` - Class name (for method calls)
- `std::string funcName` - Function name
- `std::vector<std::shared_ptr<ASTExpression>> parameters` - Function arguments

#### ASTFunctionCall : ASTField
Void function call (not used in expressions).
- `std::string funcName` - Function name
- `std::vector<std::shared_ptr<ASTExpression>> parameters` - Function arguments

#### ASTCondition : ASTExpression
Conditional expression for branching.
- `std::shared_ptr<ASTExpression> left` - Left side of condition
- `std::shared_ptr<ASTExpression> right` - Right side of condition
- `std::string op` - Comparison operator

#### ASTPrimitiveValueSettings
Settings for primitive value parsing.
- `std::string units` - Unit string for display
- `std::shared_ptr<ASTExpression> exprASTTree` - Expression tree for validation
- `struct Flags` - Boolean flags for parsing behavior
  - `bool endianBig` - Big-endian byte order
  - `bool isHidden` - Field should be hidden in output
  - `bool includeTimestamp` - Include timestamp in output
  - `bool includeRawBytes` - Include raw bytes in output
  - `bool includePacketName` - Include packet name in output
  - `bool packetShouldReturn` - Packet should return result

#### ASTPrimitiveValue : ASTField
Represents a primitive data field.
- `std::string name` - Field name
- `std::string datatype` - Data type (uint8, float, etc.)
- `size_t sizeInBits` - Size in bits
- `std::shared_ptr<ASTPrimitiveValueSettings> settings` - Parsing settings

#### ASTTypeDef : ASTField
Type definition alias.
- `std::string name` - Alias name
- `std::string newTypeName` - New type name
- `std::string existingTypeName` - Existing type to alias

#### ASTEnumVariable : ASTField
Individual enum value with attributes.
- `std::string varName` - Variable name
- `Value varValue` - Variable value
- `std::unordered_map<std::string, Value> enumAttributes` - Additional attributes

#### ASTEnum : ASTField
Enumeration definition.
- `std::string datatype` - Underlying data type
- `std::string name` - Enum name
- `size_t sizeInBits` - Size in bits
- `std::vector<std::shared_ptr<ASTEnumVariable>> variables` - Enum values

#### ASTBitfield : ASTField
Bitfield definition for compact data storage.
- `std::string name` - Bitfield name
- `size_t sizeInBits` - Total size in bits
- `std::vector<std::shared_ptr<ASTField>> subfields` - Bitfield members

#### ASTUnion : ASTField
Union definition for overlapping data.
- `std::string name` - Union name
- `size_t sizeInBits` - Size in bits
- `std::vector<std::shared_ptr<ASTField>> subfields` - Union members

#### ASTParsingCondition : ASTField
Condition for conditional parsing.
- `ConditionOperators conditionOperator` - Type of condition
- `Value parsedCheckValue` - Value to check against

#### ASTBranch : ASTField
Branching construct for conditional parsing.
- `ASTPrimitiveValue parseAs` - Value to evaluate for branching
- `std::vector<std::pair<std::shared_ptr<ASTField>, std::shared_ptr<ASTParsingCondition>>> destinationsAndConditions` - Branch destinations and conditions
- `std::shared_ptr<ASTField> destinationDefault` - Default destination

#### ASTSwitch : ASTField
Switch statement for variable-based conditional parsing.
- `std::string variableName` - Variable to switch on
- `std::vector<std::pair<std::shared_ptr<ASTField>, std::shared_ptr<ASTParsingCondition>>> destinationsAndConditions` - Case destinations and conditions
- `std::shared_ptr<ASTField> destinationDefault` - Default case

#### ASTDefault : ASTField
Default settings block.
- `std::shared_ptr<ASTPrimitiveValueSettings> settings` - Default settings

#### ASTArray : ASTField
Array definition.
- `size_t length` - Fixed array length
- `size_t sizeInBits` - Total size in bits
- `std::shared_ptr<ASTExpression> dynamicLength` - Dynamic length expression
- `std::shared_ptr<ASTPrimitiveValue> elementSchema` - Element type definition

#### ASTPacket : ASTField
Packet or segment definition.
- `std::string name` - Packet/segment name
- `size_t sizeInBits` - Total size in bits
- `std::vector<std::shared_ptr<ASTField>> fields` - Packet fields
- `std::shared_ptr<ASTPrimitiveValueSettings> defaultSettings` - Default settings

#### ASTNode
Root AST node containing all definitions.
- `NodeType type` - Node type
- `std::unordered_map<std::string, std::shared_ptr<ASTField>> properties` - All defined packets, enums, etc.

### Class: AST

Abstract Syntax Tree parser and manager.

#### Public Members

- `Interpreter* interpreter` - Reference to interpreter instance
- `std::unordered_map<std::string, size_t> primitiveBitSizes` - Bit sizes for primitive types
- `std::unordered_map<std::string, Value> definedVariables` - Defined variables
- `std::unordered_map<std::string, std::string> typedefAliases` - Type aliases
- `inline static const std::unordered_set<std::string> MYNIC_KEYWORDS` - Reserved keywords
- `inline static const std::unordered_set<std::string> MYNIC_FUNCTIONS` - Built-in functions

#### Public Methods

- `explicit AST(Mynic* myn)` - Constructor with Mynic reference
- `std::shared_ptr<ASTNode> parseTokensToAST(const std::vector<Token>& inputTokens, bool isMainFile=true)` - Parse tokens into AST
- `size_t getStructureSize(std::shared_ptr<ASTField> field)` - Calculate field size
- `size_t getTypeBitSize(const std::string& type, size_t tokenIndex)` - Get type size in bits

#### Private Methods

- `bool isKnownType(const std::string& type)` - Check if type is known
- `Token eatToken(TokenType expectedType)` - Consume expected token
- `Token eatToken(std::initializer_list<TokenType> types)` - Consume token from list
- `void eatOptionalToken(std::initializer_list<TokenType> types)` - Optionally consume token
- `void skipWhiteSpace(bool includeCommas=false, bool includeSemiColons=false)` - Skip whitespace tokens
- `Value convertTokenValue(Token token)` - Convert token to value
- `size_t parseVariableCall()` - Parse variable reference
- `std::shared_ptr<ASTField> parseField()` - Parse generic field
- `std::shared_ptr<ASTPacket> parsePacket()` - Parse packet definition
- `std::shared_ptr<ASTField> parseTypeDef()` - Parse typedef
- `std::shared_ptr<ASTField> parsePrimitive()` - Parse primitive field
- `std::shared_ptr<ASTPrimitiveValueSettings> parsePrimitiveSettings()` - Parse field settings
- `std::shared_ptr<ASTField> parseImport()` - Parse import statement
- `std::shared_ptr<ASTEnum> parseEnum()` - Parse enum definition
- `std::shared_ptr<ASTEnumVariable> parseEnumVarDefinition(ASTEnum* parent=nullptr)` - Parse enum variable
- `std::shared_ptr<ASTField> parseDefine()` - Parse define statement
- `std::shared_ptr<ASTDefault> parseDefault()` - Parse default block
- `std::shared_ptr<ASTBitfield> parseBitfield()` - Parse bitfield
- `std::shared_ptr<ASTUnion> parseUnion()` - Parse union
- `std::shared_ptr<ASTBranch> parseBranch()` - Parse branch statement
- `std::shared_ptr<ASTSwitch> parseSwitch()` - Parse switch statement
- `std::shared_ptr<ASTExpression> parseFactor()` - Parse expression factor
- `std::shared_ptr<ASTExpression> parseTerm()` - Parse expression term
- `std::shared_ptr<ASTExpression> parseExpression()` - Parse full expression
- `std::shared_ptr<ASTExpression> parseFunctionCall(bool hasClassName, Token token)` - Parse function call
- `std::shared_ptr<ASTExpression> parseComparison()` - Parse comparison expression
- `std::shared_ptr<ASTExpression> parseLogicalAnd()` - Parse logical AND
- `std::shared_ptr<ASTExpression> parseLogicalOr()` - Parse logical OR
- `std::shared_ptr<ASTFunctionCall> parseVoidFunctionCall()` - Parse void function call

## core/BitQueue.h

### Class: BitQueue

Manages bit-level reading from byte streams for packet parsing.

#### Public Methods

- `BitQueue()` - Default constructor
- `explicit BitQueue(std::vector<uint8_t> data)` - Constructor with byte data
- `bool empty() const` - Check if queue is empty
- `size_t size() const` - Get remaining bits count
- `std::optional<uint64_t> pop(size_t n)` - Extract next n bits (max 64)
- `void rewind(size_t bits)` - Rewind bit position by specified bits
- `size_t bitPos() const` - Get current bit position
- `void setBitPos(size_t newBitPos)` - Set bit position

#### Private Methods

- `std::optional<uint64_t> readBits(size_t n)` - Internal bit reading implementation

## core/ErrorHandler.h

### Enum: InterpreterWarningCodes

Warning codes for interpreter issues.
- `UNKNOWN` - Unknown warning
- `VALIDATION_FAIL` - Validation failure
- `PACKET_NOT_FOUND` - Packet definition not found
- `EXPR_VAR_NOT_PRIMITIVE` - Expression variable not primitive
- `ENUM_VAL_NOT_FOUND` - Enum value not found
- `ENUM_NOT_FOUND` - Enum not found
- `ENUM_TYPE_NOT_INT` - Enum type not integer
- `UNKNOWN_DATATYPE` - Unknown data type
- `INVALID_STRING_OPERATION` - Invalid string operation
- `INVALID_BINARY_OPERATOR_TYPE` - Invalid binary operator type
- `FUNCTION_NOT_FOUND` - Function not found
- `VARIABLE_NOT_FOUND_IN_EXPR` - Variable not found in expression
- `VARIABLE_NOT_FOUND_IN_SWITCH` - Variable not found in switch
- `ARRAY_LENGTH_NOT_INT` - Array length not integer
- `WRONG_NUMBER_OF_PARAMETERS` - Wrong parameter count
- `VARIABLE_NOT_ARITHMETIC` - Variable not arithmetic type
- `BIT_QUEUE_INDEX_OUT_OF_BOUNDS` - Bit queue index out of bounds
- `BIT_QUEUE_EMPTY` - Bit queue is empty

### Struct: InterpreterWarning

Represents a warning during interpretation.
- `InterpreterWarningCodes warningType` - Type of warning
- `std::string errorMessage` - Warning message

### Class: ErrorHandler

Handles errors and warnings during parsing and interpretation.

#### Public Static Methods

- `static void throwError(const std::string& message, std::vector<Token>& tokens, size_t i, std::string packetName="")` - Throw parsing error
- `static InterpreterWarning throwInterpreterWarning(InterpreterWarningCodes code, std::string message)` - Create warning

#### Private Static Methods

- `static void printLine(std::vector<Token>& tokens, size_t i, bool includeErrorSquiggle, blockDepth_t targetDepth=0)` - Print error line
- `static void printLine(std::vector<Token>& tokens, size_t lineNumber)` - Print specific line
- `static void printBlock(std::vector<Token>& tokens, size_t i)` - Print code block

## core/Interpreter.h

### Structs

#### InterpretedField
Base struct for interpreted fields.
- `std::string name` - Field name
- `size_t sizeInBits` - Size in bits
- `NodeType type` - Field type

#### InterpretedPacket : InterpretedField
Represents a parsed packet or segment.
- `std::vector<std::shared_ptr<InterpretedField>> fields` - Packet fields
- `std::shared_ptr<ASTPrimitiveValueSettings> settings` - Packet settings

#### InterpretedPrimitiveValue : InterpretedField
Represents a parsed primitive value.
- `Value value` - Parsed value
- `std::shared_ptr<ASTPrimitiveValueSettings> settings` - Value settings
- `std::string datatype` - Data type

#### InterpretedBitfield : InterpretedField
Represents a parsed bitfield.
- `std::string name` - Bitfield name
- `std::vector<std::shared_ptr<InterpretedField>> subfields` - Bitfield members

#### InterpretedUnionfield : InterpretedField
Represents a parsed union field.
- `std::string name` - Union name
- `std::vector<std::shared_ptr<InterpretedField>> subfields` - Union members

#### InterpretedArray : InterpretedField
Represents a parsed array.
- `std::vector<std::shared_ptr<InterpretedField>> list` - Array elements

#### DecodedPacket
Result of packet decoding operation.
- `std::string packetName` - Name of packet definition used
- `std::vector<uint8_t> rawBytes` - Original input bytes
- `std::shared_ptr<InterpretedPacket> rootField` - Parsed packet structure
- `std::string timestamp` - Decoding timestamp
- `std::vector<InterpreterWarning> warnings` - Warnings generated during parsing

### Class: Interpreter

Executes packet parsing and interpretation.

#### Public Members

- `AST* ast` - Reference to AST instance
- `BitQueue bitQueue` - Bit-level data stream
- `std::shared_ptr<InterpretedPacket> rootNode` - Root of interpreted structure
- `std::vector<std::shared_ptr<ASTEnum>> enums` - Available enums
- `std::shared_ptr<ASTPrimitiveValueSettings> globalSettings` - Global settings
- `bool terminateSignal` - Termination flag
- `bool isEndOfStream` - End of stream flag

#### Public Methods

- `Interpreter()` - Constructor
- `DecodedPacket interpretBytes(const std::vector<uint8_t>& dataBytes, const std::string& packetName, const std::shared_ptr<ASTNode>& pAstTree)` - Interpret byte data
- `std::optional<Value> tryEvaluateASTExpression(std::shared_ptr<ASTExpression> node)` - Safely evaluate expression
- `Value evaluateASTExpression(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node)` - Evaluate expression
- `bool evaluateASTCondition(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpression> node)` - Evaluate condition
- `void throwWarning(InterpreterWarningCodes code, std::string message)` - Generate warning
- `uint64_t popBits(size_t bitNum)` - Extract bits from stream

#### Private Members

- `std::shared_ptr<DecodedPacket> decodedPacket` - Current decoding result
- `std::shared_ptr<ASTNode> astTree` - AST tree reference
- `std::shared_ptr<InterpretedPacket> currentPacket` - Current packet context

#### Private Methods

- `std::shared_ptr<InterpretedField> findField(const std::shared_ptr<InterpretedField>& field, const std::string& name)` - Find field by name
- `std::shared_ptr<InterpretedPacket> interpretPacket(const ASTPacket& packetDef, std::shared_ptr<InterpretedPacket> rootNode=nullptr)` - Interpret packet
- `std::shared_ptr<InterpretedField> interpretField(std::shared_ptr<ASTField> field)` - Interpret field
- `Value interpretValue(ASTPrimitiveValue& field)` - Interpret primitive value
- `std::optional<Value> getParsedValue(const std::shared_ptr<InterpretedField>& field, const std::string& varName)` - Get parsed variable value
- `void enforcePostInterpretationSettings(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive)` - Apply post-processing settings
- `Value evaluateASTExpressionFunctionCall(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall)` - Evaluate function call
- `Value evaluateBinaryOp(std::string op, Value left, Value right)` - Evaluate binary operation

## core/lexer.h

### Type Aliases

- `typedef unsigned short blockDepth_t` - Block depth type
- `extern blockDepth_t blockDepth` - Global block depth

### Enum: TokenType

Token types for lexical analysis.
- `NEW_LINE` - Newline character
- `SEMI_COLON` - Semicolon
- `SINGLE_LINED_COMMENT` - Single-line comment
- `MULTI_LINED_COMMENT` - Multi-line comment
- `BOOL_LITERAL` - Boolean literal
- `HEX_LITERAL` - Hexadecimal literal
- `BINARY_LITERAL` - Binary literal
- `INT_LITERAL` - Integer literal
- `DOUBLE_LITERAL` - Double literal
- `STRING_LITERAL` - String literal
- `INCREMENT_BY_1` - Increment/decrement operators
- `BINARY_OPERATOR` - Arithmetic operators
- `LOGIC_GATE` - Logical operators
- `CONDITION_OPERATOR` - Comparison operators
- `IDENTIFIER` - Identifiers and keywords
- `EQUALS` - Equals sign
- `OPEN_PAREN`, `CLOSE_PAREN` - Parentheses
- `OPEN_BRACKET`, `CLOSE_BRACKET` - Curly braces
- `OPEN_SQUARE_BRACKET`, `CLOSE_SQUARE_BRACKET` - Square brackets
- `COMMA`, `PERIOD`, `COLON`, `EXCLAMATION_POINT` - Punctuation
- `ASTERICT` - Asterisk
- `FUNCTION_CALL` - Function call token

### Struct: Token

Represents a lexical token.
- `std::string value` - Token value
- `TokenType type` - Token type
- `blockDepth_t blockDepth` - Block nesting depth
- `size_t lineNumber` - Line number in source
- `unsigned short fileIndex` - File index

### Class: Lexer

Performs lexical analysis on Mynic source code.

#### Static Members

- `inline static std::vector<std::string> fileNames` - List of processed files

#### Static Methods

- `static std::vector<Token> tokenize(const std::string content)` - Tokenize source code
- `static Token createToken(const char value, const TokenType type, const blockDepth_t blockDepth, size_t lineNumber)` - Create single-character token
- `static Token createToken(const std::string value, const TokenType type, const blockDepth_t blockDepth, size_t lineNumber)` - Create multi-character token
- `static void printToken(const Token t)` - Print token to console
- `static void skipWhiteSpace(const std::vector<Token>& tokens, size_t& i, bool includeCommas=false, bool includeSemiColons=false)` - Skip whitespace tokens
- `static Token nextNonWhiteSpaceToken(const std::vector<Token>& tokens, size_t& i)` - Get next non-whitespace token
- `static bool isNextTokenType(const std::vector<Token>& tokens, size_t& i, TokenType type)` - Check if next token matches type
- `static bool isNextTokenType(const std::vector<Token>& tokens, size_t& i, std::initializer_list<TokenType> types)` - Check if next token matches any type

## core/MynicLib.h

### Class: MynicLib

Provides built-in mathematical and utility functions for Mynic expressions.

#### Public Static Methods

**Mathematical Functions:**
- `static Value mathMax(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Maximum of values
- `static Value mathMin(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Minimum of values
- `static Value mathPow(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Power function
- `static Value mathSqrt(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Square root
- `static Value mathLog(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter, uint8_t logBase = 0)` - Logarithm
- `static Value mathRound(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Round to nearest integer
- `static Value mathSign(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Sign function
- `static Value mathAbs(std::shared_ptr<InterpretedPrimitiveValue> interpretedPrimitive, std::shared_ptr<ASTExpressionFunctionCall> functionCall, Interpreter* interpreter)` - Absolute value
- `static Value mathPi()` - Pi constant
- `static Value mathE()` - Euler's number constant

**Utility Functions:**
- `static void rewind(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter)` - Rewind bit stream
- `static void skip(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter)` - Skip bits in stream
- `static void seek(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter)` - Seek to position in stream
- `static void terminateIf(std::shared_ptr<ASTFunctionCall> functionCall, Interpreter* interpreter)` - Conditionally terminate parsing</content>
<parameter name="filePath">c:\Users\benja\OneDrive - purdue.edu\Programming\C++\mynic\docs\header-reference.md