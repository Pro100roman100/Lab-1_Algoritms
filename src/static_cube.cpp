#include "static_cube.h"

StaticCube::StaticCube(float x, float z) {
    position = { x, -0.5f, z };
    color = WHITE;
}

void StaticCube::Draw() const {
    DrawCube(position, 1.0f, 1.0f, 1.0f, color);
}