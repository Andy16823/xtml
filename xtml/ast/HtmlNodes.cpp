#include "HtmlNodes.h"
#include "Utils.h"
#include "Core.h"
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

EvalResult TextNode::evaluate(Program& program)
{
	EvalResult result;
	auto value = Vars::evalExpr(m_value, program.vars);
	if (value.type != DT_UNKNOWN) {
		result.evaluated.value = value.value;
	}
	return result;
}

EvalResult HtmlBlockNode::evaluate(Program& program)
{
	EvalResult result;

	// Evaluate children
	for(auto& child : children) {
		result = merge_results(result, child->evaluate(program), true); // True to merge content
	}
	result.evaluated.value = Core::resolvePlaceholders(result.evaluated.value, program.vars); // Resolve any remaining placeholders
	result.output = result.evaluated.value;	// For HTML blocks, printed output is the same as content
	return result;
}

EvalResult HtmlStmtRootNode::evaluate(Program& program)
{
	EvalResult result;
	result = body->evaluate(program);
	return result;
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
			out << childResult.evaluated.value;
		}
		out << "</" << tagName << ">";
	}

	EvalResult result;
	result.evaluated = { out.str(), DT_STRING };
	return result;
}

EvalResult HtmlTextNode::evaluate(Program& program)
{
	EvalResult result;
	result.evaluated = { content, DT_STRING };
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
		includeFullPath = (fs::path(parentPath) / this->includePath).string();
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

EvalResult PrintNode::evaluate(Program& program)
{
	EvalResult result;
	auto exprResult = expression->evaluate(program);
	result.output = exprResult.evaluated.value;
	return result;
}

EvalResult CommentNode::evaluate(Program& program)
{
	return {}; // Comments produce no output and have no effect
}
