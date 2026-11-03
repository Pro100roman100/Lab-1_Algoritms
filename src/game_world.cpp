#include "game_world.h"
#include "enemy_spawner.h"
#include "camera_marker.h"
#include <unordered_set>

GameWorld::GameWorld() = default;

void GameWorld::InitWorld() {
    drawableObjects.GenerateGrid(Vector3(-50.f, -10.f, -50.f), CellCoord(10, 5, 10), Vector3(10.f, 5.f, 10.f));
    AddObject(new EnemySpawner());
}

void GameWorld::ClearWorld() {
    for (GameObject* object : objects) {
        if (auto* physical = dynamic_cast<PhysicsObject*>(object)) {
            physicsWorld.RemoveObject(physical);
        }
        delete object;
    }

    objects.clear();
    drawableObjects.Clear();
    updatableObjects.clear();
    target = nullptr;

    InitWorld();
}

void GameWorld::DrawWorld(Camera3D camera, bool showEditorObjects) {
    SCOPE_MARKER(Draw, Render);
    
    visibleObjects = 0;

    BeginMode3D(camera);

    if(updateFrustum)
        frustum.UpdateFrustum(camera);

    std::unordered_set<DrawableObject*> renderedObjects;
    auto& cells = drawableObjects.GetCells();
    for (int i = 0; i < cells.size(); i++) {
        auto& cell = cells[i];
        if (!cell.objects.empty() && (i == 0 || frustum.IsVisible(cell.bounds))) {
            if (showEditorObjects)
                DrawCubeWiresV((cell.bounds.max + cell.bounds.min) * 0.5f, cell.bounds.max - cell.bounds.min, GRAY);
            for (DrawableObject* object : cell.objects) {
                if (!showEditorObjects && dynamic_cast<CameraMarker*>(object) != nullptr) {
                    continue;
                }
                if (object != nullptr && object->active && !renderedObjects.contains(object) && (i != 0 || frustum.IsVisible(object->GetBounds()))) {
                    renderedObjects.insert(object);
                    object->Draw();
                    visibleObjects++;
                }
            }
        }
    }
    invisibleObjects = drawableObjects.getTotalObjects() - visibleObjects;

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

    physicsWorld.Update(deltaTime);
}

void GameWorld::AddObject(GameObject* object) {
    if (object == nullptr) {
        return;
    }

    objects.push_back(object);

    if (auto* drawable = dynamic_cast<DrawableObject*>(object)) {
        drawableObjects.AddObject(drawable);
    }

    if (auto* updatable = dynamic_cast<UpdatableObject*>(object)) {
        updatableObjects.push_back(updatable);
    }

    if (auto* physical = dynamic_cast<PhysicsObject*>(object)) {
        physicsWorld.AddObject(physical);
    }
}

void GameWorld::AddSceneObjects(std::vector<GameObject*>& objects) {
    BoundingBox bounds = drawableObjects.CalculateBoundingBox(objects);
    drawableObjects.GenerateGrid(bounds, { 10.f,10.f,10.f });
    physicsWorld.GenerateGrid(bounds, { 2.5f,2.5f,2.5f });
    for (auto object : objects) {
        AddObject(object);
    }
}

void GameWorld::RemoveObject(GameObject* object) {
    if (object == nullptr) {
        return;
    }

    if (auto* drawable = dynamic_cast<DrawableObject*>(object)) {
        drawableObjects.RemoveObject(drawable);
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

    if (auto* physical = dynamic_cast<PhysicsObject*>(object)) {
        physicsWorld.RemoveObject(physical);
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

void GameWorld::Raycast(Ray ray, RayCollision& output, DrawableObject*& object) {
    SCOPE_MARKER(Raycast, Update);
    output.hit = false;
    output.distance = INFINITY;
    object = nullptr;

    std::unordered_set<DrawableObject*> renderedObjects;

    RayCollision newCollision{};
    bool cellHit;
    auto& cells = drawableObjects.GetCells();
    for (int i = 0; i < cells.size(); i++) {
        auto& cell = cells[i];

        if (!cell.objects.empty()) {
            cellHit = true;
            if (i != 0) {
                newCollision = GetRayCollisionBox(ray, cell.bounds);
                cellHit = newCollision.hit && newCollision.distance < output.distance;
            }
            if (cellHit) {
                for (auto drawObj : cell.objects) {
                    if (renderedObjects.contains(drawObj))
                        continue;
                    renderedObjects.insert(drawObj);
                    newCollision = GetRayCollisionBox(ray, drawObj->GetBounds());
                    if (newCollision.hit && newCollision.distance < output.distance) {
                        output = newCollision;
                        object = drawObj;
                    }
                }
            }
        }
    }
}

void GameWorld::UpdatePosition(DrawableObject* object, Vector3 transform) {
    if (auto* physical = dynamic_cast<PhysicsObject*>(object)) {
        drawableObjects.RemoveObject(object);
        physicsWorld.MoveObject(physical, transform);
        drawableObjects.AddObject(object);
    }
    else
        drawableObjects.MoveObject(object, transform);
}
