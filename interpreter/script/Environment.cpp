#include "Environment.hpp"
#include "AST.hpp"

// Register a built-in C++ function by name
void Environment::registerFunction(const std::string& name, ScriptFunction func) {
	if (functionRegistry.contains(name)) {
		throw std::runtime_error("Function already registered: " + name);
	}
	functionRegistry[name] = func;
}

// Register a user-defined function (AST-based)
void Environment::registerUserFunction(const std::string& name, ASTNode* functionNode) {
	if (userFunctionRegistry.contains(name)) {
		throw std::runtime_error("User-defined function already registered: " + name);
	}
	userFunctionRegistry[name] = functionNode;
}

// Evaluate a function by name with given arguments
double Environment::evaluateFunction(const std::string& name, const std::vector<double>& args) const {
	if (functionRegistry.contains(name)) {
		return functionRegistry.at(name)(args);
	}

	if (userFunctionRegistry.contains(name)) {
		ASTNode const* functionNode = userFunctionRegistry.at(name);
		return functionNode->evaluate(*const_cast<Environment*>(this));
	}

	throw std::runtime_error("Undefined function: " + name);
}

// Declare a variable by name and type
void Environment::declareVariable(const std::string& name, ValueType type) {
	if (variableTable.contains(name)) {
		throw std::runtime_error("Variable already declared: " + name);
	}
	switch (type) {
	case ValueType::INT:
	case ValueType::FLOAT:
		// Initialize numeric types to 0
		variableTable[name] = std::make_pair(0.0, type);
		break;
	case ValueType::BOOL:
		// Initialize boolean to false
		variableTable[name] = std::make_pair(false, type);
		break;
	case ValueType::STRING:
		// Initialize string to an empty string
		variableTable[name] = std::make_pair(std::string(""), type);
		break;
	}
}

void Environment::setVariable(const std::string& name, const VariableValue& value) {
	if (!variableTable.contains(name)) {
		throw std::runtime_error("Undefined variable: " + name);
	}

	ValueType type = variableTable[name].second;

	// Perform type checks to ensure correctness
	if (type == ValueType::INT) {
		if (!std::holds_alternative<double>(value) ||
			std::get<double>(value) != static_cast<int>(std::get<double>(value))) {
			throw std::runtime_error("Type error: Expected int value for variable " + name);
		}
	}
	else if (type == ValueType::FLOAT) {
		if (!std::holds_alternative<double>(value)) {
			throw std::runtime_error("Type error: Expected float value for variable " + name);
		}
	}
	else if (type == ValueType::BOOL) {
		if (!std::holds_alternative<bool>(value)) {
			throw std::runtime_error("Type error: Expected boolean value for variable " + name);
		}
	}
	else if (type == ValueType::STRING) {
		if (!std::holds_alternative<std::string>(value)) {
			throw std::runtime_error("Type error: Expected string value for variable " + name);
		}
	}

	// Assign the value to the variable
	variableTable[name].first = value;
	std::cout << "Environment: Updated variable " << name << " to new value." << std::endl;
}

// Get a variable's value
VariableValue Environment::getVariable(const std::string& name) const {
	if (!variableTable.contains(name)) {
		throw std::runtime_error("Undefined variable: " + name);
	}
	return variableTable.at(name).first;
}