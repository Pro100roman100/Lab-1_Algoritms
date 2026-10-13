#include "editor_camera.h"

void EditorCamera::Move(Vector3 transform) {
	camera.position += transform;
	camera.target = camera.position + direction;
}

void EditorCamera::Reset() {
	camera.position = startPosition;
	yaw = -90.0f;
	pitch = -35.0f;
	direction = {
		cosf(yaw * DEG2RAD) * cosf(pitch * DEG2RAD),
		sinf(pitch * DEG2RAD),
		sinf(yaw * DEG2RAD) * cosf(pitch * DEG2RAD)
	};
	direction = Vector3Normalize(direction);
	camera.target = camera.position + direction;
}

EditorCamera::EditorCamera() {
	startPosition = { -51.0f, 60.0f, 0.0f };
	speed = 40.0f;
	mouseSensitivity = 0.08f;

	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	Reset();
}

Camera3D EditorCamera::GetCamera() const {
	return camera;
}

void EditorCamera::SetFromCamera(Camera3D source) {
	camera = source;
	direction = Vector3Normalize(camera.target - camera.position);

	yaw = atan2f(direction.z, direction.x) / DEG2RAD;
	pitch = asinf(Clamp(direction.y, -1.0f, 1.0f)) / DEG2RAD;
	pitch = Clamp(pitch, -MaxPitch, MaxPitch);
}

void EditorCamera::Update(float deltaTime) {
	Vector2 mouseDelta = GetMouseDelta();
	yaw += mouseDelta.x * mouseSensitivity;
	pitch -= mouseDelta.y * mouseSensitivity;
	pitch = Clamp(pitch, -MaxPitch, MaxPitch);

	direction = {
		cosf(yaw * DEG2RAD) * cosf(pitch * DEG2RAD),
		sinf(pitch * DEG2RAD),
		sinf(yaw * DEG2RAD) * cosf(pitch * DEG2RAD)
	};
	direction = Vector3Normalize(direction);

	const Vector3 right = Vector3Normalize(Vector3CrossProduct(direction, camera.up));
	const Vector3 viewUp = Vector3Normalize(Vector3CrossProduct(right, direction));
	Vector3 moveDirection = Vector3Zero();

	if (IsKeyDown(KEY_W)) moveDirection += direction;
	if (IsKeyDown(KEY_S)) moveDirection -= direction;
	if (IsKeyDown(KEY_D)) moveDirection += right;
	if (IsKeyDown(KEY_A)) moveDirection -= right;
	if (IsKeyDown(KEY_SPACE)) moveDirection += viewUp;
	if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) moveDirection -= viewUp;

	if (Vector3LengthSqr(moveDirection) > 0.0f) {
		moveDirection = Vector3Normalize(moveDirection);
		Move(moveDirection * speed * deltaTime);
	}
	else {
		camera.target = camera.position + direction;
	}
}

