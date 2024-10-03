#pragma once
#include <stdexcept>
#include <vector>
#include <string>

#include "Lexer.hpp"

#include "AST.hpp"

class Parser {
public:
	Parser(Lexer& lexer, Environment& env) : lexer(lexer), env(env), currentToken(lexer.getNextToken()) {}

	ASTNode* parse() {
		return parseProgram();
	}

private:
	Lexer& lexer;
	Environment& env;
	Token currentToken;

	void eat(TokenType type) {
		if (currentToken.type == type) {
			currentToken = lexer.getNextToken();
		}
		else {
			throw std::runtime_error("Unexpected token type");
		}
	}

	ASTNode* parseProgram() {
		std::vector<ASTNode*> statements;
		while (currentToken.type != TokenType::END) {
			statements.push_back(parseStatement());
		}
		return new ProgramNode(statements);
	}

	ASTNode* parseStatement() {
		if (currentToken.type == TokenType::FUNC) {
			return parseFunctionDefinition();
		}
		else if (currentToken.type == TokenType::INT || currentToken.type == TokenType::FLOAT ||
			currentToken.type == TokenType::BOOL || currentToken.type == TokenType::STRING_TYPE) {
			return parseDeclaration();
		}
		else if (currentToken.type == TokenType::IDENTIFIER) {
			std::string identifier = currentToken.stringValue;
			eat(TokenType::IDENTIFIER);
			if (currentToken.type == TokenType::ASSIGN) {
				return parseAssignment(identifier);
			}
			else if (currentToken.type == TokenType::LPAREN) {
				return parseFunctionCall(identifier);
			}
		}
		else if (currentToken.type == TokenType::RETURN) {
			eat(TokenType::RETURN);
			ASTNode* expr = parseExpression();
			eat(TokenType::SEMICOLON);
			return new ReturnNode(expr);
		}
		else if (currentToken.type == TokenType::IF) {
			return parseIfStatement();
		}
		else if (currentToken.type == TokenType::WHILE) {
			return parseWhileStatement();
		}
		else if (currentToken.type == TokenType::FOR) {
			return parseForStatement();
		}
		else if (currentToken.type == TokenType::DO) {
			return parseDoWhileStatement();
		}
		else if (currentToken.type == TokenType::LBRACE) {
			eat(TokenType::LBRACE);
			std::vector<ASTNode*> bodyStatements;
			while (currentToken.type != TokenType::RBRACE) {
				bodyStatements.push_back(parseStatement());
			}
			eat(TokenType::RBRACE);
			return new BlockNode(bodyStatements);
		}

		throw std::runtime_error("Unexpected token in statement");
		}
	

	ASTNode* parseForStatement() {
		eat(TokenType::FOR);
		eat(TokenType::LPAREN);

		// Parse the initializer (declaration or assignment)
		ASTNode* initializer = nullptr;
		if (currentToken.type == TokenType::INT || currentToken.type == TokenType::FLOAT ||
			currentToken.type == TokenType::BOOL || currentToken.type == TokenType::STRING_TYPE) {
			initializer = parseDeclaration();
		}
		else if (currentToken.type == TokenType::IDENTIFIER) {
			std::string identifier = currentToken.stringValue;
			eat(TokenType::IDENTIFIER);
			initializer = parseAssignment(identifier);
		}

		// Parse the condition
		ASTNode* condition = parseExpression();
		eat(TokenType::SEMICOLON);

		// Parse the update expression
		ASTNode* update = parseExpression();
		eat(TokenType::RPAREN);

		// Parse the loop body
		ASTNode* body = parseStatement();

		return new ForNode(initializer, condition, update, body);
	}

	ASTNode* parseDoWhileStatement() {
		eat(TokenType::DO);
		ASTNode* body = parseStatement(); // Parse the loop body

		eat(TokenType::WHILE);
		eat(TokenType::LPAREN);
		ASTNode* condition = parseExpression();
		eat(TokenType::RPAREN);
		eat(TokenType::SEMICOLON);

		return new DoWhileNode(body, condition);
	}

	ASTNode* parseFunctionDefinition() {
		eat(TokenType::FUNC);
		ValueType returnType = parseType();
		std::string functionName = currentToken.stringValue;
		eat(TokenType::IDENTIFIER);

		eat(TokenType::LPAREN);
		std::vector<std::pair<std::string, ValueType>> parameters;
		if (currentToken.type != TokenType::RPAREN) {
			do {
				ValueType paramType = parseType();
				std::string paramName = currentToken.stringValue;
				eat(TokenType::IDENTIFIER);
				parameters.emplace_back(paramName, paramType);
				if (currentToken.type == TokenType::COMMA) {
					eat(TokenType::COMMA);
				}
				else {
					break;
				}
			} while (true);
		}
		eat(TokenType::RPAREN);

		eat(TokenType::LBRACE);
		ASTNode* body = parseStatement();
		eat(TokenType::RBRACE);

		FunctionNode* functionNode = new FunctionNode(functionName, returnType, parameters, body);
		env.registerUserFunction(functionName, functionNode);
		return functionNode;
	}

	ASTNode* parseDeclaration() {
		ValueType type = parseType();
		std::string variableName = currentToken.stringValue;
		eat(TokenType::IDENTIFIER);

		ASTNode* initializer = nullptr;
		if (currentToken.type == TokenType::ASSIGN) {
			eat(TokenType::ASSIGN);
			initializer = parseExpression();
		}

		eat(TokenType::SEMICOLON);
		return new DeclarationNode(variableName, type, initializer);
	}

	ASTNode* parseAssignment(const std::string& identifier) {
		eat(TokenType::ASSIGN);
		ASTNode* valueExpr = parseExpression();
		eat(TokenType::SEMICOLON);
		return new AssignmentNode(identifier, valueExpr);
	}

	ASTNode* parseFunctionCall(const std::string& functionName) {
		eat(TokenType::LPAREN);
		std::vector<ASTNode*> arguments;
		if (currentToken.type != TokenType::RPAREN) {
			do {
				arguments.push_back(parseExpression());
				if (currentToken.type == TokenType::COMMA) {
					eat(TokenType::COMMA);
				}
				else {
					break;
				}
			} while (true);
		}
		eat(TokenType::RPAREN);
		eat(TokenType::SEMICOLON);
		return new FunctionCallNode(functionName, arguments);
	}

	ASTNode* parseIfStatement() {
		eat(TokenType::IF);
		eat(TokenType::LPAREN);
		ASTNode* condition = parseExpression();
		eat(TokenType::RPAREN);

		ASTNode* thenBranch = parseStatement();
		ASTNode* elseBranch = nullptr;

		if (currentToken.type == TokenType::ELSE) {
			eat(TokenType::ELSE);
			elseBranch = parseStatement();
		}

		return new IfNode(condition, thenBranch, elseBranch);
	}

	ASTNode* parseWhileStatement() {
		eat(TokenType::WHILE);
		eat(TokenType::LPAREN);
		ASTNode* condition = parseExpression();
		eat(TokenType::RPAREN);
		ASTNode* body = parseStatement();
		return new WhileNode(condition, body);
	}

	ASTNode* parseExpression() {
		return parseLogicalOr();
	}

	ASTNode* parseLogicalOr() {
		ASTNode* left = parseLogicalAnd();

		while (currentToken.type == TokenType::OR) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* right = parseLogicalAnd();
			left = new BinaryOpNode(op, left, right);
		}

		return left;
	}

	ASTNode* parseLogicalAnd() {
		ASTNode* left = parseEquality();

		while (currentToken.type == TokenType::AND) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* right = parseEquality();
			left = new BinaryOpNode(op, left, right);
		}

		return left;
	}

	ASTNode* parseEquality() {
		ASTNode* left = parseComparison();

		while (currentToken.type == TokenType::EQUALS || currentToken.type == TokenType::NOT_EQUALS) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* right = parseComparison();
			left = new BinaryOpNode(op, left, right);
		}

		return left;
	}

	ASTNode* parseComparison() {
		ASTNode* left = parseTerm();

		while (currentToken.type == TokenType::LESS || currentToken.type == TokenType::LESS_EQUALS ||
			currentToken.type == TokenType::GREATER || currentToken.type == TokenType::GREATER_EQUALS) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* right = parseTerm();
			left = new BinaryOpNode(op, left, right);
		}

		return left;
	}

	ASTNode* parseTerm() {
		ASTNode* left = parseFactor();

		while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* right = parseFactor();
			left = new BinaryOpNode(op, left, right);
		}

		return left;
	}

	ASTNode* parseFactor() {
		ASTNode* left = parseUnary();

		while (currentToken.type == TokenType::MULTIPLY || currentToken.type == TokenType::DIVIDE) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* right = parseUnary();
			left = new BinaryOpNode(op, left, right);
		}

		return left;
	}

	ASTNode* parseUnary() {
		if (currentToken.type == TokenType::MINUS || currentToken.type == TokenType::NOT) {
			TokenType op = currentToken.type;
			eat(op);
			ASTNode* operand = parseUnary();
			return new UnaryOpNode(op, operand);
		}

		return parsePrimary();
	}

	ASTNode* parsePrimary() {
		if (currentToken.type == TokenType::NUMBER) {
			double value = currentToken.numberValue;
			eat(TokenType::NUMBER);
			return new NumberNode(value);
		}
		else if (currentToken.type == TokenType::IDENTIFIER) {
			std::string name = currentToken.stringValue;
			eat(TokenType::IDENTIFIER);

			if (currentToken.type == TokenType::LPAREN) {
				return parseFunctionCall(name);
			}

			return new VariableNode(name);
		}
		else if (currentToken.type == TokenType::LPAREN) {
			eat(TokenType::LPAREN);
			ASTNode* expr = parseExpression();
			eat(TokenType::RPAREN);
			return expr;
		}

		throw std::runtime_error("Unexpected token in primary");
	}

	ValueType parseType() {
		if (currentToken.type == TokenType::INT) {
			eat(TokenType::INT);
			return ValueType::INT;
		}
		else if (currentToken.type == TokenType::FLOAT) {
			eat(TokenType::FLOAT);
			return ValueType::FLOAT;
		}
		else if (currentToken.type == TokenType::BOOL) {
			eat(TokenType::BOOL);
			return ValueType::BOOL;
		}
		else if (currentToken.type == TokenType::STRING_TYPE) {
			eat(TokenType::STRING_TYPE);
			return ValueType::STRING;
		}

		throw std::runtime_error("Expected type declaration");
	}
};

