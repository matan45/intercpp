#include "Lexer.hpp"

Token Lexer::getNextToken() {
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

		// Handle string literals
		if (current == '"') {
			return parseStringLiteral();
		}

		// Tokenize identifiers and keywords
		if (isalpha(current)) {
			std::string word = parseIdentifier();
			if (word == "true") {
				return Token(TokenType::TRUE);
			}
			if (word == "false") {
				return Token(TokenType::FALSE);
			}
			else if (word == "func") {
				return Token(TokenType::FUNC);
			}
			if (word == "int") {
				return Token(TokenType::INT);
			}
			if (word == "void") {
				return Token(TokenType::VOID_TYPE);
			}
			else if (word == "float") {
				return Token(TokenType::FLOAT);
			}
			else if (word == "bool") {
				return Token(TokenType::BOOL);
			}
			else if (word == "string") {
				return Token(TokenType::STRING_TYPE);
			}
			else if (word == "if") {
				return Token(TokenType::IF);
			}
			else if (word == "while") {
				return Token(TokenType::WHILE);
			}
			else if (word == "for") {
				return Token(TokenType::FOR);
			}
			else if (word == "do") {
				return Token(TokenType::DO);
			}
			else if (word == "return") {
				return Token(TokenType::RETURN);
			}

			return Token(TokenType::IDENTIFIER, word);
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

		if (current == '+') {
			if (pos + 1 < input.length() && input[pos + 1] == '+') {
				pos += 2;
				return Token(TokenType::PLUSPLUS);
			}
			++pos;
			return Token(TokenType::PLUS);
		}

		if (current == '-') {
			if (pos + 1 < input.length() && input[pos + 1] == '-') {
				pos += 2;
				return Token(TokenType::MINUSMINUS);
			}
			++pos;
			return Token(TokenType::MINUS);
		}

		// Handling single-character tokens and maintaining stack balance
		switch (current) {
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

Token Lexer::parseStringLiteral()
{
	++pos; // Skip the opening quote
	size_t startPos = pos;
	while (pos < input.length() && input[pos] != '"') {
		++pos;
	}
	if (pos >= input.length()) {
		throw std::runtime_error("Unterminated string literal");
	}
	std::string value = input.substr(startPos, pos - startPos);
	++pos; // Skip the closing quote
	return Token(TokenType::STRING_LITERAL, value);
}

double Lexer::parseNumber() {
	size_t startPos = pos;
	while (pos < input.length() && (isdigit(input[pos]) || input[pos] == '.')) {
		++pos;
	}
	return std::stod(input.substr(startPos, pos - startPos));
}

std::string Lexer::parseIdentifier() {
	size_t startPos = pos;
	while (pos < input.length() && (isalnum(input[pos]) || input[pos] == '_')) {
		++pos;
	}
	return input.substr(startPos, pos - startPos);
}