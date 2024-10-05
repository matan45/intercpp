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

VariableValue Environment::evaluateFunction(const std::string& name, const std::vector<VariableValue>& args, const std::vector<std::string>& argumentsNames) {
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
			const ValueType paramType = param.second;

			if (paramType == ValueType::CLASS) {
				// If the parameter is of type CLASS, use the corresponding class name
				const std::string& paramClassName = functionNode->parameterClassNames[i];
				if (paramClassName.empty()) {
					throw std::runtime_error("Missing class name for parameter '" + param.first + "' in function '" + functionNode->name + "'");
				}
				declareVariable(param.first, paramType, paramClassName);
			}
			else {
				// For non-class types, declare the variable as usual
				declareVariable(param.first, paramType);
			}

			// Set the value of the parameter in the environment
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

VariableValue Environment::instantiateObject(const std::string& className, const std::vector<ASTNode*>& args) {
	if (!classRegistry.contains(className)) {
        throw std::runtime_error("Undefined class: " + className);
    }

    ClassDefinitionNode* classDef = classRegistry.at(className);
    std::unordered_map<std::string, VariableValue> memberScope;

    // Initialize member variables with default values
    for (const auto& [memberName, memberNode] : classDef->members) {
        if (auto declNode = dynamic_cast<DeclarationNode*>(memberNode)) {
            memberScope[memberName] = declNode->evaluate(*this);
        }
    }

    // Create the object value with member variables
    VariableValue objectValue(memberScope);

    // Handle constructor if it exists
    if (classDef->constructor) {
        std::vector<VariableValue> evaluatedArgs;
        for (ASTNode* arg : args) {
            evaluatedArgs.push_back(arg->evaluate(*this));
        }

        // Use a new scope specifically for constructor parameters
        pushScope();

        // Declare constructor parameters
        for (size_t i = 0; i < classDef->constructor->parameters.size(); ++i) {
            const auto& param = classDef->constructor->parameters[i];
            declareVariable(param.first, param.second);
            setVariable(param.first, evaluatedArgs[i]);
        }

        // Set the current instance for member access
        currentObjectInstance = memberScope;

        // Evaluate constructor body, which may modify member variables
        classDef->constructor->body->evaluate(*this);

        // Update member variables from the currentObjectInstance after constructor execution
        memberScope = currentObjectInstance;

        // Clear current instance after constructor execution
        currentObjectInstance.clear();

        popScope();
    }

    return VariableValue(memberScope);
}



bool Environment::isClassDefined(const std::string& identifier)
{
	return classRegistry.contains(identifier);
}



// Declare a variable by name and type
void Environment::declareVariable(const std::string& name, ValueType type, const std::string& className) {
	std::cout << "Declaring variable: " << name << " in ";
	if (!variableScopes.empty()) {
		std::cout << "local scope" << std::endl;
	}
	else {
		std::cout << "global scope" << std::endl;
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
		defaultValue = VariableValue(std::vector<VariableValue>());
		break;
	case ValueType::MAP:
		defaultValue = VariableValue(std::unordered_map<std::string, VariableValue>());
		break;
	case ValueType::CLASS: {
		if (!classRegistry.contains(className)) {
			throw std::runtime_error("Undefined class type for variable: " + className);
		}
		defaultValue = VariableValue(std::unordered_map<std::string, VariableValue>());
		break;
	}
	default:
		throw std::runtime_error("Unsupported variable type for declaration.");
	}

	// Add to the innermost scope (local) or global scope
	if (!variableScopes.empty()) {
		if (variableScopes.back().contains(name)) {
			throw std::runtime_error("Variable already declared in current scope: " + name);
		}
		variableScopes.back()[name] = std::make_pair(defaultValue, type);
	}
	else {
		if (variableTable.contains(name)) {
			throw std::runtime_error("Variable already declared in global scope: " + name);
		}
		variableTable[name] = std::make_pair(defaultValue, type);
	}
}

void Environment::setVariable(const std::string& name, const VariableValue& value) {
	// First, check if this variable is a class member in the current member scope
	if (currentObjectInstance.contains(name)) {
		currentObjectInstance[name] = value;
		return;
	}

	// Traverse local scopes for the variable
	for (auto scope = variableScopes.rbegin(); scope != variableScopes.rend(); ++scope) {
		if (scope->contains(name)) {
			scope->at(name).first = value;
			return;
		}
	}

	// If no local scope has it, update it globally if it exists
	if (variableTable.contains(name)) {
		variableTable.at(name).first = value;
		return;
	}
	throw std::runtime_error("Undefined variable: " + name + ". Cannot assign value.");

}


void Environment::declareObject(const std::string& className, const std::string& objectName)
{
	if (!classRegistry.contains(className)) {
		throw std::runtime_error("Undefined class: " + className);
	}

	ClassDefinitionNode* classDef = classRegistry.at(className);
	std::unordered_map<std::string, VariableValue> memberScope;

	for (const auto& [memberName, memberNode] : classDef->members) {
		if (auto declNode = dynamic_cast<DeclarationNode*>(memberNode)) {
			memberScope[memberName] = declNode->evaluate(*this);
		}
	}

	VariableValue objectValue(memberScope);

	if (!variableScopes.empty()) {
		variableScopes.back()[objectName] = std::make_pair(objectValue, ValueType::CLASS);
	}
	else {
		variableTable[objectName] = std::make_pair(objectValue, ValueType::CLASS);
	}
}

bool Environment::isVariableDeclared(const std::string& name)
{
	// Check in the current scope if the variable is already declared
	if (!variableScopes.empty()) {
		const auto& currentScope = variableScopes.back();
		return currentScope.find(name) != currentScope.end();
	}
	return false;
}

bool Environment::isMemberFunction(const std::unordered_map<std::string, VariableValue>& objMap, const std::string& methodName)
{
	// Check if the object contains the member and if it refers to a function
	auto it = objMap.find(methodName);
	if (it != objMap.end()) {
		// Check if the value is a FunctionNode (which indicates it's callable)
		return std::holds_alternative<FunctionNode*>(it->second.value);
	}
	return false;
}

// Call a member function
VariableValue Environment::callMemberFunction(const std::unordered_map<std::string, VariableValue>& objMap, const std::string& methodName, const std::vector<VariableValue>& args) {
	if (!objMap.contains(methodName)) {
		throw std::runtime_error("Undefined member function: " + methodName);
	}

	// Ensure the member is a function
	FunctionNode* functionNode = std::get<FunctionNode*>(objMap.at(methodName).value);
	if (!functionNode) {
		throw std::runtime_error("Member " + methodName + " is not callable.");
	}

	// Create a new scope for the function call and push the member scope
	pushScope();
	currentObjectInstance = objMap;

	// Declare function arguments in the new scope
	for (size_t i = 0; i < functionNode->parameters.size(); ++i) {
		const auto& param = functionNode->parameters[i];
		declareVariable(param.first, param.second);
		setVariable(param.first, args[i]);
	}

	// Execute the function body
	VariableValue returnValue = functionNode->body->evaluate(*this);

	// Pop the function scope and clear the member scope
	popScope();
	currentObjectInstance.clear();

	return returnValue;
}

// Get a variable's value
VariableValue Environment::getVariable(const std::string& name) {
	std::cout << "Getting variable: " << name << std::endl;
	// Check if we are accessing a class member in the current object instance
	if (currentObjectInstance.contains(name)) {
		return currentObjectInstance.at(name);
	}

	// Traverse from innermost (local) to outermost (global) scope
	for (auto scope = variableScopes.rbegin(); scope != variableScopes.rend(); ++scope) {
		if (scope->contains(name)) {
			return scope->at(name).first;
		}
	}

	// Lastly, check the global scope
	if (variableTable.contains(name)) {
		return variableTable.at(name).first;
	}
	throw std::runtime_error("Undefined variable: " + name);
}

void Environment::pushScope()
{
	variableScopes.emplace_back();
	std::cout << "Environment: Pushed a new scope." << std::endl;

}
