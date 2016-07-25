#pragma once

#include "dictionary.h"
#include "typesafe_sprintf.h"

using namespace std;

void write_all_sentences_with_specific_words(wstring filepath, wstring word_list_path, config& cfg);