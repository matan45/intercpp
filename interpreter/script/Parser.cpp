#include "Parser.hpp"
#include "Types.hpp"
#include "Environment.hpp"

void Parser::eat(TokenType type) {
	if (currentToken.type == type) {
		currentToken = lexer.getNextToken();
	}
	else {
		throw std::runtime_error("Unexpected token type");
	}
}

ASTNode* Parser::parseProgram() {
	std::vector<ASTNode*> statements;
	while (currentToken.type != TokenType::END) {
		std::cout << tokenToString(currentToken) << std::endl;
		statements.push_back(parseStatement());
	}
	return new ProgramNode(statements);
}

ASTNode* Parser::parseStatement() {
	if (currentToken.type == TokenType::FUNC) {
		// Function definition
		return parseFunctionDefinition();
	}
	else if (currentToken.type == TokenType::INT ||
		currentToken.type == TokenType::FLOAT ||
		currentToken.type == TokenType::BOOL ||
		currentToken.type == TokenType::STRING_TYPE) {
		// Variable declaration
		return parseDeclaration();
	}
	else if (currentToken.type == TokenType::IDENTIFIER) {
		// Could be an assignment or a function call
		std::string identifier = currentToken.stringValue;
		eat(TokenType::IDENTIFIER);
		if (currentToken.type == TokenType::ASSIGN) {
			// Assignment statement
			return parseAssignment(identifier);
		}
		else if (currentToken.type == TokenType::LPAREN) {
			// Function call statement
			return parseFunctionCall(identifier);
		}
		else {
			// If neither assignment nor function call, it might be an error
			throw std::runtime_error("Unexpected token after identifier: " + currentToken.stringValue);
		}
	}
	else if (currentToken.type == TokenType::RETURN) {
		// Return statement
		eat(TokenType::RETURN);
		ASTNode* expr = parseExpression();
		eat(TokenType::SEMICOLON);
		return new ReturnNode(expr);
	}
	else if (currentToken.type == TokenType::IF) {
		// If statement
		return parseIfStatement();
	}
	else if (currentToken.type == TokenType::WHILE) {
		// While loop
		return parseWhileStatement();
	}
	else if (currentToken.type == TokenType::FOR) {
		// For loop
		return parseForStatement();
	}
	else if (currentToken.type == TokenType::DO) {
		// Do-while loop
		return parseDoWhileStatement();
	}
	else if (currentToken.type == TokenType::LBRACE) {
		// Block of statements (e.g., within if, while, for, or function)
		eat(TokenType::LBRACE);
		std::vector<ASTNode*> bodyStatements;
		while (currentToken.type != TokenType::RBRACE) {
			bodyStatements.push_back(parseStatement());
		}
		eat(TokenType::RBRACE);
		return new BlockNode(bodyStatements);
	}
	else if (currentToken.type == TokenType::SEMICOLON) {
		// Empty statement (e.g., after a loop or condition)
		eat(TokenType::SEMICOLON);
		return nullptr; // Empty statement, no action required
	}

	// Throw an error for an unrecognized token
	throw std::runtime_error("Unexpected token in statement: " + tokenToString(currentToken));
}


ASTNode* Parser::parseForStatement() {
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

ASTNode* Parser::parseDoWhileStatement() {
	eat(TokenType::DO);
	ASTNode* body = parseStatement(); // Parse the loop body

	eat(TokenType::WHILE);
	eat(TokenType::LPAREN);
	ASTNode* condition = parseExpression();
	eat(TokenType::RPAREN);
	eat(TokenType::SEMICOLON);

	return new DoWhileNode(body, condition);
}

ASTNode* Parser::parseFunctionDefinition() {
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

ASTNode* Parser::parseWhileStatement() {
	eat(TokenType::WHILE);
	eat(TokenType::LPAREN);
	ASTNode* condition = parseExpression();
	eat(TokenType::RPAREN);
	ASTNode* body = parseStatement();
	return new WhileNode(condition, body);
}

ASTNode* Parser::parseDeclaration() {
	ValueType type = parseType();
	std::string variableName = currentToken.stringValue;
	eat(TokenType::IDENTIFIER);

	ASTNode* initializer = nullptr;
	if (currentToken.type == TokenType::ASSIGN) {
		eat(TokenType::ASSIGN);
		initializer = parseExpression();
	}
	if (currentToken.type == TokenType::SEMICOLON) {
		eat(TokenType::SEMICOLON);
	}
	return new DeclarationNode(variableName, type, initializer);
}

ASTNode* Parser::parseAssignment(const std::string& identifier) {
	eat(TokenType::ASSIGN);
	ASTNode* valueExpr = parseExpression();
	eat(TokenType::SEMICOLON);
	return new AssignmentNode(identifier, valueExpr);
}

ASTNode* Parser::parseFunctionCall(const std::string& functionName) {
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

ASTNode* Parser::parseIfStatement() {
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

ASTNode* Parser::parseLogicalOr() {
	ASTNode* left = parseLogicalAnd();

	while (currentToken.type == TokenType::OR) {
		TokenType op = currentToken.type;
		eat(op);
		ASTNode* right = parseLogicalAnd();
		left = new BinaryOpNode(op, left, right);
	}

	return left;
}

ASTNode* Parser::parseLogicalAnd() {
	ASTNode* left = parseEquality();

	while (currentToken.type == TokenType::AND) {
		TokenType op = currentToken.type;
		eat(op);
		ASTNode* right = parseEquality();
		left = new BinaryOpNode(op, left, right);
	}

	return left;
}

ASTNode* Parser::parseEquality() {
	ASTNode* left = parseComparison();

	while (currentToken.type == TokenType::EQUALS || currentToken.type == TokenType::NOT_EQUALS) {
		TokenType op = currentToken.type;
		eat(op);
		ASTNode* right = parseComparison();
		left = new BinaryOpNode(op, left, right);
	}

	return left;
}

ASTNode* Parser::parseComparison() {
	ASTNode* left = parseTerm();

	while (currentToken.type == TokenType::LESS || currentToken.type == TokenType::LESS_EQUALS ||
		currentToken.type == TokenType::GREATER || currentToken.type == TokenType::GREATER_EQUALS) {
		TokenType op = currentToken.type;

		eat(op);
		ASTNode* right = parseTerm();

		std::cout << "parseComparison: Creating BinaryOpNode for comparison: " << tokenToString(op) << std::endl;

		left = new BinaryOpNode(op, left, right);
	}

	return left;
}

ASTNode* Parser::parseTerm() {
	ASTNode* left = parseFactor();

	while (currentToken.type == TokenType::PLUS || currentToken.type == TokenType::MINUS) {
		TokenType op = currentToken.type;
		eat(op);
		ASTNode* right = parseFactor();

		// Add debug print to indicate a BinaryOpNode is being created for arithmetic operations
		if (op == TokenType::PLUS) {
			std::cout << "parseTerm: Creating BinaryOpNode for addition (+)" << std::endl;
		}
		else if (op == TokenType::MINUS) {
			std::cout << "parseTerm: Creating BinaryOpNode for subtraction (-)" << std::endl;
		}

		left = new BinaryOpNode(op, left, right);
	}

	return left;
}

ASTNode* Parser::parseFactor() {
	ASTNode* left = parseUnary();

	while (currentToken.type == TokenType::MULTIPLY || currentToken.type == TokenType::DIVIDE) {
		TokenType op = currentToken.type;
		eat(op);
		ASTNode* right = parseUnary();

		// Add debug print to indicate a BinaryOpNode is being created for multiplication or division
		if (op == TokenType::MULTIPLY) {
			std::cout << "parseFactor: Creating BinaryOpNode for multiplication (*)" << std::endl;
		}
		else if (op == TokenType::DIVIDE) {
			std::cout << "parseFactor: Creating BinaryOpNode for division (/)" << std::endl;
		}

		left = new BinaryOpNode(op, left, right);
	}

	return left;
}

ASTNode* Parser::parseUnary() {
	if (currentToken.type == TokenType::MINUS || currentToken.type == TokenType::NOT) {
		TokenType op = currentToken.type;
		eat(op);
		ASTNode* operand = parseUnary();
		return new UnaryOpNode(op, operand);
	}

	return parsePrimary();
}


ASTNode* Parser::parsePrimary() {
	switch (currentToken.type) {
	case TokenType::NUMBER: {
		double value = currentToken.numberValue;
		std::cout << "parsePrimary: Creating NumberNode with value: " << value << std::endl;
		eat(TokenType::NUMBER);
		return new NumberNode(value);
	}
	case TokenType::STRING_LITERAL: {
		std::string value = currentToken.stringValue;
		std::cout << "parsePrimary: Creating StringNode with value: " << value << std::endl;
		eat(TokenType::STRING_LITERAL);
		return new StringNode(value);
	}
	case TokenType::IDENTIFIER: {
		std::string name = currentToken.stringValue;
		eat(TokenType::IDENTIFIER);

		if (currentToken.type == TokenType::LPAREN) {
			std::cout << "parsePrimary: Parsing function call with name: " << name << std::endl;
			return parseFunctionCall(name);
		}
		else {
			std::cout << "parsePrimary: Creating VariableNode with name: " << name << std::endl;
			return new VariableNode(name);
		}
	}
	case TokenType::LPAREN: {
		eat(TokenType::LPAREN);
		std::cout << "parsePrimary: Parsing grouped expression" << std::endl;
		ASTNode* expr = parseExpression();
		eat(TokenType::RPAREN);
		return expr;
	}
	case TokenType::TRUE: {
		eat(TokenType::TRUE);
		std::cout << "parsePrimary: Creating BooleanNode with value: true" << std::endl;
		return new BooleanNode(true);
	}
	case TokenType::FALSE: {
		eat(TokenType::FALSE);
		std::cout << "parsePrimary: Creating BooleanNode with value: false" << std::endl;
		return new BooleanNode(false);
	}
	default:
		throw std::runtime_error("Unexpected token in primary");
	}
}


ValueType Parser::parseType() {
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

std::string Parser::tokenToString(const Token& token) {
	switch (token.type) {
	case TokenType::INT: return "INT";
	case TokenType::FLOAT: return "FLOAT";
	case TokenType::BOOL: return "BOOL";
	case TokenType::STRING_TYPE: return "STRING_TYPE";
	case TokenType::IDENTIFIER: return "IDENTIFIER (" + token.stringValue + ")";
	case TokenType::NUMBER: return "NUMBER (" + std::to_string(token.numberValue) + ")";
	case TokenType::STRING_LITERAL: return "STRING_LITERAL (" + token.stringValue + ")";
	case TokenType::FUNC: return "FUNC";
	case TokenType::RETURN: return "RETURN";
	case TokenType::IF: return "IF";
	case TokenType::ELSE: return "ELSE";
	case TokenType::WHILE: return "WHILE";
	case TokenType::FOR: return "FOR";
	case TokenType::DO: return "DO";
	case TokenType::ASSIGN: return "ASSIGN (=)";
	case TokenType::LPAREN: return "LPAREN (()";
	case TokenType::RPAREN: return "RPAREN ())";
	case TokenType::LBRACE: return "LBRACE ({)";
	case TokenType::RBRACE: return "RBRACE (})";
	case TokenType::SEMICOLON: return "SEMICOLON (;)";
	case TokenType::PLUS: return "PLUS (+)";
	case TokenType::MINUS: return "MINUS (-)";
	case TokenType::MULTIPLY: return "MULTIPLY (*)";
	case TokenType::DIVIDE: return "DIVIDE (/)";
	case TokenType::AND: return "AND (&&)";
	case TokenType::OR: return "OR (||)";
	case TokenType::END: return "END (EOF)";
	case TokenType::GREATER: return "GREATER (>)";
	case TokenType::LESS: return "LESS (<)";
	case TokenType::GREATER_EQUALS: return "GREATER EQUALS (>=)";
	case TokenType::LESS_EQUALS: return "LESS EQUALS (>=)";
	default: return "UNKNOWN TOKEN";
	}
}

