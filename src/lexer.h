#ifndef LEXER_H
#define LEXER_H

#include <iostream>
#include <string>
#include <vector>
#include <cctype>

typedef unsigned short blockDepth_t;
extern blockDepth_t blockDepth;

enum TokenType { // Holds all of the allowed token values for the language
    NEW_LINE,
    SEMI_COLON,
    SINGLE_LINED_COMMENT,
    MULTI_LINED_COMMENT,
    BOOL_LITERAL,
    HEX_LITERAL,
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

struct Token {
    std::string value;
    TokenType type;
    blockDepth_t blockDepth;
    size_t lineNumber;
    unsigned short fileIndex;
};

class Lexer {
    public:
        inline static std::vector<std::string> fileNames; // Holds all of the file names that have been tokenized so far;

        static std::vector<Token> tokenize(const std::string content); // Converts the file content into its tokens. Returns an array of those tokens
        static Token createToken(const std::string value, const TokenType type, const blockDepth_t blockDepth, size_t lineNumber); // Creates and returns a token object given the specified values
        static void printToken(const Token t); // Prints a token to the terminal
        static void skipWhiteSpace(const std::vector<Token>& tokens, size_t& i, bool includeCommas=false, bool includeSemiColons=false);
        static Token nextNonWhiteSpaceToken(const std::vector<Token>& tokens, size_t& i);
        static bool isNextTokenType(const std::vector<Token>& tokens, size_t& i, TokenType type);
        static bool isNextTokenType(const std::vector<Token>& tokens, size_t& i, std::initializer_list<TokenType> types);
};

#endif