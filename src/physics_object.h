#pragma once

#include "game_object.h"
#include <memory>
#include <raymath.h>

enum class ObjectType {
    _dynamic,
    _kinematic,
    _static
};

class Collider {
public:
    BoundingBox maxBounds = {};
    float mass = 1.f;
    float inverseMass = 1.f;
    Matrix inertiaTensor = {};

    virtual ~Collider() = default;
};

struct BoxCollider : Collider {
    Vector3 size = {};

    BoxCollider(Vector3 size = {1.f, 1.f, 1.f}, float mass = 1.0f);
};
struct SphereCollider : Collider {
    float radius = 0.0f;

    SphereCollider(float radius = 1.0f, float mass = 1.0f);
};

struct PhysicsProperty {
    float elasticity = .95f;

    Vector3 velocity = { 0.0f, 0.0f, 0.0f };
    Quaternion rotationVelocity = { 0.0f, 0.0f, 0.0f, 0.0f };
    Vector3 impulse = { 0.0f, 0.0f, 0.0f };
    Quaternion rotationImpulse = { 0.0f, 0.0f, 0.0f, 0.0f };

    ObjectType type = ObjectType::_static;
    Collider* collider = nullptr;
};

class PhysicsObject;

struct CollisionResult {
    PhysicsObject* objA;
    PhysicsObject* objB;
    Vector3 position;
    Vector3 normal;
    float penetration;
};

class PhysicsObject : public virtual GameObject {
public:
    PhysicsProperty const GetProperties() const;

    virtual ~PhysicsObject() {
        delete properties.collider;
    }

    PhysicsObject(const PhysicsObject&) = delete;
    PhysicsObject& operator=(const PhysicsObject&) = delete;

    void AddImpulse(Vector3 impulse);
    void AddImpulseAtPoint(Vector3 impulse, Vector3 point);
    void AddForce(Vector3 force);
    void AddForceAtPoint(Vector3 force, Vector3 point);
    void ApplyAngularDamping(float damping);

    void RotateObject(Quaternion rotation);
    void ApplyAngularImpulse(Vector3 impulse, Vector3 point);

    virtual void OnCollisionEnter(PhysicsObject* other, const CollisionResult& result) {}
    virtual void OnCollisionStay(PhysicsObject* other, const CollisionResult& result) {}
    virtual void OnCollisionExit(PhysicsObject* other) {}

    BoundingBox GetBounds() const override;
protected:
    PhysicsObject(ObjectType type, Collider* collider, float elasticity);
    PhysicsProperty properties;
};