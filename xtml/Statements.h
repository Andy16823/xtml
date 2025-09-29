#pragma once
#include <string>
#include <vector>
#include <map>
#include "Vars.h"

enum ConditionOp {
	OP_AND,
	OP_OR,
	OP_NONE
};

struct IfBranch {
	std::string condition;
	std::string content;
};

struct IfStatement {
	std::vector<IfBranch> branches;
	bool has_else = false;
	std::string else_content;
};

class Statements
{
	
public:
	static std::vector<std::string> split_conditions(const std::string& condition);
	static std::vector<ConditionOp> parse_condition_ops(const std::string& condition);
	static std::vector<std::string> tokenize_condition(const std::string& condition);
	static bool resolve_conditions(const std::vector<std::string>& conditions, const std::vector<ConditionOp>& ops, const std::map<std::string, var>& vars);
	static bool resolve_condition(const std::string& condition, const std::map<std::string, var>& vars);
	static std::string parse_statement_condition(const std::string& statement);
	static std::map<std::string, var> resolve_if_statement(const IfStatement& if_stmt, std::map<std::string, var>& vars);
};

