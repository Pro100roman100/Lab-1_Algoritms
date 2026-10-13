#pragma once
#include "raylib.h"
#include "raymath.h"
#include "log.h"

class PlayerCamera {
private:
	static PlayerCamera* currentCamera;
	Camera3D camera;

	Vector3 startPosition;
	Vector3 direction;
	float speed;

	void Move(Vector3 direction);
	void Reset();
public:

	PlayerCamera();
	~PlayerCamera();
	static PlayerCamera& GetCamera();
	Camera3D GetCameraData() const;
	void SetPosition(Vector3 position);
	void Update(float deltaTime);
};