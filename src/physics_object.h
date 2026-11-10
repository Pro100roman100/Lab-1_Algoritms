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
    virtual RayCollision Raycast(Ray ray, Vector3 position) = 0;

    float mass = 1.f;
    float inverseMass = 1.f;
    BoundingBox bounds = {};

    virtual ~Collider() = default;
};

struct BoxCollider : Collider {
    Vector3 size = {};

    RayCollision Raycast(Ray ray, Vector3 position) override;
    BoxCollider(Vector3 size = { 1.f, 1.f, 1.f }, float mass = 1.0f);
};

struct SphereCollider : Collider {
    float radius = 0.0f;

    RayCollision Raycast(Ray ray, Vector3 position) override;
    SphereCollider(float radius = 1.0f, float mass = 1.0f);
};

struct PhysicsProperty {
    float elasticity = .95f;
    float drag = 0.05f;

    Vector3 acceleration = { 0.0f, 0.0f, 0.0f };
    Vector3 velocity = { 0.0f, 0.0f, 0.0f };
    Vector3 impulse = { 0.0f, 0.0f, 0.0f };

    ObjectType type = ObjectType::_static;
    Collider* collider = nullptr;
};

class PhysicsObject;

struct RaycastHit {
    PhysicsObject* object = nullptr;
    Vector3 point = { 0 };
    Vector3 normal = { 0 };
    float distance = 0.0f;
};
struct ContactPoint {
    Vector3 localA;
    Vector3 localB;
    Vector3 normal;
    float penetration;
    
};
struct CollisionInfo {
    PhysicsObject* a;
    PhysicsObject* b;
    
    ContactPoint point;

    void AddContactPoint(const Vector3& localA, const Vector3& localB, const Vector3& normal, float p) {
        point.localA = localA;
        point.localB = localB;
        point.normal = normal;
        point.penetration = p;
    }
};
class PhysicsObject : public virtual GameObject {
public:
    const PhysicsProperty& GetProperties() const;

    virtual ~PhysicsObject() {
        delete properties.collider;
    }

    PhysicsObject(const PhysicsObject&) = delete;
    PhysicsObject& operator=(const PhysicsObject&) = delete;

    RayCollision Raycast(Ray ray);

    void AddImpulse(Vector3 impulse);
    void AddForce(Vector3 force);

    void ApplyAcceleration(float deltaTime);

    virtual void OnCollisionEnter(PhysicsObject* other, const CollisionInfo& result) {}
    virtual void OnCollisionStay(PhysicsObject* other, const CollisionInfo& result) {}
    virtual void OnCollisionExit(PhysicsObject* other) {}

    BoundingBox GetBounds() const override;
protected:
    PhysicsObject(ObjectType type, Collider* collider, float elasticity, float drag = 0.05f);
    PhysicsProperty properties;
};