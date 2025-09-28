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
#include "Globals.h"
#include "Utils.h"
#include "Vars.h"
#include "Core.h"
#include "FunctionRegistry.h"
#include "ModuleStd.h"
#include "Module.h"
#include <filesystem>
#include <Windows.h>
#include <filesystem>

#define VERSION "0.0.0.1"

namespace fs = std::filesystem;

FunctionRegistry g_functionRegistry;

typedef Module* (*CreateModuleFunc)();

void loadModulesFromFolder(const std::string& folderPath) {
	for (const auto& entry : fs::directory_iterator(folderPath)) {
		if (entry.path().extension() == ".dll") {
			std::string dllPath = entry.path().string();
			HMODULE hModule = LoadLibraryA(dllPath.c_str());
			if (!hModule) {
				std::cerr << "Fehler: konnte " << dllPath << " nicht laden." << std::endl;
				continue;
			}

			auto createModule = (CreateModuleFunc)GetProcAddress(hModule, "CreateModule");
			if (!createModule) {
				std::cerr << "Fehler: CreateModule nicht gefunden in " << dllPath << std::endl;
				FreeLibrary(hModule);
				continue;
			}

			Module* plugin = createModule();
			plugin->RegisterFunctions(g_functionRegistry);
		}
	}
}

std::string getExeDir() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	fs::path exePath(buffer);
	return exePath.parent_path().string();
}

void action_build(const std::string& file_path) {

	// Get the raw file name
	auto file_name = Utils::file_name(file_path);
	file_name = Utils::file_name_no_ext(file_name) + ".html";

	// Get the file directory
	auto file_dir = Utils::file_path_parent(file_path);
	auto output_path = file_dir + "\\" + file_name;

	// Build the file and write to output
	std::map<std::string, var> vars;
	auto content = Core::build_file(file_path, vars);
	Core::write_file(content, output_path);
}

int main(int argc, char* argv[])  
{  
	auto exe_path = getExeDir();
	auto modules_path = exe_path + "\\modules";
	if (!fs::exists(modules_path)) {
		fs::create_directory(modules_path);
	}
	loadModulesFromFolder(modules_path);

	// Register standard functions
	ModuleStd stdModule;
	stdModule.RegisterFunctions(g_functionRegistry);
	


	if (argc < 2) {  
		Utils::printerr_ln("Usage: <command> <file_path>");
		return 1;  
	}  

	std::string command = argv[1];

	if (command == "version") {
		Utils::print_ln(std::string("xtml version: ") + VERSION);
		return 0;
	}
	else if (command == "build") {
		action_build(argv[2]);
	}


	return 0;  
}