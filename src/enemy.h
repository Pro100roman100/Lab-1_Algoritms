#pragma once

#include <raylib.h>
#include <raymath.h>
#include <memory>

#include "game_settings.h"
#include "game_object.h"

class Enemy : public UpdatableObject {
public:
	Enemy(Vector3 position);
	~Enemy();

	void Update(float deltaTime) override;
	void Draw() const override;

private:
	Color color;
	float speed;

	float attackDistance;

	static std::unique_ptr<Model> model;
	static std::unique_ptr<Texture2D> texture;

	void Move(Vector3 transform);
};