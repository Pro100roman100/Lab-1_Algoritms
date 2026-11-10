#pragma once

#include "game_object.h"
#include "physics_object.h"

class DynamicSphere : public DrawableObject, public PhysicsObject {
public:
	DynamicSphere(Vector3 position, float radius = 0.5f, float mass = 5.0f)
		: PhysicsObject(ObjectType::_dynamic, new SphereCollider(radius, mass), 0.2f) {
		this->position = position;
		if (texture == nullptr)
			texture = std::make_unique<Texture2D>(LoadTexture("resources/bowling.png"));
		if (model == nullptr) {
			model = std::make_unique<Model>(LoadModel("resources/bowling_ball.obj"));
			model->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = *texture;
		}
	}

	void Draw() const override {
		Matrix rotationMatrix = QuaternionToMatrix(rotation);
		model->transform = rotationMatrix;
		DrawModel(*model, position, dynamic_cast<SphereCollider*>(properties.collider)->radius, WHITE);
	}

private:
	inline static std::unique_ptr<Model> model;
	inline static std::unique_ptr<Texture2D> texture;
};
