#include "static_cube.h"

StaticCube::StaticCube(Vector3 position, Vector3 size = { 1.0f, 1.0f, 1.0f }) {
    this->position = position;
    this->size = size;
    color = WHITE;
}

void StaticCube::Draw() const {
    DrawCube(position, size.x, size.y, size.z, color);
}