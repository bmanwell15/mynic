#include "lexer.h"


blockDepth_t blockDepth = 0;

Token Lexer::createToken(const std::string value, const TokenType type, const blockDepth_t blockDepth, size_t lineNumber) {
    Token t;
    t.value = value;
    t.type = type;
    t.blockDepth = blockDepth;
    t.lineNumber = lineNumber;
    t.fileIndex = fileNames.size() - 1;
    return t;
}

void Lexer::printToken(const Token t) {
    std::cout << "{value='" << t.value << "', type=" << t.type;
    std::cout << ", depth=" << t.blockDepth;
    std::cout << ", line=" << t.lineNumber;
     std::cout << ", file='" << Lexer::fileNames[t.fileIndex] << "'";
    std::cout << "}" << std::endl;
}

/* This functions skips all whitespace tokens in the provided token array, modifying the index reference 'i' to point to the next non-whitespace token. 
   It can optionally include commas and semi-colons as whitespace based on the boolean flags provided. */
void Lexer::skipWhiteSpace(const std::vector<Token>& tokens, size_t& i, bool includeCommas, bool includeSemiColons) {
    while (tokens[i].type == NEW_LINE || (tokens[i].type == COMMA && includeCommas) || (tokens[i].type == SEMI_COLON && includeSemiColons))
        i++;
}

/* Returns the next non-whitespace token in the provided token array, skipping any whitespace tokens. This function will not modify the value of 'i'. */
Token Lexer::nextNonWhiteSpaceToken(const std::vector<Token>& tokens, size_t& i) {
    size_t counter = i;
    skipWhiteSpace(tokens, counter);
    return tokens[counter];
}

/* Returns true if the next token is of the specified type, skipping whitespace tokens. This function will not modify the value of 'i'. */
bool Lexer::isNextTokenType(const std::vector<Token>& tokens, size_t& i, TokenType type) {
    size_t counter = i;
    if (type != NEW_LINE)
        skipWhiteSpace(tokens, counter);
    return tokens[counter].type == type;
}

/* Returns true if the next token is of any of the specified types, skipping whitespace tokens. This function will not modify the value of 'i'. */
bool Lexer::isNextTokenType(const std::vector<Token>& tokens, size_t& i, std::initializer_list<TokenType> types) {
    for (auto type : types) {
        size_t counter = i;
        if (type != NEW_LINE)
            skipWhiteSpace(tokens, counter);

        if (tokens[counter].type == type)
            return true;
    }
    return false;
}

std::vector<Token> Lexer::tokenize(const std::string content) {
    std::vector<Token> tokens;
    size_t lineNumber = 1;
    
    for (int i = 0; i < content.size(); i++) {
        char ch = content[i];
        enum TokenType ttype;
        if (ch == '/' && content[i + 1] == '/') { // SINGLE LINED COMMENTS
            while (content[i] != '\n') {i++;}
            i--; // Keep the new line in the token stream
            lineNumber++;
        } else if ((ch == '/' && content[i + 1] == '*') || (ch == '*' && content[i + 1] == '/')) { // MULTI LINED COMMENTS
            ttype = MULTI_LINED_COMMENT;
            std::string comment = std::string(1, ch) + std::string(1, content[++i]);
            tokens.push_back(createToken(comment, ttype, blockDepth, lineNumber));
        } else if ((ch == '&' && content[i + 1] == '&') || (ch == '|' && content[i + 1] == '|')) { // LOGIC GATES &&, ||
            ttype = LOGIC_GATE;
            std::string gate = std::string(1, ch) + std::string(1, content[++i]);
            tokens.push_back(createToken(gate, ttype, blockDepth, lineNumber)); 
        } else if ((ch == '+' && content[i + 1] == '+') || (ch == '-' && content[i + 1] == '-')) { // HANDLE ASSIGNMENT OPERATOR (++ & --)
            ttype = INCREMENT_BY_1;
            std::string oporation = std::string(1, ch) + std::string(1, content[++i]);
            tokens.push_back(createToken(oporation, ttype, blockDepth, lineNumber));
        } else if ((ch == '+' || ch == '-' || ch == '*' || ch == '/') && content[i + 1] != '=') { // HANDLE BINARY OPERATORS
            ttype = BINARY_OPERATOR;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if ((ch == '+' || ch == '-' || ch == '*' || ch == '/') && content[i + 1] == '=') { // HANDLE ASSIGNMENT OPERATOR (+=, -=, *=, ...)
            ttype = VARIABLE_ASSIGNMENT_OPERATOR;
            std::string oporation = std::string(1, ch) + std::string(1, content[++i]);
            tokens.push_back(createToken(oporation, ttype, blockDepth, lineNumber));
        } else if (ch == '=' && content[i + 1] != '=') { // Make sure it's not double equals '=='
            ttype = EQUALS;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if ((ch == '=' || ch == '>' || ch == '<' || ch == '!') && content[i + 1] == '=') { // ALL conditions that are 2 chars long (<=, >=, ==, !=)
            ttype = CONDITION;
            std::string doubleEquals = std::string(1, ch) + std::string(1, content[++i]);
            tokens.push_back(createToken(doubleEquals, ttype, blockDepth, lineNumber));
        } else if (ch == '<' || ch == '>') { // ALL conditions that are only 1 char long (>, <)
            ttype = CONDITION;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if (ch == ';') {
            ttype = SEMI_COLON;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if (ch == ':') {
            ttype = COLON;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if (ch == '(') {
            ttype = OPEN_PAREN;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if (ch == '\n') {
            ttype = NEW_LINE;
            lineNumber++;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if (ch == ')') {
            ttype = CLOSE_PAREN;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber));
        } else if (ch == '{') {
            blockDepth++;
            ttype = OPEN_BRACKET;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == '}') {
            blockDepth--;
            ttype = CLOSE_BRACKET;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == '[') {
            ttype = OPEN_SQUARE_BRACKET;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == ']') {
            ttype = CLOSE_SQUARE_BRACKET;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == ',') {
            ttype = COMMA;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == '.') {
            ttype = PERIOD;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == '*') {
            ttype = ASTERICT;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (ch == '!') {
            ttype = EXCLAMATION_POINT;
            tokens.push_back(createToken(std::string(1, ch), ttype, blockDepth, lineNumber)); 
        } else if (content.substr(i, 4) == "true") {
            ttype = BOOL_LITERAL;
            tokens.push_back(createToken(content.substr(i, 4), ttype, blockDepth, lineNumber));
            i += 3;
        } else if (content.substr(i, 5) == "false") {
            ttype = BOOL_LITERAL;
            tokens.push_back(createToken(content.substr(i, 5), ttype, blockDepth, lineNumber));
            i += 4;
        } else { // HANDLE MULTI-LETTERED OPERATORS
            if (ch == '\"') { // Make a string literal
                std::string str = "\"";
                i++; ch = content[i];
                while (ch != '\"'  && i < content.size()) {
                    str += std::string(1, ch);
                    i++;
                    ch = content[i];
                }
                str += "\"";
                
                ttype = STRING_LITERAL;
                tokens.push_back(createToken(str, ttype, blockDepth, lineNumber));

            } else if (isalpha(ch) || ch == '_') { // Make an IDENTIFIER
                std::string word = "";
                while ((isalpha(ch) || isdigit(ch) || ch == '_') && i < content.size()) {
                    word += std::string(1, ch);
                    i++;
                    ch = content[i];
                }
                i--;
                
                ttype = IDENTIFIER;
                tokens.push_back(createToken(word, ttype, blockDepth, lineNumber));

            } else if (isdigit(ch)) { // Get numbers, ints and doubles
                std::string num = "";
                bool isDouble = false;
                while ((isdigit(ch) || ch == '-' || ch == '.') && i < content.size()) {
                    if (ch == '.') {isDouble = true;}
                    num += std::string(1, ch);
                    i++;
                    ch = content[i];
                }
                i--;
                
                if (isDouble) {
                    ttype = DOUBLE_LITERAL;
                } else {
                    ttype = INT_LITERAL;
                }

                tokens.push_back(createToken(num, ttype, blockDepth, lineNumber));
            }
        }
    }
    return tokens;
}