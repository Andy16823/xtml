#pragma once
#include <string>
#include <vector>
#include <map>
#include "Vars.h"
#include <memory>

class FunctionNode;
class RootNode;

/// <summary>
/// Xtml Tag Structure
/// </summary>
struct XtmlTag {
	std::string full;
	std::string head;
	std::string content;
	bool self_closing = false;
	std::map<std::string, std::string> attributes;
};

/// <summary>
/// Function Call Structure
/// </summary>
struct FunctionCall {
	std::string name;
	FunctionNode* function = nullptr;
};

/// <summary>
/// Program Structure
/// </summary>
struct Program {
	std::map<std::string, var> vars;
	std::map<std::string, FunctionCall> functions;
	std::string path;
};

class Core
{
public:
	/// <summary>
	/// Build an XTML file and return the resulting HTML content
	/// </summary>
	/// <param name="path"></param>
	/// <returns></returns>
	static std::string buildFile(const std::string& path);

	/// <summary>
	/// Build the root node for an XTML file and populate the content with evaluated HTML
	/// </summary>
	/// <param name="path"></param>
	/// <param name="content"></param>
	/// <returns></returns>
	static std::unique_ptr<RootNode> buildRoot(const std::string& path, std::string& content);

	/// <summary>
	/// Find all <xtml> tags in the content. Includes both self-closing and block tags.
	/// </summary>
	/// <param name="content"></param>
	/// <returns></returns>
	static std::vector<XtmlTag> findXtmlTags(const std::string& content);

	/// <summary>
	/// Find all tags of a specific type in the content. Includes both self-closing and block tags.
	/// </summary>
	/// <param name="content"></param>
	/// <param name="tag"></param>
	/// <returns></returns>
	static std::vector<XtmlTag> findTags(const std::string& content, const std::string& tag);

	/// <summary>
	/// Read definition blocks from the content and populate the vars map
	/// </summary>
	/// <param name="content"></param>
	/// <param name="vars"></param>
	static void readDefinitionBlocks(std::string& content, std::map<std::string, var>& vars);

	/// <summary>
	/// Write content to a file
	/// </summary>
	/// <param name="content"></param>
	/// <param name="output_path"></param>
	static void writeFile(const std::string& content, const std::string& output_path);

	/// <summary>
	/// Parse attributes from an XHTML tag string
	/// </summary>
	/// <param name="tag"></param>
	/// <returns></returns>
	static std::map<std::string, std::string> parseXtmlAttributes(const std::string& tag);

	/// <summary>
	/// Resolve placeholders in the content using the provided vars map
	/// </summary>
	/// <param name="content"></param>
	/// <param name="vars"></param>
	/// <returns></returns>
	static std::string resolvePlaceholders(const std::string& content, const std::map<std::string, var>& vars);

	/// <summary>
	/// Merge source program vars and functions into destination program
	/// </summary>
	/// <param name="source"></param>
	/// <param name="destination"></param>
	static void mergePrograms(const Program& source, Program& destination);

	/// <summary>
	/// Merge source program functions into destination program
	/// </summary>
	/// <param name="source"></param>
	/// <param name="destination"></param>
	static void mergeProgrammFunctions(const Program& source, Program& destination);
};

