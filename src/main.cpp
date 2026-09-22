#include "raylib.h"

#include "player_camera.h"
#include "static_cube.h"
#include "target.h"
#include "enemy.h"

#include "log.h"
#include "scope_marker.h"

#include <vector>
#include <list>

#include "resource_dir.h"

int main() {
	SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);

	InitWindow(1280, 720, "Lab 1");

	PlayerCamera camera;
	Target target(0, 0);

	std::vector<StaticCube> cubes;
	std::list<Enemy> enemies;

	int maxEnemies = 10;
	float spawnInterval = 1.5f;
	float spawnTimer = 0.0f;
	float spawnRadius = 40.0f;

	cubes.reserve(100 * 100);
	for (int x = -50; x < 50; x++) {
		for (int y = -50; y < 50; y++) {
			cubes.emplace_back(x + 0.5f, y + 0.5f);
		}
	}

	while (!WindowShouldClose()) {
		// UPDATE
		float deltaTime = GetFrameTime();

		Logger::BeginFrame(deltaTime);
		ScopeMarker::BeginFrame();

		SCOPE_MARKER(Frame, Frame);

		camera.Update(deltaTime);

		spawnTimer += deltaTime;
		if (spawnTimer >= spawnInterval && enemies.size() < maxEnemies) {
			spawnTimer = 0.0f;

			float angle = GetRandomValue(0, 360) * DEG2RAD;

			float spawnX = target.position.x + cos(angle) * spawnRadius;
			float spawnZ = target.position.z + sin(angle) * spawnRadius;

			enemies.emplace_back(spawnX, spawnZ);
			Logger::Log("Enemy spawned");
		}

		for (auto it = enemies.begin(); it != enemies.end(); ) {
			it->Udpate(deltaTime, target.position);

			if (it->IsAttacking(target.position)) {
				it = enemies.erase(it);
				Logger::Log("Enemy Attacked");
			}
			else {
				++it;
			}
		}

		// RENDER
		SCOPE_MARKER(Render, Render);

		BeginDrawing();
		ClearBackground(DARKBLUE);

		BeginMode3D(camera.GetCamera());

		for (StaticCube& cube : cubes) {
			cube.Draw();
		}

		target.Draw();

		for (Enemy& enemy : enemies) {
			enemy.Draw();
		}

		EndMode3D();

		Logger::DrawLogs();
		ScopeMarker::DrawMarkers();

		SCOPE_MARKER(EndDrawing, Render);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}