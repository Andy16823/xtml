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
#include "Globals.h"

using namespace std;

EvalResult ASTNode::merge_results(const EvalResult& a, const EvalResult& b, bool mergeContent)
{
	EvalResult result;
	if (mergeContent) {
		result.value = a.value + b.value;
	}
	result.output = a.output + b.output;

	if (a.should_break || b.should_break) {
		result.should_break = true;
	}else if (a.should_continue || b.should_continue) {
		result.should_continue = true;
	}

	if (a.should_return || b.should_return) {
		result.should_return = true;
	}

	return result;
}

EvalResult VarDeclNode::evaluate(Program& program)
{
	// Declare a variable
	if(this->expression == nullptr) {
		var value;
		value.value = "";
		value.type = DT_UNKNOWN;
		return EvalResult{}; // No expression to evaluate
	}

	// Evaluate the expression and assign to variable
	EvalResult eval = expression->evaluate(program);
	program.vars[name] = eval.value;

	// No output for variable declaration
	return {};
}

EvalResult BlockNode::evaluate(Program& program)
{
	EvalResult result;
	for (auto& child : children) {
		auto childResult = child->evaluate(program);
		// Handle Return
		if (childResult.should_return) {
			return childResult;
		}

		// Handle Break
		if (childResult.should_break) {
			result.should_break = true;
			return result;
		}

		// Handle Continue
		if (childResult.should_continue) {
			result.should_continue = true;
			return result;
		}

		result = merge_results(result, childResult);
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
	if(condResult.value.value == "true") {
		result = body->evaluate(program);
		return result;
	}
	
	// Try else ifs
	for (auto& elseif : elseIfs) {
		auto elseIfresult = elseif->condition->evaluate(program);
		if (elseIfresult.value.value == "true") {
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
	EvalResult result;
	auto value = Vars::evalExpr(m_value, program.vars);
	if (value.type != DT_UNKNOWN) {
		result.value.value = value.value;
	}
	return result;
}

EvalResult WhileNode::evaluate(Program& program)
{
	EvalResult result;

	auto condResult = condition->evaluate(program);
	auto cond = Utils::toBool(condResult.value.value);
	while (cond) {
		// Evaluate body
		auto bodyResult = this->body->evaluate(program);

		// Check for break
		if (bodyResult.should_break) {
			bodyResult.should_break = false;
			result = merge_results(result, bodyResult);
			break;
		}

		// Check for continue (skip to next iteration)
		if (bodyResult.should_continue) {
			bodyResult.should_continue = false;
			result = merge_results(result, bodyResult);
			// Re-evaluate condition
			condResult = condition->evaluate(program);
			cond = Utils::toBool(condResult.value.value);
			continue;
		}

		// Normal execution
		result = merge_results(result, bodyResult);
		condResult = condition->evaluate(program);
		cond = Utils::toBool(condResult.value.value);
	}

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
		Utils::throwErr("Error: For loop missing initialization statement.");
	}

	// Evaluate condition
	auto condResult = condition->evaluate(program);
	while (condResult.value.value == "true") {
		auto bodyResult = body->evaluate(program);

		// Handle break, we exit the loop
		if (bodyResult.should_break) {
			bodyResult.should_break = false;
			result = merge_results(result, bodyResult);
			break;
		}

		// Handle continue, we skip to the next iteration
		if (bodyResult.should_continue) {
			bodyResult.should_continue = false;
			result = merge_results(result, bodyResult);

			if (increment) increment->evaluate(program);
			else {
				Utils::throwErr("Error: For loop missing increment statement.");
			}
			condResult = condition->evaluate(program);
			continue;
		}

		// Normal execution
		result = merge_results(result, bodyResult);
		if (increment) increment->evaluate(program);
		else {
			Utils::throwErr("Error: For loop missing increment statement.");
		}
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
	// Note: Break stops execution of the block AND the loop
	EvalResult result;
	result.should_break = true;
	return result;
}

EvalResult ContinueNode::evaluate(Program& program)
{
	// Note: Continue stops execution the block BUT NOT the loop
	EvalResult result;
	result.should_continue = true;
	return result;
}

EvalResult FunctionNode::evaluate(Program& program)
{
	return this->body->evaluate(program);
}

EvalResult FunctionNode::callFunction(Program& program, const std::vector<std::string>& argValues)
{
	if(argValues.size() != this->arguments.size()) {
		Utils::throwErr("Error: Function " + this->name + " called with incorrect number of arguments.");
	}

	// Create a new program frame for the function call
	Program frame = Program();
	Core::mergeProgrammFunctions(program, frame);
	frame.path = program.path;

	// Bind arguments to local program variables
	for (size_t i = 0; i < this->arguments.size(); i++) {
		auto argExpr = dynamic_cast<VarExprNode*>(this->arguments[i].get());
		if (argExpr == nullptr) {
			Utils::throwErr("Error: Function argument must be a variable.");
		}
		var value;
		value.value = argValues[i];
		value.type = Utils::predictVarType(value.value);
		frame.vars[argExpr->name] = value;
	}

	// Evaluate function body in local program context with evaluate from FunctionNode
	auto result = this->evaluate(frame);
	result.should_return = false;
	return result;
}

EvalResult BinaryExprNode::evaluate(Program& program)
{
	EvalResult result;

	auto optype = Vars::getBinaryOpType(op);
	// Early check for unknown operation
	if (optype == BinaryOpType::Unknown) {
		Utils::throwErr("Error: Unknown binary operation: " + op);
	}

	if (optype == BinaryOpType::Assignment) {
		auto leftVar = dynamic_cast<VarExprNode*>(left.get());
		if (leftVar == nullptr) {
			Utils::throwErr("Error: Left side of assignment must be a variable.");
		}
		auto rightResult = right->evaluate(program);
		if(program.vars.find(leftVar->name) == program.vars.end()) {
			Utils::throwErr("Error: Undefined variable: " + leftVar->name);
		}
		program.vars[leftVar->name] = rightResult.value;
		return rightResult; 
	}

	// Logic operation
	if (optype == BinaryOpType::Logical) {
		if (op == "&&") {
			auto leftresult = left->evaluate(program);
			// Short-circuit evaluation
			if (leftresult.value.value == "false") {
				result.value = {"false", DT_BOOL};
				return result;
			}
			auto rightresult = right->evaluate(program);
			if (rightresult.value.value == "false") {
				result.value = {"false", DT_BOOL};
				return result;
			}
			result.value = { "true", DT_BOOL };
			return result;
		}
		if (op == "||") {
			auto leftresult = left->evaluate(program);
			// Short-circuit evaluation
			if (leftresult.value.value == "true") {
				result.value = {"true", DT_BOOL};
				return result;
			}
			auto rightresult = right->evaluate(program);
			if (rightresult.value.value == "true") {
				result.value = {"true", DT_BOOL};
				return result;
			}
			result.value = {"false", DT_BOOL};
			return result;
		}
	}

	// Evaluate left and right expressions
	auto leftResult = left->evaluate(program);
	auto rightResult = right->evaluate(program);

	if (optype == BinaryOpType::Comparison) {
		bool cmpResult = Vars::compareOperation(leftResult.value, rightResult.value, op);
		if (cmpResult) {
			result.value = {"true", DT_BOOL};
		}
		else {
			result.value = { "false", DT_BOOL };
		}
		return result;
	}	

	// Return result
	result.value = Vars::binaryOperation(leftResult.value, rightResult.value, op);
	return result;
}

EvalResult IntegerLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.value = {std::to_string(value), DT_NUMBER};
	return result;
}

EvalResult StringLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.value = {value, DT_STRING};
	return result;
}

EvalResult FloatLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.value = {std::to_string(value), DT_NUMBER};
	return result;
}

EvalResult BoolLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	if (value) {
		result.value = { "true", DT_BOOL };
	}
	else {
		result.value = { "false", DT_BOOL };
	}
	return result;
}

EvalResult DoubleLiteralNode::evaluate(Program& program)
{
	EvalResult result;
	result.value = { std::to_string(value), DT_NUMBER };
	return result;
}

EvalResult VarExprNode::evaluate(Program& program)
{
	EvalResult result;
	if(program.vars.find(this->name) != program.vars.end()) {
		result.value = program.vars[this->name];
	}
	else {
		Utils::throwErr("Error: Undefined variable: " + this->name);
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
			out << childResult.value.value;
		}
		out << "</" << tagName << ">";
	}

	EvalResult result;
	result.value = { out.str(), DT_STRING };
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
	result.value = { content, DT_STRING };
	return result;
}

EvalResult UnaryExprNode::evaluate(Program& program)
{
	// Check that operand is a variable
	if (auto varexpr = dynamic_cast<VarExprNode*>(operand.get())) {
		auto operandResult = varexpr->evaluate(program);

		// Check that variable exists
		if (program.vars.find(varexpr->name) == program.vars.end()) {
			Utils::throwErr("Error: Undefined variable: " + varexpr->name);
		}
		auto varRef = program.vars[varexpr->name];

		// Perform unary operation and update variable
		auto value = Vars::unaryOperation(varRef, op);
		program.vars[varexpr->name] = value;

		// Return the new value
		EvalResult result;

		if (this->isPrefix) {
			result.value = value;
		}
		else {
			result.value = operandResult.value;
		}
		return result;
	}
	
	// Check that operand is a literal
	if (auto intlit = dynamic_cast<IntegerLiteralNode*>(operand.get())) {
		var varRef = { std::to_string(intlit->value), DT_NUMBER };
		auto value = Vars::unaryOperation(varRef, op);
		EvalResult result;
		result.value = value;
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
		Utils::throwErr("Error: Undefined function: " + this->functionName);
	}
	auto function = program.functions[this->functionName].function;

	std::vector<std::string> argValues;
	for (auto& arg : arguments) {
		auto argResult = arg->evaluate(program);
		argValues.push_back(argResult.value.value);
	}
	return function->callFunction(program, argValues);
}

EvalResult ReturnNode::evaluate(Program& program)
{
	if(this->expression == nullptr) {
		Utils::throwErr("Error: Return statement missing expression.");
	}

	auto result = this->expression->evaluate(program);
	result.should_return = true;
	return result;
}

EvalResult IncludeNode::evaluate(Program& program)
{
	// Create path to include file
	std::string includeFullPath;
	if(Utils::isAbsolute(this->includePath)) {
		includeFullPath = this->includePath;
	}
	else {
		auto parentPath = Utils::filePathParent(program.path);
		includeFullPath = parentPath + "\\" + this->includePath;
	}

	// Read include file content
	Program includeProgram;
	includeProgram.path = includeFullPath;
	auto content = Utils::readFile(includeFullPath);

	// Build root for included content and replace block in content
	auto root = Core::buildRoot(includeFullPath, content);
	if (root == nullptr) {
		Utils::throwErr("Error: Could not build include file: " + includeFullPath);
	}

	// Move included root and evaluate content
	this->includedRoot = std::move(root);
	content = Core::resolvePlaceholders(content, this->includedRoot->program.vars);

	// Merge included program vars and functions into current program
	Core::mergePrograms(this->includedRoot->program, program);

	// Return included content
	EvalResult result;
	result.output = content;
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
	Utils::throwErr("Error: RootNode evaluate with Program parameter is not supported. Use the parameterless evaluate() method instead.");
	return {};
}

EvalResult HtmlBlockNode::evaluate(Program& program)
{
	EvalResult result;

	// Evaluate children
	for(auto& child : children) {
		result = merge_results(result, child->evaluate(program), true); // True to merge content
	}
	result.value.value = Core::resolvePlaceholders(result.value.value, program.vars); // Resolve any remaining placeholders
	result.output = result.value.value;	// For HTML blocks, printed output is the same as content
	return result;
}

EvalResult PrintNode::evaluate(Program& program)
{
	EvalResult result;
	auto exprResult = expression->evaluate(program);
	result.output = exprResult.value.value;
	return result;
}

EvalResult StmtExprNode::evaluate(Program& program)
{
	auto result = statement->evaluate(program);
	result.value = {result.output, DT_STRING};
	result.output = "";
	return result;
}

EvalResult NativeFunctionCallNode::evaluate(Program& program)
{
	// Build arguments
	std::vector<var> args;
	for (auto& arg : arguments) {
		auto argResult = arg->evaluate(program);
		args.push_back(argResult.value);
	}
	
	// Call native function
	var value = g_functionRegistry.CallFunction(this->namespaceName, this->functionName, args);

	// Return result
	EvalResult result;
	result.value = value;
	return result;
}

EvalResult CommentNode::evaluate(Program& program)
{
	return {}; // Comments produce no output and have no effect
}

EvalResult ArrayLiteralNode::evaluate(Program& program)
{
	return {};
}
