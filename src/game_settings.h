#pragma once

struct GameSettings {
	static GameSettings& GetInstance() {
		static GameSettings instance;
		return instance;
	}

	GameSettings(const GameSettings&) = delete;
	GameSettings& operator=(const GameSettings&) = delete;

	float spawnInterval = 1.5f;
	float minEnemySpeed = 5.0f;
	float maxEnemySpeed = 15.0f;
	int maxEnemies = 10;
	float logShowTime = 5.0f;

private:
	GameSettings() = default;
};
