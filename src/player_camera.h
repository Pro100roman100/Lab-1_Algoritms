#pragma once
#include "raylib.h"
#include "raymath.h"
#include "log.h"

class PlayerCamera {
private:
	Camera3D camera;

	Vector3 startPosition;
	Vector3 direction;
	float speed;

	void Move(Vector3 direction);
	void Reset();
public:

	PlayerCamera();
	Camera3D GetCamera() const;
	void Update(float deltaTime);
};