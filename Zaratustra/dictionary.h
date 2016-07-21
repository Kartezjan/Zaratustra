#pragma once

#include <bitset>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <wchar.h>
#include <algorithm>
#include <istream>

#include "config.h"

#define DICTIONARY_HASH_TABLE_SIZE 16777216
#define MAX_THREADS 3

using namespace std;

typedef unsigned long long ulong;


enum CONDITION_TYPE {STRING_OCCURS, FOLLOWING_CHARS_OCCUR, FOLLOWING_CHARS_NOT_OCCUR};

struct condition {
	condition() {};
	condition(CONDITION_TYPE new_type, wstring new_content) { type = new_type; content = new_content; };
	CONDITION_TYPE type;
	wstring content;
};

struct affix_entry {
	affix_entry() {};
	affix_entry(wstring string_to_remove, wstring string_to_add, wstring conditions_string){
		to_strip_off = string_to_remove;
		to_add = string_to_add;
		conditions = conditions_string;
	};
	wstring to_strip_off;
	wstring to_add;
	wstring conditions;
};

struct affix_flag {
	affix_flag() {};
	affix_flag(wstring flag_type, wchar_t flag_name, bool is_combinable, size_t flag_entry_count, vector<affix_entry> flag_entries) {
		type = flag_type;
		name = flag_name;
		combinable_with_prefixes = is_combinable;
		entry_count = flag_entry_count;
		entries = flag_entries;
	};
	wstring type;
	wchar_t name;
	bool combinable_with_prefixes;
	size_t entry_count;
	vector<affix_entry> entries;
};


void hash_all_dictionary_words(wifstream& dictionary, std::vector<wstring>* hash_table);
ulong create_hash_from_word(wstring word);
void generate_word_variants_from_aff(wifstream& dictionary, size_t line_len);