#pragma once
#include <string>
#include <sstream>
#include <utility>
#include <type_traits>
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <ctime>

void typesafe_sprintf_detail(size_t, std::string&);

template<typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::string& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find('%', starting_pos);

	if (starting_pos != std::string::npos) {
		std::ostringstream replacement;

		auto opcode = target_str[starting_pos + 1];

		if (opcode == L'f') {
			replacement << std::fixed;
			opcode = target_str[starting_pos + 2];
		}

		if (opcode >= L'0' && opcode <= L'9')
			replacement.precision(opcode - L'0');
		else if (opcode == L'*')
			replacement.precision(std::numeric_limits<typename std::decay<T>::type>::digits10);

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template<typename... A>
std::string typesafe_sprintf(std::string f, A&&... a) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

void typesafe_sprintf_detail(size_t, std::wstring&);

template<typename T, typename... A>
void typesafe_sprintf_detail(size_t starting_pos, std::wstring& target_str, T&& val, A&& ...a) {
	starting_pos = target_str.find(L'%', starting_pos);

	if (starting_pos != std::wstring::npos) {
		std::wostringstream replacement;

		auto opcode = target_str[starting_pos + 1];

		if (opcode == L'f') {
			replacement << std::fixed;
			opcode = target_str[starting_pos + 2];
		}

		if (opcode >= L'0' && opcode <= L'9')
			replacement.precision(opcode - L'0');
		else if (opcode == L'*')
			replacement.precision(std::numeric_limits<typename std::decay<T>::type>::digits10);

		replacement << val;
		target_str.replace(starting_pos, 2, replacement.str());
	}

	typesafe_sprintf_detail(starting_pos, target_str, std::forward<A>(a)...);
}

template<typename... A>
std::wstring typesafe_sprintf(std::wstring f, A&&... a) {
	typesafe_sprintf_detail(0, f, std::forward<A>(a)...);
	return f;
}

template<typename... A>
void Log(std::wstring f, A&&... a) {
	auto now = time(0);
	tm date_struct;
	localtime_s(&date_struct, &now);
	char buffer[256];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S ", &date_struct);
	auto generated_string = typesafe_sprintf(L"%x", buffer);
	generated_string.append(typesafe_sprintf(f, std::forward<A>(a)...));
	const std::locale empty_locale = std::locale::empty();
	typedef std::codecvt_utf8<wchar_t> converter_type;
	const converter_type* converter = new converter_type;
	const std::locale utf8_locale = std::locale(empty_locale, converter);
	std::wcout.imbue(utf8_locale);

	std::wcout << generated_string << std::endl;
	std::wofstream log_file("logs.txt", ios::app);
	log_file.imbue(utf8_locale);
	log_file << generated_string << std::endl;
	log_file.close();
}