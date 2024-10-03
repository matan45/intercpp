#include "AST.hpp"
#include "Environment.hpp"

VariableValue ProgramNode::evaluate(Environment& env) const {
	for (ASTNode const* statement : statements) {
		statement->evaluate(env);
	}
	return {}; // Program as a whole doesn’t return a specific value
}

ProgramNode::~ProgramNode() {
	for (ASTNode* statement : statements) {
		delete statement;
	}
}

VariableValue ForNode::evaluate(Environment& env) const {
	// Evaluate the initializer
	if (initializer) {
		initializer->evaluate(env);
	}

	while (true) {
		// Evaluate condition
		VariableValue condValue = condition->evaluate(env);
		if (auto boolPtr = std::get_if<bool>(&condValue)) {
			if (!*boolPtr) {
				break;  // Exit the loop if condition is false
			}
		}
		else {
			throw std::runtime_error("Condition for 'for' must be a boolean.");
		}

		// Execute body
		body->evaluate(env);

		// Execute update
		if (update) {
			update->evaluate(env);
		}
	}

	return {};  // No meaningful value to return for a for loop
}

ForNode::~ForNode() {
	delete initializer;
	delete condition;
	delete update;
	delete body;
}

VariableValue BooleanNode::evaluate(Environment& env) const {
	// Return 1.0 for true, 0.0 for false (to allow compatibility in numeric contexts)
	return value;
}

VariableValue StringNode::evaluate(Environment& env) const {
	// Return the string value contained in this node.
	return value;
}

VariableValue DoWhileNode::evaluate(Environment& env) const {
	do {
		body->evaluate(env);
		VariableValue condValue = condition->evaluate(env);

		// Ensure the condition is boolean
		if (auto boolPtr = std::get_if<bool>(&condValue)) {
			if (!*boolPtr) {
				break;  // Exit the loop if condition is false
			}
		}
		else {
			throw std::runtime_error("Condition for 'do-while' must be a boolean.");
		}

	} while (true);

	return {};  // No meaningful value to return for a do-while loop
}

DoWhileNode::~DoWhileNode() {
	delete body;
	delete condition;
}

VariableValue IncrementNode::evaluate(Environment& env) const {
	VariableValue value = env.getVariable(variableName);
	if (!std::holds_alternative<double>(value)) {
		throw std::runtime_error("Increment operation only supports numeric types.");
	}

	double& varValue = std::get<double>(value);
	double originalValue = varValue;

	// Update the variable in the environment
	if (incrementType == IncrementType::PREFIX) {
		++varValue;  // Increment first, then return
		env.setVariable(variableName, varValue);
		return varValue;
	}
	else {
		varValue++;  // Return original, then increment
		env.setVariable(variableName, varValue);
		return originalValue;
	}
}

VariableValue BlockNode::evaluate(Environment& env) const {
	VariableValue lastValue;
	for (const auto& statement : statements) {
		lastValue = statement->evaluate(env);
	}
	return lastValue;
}

BlockNode::~BlockNode() {
	for (ASTNode* statement : statements) {
		delete statement;
	}
}

VariableValue UnaryOpNode::evaluate(Environment& env) const {
	VariableValue operandValue = operand->evaluate(env);

	switch (op) {
	case TokenType::MINUS: {
		if (auto doublePtr = std::get_if<double>(&operandValue)) {
			return -(*doublePtr);
		}
		throw std::runtime_error("Unary minus can only be applied to numbers.");
	}
	case TokenType::NOT: {
		if (auto boolPtr = std::get_if<bool>(&operandValue)) {
			return !(*boolPtr);
		}
		throw std::runtime_error("Logical NOT can only be applied to booleans.");
	}
	default:
		throw std::runtime_error("Unsupported unary operation.");
	}
}

UnaryOpNode::~UnaryOpNode() {
	delete operand;
}

VariableValue BinaryOpNode::evaluate(Environment& env) const {
	VariableValue leftValue = left->evaluate(env);
	VariableValue rightValue = right->evaluate(env);

	// Handle numeric operations (+, -, *, /)
	if (auto leftDouble = std::get_if<double>(&leftValue)) {
		if (auto rightDouble = std::get_if<double>(&rightValue)) {
			switch (op) {
			case TokenType::PLUS:
				return *leftDouble + *rightDouble;
			case TokenType::MINUS:
				return *leftDouble - *rightDouble;
			case TokenType::MULTIPLY:
				return *leftDouble * *rightDouble;
			case TokenType::DIVIDE:
				if (*rightDouble == 0) {
					throw std::runtime_error("Division by zero.");
				}
				return *leftDouble / *rightDouble;
			case TokenType::LESS:
				return *leftDouble < *rightDouble;
			case TokenType::LESS_EQUALS:
				return *leftDouble <= *rightDouble;
			case TokenType::GREATER:
				return *leftDouble > *rightDouble;
			case TokenType::GREATER_EQUALS:
				return *leftDouble >= *rightDouble;
			default:
				throw std::runtime_error("Unsupported binary operation for doubles.");
			}
		}
	}

	// Handle string concatenation (+)
	if (auto leftStr = std::get_if<std::string>(&leftValue)) {
		if (auto rightStr = std::get_if<std::string>(&rightValue)) {
			if (op == TokenType::PLUS) {
				return *leftStr + *rightStr;
			}
			throw std::runtime_error("Unsupported operation for strings.");
		}
	}

	// Handle logical operations (&&, ||)
	if (op == TokenType::AND || op == TokenType::OR) {
		if (auto leftBool = std::get_if<bool>(&leftValue)) {
			if (auto rightBool = std::get_if<bool>(&rightValue)) {
				if (op == TokenType::AND) {
					return *leftBool && *rightBool;
				}
				else if (op == TokenType::OR) {
					return *leftBool || *rightBool;
				}
			}
		}
		else {
			throw std::runtime_error("Logical operations require boolean values.");
		}
	}

	// Handle equality and inequality operations (==, !=)
	if (op == TokenType::EQUALS || op == TokenType::NOT_EQUALS) {
		if (leftValue.index() == rightValue.index()) {
			bool result = false;

			if (auto leftDouble = std::get_if<double>(&leftValue)) {
				auto rightDouble = std::get_if<double>(&rightValue);
				result = (*leftDouble == *rightDouble);
			}
			else if (auto leftBool = std::get_if<bool>(&leftValue)) {
				auto rightBool = std::get_if<bool>(&rightValue);
				result = (*leftBool == *rightBool);
			}
			else if (auto leftStr = std::get_if<std::string>(&leftValue)) {
				auto rightStr = std::get_if<std::string>(&rightValue);
				result = (*leftStr == *rightStr);
			}

			if (op == TokenType::EQUALS) {
				return result;
			}
			else if (op == TokenType::NOT_EQUALS) {
				return !result;
			}
		}
		else {
			throw std::runtime_error("Cannot compare different types for equality.");
		}
	}

	throw std::runtime_error("Unsupported types for binary operation.");
}


BinaryOpNode::~BinaryOpNode() {
	delete left;
	delete right;
}

VariableValue WhileNode::evaluate(Environment& env) const {
	while (true) {
		VariableValue condValue = condition->evaluate(env);

		// Ensure the condition is boolean
		if (auto boolPtr = std::get_if<bool>(&condValue)) {
			if (!*boolPtr) {
				break;  // Exit the loop if condition is false
			}
		}
		else {
			throw std::runtime_error("Condition for 'while' must be a boolean.");
		}

		body->evaluate(env);
	}

	return {};  // No meaningful value to return for a while loop
}

WhileNode::~WhileNode() {
	delete condition;
	delete body;
}


VariableValue IfNode::evaluate(Environment& env) const {
	VariableValue condValue = condition->evaluate(env);

	// Ensure the condition is boolean
	if (auto boolPtr = std::get_if<bool>(&condValue)) {
		if (*boolPtr) {
			return thenBranch->evaluate(env);
		}
		else if (elseBranch != nullptr) {
			return elseBranch->evaluate(env);
		}
	}
	else {
		throw std::runtime_error("Condition for 'if' must be a boolean.");
	}

	return {};  // Return an empty value if no branch is executed
}

IfNode::~IfNode() {
	delete condition;
	delete thenBranch;
	delete elseBranch;
}

VariableValue ReturnNode::evaluate(Environment& env) const {
	VariableValue result = returnValue->evaluate(env);

	// Determine the type of result and print it accordingly
	std::cout << "ReturnNode: Returning value ";
	if (auto doublePtr = std::get_if<double>(&result)) {
		std::cout << *doublePtr << std::endl;
	}
	else if (auto boolPtr = std::get_if<bool>(&result)) {
		std::cout << (*boolPtr ? "true" : "false") << std::endl;
	}
	else if (auto strPtr = std::get_if<std::string>(&result)) {
		std::cout << *strPtr << std::endl;
	}
	else {
		std::cout << "unknown type" << std::endl;
	}

	return result;
}

ReturnNode::~ReturnNode() {
	delete returnValue;
}

VariableValue FunctionCallNode::evaluate(Environment& env) const {
	// Evaluate all arguments and collect them
	std::vector<VariableValue> evaluatedArgs;
	for (ASTNode const* arg : arguments) {
		evaluatedArgs.push_back(arg->evaluate(env));
	}

	// Call the function with the arguments and return the result
	VariableValue result = env.evaluateFunction(name, evaluatedArgs);

	return result;
}


FunctionCallNode::~FunctionCallNode() {
	for (ASTNode* arg : arguments) {
		delete arg;
	}
}

VariableValue FunctionNode::evaluate(Environment& env) const {
	// Execute the function body

	return VariableValue(); // Default return if nothing returned
}

FunctionNode::~FunctionNode() {
	delete body;
}

VariableValue DeclarationNode::evaluate(Environment& env) const {
	// Declare the variable in the current scope with a default value
	ValueType defaultType;

	// Set the default value depending on the type
	switch (type) {
	case ValueType::BOOL:
		defaultType = ValueType::BOOL;  // Boolean variables default to false
		break;
	case ValueType::INT:
		defaultType = ValueType::INT;  // Int variables default to 0 (use double to store numeric)
		break;
	case ValueType::FLOAT:
		defaultType = ValueType::FLOAT;  // Float variables also default to 0
		break;
	case ValueType::STRING:
		defaultType = ValueType::STRING;  // Strings default to empty
		break;
	default:
		throw std::runtime_error("Unknown type for variable declaration: " + variableName);
	}

	// Declare the variable in the environment
	env.declareVariable(variableName, defaultType);

	// If there is an initializer, evaluate it and set the value in the environment
	if (initializer) {
		VariableValue value = initializer->evaluate(env);
		env.setVariable(variableName, value);
	}

	return env.getVariable(variableName);  // Return the value of the declared variable
}

DeclarationNode::~DeclarationNode() {
	delete initializer;
}

VariableValue AssignmentNode::evaluate(Environment& env) const {
	VariableValue value = expression->evaluate(env);

	// Determine the type of value and print it accordingly
	std::cout << "AssignmentNode: Assigning value ";
	if (auto doublePtr = std::get_if<double>(&value)) {
		std::cout << *doublePtr;
	}
	else if (auto boolPtr = std::get_if<bool>(&value)) {
		std::cout << (*boolPtr ? "true" : "false");
	}
	else if (auto strPtr = std::get_if<std::string>(&value)) {
		std::cout << "\"" << *strPtr << "\"";
	}
	else {
		std::cout << "unknown type";
	}
	std::cout << " to variable " << variableName << std::endl;

	// Set the variable in the environment
	env.setVariable(variableName, value);
	return value;  // Return the assigned value
}

AssignmentNode::~AssignmentNode() {
	delete expression;
}

VariableValue VariableNode::evaluate(Environment& env) const {
	auto value = env.getVariable(name);
	if (std::holds_alternative<double>(value)) {
		double numValue = std::get<double>(value);
		std::cout << "VariableNode: Retrieved numeric value " << numValue << " for variable " << name << std::endl;
		return numValue;
	}
	else if (std::holds_alternative<bool>(value)) {
		bool boolValue = std::get<bool>(value);
		std::cout << "VariableNode: Retrieved boolean value " << (boolValue ? "true" : "false") << " for variable " << name << std::endl;
		return boolValue;
	}
	// Handle string type: return a default value or throw an error
	else if (std::holds_alternative<std::string>(value)) {
		std::string strValue = std::get<std::string>(value);
		std::cout << "VariableNode: Retrieved string value \"" << strValue << "\" for variable " << name << std::endl;
		// Option 1: Return a default value such as 0.0
		return strValue;
	}
	else {
		throw std::runtime_error("Expected numeric value for variable: " + name);
	}
}
