#pragma once

#include <algorithm>
#include <cctype>
#include <string>

#include "game_settings.h"
#include "log.h"

enum ParseCode {
	no_err = 0,
	invalid_name = 2,
	invalid_value = 3,
	out_of_bounds = 4,
};

class ConfigLoader {
	static std::string Trim(const std::string& value);
	static ParseCode ParseFloat(const std::string& value, float& result, float min = 0.0f, float max = FLT_MAX);
	static ParseCode ParseInt(const std::string& value, int& result, int min = 0, int max = INT_MAX);

public:
	static bool Load(const std::string& filePath);
	static ParseCode SetParameter(std::string& key, std::string& value);
	static std::string GetErrorString(ParseCode code, std::string& key, std::string& value);
};

inline void to_lower_case(std::string& string) {
	std::transform(string.begin(), string.end(), string.begin(),
		[](unsigned char character) {
			return static_cast<char>(std::tolower(character));
		});
}