#pragma once

#include "game_object.h"

class CameraMarker : public DrawableObject {
public:
    CameraMarker(Vector3 position);

    virtual void Draw() const;
    virtual BoundingBox GetBounds() const;
};