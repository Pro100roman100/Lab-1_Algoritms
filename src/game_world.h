#pragma once

#include <raylib.h>
#include <vector>

#include "scope_marker.h"
#include "log.h"
#include "game_object.h"
#include "uniform_grid.h"
#include "physics_world.h"

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
	void AddSceneObjects(std::vector<GameObject*>& objects);
	void RemoveObject(GameObject* object);

	std::vector<GameObject*>& GetObjects();
	void Raycast(Ray ray, RayCollision& output, DrawableObject*& object);

	void UpdatePosition(DrawableObject* object, Vector3 transform);

private:
	GameWorld();

	std::vector<GameObject*> objects;
	UniformGrid<DrawableObject> drawableObjects;
	std::vector<UpdatableObject*> updatableObjects;
	PhysicsWorld physicsWorld;

	Target* target = nullptr;

	Frustum frustum;

	int visibleObjects = 0;
	int invisibleObjects = 0;
};