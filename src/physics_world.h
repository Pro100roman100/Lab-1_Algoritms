#pragma once

#include <vector>

#include "physics_object.h"
#include "uniform_grid.h"
#include "scope_marker.h"

class PhysicsWorld {
public:
	void Update(float deltaTime);
	void GenerateGrid(BoundingBox box, Vector3 cellSize = { 1.f, 1.f, 1.f });

	void AddObject(PhysicsObject* object);
	void MoveObject(PhysicsObject* object, Vector3 transform);
	void RemoveObject(PhysicsObject* object);
private:
	void SolveCollisions();
	void ApplyForces(float deltaTime);

	std::vector<PhysicsObject*> dynamicObjects;
	UniformGrid<PhysicsObject> grid;
};