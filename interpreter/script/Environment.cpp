#include "Environment.hpp"
#include "AST.hpp"

Environment::Environment()
{
	// Initialize with a global scope
	variableScopes.emplace_back();
}

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

VariableValue Environment::evaluateFunction(const std::string& name,const std::vector<VariableValue>& args,const std::vector<std::string>& argumentsNames) {
	if (functionRegistry.contains(name)) {
		return functionRegistry.at(name)(args, argumentsNames, *this);
	}

	if (userFunctionRegistry.contains(name)) {
		auto* functionNode = dynamic_cast<FunctionNode*>(userFunctionRegistry.at(name));
		if (!functionNode) {
			throw std::runtime_error("Function " + name + " is not properly defined.");
		}

		if (args.size() != functionNode->parameters.size()) {
			throw std::runtime_error("Function " + name + " expects " +
				std::to_string(functionNode->parameters.size()) +
				" arguments, but got " + std::to_string(args.size()));
		}

		// Create a new local scope for this function call
		pushScope();

		// Assign arguments to the parameters in the new scope
		for (size_t i = 0; i < args.size(); ++i) {
			const auto& param = functionNode->parameters[i];
			declareVariable(param.first, param.second);
			setVariable(param.first, args[i]);
		}

		// Execute the function body
		VariableValue returnValue = functionNode->body->evaluate(*this);


		// Remove the local scope after the function completes
		popScope();

		return returnValue;
	}

	throw std::runtime_error("Undefined function: " + name);
}



// Declare a variable by name and type
void Environment::declareVariable(const std::string& name, ValueType type) {
	if (variableScopes.back().contains(name)) {
		throw std::runtime_error("Variable already declared: " + name);
	}

	VariableValue defaultValue;
	switch (type) {
	case ValueType::INT:
	case ValueType::FLOAT:
		defaultValue = VariableValue(0.0);
		break;
	case ValueType::BOOL:
		defaultValue = VariableValue(false);
		break;
	case ValueType::STRING:
		defaultValue = VariableValue("");
		break;
	case ValueType::ARRAY:
		defaultValue = VariableValue(std::vector<VariableValue>());  // Default to an empty array
		break;
	case ValueType::MAP:
		defaultValue = VariableValue(std::unordered_map<std::string, VariableValue>());  // Default to an empty map
		break;
	default:
		throw std::runtime_error("Unsupported variable type for declaration.");
	}

	variableTable[name] = std::make_pair(defaultValue, type);
}

void Environment::setVariable(const std::string& name, const VariableValue& value) {
	
	for (auto scope = variableScopes.rbegin(); scope != variableScopes.rend(); ++scope) {
		if (scope->contains(name)) {
			scope->at(name).first = value;
			return;
		}
	}
	//if not in scope then global
	if (variableTable.contains(name)) {
		variableTable.at(name).first = value;
		return;
	}
	throw std::runtime_error("Undefined variable: " + name);
}

// Get a variable's value
VariableValue Environment::getVariable(const std::string& name) {
	for (auto scope = variableScopes.rbegin(); scope != variableScopes.rend(); ++scope) {
		if (scope->contains(name)) {
			return scope->at(name).first;
		}
	}
	//if not in scope then global
	if (variableTable.contains(name)) {
		return variableTable.at(name).first;
	}

	throw std::runtime_error("Undefined variable: " + name);
}

void Environment::pushScope()
{

	variableScopes.emplace_back();

}
