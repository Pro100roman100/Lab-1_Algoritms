#include "game_world.h"
#include "enemy_spawner.h"

GameWorld::GameWorld() = default;

void GameWorld::InitWorld() {

    target = new Target({ 0.0f, 1.0f, 0.0f });
    AddObject(target);

    for (int x = -50; x < 50; ++x) {
        for (int z = -50; z < 50; ++z) {
            AddObject(new StaticCube(
                { x + 0.5f, 0.0f, z + 0.5f },
                { 1.0f, 1.0f, 1.0f }));
        }
    }

    AddObject(new EnemySpawner(20.0f));
}

void GameWorld::ReloadWorld() {
    for (auto* object : drawableObjects) {
        delete object;
    }

    drawableObjects.clear();
    updatableObjects.clear();
    target = nullptr;

    InitWorld();
}

void GameWorld::DrawWorld() {
    SCOPE_MARKER(Draw, Render);

    for (auto* object : drawableObjects) {
        if (object->active) {
            object->Draw();
        }
    }
}

void GameWorld::UpdateWorld(float deltaTime) {
    SCOPE_MARKER(Update, Update);

    for (auto it = updatableObjects.begin(); it != updatableObjects.end();) {
        UpdatableObject* object = *it;
        ++it;

        if (!object->active) {
            continue;
        }

        object->Update(deltaTime);
    }
}

void GameWorld::AddObject(DrawableObject* object) {
    if (object == nullptr) {
        return;
    }

    drawableObjects.push_back(object);
    if (auto* updatable = dynamic_cast<UpdatableObject*>(object)) {
        updatableObjects.push_back(updatable);
    }
}

void GameWorld::RemoveObject(DrawableObject* object) {
    if (object == nullptr) {
        return;
    }

    drawableObjects.remove(object);
    if (auto* updatable = dynamic_cast<UpdatableObject*>(object)) {
        updatableObjects.remove(updatable);
    }

    delete object;
}