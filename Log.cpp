#include "Log.h"

void Log(const char* arg) {
	for (int i = 0; i < strlen(arg); i++) {
		std::cout << arg[i];
	}
	std::cout << std::endl;
	//write on file
}

void Log(std::string arg) {
	const char* b = arg.c_str();
	for (int i = 0; i < arg.length(); i++) {
		std::cout << b[i];
	}
	std::cout << std::endl;
}