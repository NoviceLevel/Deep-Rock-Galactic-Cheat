#pragma once
#include <string>
#include <vector>
#include "pch.h"

#define HASH(str) std::hash<std::string>{}(str)

static ImWchar DefaultRanges[] = { 0x0020, 0x00FF, 0 };
static ImWchar PolishRanges[] = { 0x0020, 0x00FF, 0x00A0, 0x02D9, 0 };
static ImWchar ChineseRanges[] = { 
	0x0020, 0x00FF, // Basic Latin + Latin Supplement
	0x2000, 0x206F, // General Punctuation
	0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
	0x31F0, 0x31FF, // Katakana Phonetic Extensions
	0xFF00, 0xFFEF, // Half-width characters
	0x4e00, 0x9FAF, // CJK Ideograms
	0
};

struct LocaleData
{
	size_t Key;
	std::string Value;
};

struct LocalizationData
{
	std::string Name;
	size_t LocaleCode;
	ImFont** Font;
	ImFont** FontESP;

	std::vector<LocaleData> Locales;
};