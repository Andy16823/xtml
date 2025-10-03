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

string Vars::trim_var(const string& var)
{
	string trimmed = var;
	if (trimmed.rfind("@var", 0) == 0) {
		trimmed = trimmed.substr(4);
	}
	if (trimmed.rfind(";") == trimmed.size() - 1) {
		trimmed = trimmed.substr(0, trimmed.size() - 1);
	}
	return trimmed;
}

tuple<string, string> Vars::parse_var(const std::string& line)
{
	size_t eq_pos = line.find('=');
	if (eq_pos != std::string::npos) {
		std::string key = Utils::trim(line.substr(0, eq_pos));
		std::string value = Utils::trim(line.substr(eq_pos + 1));
		return make_tuple(key, value);
	}
	return make_tuple("", "");
}

vector<string> Vars::parse_tokens(const string& expr, const char ops[], bool addop)
{
	// Split expression by operator while respecting quotes
	auto tokens = vector<string>();
	auto current = string();
	bool in_quotes = false;
	size_t ops_length = strlen(ops);

	for (char ch : expr) {
		if (ch == '"') {
			in_quotes = !in_quotes;
			current += ch;
		}
		else if (!in_quotes && std::find(ops, ops + ops_length, ch) != ops + ops_length) {
			if (!current.empty()) {
				tokens.push_back(Utils::trim(current));
				current.clear();
			}
			if (addop)
				tokens.push_back(string(1, ch));
		}
		else {
			current += ch;
		}
	}

	if (!current.empty()) {
		tokens.push_back(Utils::trim(current));
	}

	return tokens;
}

string Vars::replace_vars(string& content, const map<string, var>& vars)
{
	for (const auto& var : vars) {
		const string& key = var.first;
		const string& value = var.second.value;
		string placeholder = "{{@" + key + "}}";
		size_t pos = 0;
		while ((pos = content.find(placeholder, pos)) != string::npos) {
			content.replace(pos, placeholder.length(), value);
			pos += value.length();
		}
	}
	return content;
}

string Vars::preprocess_content(const string& content)
{
	std::string result;
	bool in_string = false;
	bool last_was_space = false;

	for (char ch : content) {
		if (ch == '\t' || ch == '\n' || ch == '\r') continue;

		if (ch == '"') {
			in_string = !in_string;
			result += ch;
			last_was_space = false;
			continue;
		}

		if (std::isspace(static_cast<unsigned char>(ch)) && !in_string) {
			if (!last_was_space) {
				result += ' ';
				last_was_space = true;
			}
			continue;
		}

		if (in_string && ch == ' ') {
			if (!last_was_space) {
				result += ch;
				last_was_space = true;
			}
		}
		else {
			result += ch;
			last_was_space = false;
		}
	}

	return result;
}

bool Vars::is_string_expr(const string& expr, const map<string, var>& vars)
{
	auto tokens = parse_tokens(expr, "+", false);
	return is_string_expr(tokens, vars);
}

bool Vars::is_string_expr(vector<string>& tokens, const map<string, var>& vars)
{
	for (auto& token : tokens) {
		token = Utils::trim(token);
		if (token.empty()) continue;
		// Check if token is a string literal
		if (Utils::is_string(token)) {
			continue;
		}
		// Check if token is a variable
		else if (auto var = vars.find(token); var != vars.end()) {
			if (var->second.value.empty() || var->second.type == DT_STRING) {
				continue;
			}
			else {
				return false; // Variable is not a string
			}
		}
		else {
			return false; // Unknown token
		}
	}
	return true;
}

bool Vars::is_numeric_expr(vector<string>& tokens, const map<string, var>& vars)
{
	for (auto& token : tokens) {
		token = Utils::trim(token);
		if (token.empty() || token == "+") continue;
		// Check if token is a number literal
		if (Utils::is_number(token)) {
			continue;
		}
		// Check if token is a variable
		else if (auto var = vars.find(token); var != vars.end()) {
			if (var->second.type == DT_NUMBER && Utils::is_number(var->second.value)) {
				continue;
			}
			else {
				return false; // Variable is not a number
			}
		}
		else {
			return false; // Unknown token
		}
	}
	return true;
}

bool Vars::is_bool_expr(std::vector<std::string>& tokens, const std::map<std::string, var>& vars)
{
	for (auto& token : tokens) {
		token = Utils::trim(token);
		if (token.empty() || token == "+") continue;

		// Check if token is a boolean literal
		if (token == "true" || token == "false" || token == "1" || token == "0") {
			continue;
		}

		// Check if token is a variable
		else if (auto var = vars.find(token); var != vars.end()) {
			if (var->second.type == DT_BOOL || (var->second.type == DT_NUMBER && (var->second.value == "1" || var->second.value == "0"))) {
				continue;
			}
			else {
				return false;
			}
		}
		else {
			return false;
		}
	}
	return true;
}

bool Vars::is_function_expr(vector<string>& tokens)
{
	if (tokens.size() != 1) return false;

	auto expr = Utils::trim(tokens[0]);
	// Check if expression is a function call e.g. namespace::funcName(arg1, arg2)
	if (expr.find("::") != string::npos && expr.find('(') != string::npos && expr.find(')') != string::npos) {
		return true;
	}
	return false;
}

vector<string> Vars::parse_top_level_tokens(const string& expr)
{
	vector<string> tokens;
	string current;
	int paren_depth = 0;
	bool in_quotes = false;

	for (size_t i = 0; i < expr.size(); ++i) {
		char c = expr[i];
		
		if (c == '"' && (i == 0 || expr[i - 1] != '\\')) {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}

		if (!in_quotes) {
			if (c == '(') paren_depth++;
			else if (c == ')') paren_depth--;
			else if (c == '+' && paren_depth == 0) {
				tokens.push_back(Utils::trim(current));
				current.clear();
				continue;
			}
		}

		current += c;
	}

	if (!current.empty()) {
		tokens.push_back(Utils::trim(current));
	}

	return tokens;
}

var Vars::eval_expr(const string& expr, const map<string, var>& vars)
{
	auto outval = string();
	auto tokens = parse_top_level_tokens(expr);

	var result = { "", DT_UNKNOWN };
	for (auto token : tokens) {
		token = Utils::trim(token);
		if (token.empty()) continue;

		// Step 1: Evaluate the token to a var
		var evaledToken = { "", DT_UNKNOWN };
		if (is_function_expr(token)) {
			evaledToken = eval_func_expr(token, vars);
		}
		else if (is_array_expr(token)) {
			evaledToken = eval_array_expr(token, vars);
		}
		else if (Utils::is_string(token)) {
			auto str = Utils::trim_quotes(token);
			str = Utils::escape_str(str);
			evaledToken = { str, DT_STRING };
		}
		else if (Utils::is_number(token)) {
			evaledToken = { token, DT_NUMBER };
		}
		else if (Utils::is_bool(token)) {
			evaledToken = { (token == "true" || token == "1") ? "1" : "0", DT_BOOL };
		}
		else if (vars.find(token) != vars.end()) {
			evaledToken = vars.at(token);
		}
		else {
			Utils::throw_err("Error: Unknown token in expression: " + token);
		}

		// Step 2: Determine how to handle the evaluated token (for this case it's only + operator)
		if (result.type == DT_UNKNOWN) {
			result = evaledToken;
		}
		else {
			if (result.type == DT_STRING || evaledToken.type == DT_STRING) {
				// String concatenation
				result.value += evaledToken.value;
				result.type = DT_STRING;
			}
			else if (result.type == DT_NUMBER && evaledToken.type == DT_NUMBER) {
				// Numeric addition
				int64_t sum = std::stoll(result.value) + std::stoll(evaledToken.value);
				result.value = std::to_string(sum);
				result.type = DT_NUMBER;
			}
			else {
				Utils::throw_err("Error: Incompatible types in expression: " + expr);
			}
		}
	}
	//Utils::print_ln("Evaluated expression: " + expr + " => " + result.value + " (type: " + (result.type == DT_STRING ? "string" : result.type == DT_NUMBER ? "number" : "unknown") + ")");
	return result;
}

var Vars::eval_str_expr(vector<string>& tokens, const map<string, var>& vars)
{
	auto outval = string();
	for (auto& token : tokens) {
		token = Utils::trim(token);
		if (token.empty()) continue;

		if (Utils::is_string(token)) {
			outval += Utils::trim_quotes(token);
		}
		else if (vars.find(token) != vars.end()) {
			outval += vars.at(token).value;
		}
		else {
			Utils::printerr_ln("Error: Unknown token in expression: " + token);
			return var{ "", DT_UNKNOWN };
		}
	}

	return var{ outval, DT_STRING };
}

var Vars::eval_num_expr(vector<string>& tokens, const map<string, var>& vars)
{
	// @var total = 5 + 10;
	// @var total = 5 + 10 + var1 + var2;
	int64_t sum = 0;
	for (auto& token : tokens) {
		token = Utils::trim(token);
		if (token.empty() || token == "+") continue;
		if (Utils::is_number(token)) {
			sum += std::stoll(token);
		}
		else if (vars.find(token) != vars.end()) {
			auto var = vars.at(token);
			if (var.type == DT_NUMBER && Utils::is_number(var.value)) {
				sum += std::stoll(var.value);
			}
			else {
				Utils::printerr_ln("Error: Variable is not a number: " + token);
				return { "", DT_UNKNOWN };
			}
		}
		else {
			Utils::printerr_ln("Error: Unknown token in expression: " + token);
			return { "", DT_UNKNOWN };
		}
	}
	return { std::to_string(sum), DT_NUMBER };
}

var Vars::eval_func_expr(vector<string>& tokens, const map<string, var>& vars)
{
	// e.g. std::toUpper("hello")
	if (tokens.size() != 1) {
		Utils::throw_err("Error: Invalid function expression." );
	}
	auto expr = Utils::trim(tokens[0]);
	auto [namespaceName, functionName, args] = FunctionRegistry::ParseFunctionCall(expr);

	// Prepare funct args
	vector<var> funcArgs;
	for (auto& arg : args) {
		auto evaledArg = eval_expr(arg, vars);
		if (evaledArg.type == DT_UNKNOWN) {
			Utils::throw_err("Error: Failed to evaluate function argument: " + arg);
		}
		funcArgs.push_back(evaledArg);
	}

	// Call function
	if (g_functionRegistry.Exists(namespaceName, functionName)) {
		return g_functionRegistry.CallFunction(namespaceName, functionName, funcArgs);
	}
	else {
		Utils::throw_err("Error: Function not found: " + namespaceName + "::" + functionName);
	}
}

map<string, var> Vars::merge_vars(const map<string, var>& arr1, const map<string, var>& arr2)
{
	map<string, var> result = arr1;

	for (auto& [key, value] : arr2) {
		result[key] = value;
	}

	return result;
}

bool Vars::is_function_expr(const string& token)
{
	if (token.find("::") != string::npos && token.find('(') != string::npos && token.find(')') != string::npos) {
		return true;
	}
	return false;
}

var Vars::eval_func_expr(const string& token, const map<string, var>& vars)
{
	auto expr = Utils::trim(token);
	auto [namespaceName, functionName, args] = FunctionRegistry::ParseFunctionCall(expr);

	// Prepare funct args
	vector<var> funcArgs;
	for (auto& arg : args) {
		auto evaledArg = eval_expr(arg, vars);
		if (evaledArg.type == DT_UNKNOWN) {
			Utils::throw_err("Error: Failed to evaluate function argument: " + arg);
		}
		funcArgs.push_back(evaledArg);
	}

	// Call function
	if (g_functionRegistry.Exists(namespaceName, functionName)) {
		return g_functionRegistry.CallFunction(namespaceName, functionName, funcArgs);
	}
	else {
		Utils::throw_err("Error: Function not found: " + namespaceName + "::" + functionName);
	}
}

bool Vars::is_array_expr(const std::string& token)
{
	if (Utils::starts_with(token, "[") && Utils::ends_with(token, "]")) {
		return true;
	}
	return false;
}

var Vars::eval_array_expr(const std::string& token, const std::map<std::string, var>& vars)
{
	// e.g. [ "item1", "item2", var1, var2 ]

	vector<string> tokens;
	bool in_quotes = false;
	bool in_brackets = false;
	int paren_depth = 0;

	string current;
	for (char c : token) {
		if (c == '"' && (current.empty() || current.back() != '\\')) {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}
		if (!in_quotes) {
			if (c == '[') {
				in_brackets = true;
				continue;
			}
			else if (c == ']') {
				in_brackets = false;
				continue;
			}
			else if (c == '(') paren_depth++;
			else if (c == ')') paren_depth--;
			else if (c == ',' && paren_depth == 0 && in_brackets) {
				tokens.push_back(Utils::trim(current));
				current.clear();
				continue;
			}
		}
		current += c;
	}
	if (!current.empty()) {
		tokens.push_back(Utils::trim(current));
	}

	var result;
	result.type = DT_ARRAY;
	for (auto& item : tokens) {
		auto evaledItem = eval_expr(item, vars);
		if (evaledItem.type == DT_UNKNOWN) {
			Utils::throw_err("Error: Failed to evaluate array item: " + item);
		}
		result.array.push_back(evaledItem);
	}
	return result;
}
