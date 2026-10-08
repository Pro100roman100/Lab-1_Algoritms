#include "config_loader.h"

#include <fstream>
#include <sstream>

std::string ConfigLoader::Trim(const std::string& value) {
	const size_t first = value.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return "";
	}

	const size_t last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

ParseCode ConfigLoader::ParseFloat(const std::string& value, float& result, float min, float max) {
	try {
		const float parsed = std::stof(value);
		if (parsed <= min || parsed >= max) {
			return ParseCode::out_of_bounds;
		}
		result = parsed;
		return ParseCode::no_err;
	}
	catch (...) {
		return ParseCode::invalid_value;
	}
}
ParseCode ConfigLoader::ParseInt(const std::string& value, int& result, int min, int max) {
	try {
		const int parsed = std::stoi(value);
		if (parsed < min || parsed > max) {
			return ParseCode::out_of_bounds;
		}
		result = parsed;
		return ParseCode::no_err;
	}
	catch (...) {
		return ParseCode::invalid_value;
	}
}

ParseCode ConfigLoader::SetParameter(std::string& key, std::string& value) {
	to_lower_case(key);
	to_lower_case(value);
	GameSettings& settings = GameSettings::GetInstance();

	if (key == "spawn_interval") {
		return ParseFloat(value, settings.spawnInterval);
	}
	if (key == "min_enemy_speed") {
		const float oldValue = settings.minEnemySpeed;
		if (ParseCode code = ParseFloat(value, settings.minEnemySpeed))
			return code;
		else if(settings.minEnemySpeed > settings.maxEnemySpeed) {
			settings.minEnemySpeed = oldValue;
		}
		return ParseCode::no_err;
	}
	if (key == "max_enemy_speed") {
		const float oldValue = settings.maxEnemySpeed;
		if (ParseCode code = ParseFloat(value, settings.maxEnemySpeed))
			return code;
		else if (settings.minEnemySpeed < settings.maxEnemySpeed) {
			settings.minEnemySpeed = oldValue;
		}
		return ParseCode::no_err;
	}
	if (key == "max_enemies") {
		return ParseInt(value, settings.maxEnemies);
	}
	if (key == "max_fps") {
		ParseCode result = ParseInt(value, settings.maxFps, 15);
		if (result == ParseCode::no_err)
			SetTargetFPS(GameSettings::GetInstance().maxFps);
		return result;
	}
	if (key == "log_show_time") {
		return ParseFloat(value, settings.logShowTime);
	}

	return ParseCode::invalid_name;
}

std::string ConfigLoader::GetErrorString(ParseCode code, std::string& key, std::string& value) {
	switch (code) {
	case ParseCode::no_err:
		return "";
	case ParseCode::invalid_name:
		return "Set " + key + " error: Invalid name";
	case ParseCode::invalid_value:
		return "Set " + key + " error: Invalid value " + value;
	case ParseCode::out_of_bounds:
		return "Set " + key + " error: Value " + value + " Out of bounds ";
	default:
		return "Set " + key + " error";
	}
}

bool ConfigLoader::Load(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		return false;
	}

	GameSettings& settings = GameSettings::GetInstance();
	const float previousMinSpeed = settings.minEnemySpeed;
	const float previousMaxSpeed = settings.maxEnemySpeed;
	std::string line;
	while (std::getline(file, line)) {
		line = Trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}

		const size_t separator = line.find('=');
		if (separator == std::string::npos) {
			continue;
		}

		std::string key = Trim(line.substr(0, separator));
		std::string value = Trim(line.substr(separator + 1));

		ParseCode code = SetParameter(key, value);
		if (code != ParseCode::no_err)
			LOG(GetErrorString(code, key, value));
	}

	if (settings.minEnemySpeed > settings.maxEnemySpeed) {
		settings.minEnemySpeed = previousMinSpeed;
		settings.maxEnemySpeed = previousMaxSpeed;
		return false;
	}

	return true;
}
