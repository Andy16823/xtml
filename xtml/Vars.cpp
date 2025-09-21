#include "Vars.h"
#include "Utils.h"
#include <sstream>
#include <tuple>
#include <algorithm>
#include <cctype>
#include <vector>
#include <iterator>
#include <cstring>


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

map<string, var> Vars::parse_vars(const std::string& content)
{
	map<string, var> vars;
	// split parts on ;
	auto segments = Utils::split(content, ';');
	// Parse each line for @var declarations
	for (auto& line : segments) {
		line = Utils::trim(line);
		if (line.empty() || line[0] == '#' || line.rfind("@var") == string::npos) continue; // Skip comments and non-var lines
		line = trim_var(line);
		std::tuple<std::string, std::string> parsedVar = parse_var(line);
		std::string key = std::get<0>(parsedVar);
		std::string value = std::get<1>(parsedVar);
		var varval = eval_expr(value, vars);

		if (!key.empty() && varval.type != DT_UNKNOWN) {
			vars[key] = varval;
		}
	}
	return vars;
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

var Vars::eval_expr(const string& expr, const map<string, var>& vars)
{
	auto outval = string();
	auto tokens = parse_tokens(expr, "+", false); 

	if (is_string_expr(tokens, vars)) {
		return eval_str_expr(tokens, vars);
	}
	else if (is_numeric_expr(tokens, vars)) {
		return eval_num_expr(tokens, vars);
	}
	else {

		Utils::printerr_ln("Error: Unsupported expression type: " + expr);
		return var{ "", DT_UNKNOWN };
	}
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
