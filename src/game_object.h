#pragma once

#include <raylib.h>
#include <string>

class GameObject {
public:
    bool active = true;
    Vector3 position = { 0.0f, 0.0f, 0.0f };

    virtual ~GameObject() = default;
};

class DrawableObject : public virtual GameObject {
public:
    virtual ~DrawableObject() = default;

    virtual void Draw() const = 0;
    virtual BoundingBox GetBounds() const {
        return { position, position };
    }
};

class UpdatableObject : public virtual GameObject {
public:
    virtual ~UpdatableObject() = default;

    virtual void Update(float deltaTime) = 0;
};