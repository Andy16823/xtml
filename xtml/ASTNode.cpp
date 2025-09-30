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
	// Evaluate children
	std::string result;

	bool resolved = false;
	for (auto& if_branch : m_if_stmt.branches) {
		Utils::print_ln("If branch condition: " + if_branch.condition);
		if (Statements::evaluate_condition(if_branch.condition, if_branch.content, vars)) {
			Utils::print_ln("Condition met, processing branch content.");

			auto& children = m_branch_nodes[if_branch.uuid];
			for (auto& child : children) {
				result += child->evaluate(vars);
			}
			resolved = true;
			break;
		}
	}

	// Else branch
	if (!resolved && m_if_stmt.has_else) {
		Utils::print_ln("No conditions met, processing else branch.");

		auto& children = m_branch_nodes[m_if_stmt.uuid];
		for (auto& child : children) {
			result += child->evaluate(vars);
		}
	}

	return result;
}

void IfStatementNode::parse_braches()
{
	for (auto& if_branch : m_if_stmt.branches) {
		if (if_branch.uuid.empty()) {
			if_branch.uuid = Utils::generate_uuid();
		}
		auto statements = Core::split_statements(if_branch.content);
		auto childs = Core::parse_ast_statements(statements);
		this->m_branch_nodes[if_branch.uuid] = std::move(childs);
	}

	if (m_if_stmt.has_else) {
		if (m_if_stmt.uuid.empty()) {
			m_if_stmt.uuid = Utils::generate_uuid();
		}
		auto statements = Core::split_statements(m_if_stmt.else_content);
		auto childs = Core::parse_ast_statements(statements);
		this->m_branch_nodes[m_if_stmt.uuid] = std::move(childs);
	}
}

std::string TextNode::evaluate(std::map<std::string, var>& vars)
{
	auto value = Vars::eval_expr(m_value, vars);
	if (value.type != DT_UNKNOWN) {
		return value.value;
	}
	return std::string();
}
