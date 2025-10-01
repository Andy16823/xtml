#pragma once
#include <string>
#include <vector>
#include <map>
#include "Vars.h"
#include <memory>

class ASTNode;

enum ConditionOp {
	OP_AND,
	OP_OR,
	OP_NONE
};

class Statements
{
	
public:
	static std::vector<std::string> split_conditions(const std::string& condition);
	static std::vector<ConditionOp> parse_condition_ops(const std::string& condition);
	static std::vector<std::string> tokenize_condition(const std::string& condition);
	static bool resolve_conditions(const std::vector<std::string>& conditions, const std::vector<ConditionOp>& ops, const std::map<std::string, var>& vars);
	static bool resolve_condition(const std::string& condition, const std::map<std::string, var>& vars);
	static bool evaluate_condition(const std::string& condition_str, const std::string& content_str, std::map<std::string, var>& vars);
};

