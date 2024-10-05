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

void Environment::registerClass(const std::string& name, ClassDefinitionNode* classDef)
{
	if (classRegistry.contains(name)) {
		throw std::runtime_error("Class already registered: " + name);
	}
	classRegistry[name] = classDef;
}

VariableValue Environment::instantiateObject(const std::string& className, const std::vector<ASTNode*>& args)
{
	if (!classRegistry.contains(className)) {
		throw std::runtime_error("Undefined class: " + className);
	}

	 ClassDefinitionNode* classDef = classRegistry.at(className);
	std::unordered_map<std::string, VariableValue> objectMembers;

	// Initialize member variables
	for (const auto& [memberName, memberNode] : classDef->members) {
		if (auto declNode = dynamic_cast<DeclarationNode*>(memberNode)) {
			objectMembers[memberName] = declNode->evaluate(*this);
		}
	}

	// Call constructor if it exists
	if (classDef->constructor) {
		// Evaluate constructor arguments
		std::vector<VariableValue> evaluatedArgs;
		for (ASTNode* arg : args) {
			evaluatedArgs.push_back(arg->evaluate(*this));
		}

		// Push a new scope for the constructor
		pushScope();

		// Set constructor parameters in the new scope
		for (size_t i = 0; i < evaluatedArgs.size(); ++i) {
			const auto& param = classDef->constructor->parameters[i];
			declareVariable(param.first, param.second);
			setVariable(param.first, evaluatedArgs[i]);
		}

		// Execute the constructor body
		classDef->constructor->body->evaluate(*this);

		// Pop the constructor scope
		popScope();
	}

	// Evaluate constructor logic if needed (add your constructor logic here)

	return VariableValue(objectMembers); // Objects are represented as maps of their members
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
