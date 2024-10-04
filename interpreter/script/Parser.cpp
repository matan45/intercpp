#include "Parser.hpp"
#include "Types.hpp"
#include "Environment.hpp"

void Parser::eat(TokenType type) {
	if (currentToken.type == type) {
		previousTokens.emplace_back(currentToken);
		position++;
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
		ASTNode* node = parseStatement();
		if (node) {
			statements.push_back(node);
		}
		
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
		currentToken.type == TokenType::STRING_TYPE ||
		currentToken.type == TokenType::ARRAY ||
		currentToken.type == TokenType::MAP) {
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
		// Handle array or map index assignment (e.g., `arr[2] = 10` or `map["key"] = 20`)
		else if (currentToken.type == TokenType::LBRACKET) {
			// Parse array or map indexing (like `arr[2]`)
			eat(TokenType::LBRACKET);
			ASTNode* indexExpr = parseExpression();  // Index or key
			eat(TokenType::RBRACKET);

			// Expect an assignment to this indexed value (e.g., `arr[2] = 10`)
			if (currentToken.type == TokenType::ASSIGN) {
				eat(TokenType::ASSIGN);
				ASTNode* valueExpr = parseExpression();
				eat(TokenType::SEMICOLON);
				return new AssignmentNode(identifier, indexExpr, valueExpr);  // AssignmentNode for array/map element assignment
			}
		}
		else if (currentToken.type == TokenType::LPAREN) {
			// Function call statement
			return parseFunctionCall(identifier);
		}
		else if (currentToken.type == TokenType::PLUSPLUS || currentToken.type == TokenType::MINUSMINUS) {
			// Postfix increment or decrement statement
			TokenType op = currentToken.type;
			eat(op);
			eat(TokenType::SEMICOLON);
			return new IncrementNode(op == TokenType::PLUSPLUS ? IncrementType::POSTFIX : IncrementType::POSTFIX, identifier);
		}
		else {
			// If neither assignment nor function call, it might be an error
			throw std::runtime_error("Unexpected token after identifier: " + currentToken.stringValue);
		}
	}
	else if (currentToken.type == TokenType::PLUSPLUS || currentToken.type == TokenType::MINUSMINUS) {
		// Prefix increment or decrement statement
		TokenType op = currentToken.type;
		eat(op);
		std::string identifier = currentToken.stringValue;
		eat(TokenType::IDENTIFIER);
		eat(TokenType::SEMICOLON);
		return new IncrementNode(op == TokenType::PLUSPLUS ? IncrementType::PREFIX : IncrementType::PREFIX, identifier);
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
	else if (currentToken.type == TokenType::IMPORT)
	{
		eat(TokenType::IMPORT);
		return nullptr;
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

	ASTNode* update = nullptr;
	// Parse the update expression
	if (currentToken.type != TokenType::RPAREN) {
		// Here we handle various forms of update, including increments and assignments.
		if (currentToken.type == TokenType::PLUSPLUS || currentToken.type == TokenType::MINUSMINUS) {
			// Handle prefix increment or decrement (e.g., ++i or --i)
			TokenType op = currentToken.type;
			eat(op);
			std::string identifier = currentToken.stringValue;
			eat(TokenType::IDENTIFIER);
			update = new IncrementNode(op == TokenType::PLUSPLUS ? IncrementType::PREFIX : IncrementType::PREFIX, identifier);
		}
		else if (currentToken.type == TokenType::IDENTIFIER) {
			std::string identifier = currentToken.stringValue;
			eat(TokenType::IDENTIFIER);
			if (currentToken.type == TokenType::PLUSPLUS || currentToken.type == TokenType::MINUSMINUS) {
				// Handle postfix increment or decrement (e.g., i++ or i--)
				TokenType op = currentToken.type;
				eat(op);
				update = new IncrementNode(op == TokenType::PLUSPLUS ? IncrementType::POSTFIX : IncrementType::POSTFIX, identifier);
			}
			else {
				// Handle assignments (e.g., i = i + 1)
				eat(TokenType::ASSIGN);
				ASTNode* valueExpr = parseExpression();
				update = new AssignmentNode(identifier, valueExpr);
			}
		}
		else {
			// Parse a more complex update expression
			update = parseExpression();
		}
	}
	eat(TokenType::RPAREN);

	eat(TokenType::LBRACE);
	// Parse the block of statements that make up the function body
	std::vector<ASTNode*> bodyStatements;//also do to this in the if node and the loops
	while (currentToken.type != TokenType::RBRACE) {
		bodyStatements.push_back(parseStatement());
	}
	eat(TokenType::RBRACE);

	// Create a BlockNode to represent the entire function body
	ASTNode* body = new BlockNode(bodyStatements);

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
	// Parse the block of statements that make up the function body
	std::vector<ASTNode*> bodyStatements;//also do to this in the if node and the loops
	while (currentToken.type != TokenType::RBRACE) {
		bodyStatements.push_back(parseStatement());
	}
	eat(TokenType::RBRACE);

	// Create a BlockNode to represent the entire function body
	ASTNode* body = new BlockNode(bodyStatements);

	FunctionNode* functionNode = new FunctionNode(functionName, returnType, parameters, body);
	env.registerUserFunction(functionName, functionNode);
	return functionNode;
}

ASTNode* Parser::parseWhileStatement() {
	eat(TokenType::WHILE);
	eat(TokenType::LPAREN);
	ASTNode* condition = parseExpression();
	eat(TokenType::RPAREN);
	eat(TokenType::LBRACE);
	// Parse the block of statements that make up the function body
	std::vector<ASTNode*> bodyStatements;//also do to this in the if node and the loops
	while (currentToken.type != TokenType::RBRACE) {
		bodyStatements.push_back(parseStatement());
	}
	eat(TokenType::RBRACE);

	// Create a BlockNode to represent the entire function body
	ASTNode* body = new BlockNode(bodyStatements);
	return new WhileNode(condition, body);
}

ASTNode* Parser::parseDeclaration() {
	ValueType type = parseType();
	std::string variableName = currentToken.stringValue;
	eat(TokenType::IDENTIFIER);

	ASTNode* initializer = nullptr;
	// Handle the initialization part if present
	if (currentToken.type == TokenType::ASSIGN) {
		eat(TokenType::ASSIGN);

		if (type == ValueType::ARRAY) {
			// Expecting an array literal to initialize the array variable
			initializer = parseArrayLiteral();
		}
		else if (type == ValueType::MAP) {
			// Expecting a map literal to initialize the map variable
			initializer = parseMapLiteral();
		}
		else {
			// For scalar variables, parse the normal initializer expression
			initializer = parseExpression();
		}
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

	// Parse `else if` and `else` branches if present
	while (currentToken.type == TokenType::ELSE) {
		eat(TokenType::ELSE);

		// Check if it's an `else if`
		if (currentToken.type == TokenType::IF) {
			eat(TokenType::IF);
			eat(TokenType::LPAREN);
			ASTNode* elseIfCondition = parseExpression();  // Parse the `else if` condition
			eat(TokenType::RPAREN);
			ASTNode* elseIfBranch = parseStatement();  // Parse the `else if` block

			// Create a new IfNode for the `else if` and chain it
			elseBranch = new IfNode(elseIfCondition, elseIfBranch, elseBranch);
		}
		else {
			// It must be a simple `else` block
			elseBranch = parseStatement();
			break;  // No more `else if` after an `else`
		}
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
		eat(TokenType::NUMBER);
		std::cout << "parsePrimary: Creating NumberNode with value: " << value << std::endl;
		return new NumberNode(value);
	}
	case TokenType::STRING_LITERAL: {
		std::string value = currentToken.stringValue;
		eat(TokenType::STRING_LITERAL);

		std::cout << "parsePrimary: Creating StringNode with value: " << value << std::endl;
		return new StringNode(value);
	}
	case TokenType::IDENTIFIER: {
		std::string name = currentToken.stringValue;
		eat(TokenType::IDENTIFIER);

		if (currentToken.type == TokenType::LBRACKET) {
			// Handle map or array indexing
			eat(TokenType::LBRACKET);
			std::cout << "parsePrimary: Creating IndexNode with value: " << name << std::endl;
			ASTNode* index = parseExpression();
			eat(TokenType::RBRACKET);
			eat(TokenType::SEMICOLON);
			return new IndexNode(name, index);  // Create IndexNode for indexing into arrays or maps

		}

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
	case TokenType::LBRACE: {
		return parseMapLiteral();
	}
	case TokenType::LBRACKET: {
		return parseArrayLiteral();
	}
	default:
		throw std::runtime_error("Unexpected token in primary");
	}
}

ASTNode* Parser::parseMapLiteral()
{
	eat(TokenType::LBRACE);
	std::unordered_map<std::string, ASTNode*> elements;

	// Parse key-value pairs
	while (currentToken.type != TokenType::RBRACE) {
		ASTNode* keyNode = parseExpression();

		if (auto keyStr = dynamic_cast<StringNode*>(keyNode)) {
			std::string key = keyStr->value;
			eat(TokenType::COLON);
			ASTNode* valueNode = parseExpression();
			elements[key] = valueNode;

			if (currentToken.type == TokenType::COMMA) {
				eat(TokenType::COMMA);
			}
			else {
				break;
			}
		}
		else {
			throw std::runtime_error("Map keys must be strings.");
		}
	}

	eat(TokenType::RBRACE);
	return new MapNode(elements);
}

ASTNode* Parser::parseArrayLiteral()
{
	eat(TokenType::LBRACKET);
	std::vector<ASTNode*> elements;

	// Parse elements separated by commas
	while (currentToken.type != TokenType::RBRACKET) {
		elements.push_back(parseExpression());

		if (currentToken.type == TokenType::COMMA) {
			eat(TokenType::COMMA);
		}
		else {
			break;
		}
	}

	eat(TokenType::RBRACKET);
	return new ArrayNode(elements);
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
	else if (currentToken.type == TokenType::VOID_TYPE) {
		eat(TokenType::VOID_TYPE);
		return ValueType::VOID_TYPE;
	}
	else if (currentToken.type == TokenType::ARRAY) {
		eat(TokenType::ARRAY);
		return ValueType::ARRAY;
	}
	else if (currentToken.type == TokenType::MAP) {
		eat(TokenType::MAP);
		return ValueType::MAP;
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
	case TokenType::RBRACKET: return "RBRACE (])";
	case TokenType::LBRACKET: return "RBRACE ([)";
	case TokenType::COLON: return "RBRACE (:)";
	case TokenType::MAP: return "MAP";
	case TokenType::ARRAY: return "ARRAY";
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
	case TokenType::LESS_EQUALS: return "LESS EQUALS (<=)";
	case TokenType::IMPORT: return "#IMPORT";
	default: return "UNKNOWN TOKEN";
	}
}

