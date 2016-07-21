#include "dictionary.h"

ulong create_hash_from_word(wstring word) {
	ulong hash = 0;
	for (int i = 0; i < 6; ++i) {
		hash += (static_cast<unsigned long long>(word[(2 * i) % word.size()]) % 16) * static_cast<unsigned long long>(pow(16, i));
	}
	return hash;
}

/*ULONG create_hash_from_word(wstring word) {
	ULONG initial_hash = 0;
	int k = 0;
	int l = 0;
	size_t max = word.size() > 6 ? 6 : word.size();
	for (size_t i = 0; i < max; ++i)
		initial_hash += (static_cast<unsigned long long>(word[(3 * i) % word.size()]) % 256) * static_cast<unsigned long long>(pow(256, i));
	bitset<64> bit_array(initial_hash);
	bitset<24> hashed(0);
	for (size_t i = 0; i < 6; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			if (bit_array.test(l + j)) {
				hashed.set(k);
			}
			++k;
		}
		l += 8;
	}
	return hashed.to_ullong();
}*/

void test_hash_table_thread(vector<wstring>* hash_table, vector<pair<size_t, vector<wstring>*> >* result_table, ulong a, ulong b)
{
	for (size_t i = a; i < b; ++i) {
		if (hash_table[i].size() == 0) {
		}
		else if (hash_table[i].size() > 0) {
			auto result = make_pair(i, &hash_table[i]);
			result_table->push_back(result);
		}
	}
}

void test_hash_table(vector<wstring>* hash_table, vector<pair<size_t, vector<wstring>*> >* result_table)
{
	thread threads[8];
	for (int i = 0; i < 8; ++i)
		threads[i] = thread(test_hash_table_thread, hash_table, result_table, (i / 8) * DICTIONARY_HASH_TABLE_SIZE, (i + 1) / 8 * DICTIONARY_HASH_TABLE_SIZE);
	for (int i = 0; i < 8; ++i)
		threads[i].join();
}

void hash_all_dictionary_words(wifstream& dictionary, vector<wstring>* hash_table) {
	wstring current;
	while (dictionary >> current) {
		size_t pos = current.find_first_of(L"/");
		if (pos != wstring::npos)
			current.erase(pos, wstring::npos);
		hash_table[create_hash_from_word(current)].push_back(current);

	}

	//vector<pair<size_t, vector<wstring>*> > crowdedHashes;

	//test_hash_table(hash_table, &crowdedHashes);

	//sort(crowdedHashes.begin(), crowdedHashes.end(), [](pair<size_t, vector<wstring>*> a, pair<size_t, vector<wstring>*> b) {
	//	return a.second->size() > b.second->size();
	//});

	//int sum = 0;
	//for (int i = 0; i < crowdedHashes.size(); ++i) {
	//	sum += crowdedHashes[i].second->size();
	//}
}

void read_aff_file(wifstream& aff_file, vector<affix_flag>* affix_array){
	wstring current_line;
	current_line.resize(1024);
	while (aff_file.getline(&current_line[0], 1024) ) {
		vector<affix_entry> flag_entries;
		affix_flag current_flag;
		wstringstream wss(current_line);
		wstring current_field;
		wss >> current_field;
		if (current_field != L"SFX" && current_field != L"PFX")
			continue;
		current_flag.type = current_field;
		wss >> current_field;
		current_flag.name = current_field[0];
		wss >> current_field;
		if (current_field == L"Y")
			current_flag.combinable_with_prefixes = true;
		else
			current_flag.combinable_with_prefixes = false;
		wss >> current_field;
		current_flag.entry_count = wcstol(&current_field[0], NULL, 10);
		size_t i = 0;
		while (i < current_flag.entry_count && aff_file.getline(&current_line[0], 1024) ) {
			affix_entry current_entry;
			wstringstream wss(current_line);
			wss >> current_field;
			if (current_field != current_flag.type)
				continue;
			wss >> current_field;
			if (current_field[0] != current_flag.name)
				continue;
			wss >> current_field;
			current_entry.to_strip_off = current_field;
			wss >> current_field;
			current_entry.to_add = current_field;
			wss >> current_field;
			size_t pos = current_field.find_first_of(L'\0');
			current_field.erase(pos, wstring::npos);
			current_entry.conditions = current_field;
			flag_entries.push_back(current_entry);
			++i;
		}
		current_flag.entries = flag_entries;
		affix_array[current_flag.name].push_back(current_flag);
	}
}

bool are_conditions_met(wstring word, wstring conditions) {
	size_t overall_conditions_len = 0;
	vector<condition> conditions_vector;
	size_t open_bracket_pos = conditions.find_first_of(L"[");
	if (open_bracket_pos != 0 && open_bracket_pos != wstring::npos) {
		wstring condition_string;
		size_t len = open_bracket_pos;
		overall_conditions_len += len;
		condition_string.resize(len);
		conditions.copy(&condition_string[0], len, 0);
		conditions.erase(0, len);
		open_bracket_pos = 0;
		conditions_vector.push_back(condition(STRING_OCCURS, condition_string));
	}
	size_t closed_bracket_pos = conditions.find_first_of(L"]", open_bracket_pos + 1);
	while (open_bracket_pos != wstring::npos && closed_bracket_pos != wstring::npos) {
		condition current_condition;
		wstring condition_string;
		size_t len = closed_bracket_pos - open_bracket_pos - 1;
		++overall_conditions_len;
		condition_string.resize(len);
		conditions.copy(&condition_string[0], len, open_bracket_pos + 1);
		conditions.erase(open_bracket_pos, len + 2);
		if (condition_string[0] == '^') {
			current_condition.type = FOLLOWING_CHARS_NOT_OCCUR;
			condition_string.erase(0, 1);
		}
		else
			current_condition.type = FOLLOWING_CHARS_OCCUR;
		current_condition.content = condition_string;
		conditions_vector.push_back(current_condition);
		open_bracket_pos = conditions.find_first_of(L"[");
		closed_bracket_pos = conditions.find_first_of(L"]", open_bracket_pos + 1);
	}
	conditions_vector.push_back(condition(STRING_OCCURS, conditions));
	overall_conditions_len += conditions.length();

	size_t current_position;
	if (word.length() >= overall_conditions_len)
		current_position = word.length() - overall_conditions_len;
	else
		current_position = 0;
	
	for (int i = 0; i < conditions_vector.size(); ++i) {
		size_t pos;
		bool found = false;
		switch (conditions_vector[i].type) {
		case STRING_OCCURS:
			if (conditions_vector[i].content[0] == L'.') //the dot means that no condition is required
				return true;
			pos = word.find(conditions_vector[i].content, current_position);
			if (pos != current_position)
				return false;
			current_position += conditions_vector[i].content.size();
			break;
		case FOLLOWING_CHARS_OCCUR:
			for (int j = 0; j < conditions_vector[i].content.size(); ++j) {
				if (conditions_vector[i].content[j] == word[current_position])
					found = true;
			}
			if (!found)
				return false;
			++current_position;
			break;
		case FOLLOWING_CHARS_NOT_OCCUR:
			for (int j = 0; j < conditions_vector[i].content.size(); ++j) {
				if (conditions_vector[i].content[j] == word[current_position])
					found = true;
			}
			if (found)
				return false;
			++current_position;
			break;
		default:
			break;
		}
	}

	return true;
}

wstring substract_flags(wstring &entry) {
	size_t pos;
	wstring flags;
	flags.resize(128);
	pos = entry.find_first_of(L"/");
	if (pos == wstring::npos)
		return L"0";
	size_t char_copied = entry.copy(&flags[0], 128, pos + 1);
	entry.erase(pos, wstring::npos);
	flags.resize(char_copied);
	return flags;
}

void mutate_word(wstring word, wstring flags, vector<affix_flag>* affix_array, wstring* destination) {
	for (int i = 0; i < flags.size(); ++i) {
		bool SFX = true; //SFX if true, PFX otherwise
		vector<affix_entry>* affixes = &affix_array[flags[i]][0].entries;
		if (affix_array[flags[i]][0].type != L"SFX") {
			for (int j = 0; j < affixes->size(); ++j) {
				wstring word_copy = word;
				word_copy.insert(0, affixes->data()[j].to_add);
				word_copy.append(L"\n");
				destination->append(word_copy);
			}
			continue;
		}
		for (int j = 0; j < affixes->size(); ++j) {
			if (are_conditions_met(word, affixes->data()[j].conditions)) {
				wstring word_copy = word;
				if (affixes->data()[j].to_strip_off != L"0") {
					for (int k = affixes->data()[j].to_strip_off.size() - 1; k >= 0; --k) {
						size_t pos = word_copy.find_last_of(affixes->data()[j].to_strip_off[k]);
						if (pos != wstring::npos)
							word_copy.erase(pos, 1);
					}
				}
				if (affixes->data()[j].to_add != L"0")
					word_copy.append(affixes->data()[j].to_add);
				word_copy.append(L"\n");
				destination->append(word_copy);
			}
		}
	}
}

void word_mutation_thread(wstring content, vector<affix_flag>* affix_array, wstring* destination) {
	wstringstream content_stream(content);
	wstring current_word;
	while (content_stream >> current_word) {
		size_t pos;
		wstring flags;
		flags.resize(128);
		pos = current_word.find_first_of(L"/");
		if (pos == wstring::npos) {
			current_word.append(L"\n");
			destination->append(current_word);
			continue;
		}
		size_t char_copied = current_word.copy(&flags[0], 128, pos + 1);
		current_word.erase(pos, wstring::npos);
		flags.resize(char_copied);
		mutate_word(current_word, flags, affix_array, destination);
		current_word.append(L"\n");
		destination->append(current_word);
	}
}

void generate_word_variants_from_aff(wifstream& dictionary, size_t line_count) {
	config config = process_cfg(L"config.cfg");

	size_t dictionary_generation_cost = 0;
	thread threads[MAX_THREADS];
	wstring sources[MAX_THREADS];
	wstring destinations[MAX_THREADS];
	const locale empty_locale = locale::empty();
	typedef codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const locale utf8_locale = locale(empty_locale, converter);

	vector<affix_flag>* affix_array = new vector<affix_flag>[128];
	wifstream aff_file(config.aff_file_path);
	aff_file.imbue(utf8_locale);
	read_aff_file(aff_file, affix_array);
	aff_file.close();

	wstring current_word;

	while (dictionary >> current_word) {
		wstring flags = substract_flags(current_word);
		if (flags == L"0")
			continue;
		for (int i = 0; i < flags.size(); ++i)
			dictionary_generation_cost += affix_array[flags[i]][0].entry_count;
	}

	dictionary.clear();
	dictionary.seekg(0, dictionary.beg);
	dictionary >> line_count;

	for (int i = 0; i < MAX_THREADS; ++i) {
		bool cost_limit_reached = false;
		bool EOF_reached = false;
		size_t current_cost = 0;
		while(!cost_limit_reached && !EOF_reached) {
			wchar_t buffer[10240];
			if (!dictionary.getline(buffer, 10240))
				EOF_reached = true;
			sources[i].append(buffer);
			sources[i].append(L" ");
			wstring word = buffer;
			wstring flags = substract_flags(word);
			if (flags == L"0")
				continue;
			for (int i = 0; i < flags.size(); ++i)
				current_cost += affix_array[flags[i]][0].entry_count;
			if (current_cost > dictionary_generation_cost / MAX_THREADS)
				cost_limit_reached = true;
		}
	}

	for (int i = 0; i < MAX_THREADS; ++i)
		threads[i] = thread(word_mutation_thread, sources[i],affix_array, &destinations[i]);

	for (int i = 0; i < MAX_THREADS; ++i)
		threads[i].join();

	wofstream output(config.ext_dictionary_path);
	output.imbue(utf8_locale);
	for (int i = 0; i < MAX_THREADS; ++i) {
		output.write(&destinations[i][0], destinations[i].size());
	}
	output.close();
	delete[] affix_array;
}