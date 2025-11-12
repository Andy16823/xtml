#include "ASTNode.h"
#include "Vars.h"
#include "Utils.h"
#include "Statements.h"
#include <map>
#include <string>
#include "Core.h"
#include <iostream>
#include <sstream>

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
	// Declare a variable
	var value;
	if(this->expression == nullptr) {
		value.value = "";
		value.type = DT_UNKNOWN;
		return EvalResult{}; // No expression to evaluate
	}

	// Evaluate the expression and assign to variable
	EvalResult eval = expression->evaluate(vars);
	value.value = eval.content;
	value.type = Utils::predictVarType(value.value);
	vars[name] = value;

	// No output for variable declaration
	return {};
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

IfStatementNode::IfStatementNode()
{
	
}

EvalResult IfStatementNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;

	// Try if condition
	auto condResult = condition->evaluate(vars);
	if(condResult.content == "true") {
		result = body->evaluate(vars);
		return result;	// Return because if was executed
	}
	
	// Try else ifs
	for (auto& elseif : elseIfs) {
		auto elseIfresult = elseif->condition->evaluate(vars);
		if (elseIfresult.content == "true") {
			result = elseif->body->evaluate(vars);
			return result; // Return because else if was executed
		}
	}

	// Try else body
	if(elseBody != nullptr) {
		result = elseBody->evaluate(vars);
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

EvalResult WhileNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	return result;
}

EvalResult ForNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;

	// Declare init statement
	if (init != nullptr) {
		init->evaluate(vars);	// Initialize loop variable
	}
	else {
		Utils::throw_err("Error: For loop missing initialization statement.");
	}

	// Evaluate condition
	auto condResult = condition->evaluate(vars);
	while (condResult.content == "true") {
		auto bodyResult = body->evaluate(vars);
		result = merge_results(result, bodyResult);
		// TODO: Handle break/continue
		
		// Execute increment
		if (increment != nullptr) {
			increment->evaluate(vars);
		}
		else {
			Utils::throw_err("Error: For loop missing increment statement.");
		}

		// Re-evaluate condition
		condResult = condition->evaluate(vars);
	}

	return result;
}

EvalResult ForEachNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
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

EvalResult FunctionNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;

	// First move global vars to local scope
	this->localVars = Vars::merge_vars(vars, this->localVars);

	// Evaluate body
	result = merge_results(result, this->body->evaluate(vars));

	// Merge back local vars to global scope
	vars = Vars::merge_vars(vars, this->localVars);

	// Return result
	return result;
}

EvalResult BinaryExprNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;

	auto optype = Vars::get_binary_op_type(op);
	// Early check for unknown operation
	if (optype == BinaryOpType::Unknown) {
		Utils::throw_err("Error: Unknown binary operation: " + op);
	}

	// Check for assignment operation
	if (optype == BinaryOpType::Assignment) {
		// Assignment operation must be an var on the left side
		auto leftVar = dynamic_cast<VarExprNode*>(left.get());
		if (leftVar == nullptr) {
			Utils::throw_err("Error: Left side of assignment must be a variable.");
		}
		auto rightResult = right->evaluate(vars);
		if(vars.find(leftVar->name) == vars.end()) {
			Utils::throw_err("Error: Undefined variable: " + leftVar->name);
		}
		vars[leftVar->name].value = rightResult.content;
		return rightResult; // Return the assigned value
	}

	// Evaluate left and right expressions
	auto leftResult = left->evaluate(vars);
	auto rightResult = right->evaluate(vars);

	// Create temp vars
	var leftVar = var{ leftResult.content, Utils::predictVarType(leftResult.content) };
	var rightVar = var{ rightResult.content, Utils::predictVarType(rightResult.content) };

	if (optype == BinaryOpType::Comparison) {
		bool cmpResult = Vars::compareOperation(leftVar, rightVar, op);
		result.content = cmpResult ? "true" : "false";
		return result;
	}	

	// Musst be arithmetic or other binary operation
	var opresult = Vars::binaryOperation(leftVar, rightVar, op);

	// Return result
	result.content = opresult.value;
	return result;
}

EvalResult IntegerLiteralNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.content = std::to_string(value);
	return result;
}

EvalResult StringLiteralNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.content = value;
	return result;
}

EvalResult FloatLiteralNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.content = std::to_string(value);
	return result;
}

EvalResult BoolLiteralNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.content = value ? "true" : "false";
	return result;
}

EvalResult DoubleLiteralNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.content = std::to_string(value);
	return result;
}

EvalResult VarExprNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	if(vars.find(this->name) != vars.end()) {
		result.content = vars[this->name].value;
	}
	else {
		Utils::throw_err("Error: Undefined variable: " + this->name);
	}
	return result;
}

EvalResult StmtNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	return result;
}

EvalResult ExprNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	return result;
}

EvalResult ExprStatementNode::evaluate(std::map<std::string, var>& vars)
{
	if (auto bexpr = dynamic_cast<BinaryExprNode*>(expression.get())) {
		if (bexpr->op == "=") {
			// Assignment operation
			if (auto vv = dynamic_cast<VarExprNode*>(bexpr->left.get())) {
				auto rightResult = bexpr->right->evaluate(vars);
				vars[vv->name] = { rightResult.content, DT_STRING }; // Assume string type for simplicity
				EvalResult result;
				return result;
			}
			Utils::throw_err("Error: Left side of assignment must be a variable.");
		}
	}
	Utils::throw_err("Error: Unsupported expression statement.");
}

EvalResult HtmlStmtNode::evaluate(std::map<std::string, var>& vars)
{
	std::ostringstream out;
	out << "<" << tagName;
	for (auto& [key, value] : attributes) {
		std::string exvalue = value;
		out << " " << key << "=\"" << exvalue << "\"";
	}
	
	if (selfClosing) {
		out << " />";
	}
	else {
		out << ">";
		for (auto& child : this->children) {
			EvalResult childResult = child->evaluate(vars);
			out << childResult.content;
		}
		out << "</" << tagName << ">";
	}

	EvalResult result;
	result.content = out.str();
	result.content = Core::resolve_placeholders(result.content, vars);
	return result;
}

EvalResult HtmlStmtRootNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result = body->evaluate(vars);
	return result;
}

EvalResult HtmlTextNode::evaluate(std::map<std::string, var>& vars)
{
	EvalResult result;
	result.content = content;
	return result;
}

EvalResult UnaryExprNode::evaluate(std::map<std::string, var>& vars)
{
	// Check that operand is a variable
	auto varexpr = dynamic_cast<VarExprNode*>(operand.get());
	if(varexpr == nullptr) {
		Utils::throw_err("Error: Unary operation must be on a variable.");
	}
	auto operandResult = varexpr->evaluate(vars);

	// Check that variable exists
	if(vars.find(varexpr->name) == vars.end()) {
		Utils::throw_err("Error: Undefined variable: " + varexpr->name);
	}
	auto varRef = vars[varexpr->name];

	// Perform unary operation and update variable
	auto value = Vars::unaryOperation(varRef, op);
	vars[varexpr->name] = value;

	// Return the new value
	EvalResult result;

	if(this->isPrefix) {
		result.content = value.value;
	} else {
		result.content = operandResult.content; 
	}
	return result;
}
