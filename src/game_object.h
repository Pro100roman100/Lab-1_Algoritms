#pragma once

#include <raylib.h>

class DrawableObject {
public:
	bool active = true;
	Vector3 position = { 0.0f, 0.0f, 0.0f };

	virtual ~DrawableObject() = default;

	virtual void Draw() const = 0;
};

class UpdatableObject : public DrawableObject {
public:
	virtual ~UpdatableObject() = default;

	virtual void Update(float deltaTime) = 0;
};