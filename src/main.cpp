#include "raylib.h"

#include "player_camera.h"
#include "game_world.h"
#include "editor_camera.h"
#include "editor.h"
#include "map_save.h"

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
	Editor editor{};

	bool inEditor = false;

	PlayerCamera camera;
	EditorCamera editorCamera;

	Console console;

	MapSave::LoadFromFile();

	while (!WindowShouldClose()) {
		// UPDATE
		float deltaTime = std::min(GetFrameTime(), 0.25f);
		console.Update();

		Logger::BeginFrame(deltaTime);
		ScopeMarker::BeginFrame();

		if (!console.IsOpen()) {
			if (IsKeyPressed(KEY_F5)) {
				if (ConfigLoader::Load("config.txt")) {
					LOG("Configuration loaded");
				}
				else {
					LOG("Failed to load configuration");
				}
			}
			if (IsKeyPressed(KEY_F1)) {
				editorCamera.SetFromCamera(camera.GetCameraData());
				if (inEditor) {
					MapSave::SaveToFile("temp");
				}
				MapSave::LoadFromFile("temp");

				if (!inEditor) {
					inEditor = true;
					DisableCursor();
					LOG("Switched to editor");
				}
				else {
					inEditor = false;
					EnableCursor();
					LOG("Switched to game");
				}
			}
			if (IsKeyPressed(KEY_F2)) {
				MapSave::SaveToFile();
			}
			if (IsKeyPressed(KEY_F3)) {
				MapSave::LoadFromFile();
			}
		}


		SCOPE_MARKER(Frame, Frame);

		if(!console.IsOpen())
			inEditor
			? editorCamera.Update(deltaTime)
			: camera.Update(deltaTime);
		
		if (inEditor)
			editor.Update(editorCamera.GetCamera(), deltaTime);
		else
			gameWorld.UpdateWorld(deltaTime);

		// RENDER
		SCOPE_MARKER(Render, Render);

		BeginDrawing();
		ClearBackground(DARKBLUE);

		gameWorld.DrawWorld(inEditor
			? editorCamera.GetCamera()
			: camera.GetCameraData(), inEditor);

		if (inEditor)
			editor.Draw(editorCamera.GetCamera());

		Logger::DrawLogs();
		ScopeMarker::DrawMarkers();
		console.Draw();

		SCOPE_MARKER(EndDrawing, Render);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}