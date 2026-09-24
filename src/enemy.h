#pragma once

#include <raylib.h>
#include <raymath.h>

#include "game_settings.h"

class Enemy {
private:
	Vector3 position;
	Color color;
	float speed;

	float attackDistance;

	void Move(Vector3 transform);
public:

	Enemy(float x, float z);
	void Udpate(float deltaTime, Vector3 targetPosition);
	void Draw() const;
	bool IsAttacking(Vector3 targetPosition) const;
};