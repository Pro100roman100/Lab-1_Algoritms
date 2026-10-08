#include "target.h"

Target* Target::currentTarget = nullptr;

Target::Target(Vector3 position) {
    this->position = position;
    color = GREEN;
    currentTarget = this;
}

Target::~Target() {
    if (currentTarget == this) {
        currentTarget = nullptr;
    }
}

Target* Target::GetCurrentTarget() {
    return currentTarget;
}

void Target::Draw() const {
    DrawCube(position, 1.0f, 2.0f, 1.0f, color);
    DrawCubeWires(position, 1.0f, 2.0f, 1.0f, DARKPURPLE);
}