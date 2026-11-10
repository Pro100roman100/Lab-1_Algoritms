#pragma once

#include "target.h"
#include "player_camera.h"
#include "log.h"

class BowlingTarget : public Target, public UpdatableObject {
public:
    BowlingTarget(Vector3 position, float size = 1.f) : Target(position, false) {
        this->size = size;
        properties.type = ObjectType::_dynamic;
        delete properties.collider;
        properties.collider = new SphereCollider(.5f * size, 10.f * size);
        properties.elasticity = 0.2f;

        if (texture == nullptr)
            texture = std::make_unique<Texture2D>(LoadTexture("resources/bowling.png"));
        if (model == nullptr) {
            model = std::make_unique<Model>(LoadModel("resources/bowling_ball.obj"));
            model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
        }
    }

    void Update(float deltaTime) override {
        if (IsKeyPressed(KEY_SPACE) && !touched) {
            touched = true;
            Vector3 camPos = PlayerCamera::GetCamera().GetCameraData().position;
            Vector3 direction = Vector3Normalize(position - camPos);
            AddImpulse(direction * 500.0f * size);
        }
    }

    void Draw() const override {
        Matrix rotationMatrix = QuaternionToMatrix(rotation);
        model->transform = rotationMatrix;
        DrawModel(*model, position, 0.5f * size, { 255, 120, 120, 255 });
    }

    void OnCollisionEnter(PhysicsObject* other, const CollisionInfo& result) override {
        if (dynamic_cast<Skittle*>(other)) {
            LOG("Touched skittle");
        }
    }
    void OnCollisionExit(PhysicsObject* other) override {
        if (dynamic_cast<Skittle*>(other)) {
            LOG("Stopped touching skittle");
        }
    }

    BoundingBox GetBounds() const override {
        return {
            position + properties.collider->bounds.min,
            position + properties.collider->bounds.max
        };
    }

    float size;

private:
    inline static std::unique_ptr<Model> model;
    inline static std::unique_ptr<Texture2D> texture;

    bool touched = false;
};