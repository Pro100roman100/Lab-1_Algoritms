#pragma once
#include <raylib.h>

#include "game_object.h"

class StaticCube : public DrawableObject {
	Vector3 size;
	Color color;

	BoundingBox boundingBox;

public:
	StaticCube(Vector3 position, Vector3 size);
    void Draw() const override;
    BoundingBox GetBounds() const override;
};