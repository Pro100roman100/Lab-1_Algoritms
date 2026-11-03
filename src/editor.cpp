#include "editor.h"
#include "camera_marker.h"
#include "dynamic_sphere.h"
#include "game_world.h"
#include "skittle.h"
#include <cmath>

Editor::Editor() {
	SelectObject(selectedIndex);
}

Editor::~Editor() {
	delete currentObject;
}

void Editor::Update(Camera3D camera, float deltaTime) {
	Vector2 screenSize = {
		GetScreenWidth(),
		GetScreenHeight()
	};
	Vector2 halfSize = screenSize / 2;
	Ray cameraRay = GetScreenToWorldRay(halfSize, camera);

	if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		RayCollision collision{};
		collision.distance = FLT_MAX;
		collision.hit = false;

		RayCollision newCollision;
		GameObject* selectedObject = nullptr;

		for (GameObject* object : GameWorld::GetInstance().GetObjects()) {
			if (object->active) {
				if (DrawableObject* drawObj = dynamic_cast<DrawableObject*>(object)) {
					newCollision = GetRayCollisionBox(cameraRay, drawObj->GetBounds());
					if (newCollision.hit && newCollision.distance < collision.distance) {
						collision = newCollision;
						selectedObject = object;
					}
				}
			}
		}

		if (collision.hit && collision.distance < 100.0f) {
			GameWorld::GetInstance().RemoveObject(selectedObject);
		}
	}

	if (IsKeyPressed(KEY_ONE)) {
		selectedIndex = selectedIndex == 1 ? 0 : 1;
	}
	if (IsKeyPressed(KEY_TWO)) {
		selectedIndex = selectedIndex == 2 ? 0 : 2;
	}
	if (IsKeyPressed(KEY_THREE)) {
		selectedIndex = selectedIndex == 3 ? 0 : 3;
	}
	if (IsKeyPressed(KEY_FOUR)) {
		selectedIndex = selectedIndex == 4 ? 0 : 4;
	}
	if (IsKeyPressed(KEY_FIVE)) {
		selectedIndex = selectedIndex == 5 ? 0 : 5;
	}
	if (IsKeyPressed(KEY_SIX)) {
		selectedIndex = selectedIndex == 6 ? 0 : 6;
	}

	SelectObject(selectedIndex);
	if (currentObject == nullptr)
		return;

	Vector3 hitPosition;
	if (GetObjectPosition(cameraRay, hitPosition)) {
		currentObject->active = true;
		currentObject->position = hitPosition;
	}
	else {
		currentObject->active = false;
	}

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && currentObject->active) {
		GameWorld::GetInstance().AddObject(currentObject);
		currentObject = nullptr;
	}
}

void Editor::Draw(Camera3D camera) {

	BeginMode3D(camera);

	DrawLine3D({ -1000.0f, 0.0f, 0.0f }, { 1000.0f, 0.0f, 0.0f }, RED);
	DrawLine3D({ 0.0f, -1000.0f, 0.0f }, { 0.0f, 1000.0f, 0.0f }, GREEN);
	DrawLine3D({ 0.0f, 0.0f, -1000.0f }, { 0.0f, 0.0f, 1000.0f }, BLUE);

	if(currentObject != nullptr && currentObject->active)
		if (DrawableObject* currentDrawObj = dynamic_cast<DrawableObject*>(currentObject))
			currentDrawObj->Draw();

	EndMode3D();

	Vector2 screenSize = {
		GetScreenWidth(),
		GetScreenHeight()
	};
	Vector2 halfSize = screenSize / 2;

	DrawLine(halfSize.x - 10.0f, halfSize.y, halfSize.x + 10.0f, halfSize.y, GRAY);
	DrawLine(halfSize.x, halfSize.y - 10.0f, halfSize.x, halfSize.y + 10.0f, GRAY);
}

bool Editor::GetObjectPosition(Ray cameraRay, Vector3& output) {
	
	RayCollision collision{};
	collision.distance = FLT_MAX;
	collision.hit = false;

	DrawableObject* hitObject = nullptr;
	GameWorld::GetInstance().Raycast(cameraRay, collision, hitObject);

	if (!collision.hit || hitObject == nullptr) {
		collision = GetRayCollisionQuad(
			cameraRay,
			{ -1000.0f, 0.0f,  1000.0f },
			{ 1000.0f, 0.0f, 1000.0f },
			{ 1000.0f, 0.0f, -1000.0f },
			{ -1000.0f, 0.0f, -1000.0f }
		);
	}

	if (collision.hit && collision.distance < 100.0f) {
		output = collision.point;

		if (DrawableObject* drawObj = dynamic_cast<DrawableObject*>(currentObject)) {
			Vector3 size = drawObj->GetBounds().max - drawObj->GetBounds().min;
			Vector3 halfSize = size * 0.5f;

			float offset = std::fabs(collision.normal.x) * halfSize.x
				+ std::fabs(collision.normal.y) * halfSize.y
				+ std::fabs(collision.normal.z) * halfSize.z;

			output += collision.normal * (offset + 0.0001f);
			output = SnapToGrid(output, size);
		}
		return true;
	}
	return false;
}

void Editor::SelectObject(int index) {
	if (currentObject != nullptr)
		delete currentObject;
	switch (index) {
	case 1:
		currentObject = new StaticCube({ 0.0f,0.0f,0.0f }, { 1.0f,1.0f,1.0f });
		break;
	case 2:
		currentObject = new StaticCube({ 0.0f,0.0f,0.0f }, { 10.0f,1.0f,10.0f });
		break;
	case 3:
		currentObject = new Target({ 0.0f,0.0f,0.0f }, false);
		break;
	case 4:
		currentObject = new CameraMarker({ 0.0f, 0.0f, 0.0f });
		break;
	case 5:
		currentObject = new DynamicSphere({ 0.0f, 0.0f, 0.0f });
		break;
	case 6:
		currentObject = new Skittle({ 0.0f, 0.0f, 0.0f });
		break;
	default:
		currentObject = nullptr;
		break;
	}
}

Vector3 Editor::SnapToGrid(Vector3 position, Vector3 size) {
	return {
		std::fmod(size.x, 2.0f) == 0.0f ? std::floor(position.x + 0.5f) : std::floor(position.x) + 0.5f,
		std::fmod(size.y, 2.0f) == 0.0f ? std::floor(position.y + 0.5f) : std::floor(position.y) + 0.5f,
		std::fmod(size.z, 2.0f) == 0.0f ? std::floor(position.z + 0.5f) : std::floor(position.z) + 0.5f
	};
}