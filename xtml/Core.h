#pragma once
#include <string>
#include <vector>
#include <map>
#include "Vars.h"

using namespace std;

struct XtmlTag {
	string full;
	string head;
	string content;
	bool self_closing = false;
	map<string, string> attributes;
};

class Core
{
public:
	static vector<string> parse_blocks(const string& content, const string& start_tag, const string& end_tag);
	static map<string, var> parse_block(const std::string& content);
	static string resolve_include(const string& include_path, map<string, var>& vars, XtmlTag tag, bool resolve_global = true);
	static string remove_blocks(const string& content, const string& start_tag, const string& end_tag);
	static string clean_content(string& content);	
	static string build_file(const string& path, map<string, var>& vars);
	static string build_content(string& content, string base_path, map<string, var>& vars);
	static void write_file(const string& content, const string& output_path);
	static vector<XtmlTag> find_xtml_tags(const string& content);
	static map<string, string> parse_xtml_attributes(const string& tag);
	static map<string, var> params_to_vars(const map<string, string>& params);
	static vector<string> find_unresolved_vars(const string& content);
	static tuple<string, var> resolve_self_closing_var(XtmlTag tag);
};

