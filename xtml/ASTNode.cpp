#include "ASTNode.h"
#include "Vars.h"
#include "Utils.h"
#include "Statements.h"
#include <map>
#include <string>
#include "Core.h"

using namespace std;

EvalResult ASTNode::merge_results(const EvalResult& a, const EvalResult& b)
{
	EvalResult result;
	result.content = a.content + b.content;

	if (a.should_break || b.should_break) {
		result.should_break = true;
	}else if (a.should_continue || b.should_continue) {
		result.should_continue = true;
	}
	return result;
}

EvalResult VarDeclNode::evaluate(std::map<std::string, var>& vars)
{
	var value = Vars::eval_expr(m_expr, vars);
	if (value.type != DT_UNKNOWN) {
		vars[m_name] = value;
	}
	return EvalResult{};
}

EvalResult ASTRoot::evaluate()
{
	EvalResult result;
	for (auto& child : children) {
		result.content += child->evaluate(vars).content;
	}
	return result;
}

EvalResult BlockNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	for (auto& child : children) {
		result = merge_results(result, child->evaluate(vars));
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

EvalResult IfStatementNode::evaluate(std::map<std::string, var>& vars)
{
	// Evaluate children
	EvalResult result;

	bool resolved = false;
	for (auto& if_branch : this->m_branches) {
		if (Statements::evaluate_condition(if_branch.condition, if_branch.content, vars)) {
			for (auto& child : if_branch.children) {
				result = merge_results(result, child->evaluate(vars));
			}
			resolved = true;
			break;
		}
	}

	// Else branch
	if (!resolved && this->m_has_else) {
		for (auto& child : this->m_else_branch.children) {
			result = merge_results(result, child->evaluate(vars));
		}
	}
	return result;
}

EvalResult TextNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	auto value = Vars::eval_expr(m_value, vars);
	if (value.type != DT_UNKNOWN) {
		result.content = value.value;
	}
	return result;
}

WhileNode::WhileNode(const std::string& condition, const std::string& body)
{
	m_condition = condition;
	auto statments = Core::split_statements(body);
	auto childs = Core::parse_ast_statements(statments);
	for (auto& child : childs) {
		children.push_back(std::move(child));
	}
}

EvalResult WhileNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	while (Statements::evaluate_condition(m_condition, "", vars)) {
		for (auto& child : children) {
			auto child_result = child->evaluate(vars);
			if (child_result.should_break) {
				result = merge_results(result, child_result);
				result.should_break = false;
				return result;
			}
			else if (child_result.should_continue) {
				result = merge_results(result, child_result);
				result.should_continue = false;
				break;
			}

			result = merge_results(result, child_result);
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

EvalResult ForNode::evaluate(std::map<std::string, var>& vars)
{
	// 1. Prepare the loop variable
	auto [key, value] = Vars::parse_var(m_init);
	auto var = Vars::eval_expr(value, vars);
	if (var.type == DT_UNKNOWN) {
		Utils::throw_err("Error: Failed to evaluate for loop init expression: " + m_init);
	}
	vars[key] = var;

	// 2. Execute the loop
	EvalResult result;
	while (Statements::evaluate_condition(m_condition, "", vars)) {
		for (auto& child : children) {
			auto child_result = child->evaluate(vars);
			if (child_result.should_break) {
				result = merge_results(result, child_result);
				result.should_break = false;
				return result;
			}
			else if (child_result.should_continue) {
				result = merge_results(result, child_result);
				result.should_continue = false;
				break;
			}

			result = merge_results(result, child_result);
		}

		auto [inc_key, inc_value] = Vars::parse_var(m_increment);
		auto inc_var = Vars::eval_expr(inc_value, vars);
		if (inc_var.type == DT_UNKNOWN) {
			Utils::throw_err("Error: Failed to evaluate for loop increment expression: " + m_increment);
		}
		vars[inc_key] = inc_var;
	}

	return result;
}

std::tuple<std::string, std::string> ForEachNode::parse_declaration(const std::string& declaration)
{
	auto parts = Utils::split(declaration, ' in ');
	if (parts.size() != 3) {
		Utils::throw_err("Error: Invalid foreach declaration: " + declaration);
	}
	auto left = Vars::trim_var(parts[0]);
	auto right = Vars::trim_var(parts[2]);

	return std::make_tuple(left, right);
}

ForEachNode::ForEachNode(const std::string& expression, const std::string& body)
{
	auto [declaration, collection] = parse_declaration(expression);
	m_declaration = declaration;
	m_collection = collection;
	auto statments = Core::split_statements(body);
	auto childs = Core::parse_ast_statements(statments);
	for (auto& child : childs) {
		children.push_back(std::move(child));
	}
}

EvalResult ForEachNode::evaluate(std::map<std::string, var>& vars)
{
	var collection_var = Vars::eval_expr(m_collection, vars);
	if (collection_var.type != DT_ARRAY) {
		Utils::throw_err("Error: Foreach collection is not an array: " + m_collection);
	}

	// Iteralte all elements in the array
	EvalResult result;
	for (const auto& item : collection_var.array) {
		vars[m_declaration] = item;
		for (auto& child : children) {
			auto child_result = child->evaluate(vars);
			if (child_result.should_break) {
				result = merge_results(result, child_result);
				result.should_break = false;
				return result;
			}
			else if (child_result.should_continue) {
				result = merge_results(result, child_result);
				result.should_continue = false;
				break;
			}

			result = merge_results(result, child_result);
		}
	}
	return result;
}

EvalResult BreakNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.should_break = true;
	return result;
}

EvalResult ContinueNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.should_continue = true;
	return result;
}

