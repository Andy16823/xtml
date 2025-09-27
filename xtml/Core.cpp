#include "Core.h"
#include <regex>
#include <sstream>
#include <fstream>
#include "Utils.h"
#include "Vars.h"

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

map<string, var> Core::parse_block(const std::string& content)
{
	map<string, var> vars;
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
		var varval = Vars::eval_expr(value, vars);

		if (!key.empty() && varval.type != DT_UNKNOWN) {
			vars[key] = varval;
		}
	}
	return vars;
}

string Core::resolve_includes(const string& content, const string& base_path)
{
	// Resolve include directives in content
	string result = content;
	/*vector<tag> tags2 = find_xtml_tag(result);*/
	vector<string> tags = find_xtml_tags_str(result);
	for (auto tag : tags) {
		auto attrs = parse_xtml_attributes(tag);
		if (attrs.find("include") != attrs.end()) {
			auto vars = params_to_vars(attrs);
			auto include_path = base_path + "\\" + Utils::trim(attrs["include"]);
			auto include_content = Utils::read_file(include_path);
			include_content = build_content(include_content, Utils::file_path_parent(include_path), vars);
			result = Utils::replace(result, tag, include_content);
			Utils::print_ln("Processing include: " + include_path);
		}
	}
	return result;
}

string Core::resolve_include(const string& include_path, map<string, var>& vars, XtmlTag tag, bool resolve_global)
{
	auto local_vars = params_to_vars(tag.attributes);
	if (resolve_global) {
		local_vars.insert(vars.begin(), vars.end());
	}
	auto include_content = Utils::read_file(include_path);
	include_content = build_content(include_content, Utils::file_path_parent(include_path), local_vars);
	Utils::print_ln("Processing include: " + include_path);
	include_content = build_content(include_content, Utils::file_path_parent(include_path), local_vars);

	if (resolve_global) {
		vars.insert(local_vars.begin(), local_vars.end());
	}

	return include_content;
}

string Core::remove_blocks(const string& content, const string& start_tag, const string& end_tag)
{
	// Remove blocks from content based on start and end tags
	string pattern = start_tag + "[\\s\\S]*?" + end_tag;
	regex re(pattern);
	auto result = regex_replace(content, re, "");
	return result;
}

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

string Core::build_file(const string& path, map<string, var>& vars)
{
	Utils::print_ln(string("Building file ") + path);
	auto content = Utils::read_file(path);
	auto base_path = Utils::file_path_parent(path);

	return build_content(content, base_path, vars);
}

string Core::build_content(string& content, string base_path, map<string, var>& vars)
{
	// Parse variables from <xtml> blocks
	auto blocks = Core::find_xtml_tags(content);
	for (const auto& block : blocks) {
		if (block.self_closing && block.attributes.find("include") != block.attributes.end()) {
			bool resolve_global = true;
			if (block.attributes.find("resolve") != block.attributes.end()) {
				auto resolve_val = Utils::trim(block.attributes.at("resolve"));
				if (resolve_val == "local") {
					resolve_global = false;
				}
			}
			auto include_path = base_path + "\\" + Utils::trim(block.attributes.at("include"));
			auto include_content = Core::resolve_include(include_path, vars, block, resolve_global);
			content = Utils::replace(content, block.full, include_content);
		}
		auto preprocessed = Vars::preprocess_content(block.content);
		auto block_vars = Core::parse_block(preprocessed);
		vars.insert(block_vars.begin(), block_vars.end());
	}

	content = Vars::replace_vars(content, vars);

	// Check for unresolved variables
	auto unresolved = Core::find_unresolved_vars(content);
	if (!unresolved.empty()) {
		for (const auto& var : unresolved) {
			Utils::printerr_ln("Error: Unresolved variable: " + var);
		}
		throw std::runtime_error("Build failed due to unresolved variables.");
	}

	content = clean_content(content);
	content = Core::remove_blocks(content, "<xtml>", "</xtml>");
	content = Utils::trim(content);

	Utils::print_ln("Build completed.");
	return content;
}

void Core::write_file(const string& content, const string& output_path)
{
	std::ofstream file(output_path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not create file: " + output_path);
	}
	file << content;
	file.close();
}

vector<string> Core::find_xtml_tags_str(const string& content)
{
	vector<string> tags;
	// Example tag: <tag attr1="value1" attr2='value2'>
	regex re(R"(<xtml\b[^>]*?/?>)");
	auto beginn = sregex_iterator(content.begin(), content.end(), re);
	auto end = sregex_iterator();
	for (auto i = beginn; i != end; ++i) {
		tags.push_back(i->str());
	}
	return tags;
}

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

vector<string> Core::find_unresolved_vars(const string& content)
{
	vector<string> unresolved;
	std::regex re(R"(\{\{@([a-zA-Z0-9_]+)\}\})");
	for (auto it = std::sregex_iterator(content.begin(), content.end(), re); it != std::sregex_iterator(); ++it) {
		unresolved.push_back(it->str(1));
	}
	return unresolved;
}
