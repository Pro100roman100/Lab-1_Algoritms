#include "static_cube.h"
#include "raymath.h"

StaticCube::StaticCube(Vector3 position, Vector3 size = { 1.0f, 1.0f, 1.0f }) {
    this->position = position;
    this->size = size;
    color = WHITE;

    Vector3 halfSize = size * 0.5f;
}

void StaticCube::Draw() const {
    DrawCube(position, size.x, size.y, size.z, color);
    DrawCubeWires(position, size.x, size.y, size.z, GRAY);
}

BoundingBox StaticCube::GetBounds() const {
    return { position - size / 2, position + size / 2 };
}