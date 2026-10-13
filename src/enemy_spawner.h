#pragma once
#include "game_object.h"

#include "game_settings.h"
#include "enemy.h"
#include "log.h"
#include "target.h"

class EnemySpawner : public UpdatableObject {
public:
	EnemySpawner();
	void Update(float deltaTime) override;

	static int enemyCount;
private:
	float spawnTimer = 0.0f;
};
