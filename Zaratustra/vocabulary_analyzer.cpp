﻿#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <bitset>
#include <filesystem>

#include "httpsock.h"
#include "dictionary.h"
#include "typesafe_sprintf.h"
#include "config.h"

#define ULONG unsigned long long
#define DICTIONARY_HASH_TABLE_SIZE 16777216

using namespace std;

void typesafe_sprintf_detail(size_t, std::string&) {}
void typesafe_sprintf_detail(size_t, std::wstring&) {}

wstring remove_special_charactes(const wstring& original) {
	wstring output = L"";
	for (size_t i = 0; i < original.size(); ++i) {
		if (original[i] >= 0x21 && original[i] <= 0x40)
			continue;
		if (original[i] == L'—' || original[i] == L'„' || original[i] == L'”' || original[i] == L'»' || original[i] == L'«')
			continue;
		output.push_back(original[i]);
	}
	return output;
}

bool check_word_in_pwn(wstring word, httpSock& pwn) {
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	ofstream file;
	file.open("a.html");
	stringstream url;
	std::string narrow = converter.to_bytes(word);
	url << "/szukaj/" << narrow << ".html";
	pwn.Get(url.str());
	std::string info = pwn.RecNChars(30000);
	file << info;
	file.close();

	if (info.find("Nie znaleziono") == string::npos)
		return true;
	return false;
}

bool check_word_in_local_dictionary(wstring word, vector<wstring>* hash_table) {
	auto hash = create_hash_from_word(word);
	bool found = false;
	for (int i = 0; i < hash_table[hash].size();++i) {
		if (word == hash_table[hash][i])
			found = true;
	}
	if (found)
		return true;
	return false;
}

std::vector<wstring> detectNeologisms(const wstring& source, vector<wstring>* hash_table, wstring support_dictionary_path)
{
	const std::locale empty_locale = std::locale::empty();
	typedef std::codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const std::locale utf8_locale = std::locale(empty_locale, converter);

	httpSock pwn("sjp.pwn.pl", DEFAULT_HTTP_PORT);

	wofstream support_dictionary(support_dictionary_path, ios::app);
	support_dictionary.imbue(utf8_locale);

	size_t total_word_count = std::count(std::istreambuf_iterator<wchar_t>(wstringstream(source)), std::istreambuf_iterator<wchar_t>(), L' ') + 1;

	vector<wstring> output;
	wstringstream ss(source);
	wstring buffer;
	size_t current_word_count = 0;
	while (ss >> buffer) {
		++current_word_count;
		if (current_word_count % 1000 == 0) {
			system("cls");
			double progress = static_cast<double>(current_word_count) / total_word_count * 100;
			log(L"%x\%", progress);
		}
		std::locale::global(std::locale(""));
		auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
		f.tolower(&buffer[0], &buffer[0] + buffer.size());

		if (!check_word_in_local_dictionary(buffer, hash_table)) {
			log(L"Nie znaleziono slowa \"%x\" w slowniku lokalnym.\nSzukanie w slowniku online...", buffer);
			if (!check_word_in_pwn(buffer, pwn)) {
				log(L"Podane slowo to neologizm.");
				output.push_back(buffer);
				hash_table[create_hash_from_word(buffer)].push_back(buffer);
			}
			else {
				log(L"Podane slowo znajduje sie w slowniku online PWN.\nZostanie dodane do pomocniczego slownika lokalnego.");
				hash_table[create_hash_from_word(buffer)].push_back(buffer);
				buffer.append(L"\n");
				support_dictionary << buffer;
			}
		}
	}
	support_dictionary.close();
	return output;
}

void main(int argc, char **argv) {
	if (argc < 2)
		return;
	const std::locale empty_locale = std::locale::empty();
    typedef std::codecvt_utf8<wchar_t> converter_type;
    const converter_type* converter = new converter_type;
    const std::locale utf8_locale = std::locale(empty_locale, converter);

	config config = process_cfg(L"config.cfg");

	if (!experimental::filesystem::exists(config.ext_dictionary_path)) {
		size_t line_len = 0;
		log(L"Nie znaleziono slownika\n Generowanie slownika...");
		wifstream open_office_dictionary;
		open_office_dictionary.imbue(utf8_locale);
		open_office_dictionary.open(config.dictionary_path);
		open_office_dictionary >> line_len;
		generate_word_variants_from_aff(open_office_dictionary, line_len);
	}
	//------------------------------------------------------------
	log(L"Wczytywanie slownika...");
	wifstream dictionary;
	dictionary.imbue(utf8_locale);
	dictionary.open(config.ext_dictionary_path);
	vector<wstring>* hash_table = new vector<wstring>[DICTIONARY_HASH_TABLE_SIZE];
	hash_all_dictionary_words(dictionary, hash_table);
	dictionary.close();

	log(L"Wczytywanie slownika pomocniczego...");
	wifstream support_dictionary(config.supp_dictionary_path);
	support_dictionary.imbue(utf8_locale);
	hash_all_dictionary_words(support_dictionary, hash_table);
	support_dictionary.close();
	//-------------------------------------------------------------
	log(L"Wczytywanie tekstu zrodlowego...");
	wifstream zaratustra;
	zaratustra.imbue(utf8_locale);
	zaratustra.open(argv[1]);
	zaratustra.seekg(0, zaratustra.end);
	size_t length = zaratustra.tellg();
	zaratustra.seekg(0, zaratustra.beg);
	wstring book;
	book.resize(length);
	zaratustra.read(&book[0], length);
	zaratustra.close();
	wstring book_raw = remove_special_charactes(book);
	auto result = detectNeologisms(book_raw, hash_table, config.supp_dictionary_path);

	wofstream neologisms;
	neologisms.imbue(utf8_locale);
	neologisms.open("neologisms.txt");

	log(L"Znalezione neologizmy:");
	for (int i = 0; i < result.size(); ++i) {
		log(L"%x", result[i]);
		neologisms << result[i] << "\n";
	}

}