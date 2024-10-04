#pragma once
#include <stdexcept>
#include <vector>
#include <string>

#include "Lexer.hpp"

#include "AST.hpp"

class Parser {

private:
	Lexer& lexer;
	Environment& env;
	Token currentToken;

public:
	Parser(Lexer& lexer, Environment& env) : lexer(lexer), env(env), currentToken(lexer.getNextToken()) {}

	ASTNode* parse() {
		return parseProgram();
	}

private:
	
	void eat(TokenType type);

	ASTNode* parseProgram();
	ASTNode* parseStatement();
	
	ASTNode* parseForStatement();
	ASTNode* parseDoWhileStatement();
	ASTNode* parseWhileStatement();

	ASTNode* parseFunctionDefinition();
	ASTNode* parseFunctionCall(const std::string& functionName);

	ASTNode* parseDeclaration();
	ASTNode* parseAssignment(const std::string& identifier);

	ASTNode* parseIfStatement();

	ASTNode* parseExpression() {
		return parseLogicalOr();
	}

	ASTNode* parseLogicalOr();

	ASTNode* parseLogicalAnd();

	ASTNode* parseEquality();

	ASTNode* parseComparison();

	ASTNode* parseTerm();

	ASTNode* parseFactor();

	ASTNode* parseUnary();

	ASTNode* parsePrimary();

	ASTNode* parseMapLiteral();
	ASTNode* parseArrayLiteral();

	ValueType parseType();

	std::string tokenToString(const Token& token);
};

