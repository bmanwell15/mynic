#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

typedef unsigned short blockDepth_t;
extern blockDepth_t blockDepth;

// Token types used by the Mynic lexer.
enum TokenType {
    NEW_LINE,
    SEMI_COLON,
    SINGLE_LINED_COMMENT,
    MULTI_LINED_COMMENT,
    BOOL_LITERAL,
    HEX_LITERAL,
    BINARY_LITERAL,
    INT_LITERAL,
    DOUBLE_LITERAL,
    STRING_LITERAL,

    INCREMENT_BY_1, // ++ & --
    BINARY_OPERATOR, // +, -, *, /, %
    LOGIC_GATE, // &&, ||, !
    CONDITION_OPERATOR, // ==, !=, <=, >=, >, <
    IDENTIFIER, // Any string of text, such as variables, keywords, functions, etc. The IDENTIFIER is specified in the AST
    
    EQUALS,
    OPEN_PAREN, CLOSE_PAREN,
    OPEN_BRACKET, CLOSE_BRACKET,
    OPEN_SQUARE_BRACKET, CLOSE_SQUARE_BRACKET,
    COMMA, PERIOD, COLON, EXCLAMATION_POINT,
    ASTERICT,

    FUNCTION_CALL
};

// Lexical token produced from source text.
struct Token {
    std::string value;
    TokenType type;
    blockDepth_t blockDepth;
    size_t lineNumber;
    unsigned short fileIndex;
};

// Lexer for converting Mynic source into tokens.
class Lexer {
    public:
        inline static std::vector<std::string> fileNames; // Holds all of the file names that have been tokenized so far;

        // Tokenizes source content into a sequence of tokens.
        static std::vector<Token> tokenize(const std::string content);

        // Creates a token from a single character.
        static Token createToken(const char value, const TokenType type, const blockDepth_t blockDepth, size_t lineNumber);

        // Creates a token from a string value.
        static Token createToken(const std::string value, const TokenType type, const blockDepth_t blockDepth, size_t lineNumber);

        // Prints a single token to stdout.
        static void printToken(const Token t);

        // Skips whitespace and optional separators in token stream.
        static void skipWhiteSpace(const std::vector<Token>& tokens, size_t& i, bool includeCommas=false, bool includeSemiColons=false);

        // Returns the next non-whitespace token.
        static Token nextNonWhiteSpaceToken(const std::vector<Token>& tokens, size_t& i);

        // Checks if the next token matches the specified type.
        static bool isNextTokenType(const std::vector<Token>& tokens, size_t& i, TokenType type);

        // Checks if the next token matches any of the specified types.
        static bool isNextTokenType(const std::vector<Token>& tokens, size_t& i, std::initializer_list<TokenType> types);
};

#endif