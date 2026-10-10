#include "enemy.h"

#include "enemy_spawner.h"
#include "game_world.h"

std::unique_ptr<Model> Enemy::model;
std::unique_ptr<Texture2D> Enemy::texture;

void Enemy::Move(Vector3 transform) {
    position += transform;
}

Enemy::Enemy(Vector3 position) {
    const GameSettings& settings = GameSettings::GetInstance();
    this->position = position;
    EnemySpawner::enemyCount++;

    if (texture == nullptr)
        texture = std::make_unique<Texture2D>(LoadTexture("resources/model_texture.png"));
    if (model == nullptr) {
        model = std::make_unique<Model>(LoadModel("resources/enemy.obj"));
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
    }

    color = RED;
    attackDistance = 1.0f;
    speed = static_cast<float>(GetRandomValue(
        static_cast<int>(settings.minEnemySpeed * 100.0f),
        static_cast<int>(settings.maxEnemySpeed * 100.0f))) / 100.0f;
}

Enemy::~Enemy() {
    EnemySpawner::enemyCount--;
}


void Enemy::Update(float deltaTime) {
    Target* targetObject = Target::GetCurrentTarget();
    if (targetObject == nullptr) {
        return;
    }

    Vector3 targetPosition = targetObject->position;
    Vector3 direction = targetPosition - position;
    direction.y = 0;
    direction = Vector3Normalize(direction);

    Move(direction * speed * deltaTime);

    Vector3 target = targetPosition;
    target.y = position.y;

    if (Vector3Distance(position, target) <= attackDistance) {
        GameWorld::GetInstance().RemoveObject(this);
        return;
    }
}

void Enemy::Draw() const {
    DrawModel(*model, position, 0.5f, WHITE);
}

BoundingBox Enemy::GetBounds() const {
    return {
        position - Vector3{ 0.5f, 0.5f, 0.5f },
        position + Vector3{ 0.5f, 0.5f, 0.5f }
    };
}