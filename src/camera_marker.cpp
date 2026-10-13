#include "camera_marker.h"

CameraMarker::CameraMarker(Vector3 position) {
	this->position = position;
}

void CameraMarker::Draw() const {
	DrawSphere(position, 0.5f, RED);
}

BoundingBox CameraMarker::GetBounds() const {
	return{ { position.x - 0.5f, position.y - 0.5f, position.z - 0.5f },
			{ position.x + 0.5f, position.y + 0.5f, position.z + 0.5f } };
}
