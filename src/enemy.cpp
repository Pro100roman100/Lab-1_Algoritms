#include "enemy.h"

void Enemy::Move(Vector3 transform) {
    position += transform;
}

Enemy::Enemy(float x, float z) {
    const GameSettings& settings = GameSettings::GetInstance();
    position = { x, 0.5f, z };
    color = RED;
    attackDistance = 1.0f;
    speed = static_cast<float>(GetRandomValue(
        static_cast<int>(settings.minEnemySpeed * 100.0f),
        static_cast<int>(settings.maxEnemySpeed * 100.0f))) / 100.0f;
}

void Enemy::Udpate(float deltaTime, Vector3 targetPosition) {
    Vector3 direction = targetPosition - position;
    direction.y = 0;
    direction = Vector3Normalize(direction);

    Move(direction * speed * deltaTime);
}

void Enemy::Draw() const {
    DrawCube(position, 1.0f, 1.0f, 1.0f, color);
    DrawCubeWires(position, 1.0f, 1.0f, 1.0f, DARKPURPLE);
}

bool Enemy::IsAttacking(Vector3 targetPosition) const {
    targetPosition.y = position.y;
    return Vector3Distance(position, targetPosition) <= attackDistance;
}