#include "game_world.h"
#include "enemy_spawner.h"
#include "camera_marker.h"
#include <unordered_set>

GameWorld::GameWorld() = default;

void GameWorld::InitWorld() {
    drawableObjects.GenerateGrid(Vector3(-50.f, -10.f, -50.f), CellCoord(10, 5, 10), Vector3(10.f, 5.f, 10.f));
    physicsWorld.UpdateBVH();
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

    //if (showEditorObjects)
    //    physicsWorld.DebugDraw();

    invisibleObjects = drawableObjects.getTotalObjects() - visibleObjects;

    EndMode3D();

    DrawText(("Visible objects: " + std::to_string(visibleObjects)).c_str(), 
        5, 5, 18, BLACK);
    DrawText(("Invisible objects: " + std::to_string(invisibleObjects)).c_str(),
        200, 5, 18, BLACK);
}

void GameWorld::PhysicsUpdate()
{
    physicsWorld.UpdateBVH();
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
    if (objects.empty())
        return;

    BoundingBox bounds = drawableObjects.CalculateBoundingBox(objects);
    drawableObjects.GenerateGrid(bounds, { 10.f,10.f,10.f });
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

void GameWorld::Raycast(Ray ray, RayCollision& output, GameObject*& object) {
    SCOPE_MARKER(Raycast, Update);
    RaycastHit hit;
    output.hit = physicsWorld.Raycast(ray, hit, 100.f);
    output.distance = hit.distance;
    output.normal = hit.normal;
    output.point = hit.point;
    object = hit.object;
}

void GameWorld::MoveObject(GameObject* object, Vector3 transform) {
    if (auto* drawable = dynamic_cast<DrawableObject*>(object)) {
        if (auto* physical = dynamic_cast<PhysicsObject*>(object)) {
            drawableObjects.RemoveObject(drawable);
            physicsWorld.MoveObject(physical, transform);
            drawableObjects.AddObject(drawable);
        }
        else
            drawableObjects.MoveObject(drawable, transform);
    }
}
