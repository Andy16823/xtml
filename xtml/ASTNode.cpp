#include "ASTNode.h"
#include "Vars.h"
#include "Utils.h"
#include "Statements.h"
#include <map>
#include <string>
#include "Core.h"

using namespace std;

std::string VarDeclNode::evaluate(std::map<std::string, var>& vars)
{
	var value = Vars::eval_expr(m_expr, vars);
	if (value.type != DT_UNKNOWN) {
		vars[m_name] = value;
	}
	return "";
}

std::string ASTRoot::evaluate()
{
	std::string result;
	for (auto& child : children) {
		result += child->evaluate(vars);
	}
	return result;
}

std::string BlockNode::evaluate(std::map<std::string, var>& vars)
{
	std::string result;
	for (auto& child : children) {
		result += child->evaluate(vars);
	}
	return result;
}

std::string IfStatementNode::evaluate(std::map<std::string, var>& vars)
{
	// Resolve statement and pushback new children for the met branch
	for (auto& if_branch : m_if_stmt.branches) {
		Utils::print_ln("If branch condition: " + if_branch.condition);
		if (Statements::evaluate_condition(if_branch.condition, if_branch.content, vars)) {
			Utils::print_ln("Condition met, processing branch content.");
			auto statements = Core::split_statements(if_branch.content);
			auto child_branches = Core::parse_ast_statements(statements);
			for (auto& child : child_branches) {
				this->children.push_back(move(child));
			}
			break;
		}
	}

	// Evaluate children
	std::string result;
	for (auto& child : children) {
		result += child->evaluate(vars);
	}
	return result;
}

std::string TextNode::evaluate(std::map<std::string, var>& vars)
{
	auto value = Vars::eval_expr(m_value, vars);
	if (value.type != DT_UNKNOWN) {
		return value.value;
	}
	return std::string();
}
