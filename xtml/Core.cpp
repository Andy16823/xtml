#include "Core.h"
#include <regex>

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

string Core::remove_blocks(const string& content, const string& start_tag, const string& end_tag)
{
	// Remove blocks from content based on start and end tags
	string pattern = start_tag + "[\\s\\S]*?" + end_tag;
	regex re(pattern);
	auto result = regex_replace(content, re, "");
	return result;
}
