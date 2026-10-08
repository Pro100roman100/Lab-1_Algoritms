#pragma once

#include <raylib.h>
#include <list>

#include "scope_marker.h"
#include "log.h"
#include "game_object.h"

#include "enemy.h"
#include "static_cube.h"
#include "target.h"

class GameWorld {
public:
	static GameWorld& GetInstance() {
		static GameWorld instance;
		return instance;
	}

	GameWorld(const GameWorld&) = delete;
	GameWorld& operator=(const GameWorld&) = delete;

	void InitWorld();
	void ReloadWorld();

	void UpdateWorld(float deltaTime);
	void DrawWorld();

	void AddObject(DrawableObject* object);
	void RemoveObject(DrawableObject* object);

private:
	GameWorld();

	std::list<DrawableObject*> drawableObjects;
	std::list<UpdatableObject*> updatableObjects;
	
	class Target* target = nullptr;
};