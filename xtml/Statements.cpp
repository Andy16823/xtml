#include "Statements.h"
#include <regex>
#include "Utils.h"
#include "Vars.h"
#include "Core.h"

using namespace std;

/// <summary>
/// Split conditions by && and || while respecting quotes and parentheses
/// </summary>
/// <param name="condition"></param>
/// <returns></returns>
std::vector<std::string> Statements::split_conditions(const std::string& condition)
{
	std::vector<std::string> conditions;
	string current;
	bool in_quotes = false;
	int paren_depth = 0;

	// Step 1: Split statements by && and ||
	for (char c : condition) {
		if (c == '"' || c == '\'') {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}
		if (!in_quotes) {
			if (c == '(') {
				paren_depth++;
			}
			else if (c == ')') {
				paren_depth--;
			}
			else if ((c == '&' || c == '|') && paren_depth == 0) {
				if (!current.empty()) {
					conditions.push_back(Utils::trim(current));
					current.clear();
				}
				// Skip the next character as well for && or ||
				continue;
			}
		}
		current += c;
	}
	if (!current.empty()) {
		conditions.push_back(Utils::trim(current));
	}

	// Step 2: Remove surrounding parentheses from each condition
	for (auto& cond : conditions) {
		if (cond.front() == '(' && cond.back() == ')') {
			cond = Utils::trim(Utils::parse_parantheses(cond));
		}
	}


	return conditions;
}

/// <summary>
/// Parse condition operators (&&, ||) from a condition string
/// </summary>
/// <param name="condition"></param>
/// <returns></returns>
std::vector<ConditionOp> Statements::parse_condition_ops(const std::string& condition)
{
	vector<ConditionOp> ops;
	string current;
	bool in_quotes = false;
	int paren_depth = 0;

	for (char c : condition) {
		if (c == '"' || c == '\'') {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}
		if (!in_quotes) {
			if (c == '(') {
				paren_depth++;
				if (paren_depth == 1) continue;
			}
			else if (c == ')') {
				paren_depth--;
				if (paren_depth == 0) continue;
			}
			else if (c == '&' && paren_depth == 0) {
				current.push_back(c);
				continue;
			}
			else if (c == '|' && paren_depth == 0) {
				current.push_back(c);
				continue;
			}
		}

		if (current == "&&") {
			ops.push_back(OP_AND);
			current.clear();
		}
		else if (current == "||") {
			ops.push_back(OP_OR);
			current.clear();
		}
		else {
			current.clear();
		}
	}

	return ops;
}

/// <summary>
/// Tokenize a condition string into individual tokens
/// </summary>
/// <param name="condition"></param>
/// <returns></returns>
std::vector<std::string> Statements::tokenize_condition(const std::string& condition)
{
	vector<string> tokens;
	string current;
	bool in_quotes = false;
	for (char c : condition) {
		if (c == '"' || c == '\'') {
			in_quotes = !in_quotes;
			current += c;
			continue;
		}
		if (!in_quotes && isspace(c)) {
			if (!current.empty()) {
				tokens.push_back(Utils::trim(current));
				current.clear();
			}
			continue;
		}
		current += c;
	}

	if (!current.empty()) {
		tokens.push_back(Utils::trim(current));
	}

	return tokens;
}

/// <summary>
/// Resolve multiple conditions combined with AND/OR
/// </summary>
/// <param name="conditions"></param>
/// <param name="ops"></param>
/// <param name="vars"></param>
/// <returns></returns>
bool Statements::resolve_conditions(const std::vector<std::string>& conditions, const std::vector<ConditionOp>& ops, const std::map<std::string, var>& vars)
{
	if (conditions.empty()) return false;
	if (conditions.size() != ops.size() + 1) {
		Utils::throw_err("Error: Mismatched conditions and operators.");
		return false;
	}
	std::vector<bool> cond_results;
	// Resolve each condition
	for (const auto& condition : conditions) {
		auto condition_trimmed = Utils::trim(condition);
		bool cond_result = resolve_condition(condition_trimmed, vars);
		cond_results.push_back(cond_result);
	}

	// Combine condition results based on operators
	bool final_result = cond_results[0];
	for (size_t i = 0; i < ops.size(); ++i) {
		if (ops[i] == OP_AND) {
			final_result = final_result && cond_results[i + 1];
		}
		else if (ops[i] == OP_OR) {
			final_result = final_result || cond_results[i + 1];
		}
	}

	return final_result;
}

/// <summary>
/// Resolve a condition string to a boolean value
/// </summary>
/// <param name="condition"></param>
/// <param name="vars"></param>
/// <returns></returns>
bool Statements::resolve_condition(const std::string& condition, const std::map<std::string, var>& vars)
{
	bool result = false;
	auto tokens = tokenize_condition(condition);
	var left, right;

	if (tokens.size() != 3) {
		// Multi-part condition resolve
		auto conds = split_conditions(condition);
		auto cond_ops = parse_condition_ops(condition);
		return resolve_conditions(conds, cond_ops, vars);
	}

	left = Vars::eval_expr(tokens[0], vars);
	right = Vars::eval_expr(tokens[2], vars);
	std::string op = tokens[1];

	if (left.type == DT_UNKNOWN || right.type == DT_UNKNOWN) {
		Utils::throw_err("Error: Unknown variable in condition: " + condition);
		return false;
	}

	if (left.type != right.type) {
		Utils::throw_err("Error: Type mismatch in condition: " + condition);
		return false;
	}

	if (left.type == DT_STRING) {
		if (op == "==") return left.value == right.value;
		if (op == "!=") return left.value != right.value;
		Utils::throw_err("Error: Invalid operator for string comparison: " + op);
		return false;
	}
	else if (left.type == DT_NUMBER) {
		int64_t left_num = std::stoll(left.value);
		int64_t right_num = std::stoll(right.value);
		if (op == "==") return left_num == right_num;
		if (op == "!=") return left_num != right_num;
		if (op == "<") return left_num < right_num;
		if (op == "<=") return left_num <= right_num;
		if (op == ">") return left_num > right_num;
		if (op == ">=") return left_num >= right_num;
		Utils::throw_err("Error: Invalid operator for numeric comparison: " + op);
		return false;
	}
	else if (left.type == DT_BOOL) {
		bool left_bool = (left.value == "true");
		bool right_bool = (right.value == "true");
		if (op == "==") return left_bool == right_bool;
		if (op == "!=") return left_bool != right_bool;
		Utils::throw_err("Error: Invalid operator for boolean comparison: " + op);
		return false;
	}

	return false;
}


bool Statements::evaluate_condition(const std::string& condition_str, const std::string& content_str, std::map<std::string, var>& vars)
{
	auto condition = Utils::trim(condition_str);
	if (condition.empty()) {
		Utils::throw_err("Error: Empty condition in if statement.");
		return false;
	}

	vector<bool> cond_results;
	auto conditions = split_conditions(condition);
	auto ops = parse_condition_ops(condition);
	bool final_result = resolve_conditions(conditions, ops, vars);

	if (final_result) {
		return true;
	}
	return false;
}
