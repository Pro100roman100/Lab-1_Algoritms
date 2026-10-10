#pragma once

#include "raylib.h"

class Frustum {
public:
	void UpdateFrustum(Camera3D camera);

	bool IsVisible(const BoundingBox& bounds) const;

private:
	struct Plane {
		Vector3 normal;
		float distance;
	};

	Plane planes[6];

	float NearPlane = 0.1f;
	float FarPlane = 1000.0f;
};
