#include "frustum.h"

#include <cmath>

#include "raymath.h"

void Frustum::UpdateFrustum(Camera3D camera) {
	const float aspect = static_cast<float>(GetScreenWidth()) /
		static_cast<float>(GetScreenHeight());
	const float verticalScale = tanf(camera.fovy * DEG2RAD * 0.5f);
	const float horizontalScale = verticalScale * aspect;

	const Vector3 forward = Vector3Normalize(camera.target - camera.position);
	const Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
	const Vector3 up = Vector3Normalize(Vector3CrossProduct(right, forward));

	const auto makePlane = [](Vector3 normal, Vector3 point) {
		return Plane{ normal, -Vector3DotProduct(normal, point) };
	};

	planes[0] = makePlane(forward, camera.position + forward * NearPlane);
	planes[1] = makePlane(forward * -1.0f, camera.position + forward * FarPlane);
	planes[2] = makePlane(right + forward * horizontalScale, camera.position);
	planes[3] = makePlane(right * -1.0f + forward * horizontalScale, camera.position);
	planes[4] = makePlane(up + forward * verticalScale, camera.position);
	planes[5] = makePlane(up * -1.0f + forward * verticalScale, camera.position);
}

bool Frustum::IsVisible(const BoundingBox& bounds) const {
	const Vector3 center = (bounds.min + bounds.max) * 0.5f;
	const Vector3 extents = (bounds.max - bounds.min) * 0.5f;

	for (const Plane& plane : planes) {
		const float distance = Vector3DotProduct(plane.normal, center) + plane.distance;
		const float radius = fabsf(plane.normal.x) * extents.x +
			fabsf(plane.normal.y) * extents.y +
			fabsf(plane.normal.z) * extents.z;

		if (distance + radius < 0.0f) {
			return false;
		}
	}

	return true;
}
