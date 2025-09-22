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

	// Get the raw file name
	auto file_name = Utils::file_name(file_path);
	file_name = Utils::file_name_no_ext(file_name) + ".html";

	// Get the file directory
	auto file_dir = Utils::file_path_parent(file_path);
	auto output_path = file_dir + "\\" + file_name;

	// Build the file and write to output
	map<string, var> vars;
	auto content = Core::build_file(file_path, vars);
	Core::write_file(content, output_path);
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