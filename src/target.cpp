#include "target.h"
#include "raymath.h"

#include "game_world.h"
#include "enemy_spawner.h"

Target* Target::currentTarget = nullptr;

Target::Target(Vector3 position, bool setMain) 
    : PhysicsObject(ObjectType::_static, new BoxCollider({1.f, 2.f, 1.f}), 0.5f) {
    this->position = position;
    color = GREEN;
    if (setMain) {
        GameWorld::GetInstance().AddObject(new EnemySpawner());
    }
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

BoundingBox Target::GetBounds() const {
    return {
        position - Vector3{ 0.5f, 1.0f, 0.5f },
        position + Vector3{ 0.5f, 1.0f, 0.5f }
    };
}