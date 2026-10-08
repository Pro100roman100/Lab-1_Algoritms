#include "enemy_spawner.h"

#include "game_world.h"

int EnemySpawner::enemyCount = 0;

EnemySpawner::EnemySpawner(float spawnRadius)
	: spawnRadius(spawnRadius) {
}

void EnemySpawner::Update(float deltaTime) {
	spawnTimer += deltaTime;

	const GameSettings& settings = GameSettings::GetInstance();
	if (spawnTimer < settings.spawnInterval || enemyCount >= settings.maxEnemies) {
		return;
	}

	spawnTimer = 0.0f;
	Target* target = Target::GetCurrentTarget();
	if (target == nullptr) {
		return;
	}

	float angle = GetRandomValue(0, 360) * DEG2RAD;
	Vector3 spawnPosition = {
		target->position.x + cos(angle) * spawnRadius,
		target->position.y,
		target->position.z + sin(angle) * spawnRadius
	};

	GameWorld::GetInstance().AddObject(new Enemy(spawnPosition));
}

void EnemySpawner::Draw() const {
}
