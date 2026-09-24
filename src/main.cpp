#include "raylib.h"

#include "player_camera.h"
#include "static_cube.h"
#include "target.h"
#include "enemy.h"

#include "log.h"
#include "scope_marker.h"
#include "config_loader.h"
#include "console.h"

#include <vector>
#include <list>

#include "resource_dir.h"

int main() {
	SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);

	InitWindow(1280, 720, "Lab 1");

	PlayerCamera camera;
	Target target(0, 0);
	Console console;

	std::vector<StaticCube> cubes;
	std::list<Enemy> enemies;

	float spawnTimer = 0.0f;
	float spawnRadius = 40.0f;

	cubes.reserve(100 * 100);
	for (int x = -50; x < 50; x++) {
		for (int y = -50; y < 50; y++) {
			cubes.emplace_back(x + 0.5f, y + 0.5f);
		}
	}
	
	if (ConfigLoader::Load("config.txt")) {
		LOG("Configuration loaded");
	}
	else {
		LOG("Failed to load configuration");
	}

	while (!WindowShouldClose()) {
		// UPDATE
		float deltaTime = GetFrameTime();
		console.Update();

		Logger::BeginFrame(deltaTime);
		ScopeMarker::BeginFrame();

		if (!console.IsOpen() && IsKeyPressed(KEY_F5)) {
			if (ConfigLoader::Load("config.txt")) {
				LOG("Configuration loaded");
			}
			else {
				LOG("Failed to load configuration");
			}
		}

		SCOPE_MARKER(Frame, Frame);

		if(!console.IsOpen())
			camera.Update(deltaTime);

		spawnTimer += deltaTime;
		const GameSettings& settings = GameSettings::GetInstance();
		if (spawnTimer >= settings.spawnInterval && enemies.size() < settings.maxEnemies) {
			spawnTimer = 0.0f;

			float angle = GetRandomValue(0, 360) * DEG2RAD;

			float spawnX = target.position.x + cos(angle) * spawnRadius;
			float spawnZ = target.position.z + sin(angle) * spawnRadius;

			enemies.emplace_back(spawnX, spawnZ);
			LOG("Enemy spawned");
		}

		for (auto it = enemies.begin(); it != enemies.end(); ) {
			it->Udpate(deltaTime, target.position);

			if (it->IsAttacking(target.position)) {
				it = enemies.erase(it);
				LOG("Enemy Attacked");
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
		console.Draw();

		SCOPE_MARKER(EndDrawing, Render);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}