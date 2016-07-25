#include "vocabulary_analyzer.h"
#include "simple_script_parser.h"
#include "word_search.h"

void main(int argc, char **argv) {
	Log(L"Zaratustra launched");
	Log(L"Reading config file...");
	config cfg = process_cfg(L"config.cfg");
	Log(L"Reading script file (%x)...", argv[1]);
	auto commands = commands_queue(argv[1]);
	for (int i = 0; i < commands.size(); ++i) {
		switch (commands[i].type) {
		case CMD_TYPE::CMD_ANALIZE_VOCABULARY:
			vocabulary_analyzer(commands[i].args[0], cfg);
			break;
		case CMD_TYPE::CMD_WORD_SEARCH:
			write_all_sentences_with_specific_words(commands[i].args[0], commands[i].args[1], cfg);
			break;
		default:
			Log(L"Unknown command in line %x", i + 1);
		}
	}
}