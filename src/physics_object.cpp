#include "physics_object.h"
#include <cmath>

BoundingBox PhysicsObject::GetBounds() const {
    if (!properties.collider) {
        return { position, position };
    }
    const BoundingBox bounds = properties.collider->bounds;
    return {
        bounds.min + position,
        bounds.max + position
    };
}

RayCollision BoxCollider::Raycast(Ray ray, Vector3 position) {
    return GetRayCollisionBox(ray, { bounds.min + position, bounds.max + position });
}

BoxCollider::BoxCollider(Vector3 size, float mass) : size(size) {
    bounds.min = size / -2;
    bounds.max = size / 2;

    this->mass = mass > 0.0f ? mass : 0.0f;
    inverseMass = this->mass > 0.0f ? 1.0f / this->mass : 0.0f;
}

RayCollision SphereCollider::Raycast(Ray ray, Vector3 position) {
    return GetRayCollisionSphere(ray, position, radius);
}

SphereCollider::SphereCollider(float radius, float mass) : radius(radius) {
    bounds = { Vector3{ -radius, -radius, -radius }, Vector3{ radius, radius, radius } };

    this->mass = mass > 0.0f ? mass : 0.0f;
    inverseMass = this->mass > 0.0f ? 1.0f / this->mass : 0.0f;
}

PhysicsObject::PhysicsObject(ObjectType type, Collider* collider, float elasticity, float drag) {
    properties.collider = collider;
    properties.elasticity = elasticity;
    properties.type = type;
    properties.drag = drag;

    if (type != ObjectType::_dynamic) {
        properties.collider->inverseMass = 0;
        properties.collider->mass = 0;
    }
}

const PhysicsProperty& PhysicsObject::GetProperties() const {
    return properties;
}

RayCollision PhysicsObject::Raycast(Ray ray) {
    return properties.collider->Raycast(ray, position);
}

void PhysicsObject::AddImpulse(Vector3 impulse) {
    if (properties.collider) {
        properties.velocity += impulse * properties.collider->inverseMass;
    }
}

void PhysicsObject::AddForce(Vector3 force) {
    if (properties.collider) {
        properties.acceleration += force * properties.collider->inverseMass;
    }
}

void PhysicsObject::ApplyAcceleration(float deltaTime) {
    properties.velocity += properties.acceleration * deltaTime;
    properties.acceleration = { 0.f, 0.f, 0.f };
}