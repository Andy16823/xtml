#include "Core.h"
#include <regex>
#include <sstream>
#include <fstream>
#include "Utils.h"
#include "Vars.h"
#include "Statements.h"

using namespace std;

/// <summary>
/// Parse content into blocks based on start and end tags
/// </summary>
/// <param name="content"></param>
/// <param name="start_tag"></param>
/// <param name="end_tag"></param>
/// <returns></returns>
vector<string> Core::parse_blocks(const string& content, const string& start_tag, const string& end_tag)
{
	// Parse content into blocks based on start and end tags
	vector<string> blocks;
	string pattern = start_tag + "([\\s\\S]*?)" + end_tag;
	regex re(pattern);

	auto beginn = sregex_iterator(content.begin(), content.end(), re);
	auto endd = sregex_iterator();

	for (auto i = beginn; i != endd; ++i) {
		blocks.push_back(i->str(1)); // Capture group 1
	}
	
	return blocks;
}

/// <summary>
/// Parse a single block for variable declarations
/// </summary>
/// <param name="content"></param>
/// <returns></returns>
map<string, var> Core::parse_block(const std::string& content, map<string, var>& vars)
{
	map<string, var> local_vars;
	local_vars.insert(vars.begin(), vars.end());

	// split parts on ;
	auto segments = Utils::split(content, ';');
	// Parse each line for @var declarations
	for (auto& line : segments) {	

 		line = Utils::trim(line);
		if (line.empty()) continue;
		line = Vars::trim_var(line);

		if (line.starts_with("@include")) {
			// Handle includes
		}

		std::tuple<std::string, std::string> parsedVar = Vars::parse_var(line);
		std::string key = std::get<0>(parsedVar);
		std::string value = std::get<1>(parsedVar);
		var varval = Vars::eval_expr(value, local_vars);

		if (!key.empty() && varval.type != DT_UNKNOWN) {
			local_vars[key] = varval;
		}
	}
	return local_vars;
}

/// <summary>
/// Resolve an include directive
/// </summary>
/// <param name="include_path"></param>
/// <param name="vars"></param>
/// <param name="tag"></param>
/// <param name="resolve_global"></param>
/// <returns></returns>
string Core::resolve_include(const string& include_path, map<string, var>& vars, XtmlTag tag, bool resolve_global)
{
	// Resolve an include directive
	Utils::print_ln("Resolving include: " + include_path);
	map<string, var> local_vars;

	// Copy global vars to local if resolve as global
	if (resolve_global) {
		local_vars.insert(vars.begin(), vars.end());
	}

	// Parse parameters from tag attributes and resolve them if needed
	auto param_vars = params_to_vars(tag.attributes);
	for (auto& [k, v] : param_vars) {
		v.value = Vars::replace_vars(v.value, vars);
	}
	local_vars = Vars::merge_vars(local_vars, param_vars);

	// Read and build included content
	auto include_content = Utils::read_file(include_path);
	Utils::print_ln("Processing include: " + include_path);
	include_content = build_content(include_content, Utils::file_path_parent(include_path), local_vars);

	// Merge local vars back to global if resolve as global
	if (resolve_global) {
		vars = Vars::merge_vars(vars, local_vars);
	}

	return include_content;
}

/// <summary>
/// Remove blocks from content based on start and end tags
/// </summary>
/// <param name="content"></param>
/// <param name="start_tag"></param>
/// <param name="end_tag"></param>
/// <returns></returns>
string Core::remove_blocks(const string& content, const string& start_tag, const string& end_tag)
{
	// Remove blocks from content based on start and end tags
	string pattern = start_tag + "[\\s\\S]*?" + end_tag;
	regex re(pattern);
	auto result = regex_replace(content, re, "");
	return result;
}

/// <summary>
/// Clean content by removing comments and trimming whitespace
/// </summary>
/// <param name="content"></param>
/// <returns></returns>
string Core::clean_content(string& content)
{
	// Remove comments and trim whitespace
	std::istringstream stream(content);
	std::string line;
	std::string cleaned;
	while (std::getline(stream, line)) {
		line = Utils::trim(line);
		if (line.starts_with("@var")) {
			continue;
		}
		cleaned += line + "\n";
	}
	return cleaned;
}

/// <summary>
/// Build a file by processing its content and resolving includes and variables
/// </summary>
/// <param name="path"></param>
/// <param name="vars"></param>
/// <returns></returns>
string Core::build_file(const string& path, map<string, var>& vars)
{
	Utils::print_ln(string("Building file ") + path);
	auto content = Utils::read_file(path);
	auto base_path = Utils::file_path_parent(path);

	return build_content(content, base_path, vars);
}

/// <summary>
/// Build content by processing includes and variables
/// </summary>
/// <param name="content"></param>
/// <param name="base_path"></param>
/// <param name="vars"></param>
/// <returns></returns>
std::string Core::build_content(string& content, string base_path, map<string, var>& vars)
{
	auto ast_root = std::make_unique<ASTRoot>();
	ast_root->merge_vars(vars); // Initialize with global vars

	auto blocks = Core::find_xtml_tags(content);
	for (const auto& block : blocks) {
		auto block_node = std::make_unique<BlockNode>();
		if (block.self_closing && block.attributes.find("include") != block.attributes.end()) {
			bool resolve_global = true;
			if (block.attributes.find("resolve") != block.attributes.end()) {
				auto resolve_val = Utils::trim(block.attributes.at("resolve"));
				if (resolve_val == "local") {
					resolve_global = false;
				}
			}
			auto include_path = base_path + "\\" + Utils::trim(block.attributes.at("include"));
			auto include_content = Core::resolve_include(include_path, ast_root->vars, block, resolve_global);
			content = Utils::replace(content, block.full, include_content);
		}
		else if (block.self_closing && block.attributes.find("define") != block.attributes.end()) {
			// Resolve self-closing var declaration later Todo
			auto [var_key, var_value] = Core::resolve_self_closing_var(block);
			vars[var_key] = var_value;
			content = Utils::replace(content, block.full, "");
		}

		auto preprocessed = Vars::preprocess_content(block.content);
		auto statements = Core::split_statements(preprocessed);
		auto childs = parse_ast_statements(statements);
		for (auto& child : childs) {
			block_node->add_child(move(child));
		}
		ast_root->add_child(move(block_node));

		// Evaluate AST to resolve includes and var declarations
		auto test = ast_root->evaluate();
		content = Utils::replace(content, block.full, test);
		// Exchange content with evaluated content
	}

	content = resolve_placeholders(content, ast_root->vars);

	// Check for unresolved variables
	auto unresolved = Core::find_unresolved_vars(content);
	if (!unresolved.empty()) {
		for (const auto& var : unresolved) {
			Utils::printerr_ln("Error: Unresolved variable: " + var);
			Utils::printerr_ln("Stack trace:");
			Utils::printerr_ln(content);
		}
		Utils::throw_err("Build failed due to unresolved variables.");
	}

	content = clean_content(content);
	content = Core::remove_blocks(content, "<xtml>", "</xtml>");
	content = Utils::trim(content);


	Utils::print_ln("Build completed.");
	return content;
}

/// <summary>
/// Write content to a file
/// </summary>
/// <param name="content"></param>
/// <param name="output_path"></param>
void Core::write_file(const string& content, const string& output_path)
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
vector<XtmlTag> Core::find_xtml_tags(const string& content) {
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

		tag.attributes = Core::parse_xtml_attributes(tag.head);
		tags.push_back(std::move(tag));
	}

	return tags;
}

/// <summary>
/// Parse attributes from an XHTML tag string
/// </summary>
/// <param name="tag"></param>
/// <returns></returns>
map<string, string> Core::parse_xtml_attributes(const string& tag)
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

/// <summary>
/// Map parameters (string) to vars (var)
/// </summary>
/// <param name="params"></param>
/// <returns></returns>
map<string, var> Core::params_to_vars(const map<string, string>& params)
{
	// Convert string parameters to var types
	map<string, var> vars;
	for (const auto& [key, value] : params) {
		if (Utils::starts_with(key, "param-")) {
			auto new_key = key.substr(6);
            vars[new_key] = var{ value, DT_STRING };
			continue;
		}
	}
	return vars;
}

/// <summary>
/// Find unresolved variables in the content
/// </summary>
/// <param name="content"></param>
/// <returns></returns>
vector<string> Core::find_unresolved_vars(const string& content)
{
	vector<string> unresolved;
	std::regex re(R"(\{\{@([a-zA-Z0-9_]+)\}\})");
	for (auto it = std::sregex_iterator(content.begin(), content.end(), re); it != std::sregex_iterator(); ++it) {
		unresolved.push_back(it->str(1));
	}
	return unresolved;
}

/// <summary>
/// Resolve a self-closing <xtml> tag that defines a variable
/// e.g. <xtml define="varName" value="varValue" type="string" />
/// types: string (default), number
/// </summary>
/// <param name="tag"></param>
/// <returns></returns>
tuple<string, var> Core::resolve_self_closing_var(XtmlTag tag)
{
	auto var_key = tag.attributes.contains("define") ? Utils::trim(tag.attributes.at("define")) : "";
	if (var_key.empty()) {
		Utils::printerr_ln("Error: Variable key is empty.");
		Utils::printerr_ln("Stack trace:");
		Utils::printerr_ln(tag.full);
		throw std::runtime_error("Variable key is empty.");
	}

	auto var_value = tag.attributes.contains("value") ? Utils::trim(tag.attributes.at("value")) : "";
	auto var_type = tag.attributes.contains("type") ? Utils::trim(tag.attributes.at("type")) : "string";

	if (var_value.empty()) {
		Utils::printerr_ln("Error: Variable value is empty for variable: " + var_key);
		Utils::printerr_ln("Stack trace:");
		Utils::printerr_ln(tag.full);
		throw std::runtime_error("Variable value is empty.");
	}

	if (var_type == "string") {
		return tuple(var_key, var{ var_value, DT_STRING });
	}
	else if (var_type == "number") {
		if (Utils::is_number(var_value)) {
			return tuple(var_key, var{ var_value, DT_NUMBER });
		}
		else {
			Utils::printerr_ln("Error: Invalid number value for variable: " + var_key);
			Utils::printerr_ln("Stack trace:");
			Utils::printerr_ln(tag.full);
			throw std::runtime_error("Invalid number value.");
		}
	}
	else {
		Utils::printerr_ln("Error: Unknown variable type: " + var_type + " for variable: " + var_key);
		Utils::printerr_ln("Stack trace:");
		Utils::printerr_ln(tag.full);
		throw std::runtime_error("Unknown variable type.");
	}
}

std::string Core::resolve_placeholders(const std::string& content, const std::map<std::string, var>& vars)
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
			var var_val = Vars::eval_expr(var_name, vars);
			results[placeholder] = var_val;
		}
		else if (Vars::is_function_expr(inner)) {
			var func_val = Vars::eval_func_expr(inner, vars);
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

std::vector<std::string> Core::split_statements(const std::string& input)
{
	vector<string> result;
	string current;
	int brace_level = 0;

	char quote_char = '\0';
	bool in_quotes = false;

	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];
		current.push_back(c);

		if ((c == '"' || c == '\'') && (quote_char == '\0' || quote_char == c)) {
			quote_char = (quote_char == '\0') ? c : '\0';
		}

		if (quote_char != '\0') continue;

		if (c == '{') {
			brace_level++;
		}
		else if (c == '}') {
			brace_level--;
			if (brace_level == 0) {
				result.push_back(Utils::trim(current));
				current.clear();
			}
		}
		else if (c == ';' && brace_level == 0) {
			result.push_back(Utils::trim(current));
			current.clear();
		}
	}

	if (!current.empty()) {
		result.push_back(Utils::trim(current));
	}

	return result;
}

/// <summary>
/// Extract code section within the first level of braces
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
std::string Core::extract_code_section(const std::string& input)
{
	string result;
	int brace_level = 0;
	bool in_quote = false;

	for (size_t i = 0; i < input.length(); ++i) {
		char c = input[i];

		if (c == '"' || c == '\'') {
			in_quote = !in_quote;
		}

		if (!in_quote) {
			if (c == '{') {
				brace_level++;
				if (brace_level == 1) continue; // Skip the opening brace
			}
			else if (c == '}') {
				brace_level--;
				if (brace_level == 0) break; // Stop at the closing brace
			}
		}
		if (brace_level > 0) {
			result.push_back(c);
		}
	}

	return result;
}

std::vector<unique_ptr<ASTNode>> Core::parse_ast_statements(const std::vector<std::string>& statements)
{
	vector<unique_ptr<ASTNode>> nodes;
	bool in_if = false;
	IfStatement if_stmt;

	// Parse each statement for variable declarations
	for (const auto& stmt : statements) {
		auto line = Utils::trim(stmt);

		if (Utils::starts_with(line, "@var")) {
			// Parse simple @var declarations
			line = Vars::trim_var(line);
			auto [key, value] = Vars::parse_var(line);
			auto node = std::make_unique<VarDeclNode>(key, value);
			nodes.push_back(std::move(node));
		}
		else if (Utils::starts_with(line, "@print")) {
			// Handle print statements later Todo
			auto condition = Utils::parse_parantheses(line);
			auto node = std::make_unique<TextNode>(condition);
			nodes.push_back(std::move(node));
		}
		else if (Utils::starts_with(line, "@if")) {
			if (in_if) {
				auto if_node = std::make_unique<IfStatementNode>(if_stmt);
				nodes.push_back(std::move(if_node));
				in_if = false;
			}
			in_if = true;
			if_stmt = IfStatement();
			IfBranch branch;
			branch.condition = Utils::parse_parantheses(line);
			branch.content = Core::extract_code_section(line);
			if_stmt.branches.push_back(branch);
		}
		else if (Utils::starts_with(line, "@else if"))
		{
			if (in_if) {
				IfBranch branch;
				branch.condition = Utils::parse_parantheses(line);
				branch.content = Core::extract_code_section(line);
				if_stmt.branches.push_back(branch);
			}
			else {
				Utils::throw_err("Error: @else if without matching @if.");
			}
		}
		else if (Utils::starts_with(line, "@else")) {
			if (in_if) {
				if_stmt.has_else = true;
				if_stmt.else_content = Core::extract_code_section(line);
				auto node = std::make_unique<IfStatementNode>(if_stmt);
				nodes.push_back(std::move(node));
				in_if = false;
				if_stmt = IfStatement();
			}
			else {
				Utils::throw_err("Error: @else without matching @if.");
			}
		}
	}
	// Resolve if statement
	if (in_if) {
		auto if_node = std::make_unique<IfStatementNode>(if_stmt);
		nodes.push_back(std::move(if_node));
		in_if = false;
		Utils::print_ln("Resolving last if statement.");
	}

	return nodes;
}

