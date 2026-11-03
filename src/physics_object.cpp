#include "physics_object.h"
#include <cmath>

BoxCollider::BoxCollider(Vector3 size, float mass) : size(size) {
    const float radius = Vector3Length(size) * 0.5f;
    maxBounds = { Vector3{ -radius, -radius, -radius }, Vector3{ radius, radius, radius } };

    this->mass = mass > 0.0f ? mass : 0.0f;
    inverseMass = this->mass > 0.0f ? 1.0f / this->mass : 0.0f;
    inertiaTensor = MatrixIdentity();
    inertiaTensor.m0 = this->mass * (size.y * size.y + size.z * size.z) / 12.0f;
    inertiaTensor.m5 = this->mass * (size.x * size.x + size.z * size.z) / 12.0f;
    inertiaTensor.m10 = this->mass * (size.x * size.x + size.y * size.y) / 12.0f;
}

SphereCollider::SphereCollider(float radius, float mass) : radius(radius) {
    const float extent = std::abs(radius);
    maxBounds = { Vector3{ -extent, -extent, -extent }, Vector3{ extent, extent, extent } };

    this->mass = mass > 0.0f ? mass : 0.0f;
    inverseMass = this->mass > 0.0f ? 1.0f / this->mass : 0.0f;
    const float inertia = 0.4f * this->mass * radius * radius;
    inertiaTensor = MatrixIdentity();
    inertiaTensor.m0 = inertia;
    inertiaTensor.m5 = inertia;
    inertiaTensor.m10 = inertia;
}

PhysicsObject::PhysicsObject(ObjectType type, Collider* collider, float elasticity) {
    properties.collider = collider;
    properties.elasticity = elasticity;
    properties.type = type;
}

PhysicsProperty const PhysicsObject::GetProperties() const {
    return properties;
}

void PhysicsObject::AddImpulse(Vector3 impulse) {
    if (properties.collider) {
        properties.velocity += Vector3Scale(impulse, properties.collider->inverseMass);
    }
}

void PhysicsObject::AddImpulseAtPoint(Vector3 impulse, Vector3 point) {
    AddImpulse(impulse);
    ApplyAngularImpulse(impulse, point);
}

void PhysicsObject::AddForce(Vector3 force) {
    if (properties.collider) {
        properties.velocity += Vector3Scale(force, properties.collider->inverseMass);
    }
}

void PhysicsObject::AddForceAtPoint(Vector3 force, Vector3 point) {
    AddForce(force);
    ApplyAngularImpulse(force, point);
}

void PhysicsObject::ApplyAngularDamping(float damping) {
    const float dampingFactor = 1.0f - damping;
    properties.rotationVelocity.x *= dampingFactor;
    properties.rotationVelocity.y *= dampingFactor;
    properties.rotationVelocity.z *= dampingFactor;
}

void PhysicsObject::RotateObject(Quaternion rotation) {
    this->rotation = QuaternionNormalize(QuaternionMultiply(this->rotation, rotation));
}

BoundingBox PhysicsObject::GetBounds() const {
    if (!properties.collider) {
        return { position, position };
    }
    return {
        properties.collider->maxBounds.min + position,
        properties.collider->maxBounds.max + position
    };
}


void PhysicsObject::ApplyAngularImpulse(Vector3 impulse, Vector3 point) {
    if (!properties.collider || properties.collider->inverseMass <= 0.0f) {
        return;
    }

    const Vector3 torque = Vector3CrossProduct(point, impulse);
    const Quaternion inverseRotation = QuaternionInvert(rotation);
    const Vector3 localTorque = Vector3RotateByQuaternion(torque, inverseRotation);
    const Matrix& inertiaTensor = properties.collider->inertiaTensor;
    Vector3 localAngularVelocity = { 0.0f, 0.0f, 0.0f };
    if (inertiaTensor.m0 > 0.0f) localAngularVelocity.x = localTorque.x / inertiaTensor.m0;
    if (inertiaTensor.m5 > 0.0f) localAngularVelocity.y = localTorque.y / inertiaTensor.m5;
    if (inertiaTensor.m10 > 0.0f) localAngularVelocity.z = localTorque.z / inertiaTensor.m10;

    const Vector3 angularVelocity = Vector3RotateByQuaternion(localAngularVelocity, rotation);
    properties.rotationVelocity.x += angularVelocity.x;
    properties.rotationVelocity.y += angularVelocity.y;
    properties.rotationVelocity.z += angularVelocity.z;
}