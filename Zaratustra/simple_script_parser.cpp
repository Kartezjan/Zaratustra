#include "simple_script_parser.h"

std::vector<command> commands_queue(std::string file_path) {
	std::vector<command> output;
	const std::locale empty_locale = std::locale::empty();
	typedef std::codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const std::locale utf8_locale = std::locale(empty_locale, converter);
	std::wifstream script_file;
	script_file.imbue(utf8_locale);
	script_file.open(file_path);
	wchar_t *buffer = new wchar_t[1024];
	while (script_file.getline(buffer, 1024)) {
		command current_command;
		std::wstringstream current_line(buffer);
		std::wstring current_arg;
		current_line >> current_arg;
		if (current_arg == L"analize_vocabulary")
			current_command.type = CMD_TYPE::CMD_ANALIZE_VOCABULARY;
		else if (current_arg == L"search_word")
			current_command.type = CMD_TYPE::CMD_WORD_SEARCH;
		else {
			current_command.type = CMD_TYPE::CMD_UNKNOWN;
			continue;
		}
		size_t quote_pos = current_line.str().find_first_of(L'\"');
		while (!current_line.eof()) {
			if (quote_pos >= static_cast<size_t>(current_line.tellg()) + 2)  {
				current_line >> current_arg;
				current_command.args.push_back(current_arg);
			}
			else {
				std::wstring quote_string;
				quote_pos = current_line.str().find_first_of(L'\"', quote_pos + 1);
				while (quote_pos > static_cast<size_t>(current_line.tellg()) && !current_line.eof()) {
					current_line >> current_arg;
					quote_string.append(current_arg);
					quote_string.append(L" ");
				}
				quote_string.erase(quote_string.size() - 1, 1);
				quote_pos = quote_string.find_first_of(L'\"');
				while (quote_pos != std::wstring::npos) {
					quote_string.erase(quote_pos, 1);
					quote_pos = quote_string.find_first_of(L'\"', quote_pos + 1);
				}
				current_command.args.push_back(quote_string);
			}
		} 
		output.push_back(current_command);
	}

	return output;
}