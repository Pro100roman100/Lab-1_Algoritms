#include "game_world.h"
#include "enemy_spawner.h"
#include "camera_marker.h"

GameWorld::GameWorld() = default;

void GameWorld::InitWorld() {
    AddObject(new EnemySpawner());
}

void GameWorld::ClearWorld() {
    for (GameObject* object : objects) {
        delete object;
    }

    objects.clear();
    drawableObjects.clear();
    updatableObjects.clear();
    target = nullptr;

    InitWorld();
}

void GameWorld::DrawWorld(Camera3D camera, bool showEditorObjects) {
    SCOPE_MARKER(Draw, Render);
    
    visibleObjects = invisibleObjects = 0;

    BeginMode3D(camera);

    if(updateFrustum)
        frustum.UpdateFrustum(camera);

    for (DrawableObject* object : drawableObjects) {
        if (!showEditorObjects && dynamic_cast<CameraMarker*>(object) != nullptr) {
            continue;
        }

        if (object != nullptr && object->active && frustum.IsVisible(object->GetBounds())) {
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

    const std::vector<UpdatableObject*> updateList = updatableObjects;
    for (auto object : updateList) {
        if (object != nullptr && object->active) {
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
        for (int i = 0; i < drawableObjects.size(); i++) {
            if (drawableObjects[i] == drawable) {
                drawableObjects[i] = drawableObjects.back();
                drawableObjects.pop_back();
                break;
            }
        }
    }

    if (auto* updatable = dynamic_cast<UpdatableObject*>(object)) {
        for (int i = 0; i < updatableObjects.size(); i++) {
            if (updatableObjects[i] == updatable) {
                updatableObjects[i] = updatableObjects.back();
                updatableObjects.pop_back();
                break;
            }
        }
    }

    for (int i = 0; i < objects.size(); i++) {
        if (objects[i] == object) {
            objects[i] = objects.back();
            objects.pop_back();
            break;
        }
    }

    delete object;
}

std::vector<GameObject*>& const GameWorld::GetObjects()
{
    return objects;
}
