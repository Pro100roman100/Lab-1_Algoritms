#pragma once
#include <raylib.h>

#include "game_object.h"

class Target : public DrawableObject {
public:
	Target(Vector3 position);
	~Target();
	static Target* GetCurrentTarget();
	void Draw() const override;
	BoundingBox GetBounds() const override;

private:
	static Target* currentTarget;
	Color color;
};

