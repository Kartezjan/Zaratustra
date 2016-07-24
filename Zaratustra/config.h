#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>

#define ENTRY_TYPE_COUNT 4

using namespace std;

enum config_entry_type {
	CFG_UNKNOWN, CFG_DICTIONARY, CFG_EXT_DICTIONARY, CFG_SUPP_DICTIONARY, CFG_AFF_FILE, CFG_ZST_MAIN_FILE, CFG_ZST_SUPP_FILE
};

struct config_entry {
	config_entry_type type;
	wstring value;
};

struct config {
	wstring supp_dictionary_path;
	wstring dictionary_path;
	wstring ext_dictionary_path;
	wstring aff_file_path;
	wstring zst_main_file_path;
	wstring zst_supp_file_path;
};

std::vector<config_entry> load_cfg_file(std::wifstream &config_file);
config process_cfg(wstring);


