#pragma once
#include <string>
#include <vector>
#include <map>
#include "Vars.h"
#include <memory>

class FunctionNode;
class RootNode;

struct XtmlTag {
	std::string full;
	std::string head;
	std::string content;
	bool self_closing = false;
	std::map<std::string, std::string> attributes;
};

struct FunctionCall {
	std::string name;
	FunctionNode* function = nullptr;
};

struct Program {
	std::map<std::string, var> vars;
	std::map<std::string, FunctionCall> functions;
	std::string path;
};

class Core
{
public:
	static std::vector<std::string> parse_blocks(const std::string& content, const std::string& start_tag, const std::string& end_tag);
	static std::map<std::string, var> parse_block(const std::string& content, std::map<std::string, var>& vars);
	static std::string remove_blocks(const std::string& content, const std::string& start_tag, const std::string& end_tag);
	static std::string clean_content(std::string& content);	

	static std::string buildFile(const std::string& path);
	static std::unique_ptr<RootNode> buildRoot(const std::string& path, std::string& content);

	static std::vector<XtmlTag> findTags(const std::string& content, const std::string& tag);
	static void readDefinitionBlocks(std::string& content, std::map<std::string, var>& vars);

	static void write_file(const std::string& content, const std::string& output_path);
	static std::vector<XtmlTag> find_xtml_tags(const std::string& content);
	static std::map<std::string, std::string> parse_xtml_attributes(const std::string& tag);
	static std::map<std::string, var> params_to_vars(const std::map<std::string, std::string>& params);
	static std::vector<std::string> find_unresolved_vars(const std::string& content);
	static std::tuple<std::string, var> resolve_self_closing_var(XtmlTag tag);
	static std::string resolve_placeholders(const std::string& content, const std::map<std::string, var>& vars);
	static std::vector<std::string> split_statements(const std::string& input);
	static std::string extract_code_section(const std::string& input);
	static void mergePrograms(const Program& source, Program& destination);
	static void mergeProgrammFunctions(const Program& source, Program& destination);
};

