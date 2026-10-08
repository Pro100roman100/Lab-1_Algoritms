#pragma once
#include "game_object.h"

#include "game_settings.h"
#include "enemy.h"
#include "log.h"
#include "target.h"

class EnemySpawner : public UpdatableObject {
public:
	EnemySpawner(float spawnRadius);
	void Update(float deltaTime) override;
	void Draw() const override;

	static int enemyCount;
private:

	float spawnTimer = 0.0f;
	float spawnRadius = 40.0f;
};
