#include "Parser.hpp"
#include "Types.hpp"
#include "Environment.hpp"
#include "Lexer.hpp"


// Helper function to print a VariableValue, used for arrays and maps
void printVariableValue(const VariableValue& value) {
	if (auto doublePtr = std::get_if<double>(&value.value)) {
		std::cout << *doublePtr;
	}
	else if (auto boolPtr = std::get_if<bool>(&value.value)) {
		std::cout << (*boolPtr ? "true" : "false");
	}
	else if (auto strPtr = std::get_if<std::string>(&value.value)) {
		std::cout << "\"" << *strPtr << "\"";
	}
	else if (auto arrayPtr = std::get_if<std::vector<VariableValue>>(&value.value)) {
		std::cout << "[";
		for (size_t i = 0; i < arrayPtr->size(); ++i) {
			if (i > 0) {
				std::cout << ", ";
			}
			printVariableValue((*arrayPtr)[i]);
		}
		std::cout << "]";
	}
	else if (auto mapPtr = std::get_if<std::unordered_map<std::string, VariableValue>>(&value.value)) {
		std::cout << "{";
		bool first = true;
		for (const auto& pair : *mapPtr) {
			if (!first) {
				std::cout << ", ";
			}
			first = false;
			std::cout << "\"" << pair.first << "\": ";
			printVariableValue(pair.second);
		}
		std::cout << "}";
	}
	else {
		std::cout << "unknown type";
	}
}

VariableValue print(const std::vector<VariableValue>& args, const std::vector<std::string>& argumentsNames, Environment& env) {
	if (args.size() != 1) {
		throw std::runtime_error("print expects 1 argument");
	}

	// Determine the type of the argument and print accordingly
	const VariableValue& value = args[0];
	if (auto doublePtr = std::get_if<double>(&value.value)) {
		std::cout << "Print from script: " << *doublePtr << std::endl;
	}
	else if (auto boolPtr = std::get_if<bool>(&value.value)) {
		std::cout << "Print from script: " << (*boolPtr ? "true" : "false") << std::endl;
	}
	else if (auto strPtr = std::get_if<std::string>(&value.value)) {
		std::cout << "Print from script: " << *strPtr << std::endl;
	}
	else if (auto arrayPtr = std::get_if<std::vector<VariableValue>>(&value.value)) {
		// Print array elements
		std::cout << "Print from script: [";
		for (size_t i = 0; i < arrayPtr->size(); ++i) {
			if (i > 0) {
				std::cout << ", ";
			}
			// Print each element in the array recursively
			const VariableValue& element = (*arrayPtr)[i];
			printVariableValue(element);
		}
		std::cout << "]" << std::endl;
	}
	else if (auto mapPtr = std::get_if<std::unordered_map<std::string, VariableValue>>(&value.value)) {
		// Print map key-value pairs
		std::cout << "Print from script: {";
		bool first = true;
		for (const auto& pair : *mapPtr) {
			if (!first) {
				std::cout << ", ";
			}
			first = false;
			std::cout << "\"" << pair.first << "\": ";
			// Print each value in the map recursively
			printVariableValue(pair.second);
		}
		std::cout << "}" << std::endl;
	}
	else {
		std::cout << "Print from script: unknown type" << std::endl;
	}

	return VariableValue(); // Return 0 since print does not produce a value
}

int test1() {
	Environment env;

	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
		#import "test.t"

        int x = 8;
        int y;
        y = x - 3;
        if (y > 0) {
            print(y);
        }
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}



int test2() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
        int x = 5;
		if (x > 3) { x = x + 1; }
		while (x < 10) { x = x + 1;  print(x); }
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test3() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
       int x = 5;
        float y = 2.5;
        bool isTrue = true;
        string z = "hi from script";

        print(x);
        print(y);
        print(isTrue);
        print(z);
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test4() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
       func int add(int a, int b) {
			return a + b;
		}

		int x = add(5, 10);  // Correct usage
		print(x);
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test5() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(

func int sumUpTo(int n) {
    int sum = 0;
    int i = 1;

    while (i <= n) {
        sum = sum + i;
        i = i + 1;
    }

    return sum;
}

func int factorial(int n) {
    int result = 1;

    for (int i = 1; i <= n; i = i + 1) {
        result = result * i;
    }

    return result;
}

func string sayHello(){
	return "hi";
}

func int incrementTest(int n) {
    int i = 0;

    // Prefix increment
    ++i;

    // Postfix increment
    i++;

    // Using while loop with increment
    while (i < n) {
        i++;
    }

    return i;
}

func void main(){
    int result1 = sumUpTo(5);      // Should return 15
    int result2 = factorial(5);    // Should return 120
    int result3 = incrementTest(5);  
    string result4 = sayHello();  

    // Assume a print function exists
    print(result1);
    print(result2);
	print(result3);
	print(result4);
}

main();

    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test6() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);
	
	// Example script with for loop and do-while loop
	std::string input = R"(
func bool evaluateLogic(bool a, bool b) {
    return a && b;  // Logical AND
}

func bool evaluateComplexLogic(bool a, bool b, bool c) {
    return (a && b) || c;  // Logical AND followed by OR
}

func bool testLogic() {
    bool tt = true;
    bool gg = false;
    bool ff = true;

    bool result1 = evaluateLogic(tt, gg);        // Should return false (true && false)
    bool result2 = evaluateComplexLogic(tt, gg, ff); // Should return true ((true && false) || true)


    return result1 || result2;  // This should return true
}


    bool result3 = testLogic();    

    // Assume a print function exists
    print(result3);

    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}


int test7() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
		int i;
        for (i = 0; i < 5; i = i + 1) {
            print(i);
        }

        int x = 0;
        do {
            print(x);
            x = x + 1;
        } while (x < 3);
    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}


int test8() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop, which also includes a function definition.
	std::string input = R"(
        func int multiply(int a, int b) {
           int result = a * b;
		   print(result);
		   return result;
        }
    )";

	// Create Lexer and Parser
	Lexer lexer(input);
	Parser parser(lexer, env);

	// Parse the script and register the function
	ASTNode* root{ nullptr };
	try {
		root = parser.parse();
	}
	catch (const std::exception& e) {
		std::cerr << "Error during parsing or evaluation: " << e.what() << std::endl;
		return -1;  // Return early due to error
	}

	//run the program
	try {
		root->evaluate(env);  // This will register the function "multiply" in the environment
	}
	catch (const std::exception& e) {
		std::cerr << "Error during parsing or evaluation: " << e.what() << std::endl;
		return -1;  // Return early due to error
	}


	// Now that the function is defined in the environment, call it from C++
	try {
		// Arguments to pass to the function
		std::vector<VariableValue> args = { VariableValue(5.0), VariableValue(10.0) };
		// Evaluate the function in the script environment
		VariableValue result = env.evaluateFunction("multiply", args);

		// Determine the type of result and print accordingly
		std::cout << "Result of multiply(5, 10): ";
		if (auto doublePtr = std::get_if<double>(&result.value)) {
			std::cout << *doublePtr << std::endl;
		}
		else if (auto boolPtr = std::get_if<bool>(&result.value)) {
			std::cout << (*boolPtr ? "true" : "false") << std::endl;
		}
		else if (auto strPtr = std::get_if<std::string>(&result.value)) {
			std::cout << *strPtr << std::endl;
		}
		else {
			std::cout << "unknown type" << std::endl;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "Error during function call: " << e.what() << std::endl;
	}

	delete root;
	return 0;
}

VariableValue addNumbers(const std::vector<VariableValue>& args, const std::vector<std::string>& argumentsNames, Environment& env2) {
	if (args.size() != 2) {
		throw std::runtime_error("addNumbers expects exactly 2 arguments.");
	}

	// Extract and handle types for addition or concatenation
	if (auto doublePtr1 = std::get_if<double>(&args[0].value)) {
		if (auto doublePtr2 = std::get_if<double>(&args[1].value)) {
			// If both arguments are doubles, add them
			return VariableValue(*doublePtr1 + *doublePtr2);
		}
	}
	else if (auto strPtr1 = std::get_if<std::string>(&args[0].value)) {
		if (auto strPtr2 = std::get_if<std::string>(&args[1].value)) {
			// If both arguments are strings, concatenate them
			return VariableValue(*strPtr1 + *strPtr2);
		}
	}

	throw std::runtime_error("addNumbers expects either two numbers or two strings.");
}

int test9() {
	Environment env;

	// Register a built-in print function
	 // Register the print function
	env.registerFunction("print", print);

	// Register the C++ function addNumbers into the environment
	env.registerFunction("addNumbers", addNumbers);

	// Example script with for loop and do-while loop, which also includes a function definition.
	std::string input = R"(
         int result = addNumbers(7, 8);
		 print(result);
    )";

	// Create Lexer and Parser
	Lexer lexer(input);
	Parser parser(lexer, env);

	// Parse the script and register the function
	ASTNode* root{ nullptr };
	try {
		root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error during parsing or evaluation: " << e.what() << std::endl;
		return -1;  // Return early due to error
	}


	return 0;
}

int test10() {
	Environment env;

	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
		func bool evaluateLogic(bool a) {
			return a;  // Logical AND
		}
		
		//TODO issue of declaration inside function
		func bool testLogic() {
			bool a = true;
			bool b = true;
			bool c = true;

			if(a){
				return a;
			}
			else if(b){
				return b;
			}
			else {
				return c;
			}

			return a;
		}


    bool a = testLogic();    

    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}



int test11() {
	Environment env;

	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
		array arr;
		arr = [1, 2, 3, 4];
		int x = arr[2];  // Should retrieve the value 3
		print(x);

		map myMap = { "key1": 10, "key2": 20 };
		int y = myMap["key2"];  // Should retrieve the value 20
		print(y);

		print(arr);
		print(myMap);

    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}




int test12() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);


	env.registerFunction("push", [&](const std::vector<VariableValue>& args,const std::vector<std::string>& argumentsNames, Environment& env3) -> VariableValue {
		if (args.size() != 2) {
			throw std::runtime_error("push expects 2 arguments: a variable name and a value to add");
		}

		// Ensure the first argument is a string representing the variable name
		if (argumentsNames.size() == 0) {
			throw std::runtime_error("push expects the first argument to be a variable name as a string");
		}

		std::string varName = argumentsNames[0];

		// Retrieve the array from the environment
		VariableValue arrayVar = env3.getVariable(varName);
		auto arrayPtr = std::get_if<std::vector<VariableValue>>(&arrayVar.value);
		if (!arrayPtr) {
			throw std::runtime_error("push expects the variable to be an array");
		}

		// Push the new value onto the array
		arrayPtr->push_back(args[1]);

		env3.setVariable(varName, VariableValue(*arrayPtr));

		return VariableValue(); // Return empty value to indicate success
		});

	env.registerFunction("pop", [&](const std::vector<VariableValue>& args,const std::vector<std::string>& argumentsNames, Environment& env3) -> VariableValue {
		if (args.size() != 1) {
			throw std::runtime_error("push expects 1 arguments: a variable name and a value to add");
		}

		// Ensure the first argument is a string representing the variable name
		if (argumentsNames.empty()) {
			throw std::runtime_error("push expects the first argument to be a variable name as a string");
		}

		std::string varName = argumentsNames[0];

		// Retrieve the array from the environment
		VariableValue arrayVar = env3.getVariable(varName);
		auto arrayPtr = std::get_if<std::vector<VariableValue>>(&arrayVar.value);
		if (!arrayPtr) {
			throw std::runtime_error("push expects the variable to be an array");
		}

		// Pop the last element from the array and return it
		VariableValue removedValue = arrayPtr->back();
		arrayPtr->pop_back();

		env3.setVariable(varName, VariableValue(*arrayPtr));

		return VariableValue(); // Return empty value to indicate success
		});

	// Example script with for loop and do-while loop
	std::string input = R"(
		// Example script
		array arr = [1, 2, 3];
		print(arr);  // Should print: [1, 2, 3]

		push(arr, 4);
		print(arr);  // Should print: [1, 2, 3, 4]

		pop(arr);
		print(arr);  // Should print: [1, 2, 3]

    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}

int test13() {
	Environment env;

	// Register a built-in print function
	env.registerFunction("print", print);

	// Example script with for loop and do-while loop
	std::string input = R"(
		class MyClass {
    int value;

    func MyClass(int initialValue) {
        value = initialValue; // Constructor
    }

    func void setValue(int newValue) {
        value = newValue;
    }

    func int getValue() {
        return value;
    }
}

func MyClass processObject(MyClass inputObj) {
	int val = inputObj.getValue();
	int x = val + 10;
    MyClass newObj = new MyClass(x);
    return newObj;
}

	MyClass obj = new MyClass(20);
    MyClass result;
	result = processObject(obj);
    int finalValue = obj.getValue();
    print(finalValue); // Should print 30

    )";

	Lexer lexer(input);
	Parser parser(lexer, env);

	try {
		ASTNode* root = parser.parse();
		root->evaluate(env);
		delete root;
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}




int main() {

 /*	test1();
 	test2();
 	test3();
 	test4();
 	test5();
 	test6();
 	test7();
 	test8();
 	test9();
 	test10();
  	test11();
 	test12();*/
 	test13();

	// imports from other file need to test
	// classes constructor new object method dot assignment destructor delete
	// refactor
	// 
	// enum
	// switch case
	// final for variables
	// 
	// validations
	//globals functions and variables needs to be equines
	// class names need to be equines 
	//cant create functions declensions inside if loops and others functions
	//also inside class functions and variables needs to be equines
}

