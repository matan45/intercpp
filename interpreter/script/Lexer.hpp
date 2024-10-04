#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <stack>
#include <set>

#include "Types.hpp"

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
private:
	std::string input;
	size_t pos;
	std::stack<char> balanceStack;  // Stack for keeping track of parentheses and braces
	std::set<std::string> includedFiles;

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
		{"false", TokenType::FALSE},
		{"void", TokenType::VOID_TYPE},
		{"#import", TokenType::IMPORT}
	};

public:
    explicit Lexer(const std::string& input) : input(input), pos(0) {}

    Token getNextToken();

private:
    Token parseStringLiteral();
    double parseNumber();
    std::string parseIdentifier();
	std::string readFile(const std::string& filePath);
};
