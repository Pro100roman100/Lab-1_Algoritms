#pragma once
#include <raylib.h>

class StaticCube
{
	Vector3 position;
	Color color;

public:
	StaticCube(float x, float z);
    void Draw() const;
};

