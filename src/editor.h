#pragma once

#include "game_world.h"
#include "game_object.h"

class Editor {
public:
	Editor();
	~Editor();

	void Update(Camera3D camera, float deltaTime);
	void Draw(Camera3D camera);

private:
	GameObject* currentObject = nullptr;

	int selectedIndex = 1;

	bool GetObjectPosition(Ray cameraRay, Vector3& output);
	void SelectObject(int obj);
	Vector3 SnapToGrid(Vector3 position, Vector3 size);
};