#pragma once
#include <raylib.h>

#include "game_object.h"
#include "physics_object.h"

class StaticCube : public DrawableObject, public PhysicsObject {
public:
	Vector3 size;

	StaticCube(Vector3 position, Vector3 size);
    void Draw() const override;
    BoundingBox GetBounds() const override;

private:
	Color color;
};