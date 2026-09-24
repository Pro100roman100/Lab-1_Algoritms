#pragma once

#include <algorithm>
#include <cctype>
#include <string>

#include "game_settings.h"
#include "log.h"

class ConfigLoader {
public:
	static bool Load(const std::string& filePath);
	static bool SetParameter(std::string& key, std::string& value);
};

inline void to_lower_case(std::string& string) {
	std::transform(string.begin(), string.end(), string.begin(),
		[](unsigned char character) {
			return static_cast<char>(std::tolower(character));
		});
}