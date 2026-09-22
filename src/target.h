#pragma once
#include <raylib.h>

class Target {
	Color color;

public:
	Vector3 position;

	Target(float x, float z);
	void Draw() const;
};

