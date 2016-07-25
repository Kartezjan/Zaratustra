#include "word_search.h"

bool match_words(wstring sentence, vector<wstring> &word_list) {
	bool found = false;
	wstringstream wss(sentence);
	wstring buffer;
	locale::global(locale(""));
	auto& f = use_facet<ctype<wchar_t>>(locale());
	while (wss >> buffer) {
		f.tolower(&buffer[0], &buffer[0] + buffer.size());

		for (int i = 0; i < word_list.size(); ++i) {
			if (buffer == word_list[i]) {
				found = true;
				break;
			}
		}
	}
	return found;
}

void write_all_sentences_with_specific_words(wstring filepath, wstring word_list_path, config& cfg) {
	const locale empty_locale = locale::empty();
	typedef codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const locale utf8_locale = locale(empty_locale, converter);

	wofstream output("output.txt", ios::app);
	output.imbue(utf8_locale);

	Log(L"Reading aff file...");
	vector<affix_flag>* affix_array = new vector<affix_flag>[128];
	wifstream aff_file(cfg.aff_file_path);
	aff_file.imbue(utf8_locale);
	read_aff_file(aff_file, affix_array);
	aff_file.close();

	Log(L"Searching for specified words...");
	aff_file.open(word_list_path);
	wstring words;
	wchar_t *buffer = new wchar_t[1024];
	while (aff_file.getline(buffer, 1024)) {
		wstring current(buffer);
		auto flags = substract_flags(current);
		if(flags != L"0")
			mutate_word(current, flags, affix_array, &words);
		words.append(current);
		words.append(L"\n");
	}
	aff_file.close();

	wstring current_word;
	vector<wstring> word_list;
	wstringstream wss(words);
	while (wss >> current_word)
		word_list.push_back(current_word);


	delete[] buffer;
	delete[] affix_array;

	wchar_t *current_text_line = new wchar_t[262144];
	aff_file.open(filepath);
	while (aff_file.getline(current_text_line, 262144)) {
	wstring current_line(current_text_line);
		size_t dot_pos = current_line.find_first_of(L".?!");
		while (current_line.size() != 0) {
			wstring current_sentence;
			if (dot_pos != wstring::npos)
				current_sentence.resize(dot_pos + 1);
			else {
				current_sentence.resize(current_line.size());
				--dot_pos; 
			}
			current_line.copy(&current_sentence[0], dot_pos, 0);
			current_line.erase(0, dot_pos + 1);
			if (match_words(current_sentence, word_list)) {
				current_sentence.erase(current_sentence.size() - 1, 1);
				current_sentence.append(L"\n");
				output.write(&current_sentence[0], current_sentence.size());
			}
			dot_pos = current_line.find_first_of(L".?!");
		}
	}
	output.close();
}