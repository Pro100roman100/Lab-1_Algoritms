#include "player_camera.h"

void PlayerCamera::Move(Vector3 transform) {
	camera.position += transform;
	camera.target = camera.position + direction;
}

void PlayerCamera::Reset() {
	camera.position = startPosition;
	camera.target = camera.position + direction;
}

PlayerCamera::PlayerCamera() {
	float angle = 60 * (PI / 180);
	direction = Vector3(cos(angle), -sin(angle), 0);
	speed = 40;
	startPosition = { -51.0f, 60.0f, 0.0f };

	camera.position = startPosition;
	camera.target = camera.position + direction;
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}

Camera3D PlayerCamera::GetCamera() const {
	return camera;
}

void PlayerCamera::Update(float deltaTime) {
	Vector3 moveDirection = Vector3Zero();

	if (IsKeyDown(KEY_W))
		moveDirection += { 1.0f, 0.0f, 0.0f };
	if (IsKeyDown(KEY_S))
		moveDirection += { -1.0f, 0.0f, 0.0f };
	if (IsKeyDown(KEY_A))
		moveDirection += { 0.0f, 0.0f, -1.0f };
	if (IsKeyDown(KEY_D))
		moveDirection += { 0.0f, 0.0f, 1.0f };

	moveDirection = Vector3Normalize(moveDirection);
	moveDirection *= speed * deltaTime;
	Move(moveDirection);

	if (IsKeyPressed(KEY_F1)) {
		LOG("Camera reset");
		Reset();
	}
}
