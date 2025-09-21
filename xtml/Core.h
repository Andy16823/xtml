#pragma once
#include <string>
#include <vector>

using namespace std;

class Core
{
public:
	static vector<string> parse_blocks(const string& content, const string& start_tag, const string& end_tag);
	static string remove_blocks(const string& content, const string& start_tag, const string& end_tag);
};

