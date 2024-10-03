#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <stack>

enum class TokenType {
    IDENTIFIER,
    NUMBER,
    STRING_LITERAL,
    PLUS, MINUS, MULTIPLY, DIVIDE,
    ASSIGN, // =
    LPAREN, RPAREN, // ( )
    LBRACE, RBRACE, // { }
    COMMA, SEMICOLON, // , ;
    AND, OR, NOT, // &&, ||, !
    EQUALS, NOT_EQUALS, // ==, !=
    LESS, LESS_EQUALS, GREATER, GREATER_EQUALS, // <, <=, >, >=
    FUNC, RETURN, IF, ELSE, WHILE, FOR, // Keywords
    INT, FLOAT, BOOL, STRING_TYPE, // Data types
    TRUE, FALSE, DO,
    END // End of input
};

// Token representation
struct Token {
    TokenType type;
    double numberValue;
    std::string stringValue;

    Token(TokenType type) : type(type), numberValue(0) {}
    Token(TokenType type, double numberValue) : type(type), numberValue(numberValue) {}
    Token(TokenType type, const std::string& stringValue) : type(type), stringValue(stringValue) {}
};

// Lexer to tokenize input code
class Lexer {
public:
    explicit Lexer(const std::string& input) : input(input), pos(0) {}

    Token getNextToken() {
        while (pos < input.length()) {
            char current = input[pos];

            // Skip whitespaces
            if (isspace(current)) {
                ++pos;
                continue;
            }

            // Skip single-line comments (`//`)
            if (current == '/' && pos + 1 < input.length() && input[pos + 1] == '/') {
                pos += 2;  // Skip `//`
                while (pos < input.length() && input[pos] != '\n') {
                    ++pos;  // Skip until the end of the line
                }
                continue;
            }

            // Skip multi-line comments (`/* ... */`)
            if (current == '/' && pos + 1 < input.length() && input[pos + 1] == '*') {
                pos += 2;  // Skip `/*`
                while (pos + 1 < input.length() && !(input[pos] == '*' && input[pos + 1] == '/')) {
                    ++pos;  // Skip until closing `*/`
                }
                pos += 2;  // Skip `*/`
                continue;
            }

            // Tokenize numbers
            if (isdigit(current) || current == '.') {
                return Token(TokenType::NUMBER, parseNumber());
            }

            // Tokenize identifiers and keywords
            if (isalpha(current) || current == '_') {
                std::string identifier = parseIdentifier();
                if (keywords.find(identifier) != keywords.end()) {
                    return Token(keywords[identifier]);
                }
                return Token(TokenType::IDENTIFIER, identifier);
            }

            // Handling two-character logical operators `&&` and `||`
            if (current == '&' && pos + 1 < input.length() && input[pos + 1] == '&') {
                pos += 2;
                return Token(TokenType::AND);
            }
            if (current == '|' && pos + 1 < input.length() && input[pos + 1] == '|') {
                pos += 2;
                return Token(TokenType::OR);
            }

            // Handling comparison operators (`==`, `!=`, `<`, `>`, `<=`, `>=`)
            if (current == '=' && pos + 1 < input.length() && input[pos + 1] == '=') {
                pos += 2;
                return Token(TokenType::EQUALS);
            }
            if (current == '!' && pos + 1 < input.length() && input[pos + 1] == '=') {
                pos += 2;
                return Token(TokenType::NOT_EQUALS);
            }
            if (current == '<') {
                if (pos + 1 < input.length() && input[pos + 1] == '=') {
                    pos += 2;
                    return Token(TokenType::LESS_EQUALS);
                }
                ++pos;
                return Token(TokenType::LESS);
            }
            if (current == '>') {
                if (pos + 1 < input.length() && input[pos + 1] == '=') {
                    pos += 2;
                    return Token(TokenType::GREATER_EQUALS);
                }
                ++pos;
                return Token(TokenType::GREATER);
            }

            // Handling single-character tokens and maintaining stack balance
            switch (current) {
            case '+': ++pos; return Token(TokenType::PLUS);
            case '-': ++pos; return Token(TokenType::MINUS);
            case '*': ++pos; return Token(TokenType::MULTIPLY);
            case '/': ++pos; return Token(TokenType::DIVIDE);
            case '=': ++pos; return Token(TokenType::ASSIGN);
            case '!': ++pos; return Token(TokenType::NOT);
            case '(':
                balanceStack.push('(');
                ++pos;
                return Token(TokenType::LPAREN);
            case ')':
                if (balanceStack.empty() || balanceStack.top() != '(') {
                    throw std::runtime_error("Unmatched closing parenthesis");
                }
                balanceStack.pop();
                ++pos;
                return Token(TokenType::RPAREN);
            case '{':
                balanceStack.push('{');
                ++pos;
                return Token(TokenType::LBRACE);
            case '}':
                if (balanceStack.empty() || balanceStack.top() != '{') {
                    throw std::runtime_error("Unmatched closing brace");
                }
                balanceStack.pop();
                ++pos;
                return Token(TokenType::RBRACE);
            case ',': ++pos; return Token(TokenType::COMMA);
            case ';': ++pos; return Token(TokenType::SEMICOLON);
            default: throw std::runtime_error("Unexpected character");
            }
        }

        // Check if the stack is empty at the end of input
        if (!balanceStack.empty()) {
            throw std::runtime_error("Unmatched opening symbols found");
        }

        return Token(TokenType::END);
    }

private:
    std::string input;
    size_t pos;
    std::stack<char> balanceStack;  // Stack for keeping track of parentheses and braces

    // List of keywords and their corresponding TokenType
    std::unordered_map<std::string, TokenType> keywords = {
        {"func", TokenType::FUNC},
        {"return", TokenType::RETURN},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"do", TokenType::DO},
        {"for", TokenType::FOR},
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT},
        {"bool", TokenType::BOOL},
        {"string", TokenType::STRING_TYPE},
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE}
    };

    double parseNumber() {
        size_t startPos = pos;
        while (pos < input.length() && (isdigit(input[pos]) || input[pos] == '.')) {
            ++pos;
        }
        return std::stod(input.substr(startPos, pos - startPos));
    }

    std::string parseIdentifier() {
        size_t startPos = pos;
        while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
            ++pos;
        }
        return input.substr(startPos, pos - startPos);
    }
};
