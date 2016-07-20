#include "config.h"

std::vector<config_entry> load_cfg_file(std::wifstream &config_file) {
	std::vector<config_entry> output;
	wstring buffer;
	buffer.resize(1024);
	while (config_file.getline(&buffer[0], 1024)) {
		wstring value;
		config_entry entry;
		value.resize(1024);
		size_t eqs_pos = buffer.find_first_of(L'=');
		if (eqs_pos == wstring::npos) {
			buffer.clear();
			buffer.resize(1024);
			continue;
		}
		buffer.copy(&value[0], wstring::npos, eqs_pos + 1);
		buffer.erase(eqs_pos, wstring::npos);
		if (buffer == L"dictionary")
			entry.type = CFG_DICTIONARY;
		else if (buffer == L"ext_dictionary")
			entry.type = CFG_EXT_DICTIONARY;
		else if (buffer == L"supp_dictionary")
			entry.type = CFG_SUPP_DICTIONARY;
		else if (buffer == L"aff_file")
			entry.type = CFG_AFF_FILE;
		else
			entry.type = CFG_UNKNOWN;
		entry.value = value;
		output.push_back(entry);
		buffer.clear();
		buffer.resize(1024);
	}
	return output;
}

config process_cfg(wstring cfg_path) {
	const std::locale empty_locale = std::locale::empty();
	typedef std::codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const std::locale utf8_locale = std::locale(empty_locale, converter);
	wifstream cfg_file(cfg_path);
	cfg_file.imbue(utf8_locale);
	auto entries = load_cfg_file(cfg_file);
	config configured;
	for(int i = 0; i < entries.size(); ++i)
		switch (entries[i].type) {
		case CFG_DICTIONARY:
			configured.dictionary_path = entries[i].value;
			break;
		case CFG_EXT_DICTIONARY:
			configured.ext_dictionary_path = entries[i].value;
			break;
		case CFG_SUPP_DICTIONARY:
			configured.supp_dictionary_path = entries[i].value;
			break;
		case CFG_AFF_FILE:
			configured.aff_file_path = entries[i].value;
			break;
		}

	return configured;
}
