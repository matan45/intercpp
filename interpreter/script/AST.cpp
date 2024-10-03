#include "AST.hpp"
#include "Environment.hpp"

double ProgramNode::evaluate(Environment& env) const {
	for (ASTNode const* statement : statements) {
		statement->evaluate(env);
	}
	return 0; // Program as a whole doesn’t return a specific value
}

ProgramNode::~ProgramNode() {
	for (ASTNode* statement : statements) {
		delete statement;
	}
}

double ForNode::evaluate(Environment& env) const {
	// Evaluate the initializer
	if (initializer) {
		initializer->evaluate(env);
	}

	// Loop while the condition is true
	double result = 0;
	while (condition->evaluate(env) != 0) {
		result = body->evaluate(env);
		if (update) {
			update->evaluate(env);
		}
	}
	return result;
}

ForNode::~ForNode() {
	delete initializer;
	delete condition;
	delete update;
	delete body;
}

double BooleanNode::evaluate(Environment& env) const {
	// Return 1.0 for true, 0.0 for false (to allow compatibility in numeric contexts)
	return value ? 1.0 : 0.0;
}

bool BooleanNode::evaluateBool(Environment& env) const {
	return value;
}


double StringNode::evaluate(Environment& env) const  {
	// This method returns 0 as this is primarily for printing purposes.
	return 0;
}

std::string StringNode::evaluateString(Environment& env) const {
	return value;
}

double DoWhileNode::evaluate(Environment& env) const  {
	double result = 0;
	do {
		result = body->evaluate(env);
	} while (condition->evaluate(env) != 0);
	return result;
}

DoWhileNode::~DoWhileNode() {
	delete body;
	delete condition;
}

double BlockNode::evaluate(Environment& env) const  {
	for (ASTNode* statement : statements) {
		statement->evaluate(env);
	}
	return 0; // A block doesn’t return a value directly
}

BlockNode::~BlockNode() {
	for (ASTNode* statement : statements) {
		delete statement;
	}
}

double UnaryOpNode::evaluate(Environment& env) const  {
	double value = operand->evaluate(env);
	switch (op) {
	case TokenType::MINUS: return -value;
	case TokenType::NOT: return (value == 0) ? 1 : 0;
	default: throw std::runtime_error("Unknown unary operator");
	}
}

UnaryOpNode::~UnaryOpNode() {
	delete operand;
}

double BinaryOpNode::evaluate(Environment& env) const  {
	double leftValue = left->evaluate(env);
	double rightValue = right->evaluate(env);

	switch (op) {
	case TokenType::PLUS: return leftValue + rightValue;
	case TokenType::MINUS: return leftValue - rightValue;
	case TokenType::MULTIPLY: return leftValue * rightValue;
	case TokenType::DIVIDE:
		if (rightValue == 0) {
			throw std::runtime_error("Division by zero");
		}
		return leftValue / rightValue;
	case TokenType::AND: return (leftValue != 0 && rightValue != 0) ? 1 : 0;
	case TokenType::OR: return (leftValue != 0 || rightValue != 0) ? 1 : 0;
	case TokenType::EQUALS: return (leftValue == rightValue) ? 1 : 0;
	case TokenType::NOT_EQUALS: return (leftValue != rightValue) ? 1 : 0;
	case TokenType::LESS: return (leftValue < rightValue) ? 1 : 0;
	case TokenType::LESS_EQUALS: return (leftValue <= rightValue) ? 1 : 0;
	case TokenType::GREATER: return (leftValue > rightValue) ? 1 : 0;
	case TokenType::GREATER_EQUALS: return (leftValue >= rightValue) ? 1 : 0;
	default: throw std::runtime_error("Unknown binary operator");
	}
}

BinaryOpNode::~BinaryOpNode() {
	delete left;
	delete right;
}

double WhileNode::evaluate(Environment& env) const  {
	double result = 0;
	while (condition->evaluate(env) != 0) {
		result = body->evaluate(env);
	}
	return result;
}

WhileNode::~WhileNode() {
	delete condition;
	delete body;
}


double IfNode::evaluate(Environment& env) const  {
	double conditionValue = condition->evaluate(env);
	std::cout << "IfNode: conditionValue" << conditionValue << std::endl;
	if (conditionValue != 0) {
		return thenBranch->evaluate(env);
	}
	else if (elseBranch) {
		return elseBranch->evaluate(env);
	}
	return 0;
}

IfNode::~IfNode() {
	delete condition;
	delete thenBranch;
	delete elseBranch;
}

double ReturnNode::evaluate(Environment& env) const  {
	return returnValue->evaluate(env);
}

ReturnNode::~ReturnNode() {
	delete returnValue;
}

double FunctionCallNode::evaluate(Environment& env) const  {
	if (name == "print") {
		if (arguments.size() != 1) {
			throw std::runtime_error("print expects 1 argument");
		}

		ASTNode* arg = arguments[0];

		// Handle variable node
		if (auto varNode = dynamic_cast<VariableNode*>(arg)) {
			auto value = env.getVariable(varNode->name);
			if (std::holds_alternative<double>(value)) {
				std::cout << "Print from script: " << std::get<double>(value) << std::endl;
			}
			else if (std::holds_alternative<std::string>(value)) {
				std::cout << "Print from script: " << std::get<std::string>(value) << std::endl;
			}
			else if (std::holds_alternative<bool>(value)) {
				std::cout << "Print from script: " << (std::get<bool>(value) ? "true" : "false") << std::endl;
			}
			else {
				throw std::runtime_error("Unsupported type for print function");
			}
		}
		// Handle direct number or string nodes
		else if (auto numNode = dynamic_cast<NumberNode*>(arg)) {
			std::cout << "Print from script: " << numNode->value << std::endl;
		}
		else if (auto strNode = dynamic_cast<StringNode*>(arg)) {
			std::cout << "Print from script: " << strNode->value << std::endl;
		}
		else {
			throw std::runtime_error("Unsupported type for print function");
		}
	}
	else {
		throw std::runtime_error("Undefined function: " + name);
	}

	return 0; // Function calls like print do not return values
}

FunctionCallNode::~FunctionCallNode() {
	for (ASTNode* arg : arguments) {
		delete arg;
	}
}

double FunctionNode::evaluate(Environment& env) const  {
	// FunctionNode itself is not "evaluated"; it's stored in the environment
	throw std::runtime_error("FunctionNode cannot be directly evaluated.");
}

FunctionNode::~FunctionNode() {
	delete body;
}

double DeclarationNode::evaluate(Environment& env) const  {
	// Declare the variable in the environment
	env.declareVariable(variableName, type);

	// If there is an initializer, evaluate it and set the value in the environment
	if (initializer) {
		VariableValue value;

		switch (type) {
		case ValueType::INT:
		case ValueType::FLOAT: {
			double evaluatedValue = initializer->evaluate(env);
			value = evaluatedValue;
			std::cout << "DeclarationNode: Initializing variable " << variableName << " with value " << evaluatedValue << std::endl;
			break;
		}
		case ValueType::BOOL: {
			bool evaluatedValue = initializer->evaluate(env) != 0;
			value = evaluatedValue;
			std::cout << "DeclarationNode: Initializing boolean variable " << variableName << " with value " << (evaluatedValue ? "true" : "false") << std::endl;
			break;
		}
		case ValueType::STRING: {
			// Assuming the initializer can be evaluated to a string
			auto strNode = dynamic_cast<StringNode*>(initializer);
			if (strNode) {
				value = strNode->evaluateString(env);
				std::cout << "DeclarationNode: Initializing string variable " << variableName << " with value \"" << std::get<std::string>(value) << "\"" << std::endl;
			}
			else {
				throw std::runtime_error("Type error: Expected string initializer for variable " + variableName);
			}
			break;
		}
		default:
			throw std::runtime_error("Unknown value type for variable " + variableName);
		}

		// Set the value in the environment
		env.setVariable(variableName, value);
	}
	return 0;
}

DeclarationNode::~DeclarationNode() {
	delete initializer;
}

double AssignmentNode::evaluate(Environment& env) const  {
	double value = expression->evaluate(env);
	std::cout << "AssignmentNode: Assigning value " << value << " to variable " << variableName << std::endl;
	env.setVariable(variableName, value);
	return value;
}

AssignmentNode::~AssignmentNode() {
	delete expression;
}

double VariableNode::evaluate(Environment& env) const  {
	auto value = env.getVariable(name);
	if (std::holds_alternative<double>(value)) {
		double numValue = std::get<double>(value);
		std::cout << "VariableNode: Retrieved numeric value " << numValue << " for variable " << name << std::endl;
		return numValue;
	}
	else if (std::holds_alternative<bool>(value)) {
		bool boolValue = std::get<bool>(value);
		std::cout << "VariableNode: Retrieved boolean value " << (boolValue ? "true" : "false") << " for variable " << name << std::endl;
		return boolValue ? 1.0 : 0.0;
	}
	// Handle string type: return a default value or throw an error
	else if (std::holds_alternative<std::string>(value)) {
		std::string strValue = std::get<std::string>(value);
		std::cout << "VariableNode: Retrieved string value \"" << strValue << "\" for variable " << name << std::endl;
		// Option 1: Return a default value such as 0.0
		return 0.0;

		// Option 2: Throw an error if evaluating a string as a double is not allowed
		// throw std::runtime_error("Cannot evaluate a string value as a double for variable: " + name);
	}
	else {
		throw std::runtime_error("Expected numeric value for variable: " + name);
	}
}
