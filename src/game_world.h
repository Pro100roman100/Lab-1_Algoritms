#pragma once

#include <raylib.h>
#include <list>

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
	void ReloadWorld();

	void UpdateWorld(float deltaTime);
	void DrawWorld(Camera3D camera);

	void AddObject(GameObject* object);
	void RemoveObject(GameObject* object);
private:
	GameWorld();

	std::list<GameObject*> objects;
	std::list<DrawableObject*> drawableObjects;
	std::list<UpdatableObject*> updatableObjects;

	Target* target = nullptr;

	Frustum frustum;

	int visibleObjects;
	int invisibleObjects;
};
