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

void IfStatementNode::parse_branch(Branch& branch)
{
	auto statements = Core::split_statements(branch.content);
	auto childs = Core::parse_ast_statements(statements);
	for (auto& child : childs) {
		branch.children.push_back(std::move(child));
	}
}

IfStatementNode::IfStatementNode()
{
	this->m_has_else = false;
}

void IfStatementNode::add_branch(std::string condition, std::string content)
{
	Branch elif_branch;
	elif_branch.condition = condition;
	elif_branch.content = content;
	this->parse_branch(elif_branch);
	this->m_branches.push_back(std::move(elif_branch));
}

void IfStatementNode::add_else(std::string content)
{
	this->m_has_else = true;
	this->m_else_branch.content = content;
	this->m_else_branch.condition = "else";

	auto statements = Core::split_statements(content);
	auto childs = Core::parse_ast_statements(statements);
	for (auto& child : childs) {
		this->m_else_branch.children.push_back(std::move(child));
	}
}

std::string IfStatementNode::evaluate(std::map<std::string, var>& vars)
{
	// Evaluate children
	std::string result;

	bool resolved = false;
	for (auto& if_branch : this->m_branches) {
		Utils::print_ln("If branch condition: " + if_branch.condition);
		if (Statements::evaluate_condition(if_branch.condition, if_branch.content, vars)) {
			Utils::print_ln("Condition met, processing branch content.");
			for (auto& child : if_branch.children) {
				result += child->evaluate(vars);
			}
			resolved = true;
			break;
		}
	}

	// Else branch
	if (!resolved && this->m_has_else) {
		Utils::print_ln("No conditions met, processing else branch.");
		for (auto& child : this->m_else_branch.children) {
			result += child->evaluate(vars);
		}
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

WhileNode::WhileNode(const std::string& condition, const std::string& body)
{
	m_condition = condition;
	auto statments = Core::split_statements(body);
	auto childs = Core::parse_ast_statements(statments);
	for (auto& child : childs) {
		children.push_back(std::move(child));
	}
	Utils::printerr_ln("WhileNode created with condition: " + children.size());
}

std::string WhileNode::evaluate(std::map<std::string, var>& vars)
{
	Utils::printerr_ln("Evaluating WhileNode with condition: " + m_condition);
	std::string result;
	while (Statements::evaluate_condition(m_condition, "", vars)) {
		Utils::printerr_ln("While condition met, processing body.");
		for (auto& child : children) {
			result += child->evaluate(vars);
		}
	}

	return result;
}

void ForNode::parse_loop(const std::string& loop_expr, const std::string& body)
{

}

ForNode::ForNode(const std::string& loop_expr, const std::string& body)
{
	auto expressions = Core::split_statements(loop_expr);
	if (expressions.size() != 3) {
		Utils::throw_err("Error: Invalid for loop expression: " + loop_expr);
		return;
	}
	m_init = Vars::trim_var(expressions[0]);
	m_condition = Vars::trim_var(expressions[1]);
	m_increment = Vars::trim_var(expressions[2]);

	auto statments = Core::split_statements(body);
	auto childs = Core::parse_ast_statements(statments);
	for (auto& child : childs) {
		children.push_back(std::move(child));
	}
}

std::string ForNode::evaluate(std::map<std::string, var>& vars)
{
	// 1. Prepare the loop variable
	auto [key, value] = Vars::parse_var(m_init);
	auto var = Vars::eval_expr(value, vars);
	if (var.type == DT_UNKNOWN) {
		Utils::throw_err("Error: Failed to evaluate for loop init expression: " + m_init);
		return std::string();
	}
	vars[key] = var;

	// 2. Execute the loop
	std::string result;
	while (Statements::evaluate_condition(m_condition, "", vars)) {
		for (auto& child : children) {
			result += child->evaluate(vars);
		}

		auto [inc_key, inc_value] = Vars::parse_var(m_increment);
		auto inc_var = Vars::eval_expr(inc_value, vars);
		if (inc_var.type == DT_UNKNOWN) {
			Utils::throw_err("Error: Failed to evaluate for loop increment expression: " + m_increment);
			return std::string();
		}
		vars[inc_key] = inc_var;
	}

	return result;
}
