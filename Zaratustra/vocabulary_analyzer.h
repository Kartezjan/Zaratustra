#pragma once
#include <iostream>
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

void vocabulary_analyzer(wstring filename, config &config);
