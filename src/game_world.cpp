#include "game_world.h"
#include "enemy_spawner.h"

GameWorld::GameWorld() = default;

void GameWorld::InitWorld() {
    target = new Target({ 0.0f, 1.0f, 0.0f });
    AddObject(target);

    for (int x = -5; x < 5; ++x) {
        for (int z = -5; z < 5; ++z) {
            AddObject(new StaticCube(
                { 10.f * x + 5.f, 0.0f, 10.f * z + 5.f },
                { 10.0f, 1.0f, 10.0f }));
        }
    }

    AddObject(new EnemySpawner(20.0f));
}

void GameWorld::ReloadWorld() {
    for (GameObject* object : objects) {
        delete object;
    }

    objects.clear();
    drawableObjects.clear();
    updatableObjects.clear();
    target = nullptr;

    InitWorld();
}

void GameWorld::DrawWorld(Camera3D camera) {
    SCOPE_MARKER(Draw, Render);
    
    visibleObjects = 0;
    invisibleObjects = 0;

    BeginMode3D(camera);

    if(updateFrustum)
        frustum.UpdateFrustum(camera);

    for (DrawableObject* object : drawableObjects) {
        if (object->active && frustum.IsVisible(object->GetBounds())) {
            visibleObjects++;
            object->Draw();
        }
        else
            invisibleObjects++;
    }

    EndMode3D();

    DrawText(("Visible objects: " + std::to_string(visibleObjects)).c_str(), 
        5, 5, 18, BLACK);
    DrawText(("Invisible objects: " + std::to_string(invisibleObjects)).c_str(),
        200, 5, 18, BLACK);
}

void GameWorld::UpdateWorld(float deltaTime) {
    SCOPE_MARKER(Update, Update);

    for (auto it = updatableObjects.begin(); it != updatableObjects.end();) {
        UpdatableObject* object = *it;
        ++it;

        if (object->active) {
            object->Update(deltaTime);
        }
    }
}

void GameWorld::AddObject(GameObject* object) {
    if (object == nullptr) {
        return;
    }

    objects.push_back(object);

    if (auto* drawable = dynamic_cast<DrawableObject*>(object)) {
        drawableObjects.push_back(drawable);
    }

    if (auto* updatable = dynamic_cast<UpdatableObject*>(object)) {
        updatableObjects.push_back(updatable);
    }
}

void GameWorld::RemoveObject(GameObject* object) {
    if (object == nullptr) {
        return;
    }

    if (auto* drawable = dynamic_cast<DrawableObject*>(object)) {
        drawableObjects.remove(drawable);
    }

    if (auto* updatable = dynamic_cast<UpdatableObject*>(object)) {
        updatableObjects.remove(updatable);
    }

    objects.remove(object);
    delete object;
}