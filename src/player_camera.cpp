#include "player_camera.h"
#include "target.h"

PlayerCamera* PlayerCamera::currentCamera = nullptr;

void PlayerCamera::Move(Vector3 transform) {
	camera.position += transform;

	Target* target = Target::GetCurrentTarget();
	if (target != nullptr) {
		camera.target = target->position;

		Vector3 targetDir = target->position - camera.position;
		targetDir.y = 0;
		float dist = Vector3Length(targetDir);

		if (dist < 10.f && dist > 0.f) {
			camera.position -= Vector3Normalize(targetDir) * (10.f - dist);
		}
	}
	else {
		camera.target = camera.position + direction;
	}
}

void PlayerCamera::Reset() {
	camera.position = startPosition;
	if (Target::GetCurrentTarget() != nullptr)
		camera.target = Target::GetCurrentTarget()->position;
	else
		camera.target = camera.position + direction;
}

PlayerCamera::PlayerCamera() {
	currentCamera = this;
	float angle = 60.0f * (PI / 180.0f);
	direction = Vector3(cosf(angle), -sinf(angle), 0);
	speed = 40;
	startPosition = { -51.0f, 60.0f, 0.0f };

	camera.position = startPosition;
	camera.target = camera.position + direction;
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}

PlayerCamera::~PlayerCamera() {
	if (currentCamera == this)
		currentCamera = nullptr;
}

PlayerCamera& PlayerCamera::GetCamera() {
	return *currentCamera;
}

Camera3D PlayerCamera::GetCameraData() const {
	return camera;
}

void PlayerCamera::SetPosition(Vector3 position) {
	camera.position = position;
	camera.target = camera.position + direction;
}

void PlayerCamera::Update(float deltaTime) {
	Vector3 moveDirection = Vector3Zero();

	if (Target::GetCurrentTarget() != nullptr)
		camera.target = Target::GetCurrentTarget()->position;

	Vector3 forward = camera.target - camera.position;
	forward.y = 0.0f;
	if (Vector3LengthSqr(forward) > 0.0f)
		forward = Vector3Normalize(forward);
	else
		forward = { 1.0f, 0.0f, 0.0f };

	const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

	if (IsKeyDown(KEY_W))
		moveDirection += forward;
	if (IsKeyDown(KEY_S))
		moveDirection -= forward;
	if (IsKeyDown(KEY_A))
		moveDirection -= right;
	if (IsKeyDown(KEY_D))
		moveDirection += right;

	if (Vector3LengthSqr(moveDirection) > 0.0f) {
		moveDirection = Vector3Normalize(moveDirection);
		moveDirection *= speed * deltaTime;
		Move(moveDirection);
	}
}
