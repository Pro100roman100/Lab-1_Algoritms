#include "target.h"

Target::Target(float x, float z) {
    position = { x, 1.0f, z };
    color = GREEN;
}

void Target::Draw() const {
    DrawCube(position, 1.0f, 2.0f, 1.0f, color);
    DrawCubeWires(position, 1.0f, 2.0f, 1.0f, DARKPURPLE);
}