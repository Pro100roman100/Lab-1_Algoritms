#include "skittle.h"

std::unique_ptr<Model> Skittle::model;
std::unique_ptr<Texture2D> Skittle::texture;


Skittle::Skittle(Vector3 position)
    : PhysicsObject(ObjectType::_dynamic, new BoxCollider({0.8f, 2.f, 0.8f}, 2.f), 0.15f) {
    this->position = position;
    if (texture == nullptr)
        texture = std::make_unique<Texture2D>(LoadTexture("resources/skittle.png"));
    if (model == nullptr) {
        model = std::make_unique<Model>(LoadModel("resources/skittle.obj"));
        model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
    }
}

void Skittle::Draw() const {
    DrawModel(*model, position, 1, WHITE);
}

BoundingBox Skittle::GetBounds() const {
    return {
        position - Vector3{ 0.45f, 1.f, 0.45f },
        position + Vector3{ 0.45f, 1.f, 0.45f }
    };
}