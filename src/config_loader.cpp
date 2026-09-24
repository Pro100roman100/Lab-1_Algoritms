#include "config_loader.h"

#include <fstream>
#include <sstream>

namespace {
std::string Trim(const std::string& value) {
	const size_t first = value.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return "";
	}

	const size_t last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

bool ParsePositiveFloat(const std::string& value, float& result) {
	try {
		const float parsed = std::stof(value);
		if (parsed <= 0.0f) {
			return false;
		}
		result = parsed;
		return true;
	}
	catch (...) {
		return false;
	}
}

bool ParsePositiveInt(const std::string& value, int& result) {
	try {
		const int parsed = std::stoi(value);
		if (parsed <= 0) {
			return false;
		}
		result = parsed;
		return true;
	}
	catch (...) {
		return false;
	}
}
}

bool ConfigLoader::SetParameter(std::string& key, std::string& value) {
	to_lower_case(key);
	to_lower_case(value);
	GameSettings& settings = GameSettings::GetInstance();

	if (key == "spawn_interval") {
		return ParsePositiveFloat(value, settings.spawnInterval);
	}
	if (key == "min_enemy_speed") {
		const float oldValue = settings.minEnemySpeed;
		if (!ParsePositiveFloat(value, settings.minEnemySpeed) ||
			settings.minEnemySpeed > settings.maxEnemySpeed) {
			settings.minEnemySpeed = oldValue;
			return false;
		}
		return true;
	}
	if (key == "max_enemy_speed") {
		const float oldValue = settings.maxEnemySpeed;
		if (!ParsePositiveFloat(value, settings.maxEnemySpeed) ||
			settings.maxEnemySpeed < settings.minEnemySpeed) {
			settings.maxEnemySpeed = oldValue;
			return false;
		}
		return true;
	}
	if (key == "max_enemies") {
		return ParsePositiveInt(value, settings.maxEnemies);
	}
	if (key == "log_show_time") {
		return ParsePositiveFloat(value, settings.logShowTime);
	}

	return false;
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

		if (!SetParameter(key, value)) {
			LOG("Unknown key " + key);
		}
	}

	if (settings.minEnemySpeed > settings.maxEnemySpeed) {
		settings.minEnemySpeed = previousMinSpeed;
		settings.maxEnemySpeed = previousMaxSpeed;
		return false;
	}

	return true;
}
