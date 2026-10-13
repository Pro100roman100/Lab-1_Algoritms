#pragma once

#include <raylib.h>
#include <raymath.h>

class EditorCamera {
private:
	Camera3D camera;

	Vector3 startPosition;
	Vector3 direction;
	
	float yaw;
	float pitch;
	const float MaxPitch = 89.0f;

	float speed;
	float mouseSensitivity;

	void Move(Vector3 direction);
	void Reset();
public:

	EditorCamera();
	Camera3D GetCamera() const;
	void SetFromCamera(Camera3D source);
	void Update(float deltaTime);
};