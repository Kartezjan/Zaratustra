#pragma once

#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <sstream>
#include <iostream>

enum class CMD_TYPE {CMD_UNKNOWN, CMD_ANALIZE_VOCABULARY, CMD_WORD_SEARCH};

struct command {
	CMD_TYPE type;
	std::vector<std::wstring> args;
};

std::vector<command> commands_queue(std::string file_path);