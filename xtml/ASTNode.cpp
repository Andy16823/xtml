#include "ASTNode.h"
#include "Vars.h"
#include "Utils.h"
#include "Statements.h"
#include <map>
#include <string>
#include "Core.h"
#include <iostream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"

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

EvalResult VarDeclNode::evaluate(Program& program)
{
	// Declare a variable
	var value;
	if(this->expression == nullptr) {
		value.value = "";
		value.type = DT_UNKNOWN;
		return EvalResult{}; // No expression to evaluate
	}

	// Evaluate the expression and assign to variable
	EvalResult eval = expression->evaluate(program);
	value.value = eval.content;
	value.type = Utils::predictVarType(value.value);
	program.vars[name] = value;

	// No output for variable declaration
	return {};
}

EvalResult BlockNode::evaluate(Program& program)
{
	EvalResult result;
	for (auto& child : children) {
		result = merge_results(result, child->evaluate(program));
	}
	return result;
}

IfStatementNode::IfStatementNode()
{
	
}

EvalResult IfStatementNode::evaluate(Program& program)
{
	EvalResult result;

	// Try if condition
	auto condResult = condition->evaluate(program);
	if(condResult.content == "true") {
		result = body->evaluate(program);
		return result;	// Return because if was executed
	}
	
	// Try else ifs
	for (auto& elseif : elseIfs) {
		auto elseIfresult = elseif->condition->evaluate(program);
		if (elseIfresult.content == "true") {
			result = elseif->body->evaluate(program);
			return result; // Return because else if was executed
		}
	}

	// Try else body
	if(elseBody != nullptr) {
		result = elseBody->evaluate(program);
	}
	return result;
}

EvalResult TextNode::evaluate(Program& program)
{
	//TODO: Refactor here to ???
	EvalResult result;
	auto value = Vars::eval_expr(m_value, program.vars);
	if (value.type != DT_UNKNOWN) {
		result.content = value.value;
	}
	return result;
}

EvalResult WhileNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult ForNode::evaluate(Program& program)
{
	EvalResult result;

	// Declare init statement
	if (init != nullptr) {
		init->evaluate(program);	// Initialize loop variable
	}
	else {
		Utils::throw_err("Error: For loop missing initialization statement.");
	}

	// Evaluate condition
	auto condResult = condition->evaluate(program);
	while (condResult.content == "true") {
		auto bodyResult = body->evaluate(program);
		result = merge_results(result, bodyResult);
		// TODO: Handle break/continue
		
		// Execute increment
		if (increment != nullptr) {
			increment->evaluate(program);
		}
		else {
			Utils::throw_err("Error: For loop missing increment statement.");
		}

		// Re-evaluate condition
		condResult = condition->evaluate(program);
	}

	return result;
}

EvalResult ForEachNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult BreakNode::evaluate(Program& program)
{
	EvalResult result;
	result.should_break = true;
	return result;
}

EvalResult ContinueNode::evaluate(Program& program)
{
	EvalResult result;
	result.should_continue = true;
	return result;
}

EvalResult FunctionNode::evaluate(Program& program)
{
	EvalResult result;
	// Evaluate body
	result = merge_results(result, this->body->evaluate(program));
	// Return result
	return result;
}

EvalResult FunctionNode::callFunction(Program& program, const std::vector<std::string>& argValues)
{
	// Bind arguments to local program variables
	for (size_t i = 0; i < this->arguments.size(); i++) {
		auto argExpr = dynamic_cast<VarExprNode*>(this->arguments[i].get());
		if (argExpr == nullptr) {
			Utils::throw_err("Error: Function argument must be a variable.");
		}
		var value;
		value.value = argValues[i];
		value.type = Utils::predictVarType(value.value);
		this->localProgram.vars[argExpr->name] = value;
	}

	// Bind global functions to local program
	Core::mergeProgrammFunctions(program, localProgram);

	return this->body->evaluate(this->localProgram);
}

EvalResult BinaryExprNode::evaluate(Program& program)
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
		auto rightResult = right->evaluate(program);
		if(program.vars.find(leftVar->name) == program.vars.end()) {
			Utils::throw_err("Error: Undefined variable: " + leftVar->name);
		}
		program.vars[leftVar->name].value = rightResult.content;
		return rightResult; // Return the assigned value
	}

	// Evaluate left and right expressions
	auto leftResult = left->evaluate(program);
	auto rightResult = right->evaluate(program);

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

EvalResult IntegerLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.content = std::to_string(value);
	return result;
}

EvalResult StringLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.content = value;
	return result;
}

EvalResult FloatLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.content = std::to_string(value);
	return result;
}

EvalResult BoolLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.content = value ? "true" : "false";
	return result;
}

EvalResult DoubleLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.content = std::to_string(value);
	return result;
}

EvalResult VarExprNode::evaluate(Program& program)
{
	EvalResult result;
	if(program.vars.find(this->name) != program.vars.end()) {
		result.content = program.vars[this->name].value;
	}
	else {
		Utils::throw_err("Error: Undefined variable: " + this->name);
	}
	return result;
}

EvalResult StmtNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult ExprNode::evaluate(Program& program)
{
	EvalResult result;
	return result;
}

EvalResult ExprStatementNode::evaluate(Program& program)
{
	return this->expression->evaluate(program);
}

EvalResult HtmlStmtNode::evaluate(Program& program)
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
			EvalResult childResult = child->evaluate(program);
			out << childResult.content;
		}
		out << "</" << tagName << ">";
	}

	EvalResult result;
	result.content = out.str();
	return result;
}

EvalResult HtmlStmtRootNode::evaluate(Program& program)
{
	EvalResult result;
	result = body->evaluate(program);
	return result;
}

EvalResult HtmlTextNode::evaluate(Program& program)
{
	EvalResult result;
	result.content = content;
	return result;
}

EvalResult UnaryExprNode::evaluate(Program& program)
{
	// Check that operand is a variable
	if (auto varexpr = dynamic_cast<VarExprNode*>(operand.get())) {
		auto operandResult = varexpr->evaluate(program);

		// Check that variable exists
		if (program.vars.find(varexpr->name) == program.vars.end()) {
			Utils::throw_err("Error: Undefined variable: " + varexpr->name);
		}
		auto varRef = program.vars[varexpr->name];

		// Perform unary operation and update variable
		auto value = Vars::unaryOperation(varRef, op);
		program.vars[varexpr->name] = value;

		// Return the new value
		EvalResult result;

		if (this->isPrefix) {
			result.content = value.value;
		}
		else {
			result.content = operandResult.content;
		}
		return result;
	}
	
	// Check that operand is a literal
	if (auto intlit = dynamic_cast<IntegerLiteralNode*>(operand.get())) {
		var varRef = { std::to_string(intlit->value), DT_NUMBER };
		auto value = Vars::unaryOperation(varRef, op);
		EvalResult result;
		result.content = value.value;
		return result;
	}
}

EvalResult XtmlBlockNode::evaluate(Program& program)
{
	EvalResult result;

	// First move global vars to local scope
	Core::mergePrograms(program, localProgram);

	// Evaluate body
	result = merge_results(result, this->body->evaluate(this->localProgram));

	// Merge back local vars to global scope
	Core::mergePrograms(this->localProgram, program);

	// Return result
	return result;
}

EvalResult FunctionDeclNode::evaluate(Program& program)
{
	FunctionCall funcCall;
	funcCall.name = function->name;
	funcCall.function = function.get();
	program.functions[function->name] = funcCall;

	return {};
}

EvalResult FunctionCallNode::evaluate(Program& program)
{
	EvalResult result;
	if (program.functions.find(this->functionName) == program.functions.end()) {
		Utils::throw_err("Error: Undefined function: " + this->functionName);
	}
	auto function = program.functions[this->functionName].function;

	std::vector<std::string> argValues;
	for (auto& arg : arguments) {
		auto argResult = arg->evaluate(program);
		argValues.push_back(argResult.content);
	}
	return function->callFunction(program, argValues);
}

EvalResult ReturnNode::evaluate(Program& program)
{
	if(this->expression == nullptr) {
		Utils::throw_err("Error: Return statement missing expression.");
	}
	return this->expression->evaluate(program);
}

EvalResult IncludeNode::evaluate(Program& program)
{
	// Create path to include file
	std::string includeFullPath;
	if(Utils::is_path_absolute(this->includePath)) {
		includeFullPath = this->includePath;
	}
	else {
		auto parentPath = Utils::file_path_parent(program.path);
		includeFullPath = parentPath + "\\" + this->includePath;
	}

	// Read include file content
	Program includeProgram;
	includeProgram.path = includeFullPath;
	auto content = Utils::read_file(includeFullPath);

	// Build root for included content
	auto root = Core::buildRoot(includeFullPath, content);
	if (root == nullptr) {
		Utils::throw_err("Error: Could not build include file: " + includeFullPath);
	}

	// Move included root and evaluate content
	this->includedRoot = std::move(root);
	content = this->includedRoot->evaluate().content;
	content = Core::resolve_placeholders(content, this->includedRoot->program.vars);

	// Merge included program vars and functions into current program
	Core::mergePrograms(this->includedRoot->program, program);

	// Return included content
	EvalResult result;
	result.content = content;
	return result;
}

EvalResult RootNode::evaluate()
{
	EvalResult result;
	Program& p = this->program;
	for (auto& node : nodes) {
		result = merge_results(result, node->evaluate(p));
	}
	return result;
}

EvalResult RootNode::evaluate(Program& program)
{
	Utils::throw_err("Error: RootNode evaluate with Program parameter is not supported. Use the parameterless evaluate() method instead.");
	return {};
}

EvalResult HtmlBlockNode::evaluate(Program& program)
{
	EvalResult result;

	// Evaluate children
	for(auto& child : children) {
		result = merge_results(result, child->evaluate(program));
	}
	// Resolve all placeholders within the html block
	result.content = Core::resolve_placeholders(result.content, program.vars); 
	return result;
}
