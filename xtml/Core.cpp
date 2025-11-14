#include "Core.h"
#include <regex>
#include <sstream>
#include <fstream>
#include "Utils.h"
#include "Vars.h"
#include "Statements.h"
#include "Lexer.h"
#include "Parser.h"
#include "ASTNode.h"

using namespace std;

std::string Core::buildFile(const std::string& path)
{
	// Read file content
	auto content = Utils::read_file(path);

	// Build root node and evaluate content
	auto root = Core::buildRoot(path, content);

	// Resolve the remaining placeholders in the content
	content = Core::resolvePlaceholders(content, root->program.vars);
	Utils::print_ln("Build completed.");

	// Return final content
	return content;
}

std::unique_ptr<RootNode> Core::buildRoot(const std::string& path, std::string& content)
{
	// Find all <xtml> tags
	auto blocks = Core::findXtmlTags(content);

	// Create the root node for this file
	auto root = std::make_unique<RootNode>();
	root->program.path = path;

	// Read definition blocks and populate root program vars
	Core::readDefinitionBlocks(content, root->program.vars);

	// Parse blocks and evaluate the content
	for (const auto& block : blocks) {
		if (!block.self_closing) {

			// Create tokens from block content
			Lexer lexer(block.content);
			auto tokens = lexer.tokenize();

			// Parse tokens into AST
			Parser parser(tokens);
			auto xmltBlock = parser.parse();

			// Evaluate the block and replace in content
			auto result = xmltBlock->evaluate(root->program);
			content = Utils::replace(content, block.full, result.output);

			// Add block to root nodes to prevent dangling pointer issues with program function ptrs
			root->nodes.push_back(std::move(xmltBlock));
		}
	}

	// Return the built root node
	Utils::printerr_ln("Finished building root for file: " + path);
	return root;
}

std::vector<XtmlTag> Core::findTags(const std::string& content, const std::string& tag)
{
	vector<XtmlTag> tags;
	std::string pattern = "<" + tag + R"(\b([^>]*)\/>|<)" + tag + R"(\b([^>]*)>([\s\S]*?)<\/)" + tag + ">";
	regex re(pattern);

	auto it = sregex_iterator(content.begin(), content.end(), re);
	auto end = sregex_iterator();

	for (; it != end; ++it) {
		smatch m = *it;
		XtmlTag tag;

		if (m[1].matched) {
			// matched first alternative: self-closing
			tag.full = m.str(0);
			tag.head = string("<xtml") + m[1].str() + "/>";
			tag.content = "";
			tag.self_closing = true;
		}
		else {
			// matched second alternative: block
			tag.full = m.str(0);
			tag.head = string("<xtml") + m[2].str() + ">";
			tag.content = m[3].str();
			tag.self_closing = false;
		}

		tag.attributes = Core::parseXtmlAttributes(tag.head);
		tags.push_back(std::move(tag));
	}

	return tags;
}

void Core::readDefinitionBlocks(std::string& content, std::map<std::string, var>& vars)
{
	auto blocks = Core::findTags(content, "xtmldef");
	for (const auto& block : blocks) {
		if (block.attributes.find("name") == block.attributes.end()) {
			Utils::throw_err("Error: xtmldef block missing 'name' attribute.");
		}
		auto var_name = Utils::trim(block.attributes.at("name"));

		if (block.self_closing) {
			if (block.attributes.find("define") == block.attributes.end()) {
				Utils::throw_err("Error: Self-closing xtmldef block missing 'define' attribute.");
			}
			auto define = Utils::trim(block.attributes.at("define"));
			var value = var{ define, Utils::predictVarType(define) };
			vars[var_name] = value;
			Utils::print_ln("Defined variable '" + var_name + "' with value '" + define + "' from self-closing xtmldef.");
		}
		else {	
			var value = var{ Utils::trim(block.content), DT_STRING };
			vars[var_name] = value;
			Utils::print_ln("Defined variable '" + var_name + "' with value '" + block.content + "' from xtmldef block.");
		}
		content = Utils::replace(content, block.full, "");
	}
}

/// <summary>
/// Write content to a file
/// </summary>
/// <param name="content"></param>
/// <param name="output_path"></param>
void Core::writeFile(const string& content, const string& output_path)
{
	std::ofstream file(output_path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not create file: " + output_path);
	}
	file << content;
	file.close();
}

/// <summary>
/// Find all <xtml> tags in the content. Includes both self-closing and block tags.
/// </summary>
/// <param name="content"></param>
/// <returns></returns>
vector<XtmlTag> Core::findXtmlTags(const string& content) {
	vector<XtmlTag> tags;

	// Kombinierte Regex:
	// 1. Alternative: self-closing <xtml ... />
	// 2. Alternative: block <xtml ...> ... </xtml>
	// ([\s\S]*?) wird benutzt als dotall-Ersatz für inneren Inhalt
	regex re(R"(<xtml\b([^>]*)\/>|<xtml\b([^>]*)>([\s\S]*?)<\/xtml>)");

	auto it = sregex_iterator(content.begin(), content.end(), re);
	auto end = sregex_iterator();

	for (; it != end; ++it) {
		smatch m = *it;
		XtmlTag tag;

		if (m[1].matched) {
			// matched first alternative: self-closing
			tag.full = m.str(0);
			tag.head = string("<xtml") + m[1].str() + "/>";
			tag.content = "";
			tag.self_closing = true;
		}
		else {
			// matched second alternative: block
			tag.full = m.str(0);
			tag.head = string("<xtml") + m[2].str() + ">";
			tag.content = m[3].str();
			tag.self_closing = false;
		}

		tag.attributes = Core::parseXtmlAttributes(tag.head);
		tags.push_back(std::move(tag));
	}

	return tags;
}

/// <summary>
/// Parse attributes from an XHTML tag string
/// </summary>
/// <param name="tag"></param>
/// <returns></returns>
map<string, string> Core::parseXtmlAttributes(const string& tag)
{
	map<string, string> attributes;
	// Example tag: <tag attr1="value1" attr2='value2'>
    //regex re(R"((\w+)\s*=\s*\"([^\"]*)\")");
	regex re(R"(([\w-]+)\s*=\s*\"([^\"]*)\")");
	auto beginn = sregex_iterator(tag.begin(), tag.end(), re);
	auto endd = sregex_iterator();
	for (auto i = beginn; i != endd; ++i) {
		string key = (*i)[1].str();
		string value = (*i)[2].str();
		attributes[key] = value;
	}

	return attributes;
}


std::string Core::resolvePlaceholders(const std::string& content, const std::map<std::string, var>& vars)
{
	// Resolving playeholders like {{@varName}} or {{namespace::funcName(arg1, arg2)}}

	map<string, var> results;
	string result = content;
	regex re(R"(\{\{([^\}]+)\}\})");

	auto it = sregex_iterator(content.begin(), content.end(), re);
	auto end = sregex_iterator();

	for (; it != end; ++it) {
		std::smatch match = *it;
		std::string placeholder = match.str(0); // Full match including {{}}
		std::string inner = match.str(1); // Inner content

		inner = Utils::trim(inner);
		if (inner[0] == '@') {
			string var_name = inner.substr(1);
			var var_val = Vars::evalExpr(var_name, vars);
			results[placeholder] = var_val;
		}
		else if (Vars::isFunctionExpr(inner)) {
			var func_val = Vars::evalFuncExpr(inner, vars);
			results[placeholder] = func_val;
		}
		else {
			Utils::throw_err("Error: Unknown placeholder format: " + placeholder);
		}
	}

	for (const auto& [placeholder, var_val] : results) {
		result = Utils::replace(result, placeholder, var_val.value);
	}

	return result;
}

void Core::mergePrograms(const Program& source, Program& destination)
{
	// Merge variables
	for (const auto& [key, value] : source.vars) {
		destination.vars[key] = value;
	}
	// Merge functions
	Core::mergeProgrammFunctions(source, destination);

	// Merge path
	destination.path = source.path;
}

void Core::mergeProgrammFunctions(const Program& source, Program& destination)
{
	for (const auto& [key, funcCall] : source.functions) {
		destination.functions[key] = funcCall;
	}
}
