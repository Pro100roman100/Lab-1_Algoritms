#include "raylib.h"

#include "player_camera.h"
#include "game_world.h"

#include "log.h"
#include "scope_marker.h"
#include "config_loader.h"
#include "console.h"

int main() {
	SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

	if (ConfigLoader::Load("config.txt")) {
		LOG("Configuration loaded");
	}
	else {
		LOG("Failed to load configuration");
	}

	InitWindow(1280, 720, "Lab 1");

	GameWorld& gameWorld = GameWorld::GetInstance();

	PlayerCamera camera;
	Console console;

	gameWorld.InitWorld();

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

		gameWorld.UpdateWorld(deltaTime);

		// RENDER
		SCOPE_MARKER(Render, Render);

		BeginDrawing();
		ClearBackground(DARKBLUE);

		gameWorld.DrawWorld(camera.GetCamera());

		Logger::DrawLogs();
		ScopeMarker::DrawMarkers();
		console.Draw();

		SCOPE_MARKER(EndDrawing, Render);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}