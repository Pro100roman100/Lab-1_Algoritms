#pragma once

#include <raylib.h>
#include <vector>

#include "scope_marker.h"
#include "log.h"
#include "game_object.h"

#include "enemy.h"
#include "static_cube.h"
#include "target.h"
#include "frustum.h"

class GameWorld {
public:
	static GameWorld& GetInstance() {
		static GameWorld instance;
		return instance;
	}

	GameWorld(const GameWorld&) = delete;
	GameWorld& operator=(const GameWorld&) = delete;

	bool updateFrustum = true;

	void InitWorld();
	void ClearWorld();

	void UpdateWorld(float deltaTime);
	void DrawWorld(Camera3D camera, bool showEditorObjects = false);

	void AddObject(GameObject* object);
	void RemoveObject(GameObject* object);

	std::vector<GameObject*>& GetObjects();

private:
	GameWorld();

	std::vector<GameObject*> objects;
	std::vector<DrawableObject*> drawableObjects;
	std::vector<UpdatableObject*> updatableObjects;

	Target* target = nullptr;

	Frustum frustum;

	int visibleObjects = 0;
	int invisibleObjects = 0;
};