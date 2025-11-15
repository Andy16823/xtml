#pragma once
#include "Vars.h"
#include "Utils.h"
#include <sstream>
#include <tuple>
#include <algorithm>
#include <cctype>
#include <vector>
#include <iterator>
#include "Globals.h"

using namespace std;

bool Vars::isFunctionExpr(vector<string>& tokens)
{
	if (tokens.size() != 1) return false;

	auto expr = Utils::trim(tokens[0]);
	// Check if expression is a function call e.g. namespace::funcName(arg1, arg2)
	if (expr.find("::") != string::npos && expr.find('(') != string::npos && expr.find(')') != string::npos) {
		return true;
	}
	return false;
}

var Vars::evalExpr(const string& expr, const map<string, var>& vars)
{
	// With the lexer/parser the evaluation is now handled in AST nodes.
	// Only simple variable lookup is needed here.
	var result{ "", DT_UNKNOWN };
	if(vars.find(expr) != vars.end()) {
		return vars.at(expr);
	}
	return result;
}

var Vars::evalFuncExpr(vector<string>& tokens, const map<string, var>& vars)
{
	// e.g. std::toUpper("hello")
	if (tokens.size() != 1) {
		Utils::throwErr("Error: Invalid function expression." );
	}
	auto expr = Utils::trim(tokens[0]);
	auto [namespaceName, functionName, args] = FunctionRegistry::ParseFunctionCall(expr);

	// Prepare funct args
	vector<var> funcArgs;
	for (auto& arg : args) {
		auto evaledArg = evalExpr(arg, vars);
		if (evaledArg.type == DT_UNKNOWN) {
			Utils::throwErr("Error: Failed to evaluate function argument: " + arg);
		}
		funcArgs.push_back(evaledArg);
	}

	// Call function
	if (g_functionRegistry.Exists(namespaceName, functionName)) {
		return g_functionRegistry.CallFunction(namespaceName, functionName, funcArgs);
	}
	else {
		Utils::throwErr("Error: Function not found: " + namespaceName + "::" + functionName);
	}
}

bool Vars::isFunctionExpr(const string& token)
{
	if (token.find("::") != string::npos && token.find('(') != string::npos && token.find(')') != string::npos) {
		return true;
	}
	return false;
}

var Vars::evalFuncExpr(const string& token, const map<string, var>& vars)
{
	auto expr = Utils::trim(token);
	auto [namespaceName, functionName, args] = FunctionRegistry::ParseFunctionCall(expr);

	// Prepare funct args
	vector<var> funcArgs;
	for (auto& arg : args) {
		auto evaledArg = evalExpr(arg, vars);
		if (evaledArg.type == DT_UNKNOWN) {
			Utils::throwErr("Error: Failed to evaluate function argument: " + arg);
		}
		funcArgs.push_back(evaledArg);
	}

	// Call function
	if (g_functionRegistry.Exists(namespaceName, functionName)) {
		return g_functionRegistry.CallFunction(namespaceName, functionName, funcArgs);
	}
	else {
		Utils::throwErr("Error: Function not found: " + namespaceName + "::" + functionName);
	}
}

var Vars::binaryOperation(const var& left, const var& right, const std::string& op)
{
	if(left.type != right.type) {
		// Allow string concatenation with mixed types by converting non-string to string
		if ((left.type == DT_STRING || right.type == DT_STRING)) {
			return stringOperation(left, right, op);
		}
		Utils::throwErr("Error: Type mismatch in binary operation.");
	}
	if(left.type == DT_STRING) {
		return stringOperation(left, right, op);
	}
	if(left.type == DT_NUMBER) {
		return numericOperation(left, right, op);
	}
	if(left.type == DT_BOOL) {
		return booleanOperation(left, right, op);
	}

	Utils::throwErr("Error: Unsupported type in binary operation.");
}

var Vars::stringOperation(const var& left, const var& right, const std::string& op)
{
	// Allow allways string concatenation by converting non-string to string
	if(op == "+") {
		return var{ left.value + right.value, DT_STRING };
	} 

	Utils::throwErr("Error: Unknown string operation: " + op);
}

var Vars::numericOperation(const var& left, const var& right, const std::string& op)
{
	if (left.type != DT_NUMBER || right.type != DT_NUMBER) {
		Utils::throwErr("Error: Numeric operation on non-number types.");
	}
	int64_t leftNum = std::stoll(left.value);
	int64_t rightNum = std::stoll(right.value);

	if (op == "+") {
		return var{ std::to_string(leftNum + rightNum), DT_NUMBER };
	}
	if(op == "-") {
		return var{ std::to_string(leftNum - rightNum), DT_NUMBER };
	}
	if (op == "*")
	{
		return var{ std::to_string(leftNum * rightNum), DT_NUMBER };
	}
	if (op == "/")
	{
		if (rightNum == 0) {
			Utils::throwErr("Error: Division by zero.");
		}
		return var{ std::to_string(leftNum / rightNum), DT_NUMBER };
	}
	if (op == "%")
	{
		if (rightNum == 0) {
			Utils::throwErr("Error: Modulo by zero.");
		}
		return var{ std::to_string(leftNum % rightNum), DT_NUMBER };
	}
	if(op == "^")
	{
		int64_t result = 1;
		for (int64_t i = 0; i < rightNum; ++i) {
			result *= leftNum;
		}
		return var{ std::to_string(result), DT_NUMBER };
	}
	if(op == "//")
	{
		if (rightNum == 0) {
			Utils::throwErr("Error: Floor division by zero.");
		}
		return var{ std::to_string(leftNum / rightNum), DT_NUMBER };
	}

	Utils::throwErr("Error: Unknown numeric operation: " + op);
}

var Vars::booleanOperation(const var& left, const var& right, const std::string& op)
{
	if (left.type != DT_BOOL || right.type != DT_BOOL) {
		Utils::throwErr("Error: Boolean operation on non-boolean types.");
	}

	bool leftBool = (left.value == "true" || left.value == "1");
	bool rightBool = (right.value == "true" || right.value == "1");

	if (op == "&&") {
		return var{ (leftBool && rightBool) ? "true" : "false", DT_BOOL };
	}
	if (op == "||") {
		return var{ (leftBool || rightBool) ? "true" : "false", DT_BOOL };
	}
	Utils::throwErr("Error: Unknown boolean operation: " + op);
}

bool Vars::compareOperation(const var& left, const var& right, const std::string& op)
{
	if(left.type != right.type) {
		Utils::throwErr("Error: Type mismatch in comparison operation.");
	}
	if(left.type == DT_STRING) {
		return compareStrings(left.value, right.value, op);
	}
	if(left.type == DT_NUMBER) {
		return compareNumbers(std::stoll(left.value), std::stoll(right.value), op);
	}
	if(left.type == DT_BOOL) {
		bool leftBool = (left.value == "true" || left.value == "1");
		bool rightBool = (right.value == "true" || right.value == "1");
		return compareBooleans(leftBool, rightBool, op);
	}
	Utils::throwErr("Error: Unsupported type in comparison operation.");
}

bool Vars::compareStrings(const std::string& left, const std::string& right, const std::string& op)
{
	if (op == "==") {
		return left == right;
	}
	else if (op == "!=") {
		return left != right;
	}
	else if (op == "<") {
		return left < right;
	}
	else if (op == "<=") {
		return left <= right;
	}
	else if (op == ">") {
		return left > right;
	}
	else if (op == ">=") {
		return left >= right;
	}
	Utils::throwErr("Error: Unknown comparison operation: " + op);
}

bool Vars::compareNumbers(int64_t left, int64_t right, const std::string& op)
{
	if (op == "==") {
		return left == right;
	}
	else if (op == "!=") {
		return left != right;
	}
	else if (op == "<") {
		return left < right;
	}
	else if (op == "<=") {
		return left <= right;
	}
	else if (op == ">") {
		return left > right;
	}
	else if (op == ">=") {
		return left >= right;
	}
	Utils::throwErr("Error: Unknown comparison operation: " + op);
}

bool Vars::compareBooleans(bool left, bool right, const std::string& op)
{
	if (op == "==") {
		return left == right;
	}
	else if (op == "!=") {
		return left != right;
	}
	Utils::throwErr("Error: Unknown comparison operation: " + op);
}

var Vars::unaryOperation(const var& operand, const std::string& op)
{
	if(operand.type == DT_NUMBER) {
		int64_t num = std::stoll(operand.value);
		if (op == "--") {
			return var{ std::to_string(num - 1), DT_NUMBER };
		}
		if (op == "++") {
			return var{ std::to_string(num + 1), DT_NUMBER };
		}
		if(op == "-") {
			return var{ std::to_string(-num), DT_NUMBER };
		}
		if(op == "+") {
			return var{ std::to_string(+num), DT_NUMBER };
		}
		Utils::throwErr("Error: Unknown unary operation: " + op);
	}	
	
	Utils::throwErr("Error: Unary operation on unsupported type.");
}

BinaryOpType Vars::getBinaryOpType(const std::string& op)
{
	if (op == "=") {
		return BinaryOpType::Assignment;
	}
	else if (op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=") {
		return BinaryOpType::Comparison;
	}
	else if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^" || op == "//") {
		return BinaryOpType::Arithmetic;
	}
	else if (op == "&&" || op == "||") {
		return BinaryOpType::Logical;
	}
	return BinaryOpType::Unknown;
}

var operator+(const var& lhs, const var& rhs)
{
	var result;

	if (lhs.type == DT_NUMBER && rhs.type == DT_NUMBER) {
		int l = std::stoll(lhs.value);
		int r = std::stoll(rhs.value);
		result.value = std::to_string(l + r);
		result.type = DT_NUMBER;
		return result;
	}

	if (lhs.type == DT_STRING && rhs.type == DT_STRING) {
		result.value = lhs.value + rhs.value;
		result.type = DT_STRING;
		return result;
	}

	if (lhs.type == DT_STRING && rhs.type == DT_NUMBER) {
		result.value = lhs.value + rhs.value;
		result.type = DT_STRING;
		return result;
	}

	if (lhs.type == DT_NUMBER && rhs.type == DT_STRING) {
		result.value = lhs.value + rhs.value;
		result.type = DT_STRING;
		return result;
	}

	if (lhs.type == DT_BOOL && rhs.type == DT_BOOL) {
		bool l = Utils::toBool(lhs.value);
		bool r = Utils::toBool(rhs.value);
		result.value = (l || r) ? "true" : "false";
		result.type = DT_BOOL;
		return result;
	}

	if (lhs.type == DT_BOOL && rhs.type == DT_NUMBER) {
		bool l = Utils::toBool(lhs.value);
		int r = std::stoll(rhs.value);
		result.value = (l || r != 0) ? "true" : "false";
		result.type = DT_BOOL;
		return result;
	}

	if (lhs.type == DT_NUMBER && rhs.type == DT_BOOL) {
		int l = std::stoll(lhs.value);
		bool r = Utils::toBool(rhs.value);
		result.value = (l != 0 || r) ? "true" : "false";
		result.type = DT_BOOL;
		return result;
	}

	if (lhs.type == DT_ARRAY && rhs.type == DT_ARRAY) {
		result.array = lhs.array;
		result.array.insert(result.array.end(), rhs.array.begin(), rhs.array.end());
		result.type = DT_ARRAY;
		return result;
	}

	// Fallback to string concatenation
	result.value = lhs.value + rhs.value;
	result.type = DT_STRING;
	return result;
}
