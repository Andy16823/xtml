#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <map>
#include <tuple>
#include "Utils.h"
#include "Vars.h"
#include "Core.h"

#define VERSION "0.0.0.1"

using namespace std;

string cleanup_content(string& content) {
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

void create_file(string& content, const string& output_path) {
	std::ofstream file(output_path);
	if (!file.is_open()) {
		throw std::runtime_error("Could not create file: " + output_path);
	}
	file << content;
	file.close();
}

void action_build(const string& file_path) {
	Utils::print_ln(string("Building file ") + file_path);

	auto file_name = Utils::file_name(file_path);
	file_name = Utils::file_name_no_ext(file_name) + ".html";
	auto file_dir = Utils::file_path_parent(file_path);
	Utils::print_ln(string("Output file name: ") + file_dir + string("\\") + file_name);

	auto content = Utils::read_file(file_path);

	map<string, var> vars;
	auto blocks = Core::parse_blocks(content, "<xtml>", "</xtml>");
	for (const auto& block : blocks) {
		auto preprocessed = Vars::preprocess_content(block);
		auto block_vars = Vars::parse_vars(preprocessed);
		vars.insert(block_vars.begin(), block_vars.end());
	}

	//Utils::print_ln("Parsed Variables:");
	//for (const auto& var : vars) {
	//	const string& key = var.first;
	//	const string& value = var.second.value;
	//	Utils::print_ln("Key: " + key + ", Value: " + value);
	//}

	content = Vars::replace_vars(content, vars);
	content = cleanup_content(content);
	content = Core::remove_blocks(content, "<xtml>", "</xtml>");
	content = Utils::trim(content);
	create_file(content, file_dir + "\\" + file_name);
	Utils::print_ln("Build completed.");
}

int main(int argc, char* argv[])  
{  
   if (argc < 2) {  
	   Utils::printerr_ln("Usage: <command> <file_path>");
       return 1;  
   }  

   string command = argv[1];

   if (command == "version") {
	   Utils::print_ln(string("xtml version: ") + VERSION);
	   return 0;
   }
   else if (command == "build") {
	   action_build(argv[2]);
   }


   return 0;  
}