#include "physics_object.h"
#include <cmath>

BoundingBox BoxCollider::GetBounds(const Quaternion& rotation)
{
    if (!QuaternionEquals(rotation, lastRot)) {
        lastRot = rotation;

        const Matrix r = QuaternionToMatrix(rotation);
        const Vector3 half = size / 2;
        const Vector3 extent = {
            fabsf(r.m0) * half.x + fabsf(r.m4) * half.y + fabsf(r.m8) * half.z,
            fabsf(r.m1) * half.x + fabsf(r.m5) * half.y + fabsf(r.m9) * half.z,
            fabsf(r.m2) * half.x + fabsf(r.m6) * half.y + fabsf(r.m10) * half.z
        };
        bounds.min = Vector3Negate(extent) * 1.05f;
        bounds.max = extent * 1.05f;
    }
    return bounds;
}

BoundingBox SphereCollider::GetBounds(const Quaternion&)
{
    return bounds;
}

BoundingBox PhysicsObject::GetBounds() const {
    if (!properties.collider) {
        return { position, position };
    }
    const BoundingBox bounds = properties.collider->GetBounds(rotation);
    return {
        bounds.min + position,
        bounds.max + position
    };
}

BoxCollider::BoxCollider(Vector3 size, float mass) : size(size) {
    const float radius = Vector3Length(size) * 0.5f;
    bounds = GetBounds(QuaternionIdentity());

    lastRot = QuaternionIdentity();

    this->mass = mass > 0.0f ? mass : 0.0f;
    inverseMass = this->mass > 0.0f ? 1.0f / this->mass : 0.0f;
    inertiaTensor = MatrixIdentity();
    inertiaTensor.m0 = this->mass * (size.y * size.y + size.z * size.z) / 12.0f;
    inertiaTensor.m5 = this->mass * (size.x * size.x + size.z * size.z) / 12.0f;
    inertiaTensor.m10 = this->mass * (size.x * size.x + size.y * size.y) / 12.0f;
}

SphereCollider::SphereCollider(float radius, float mass) : radius(radius) {
    const float extent = std::abs(radius);
    bounds = { Vector3{ -extent, -extent, -extent }, Vector3{ extent, extent, extent } };

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

    if (type != ObjectType::_dynamic)
        properties.collider->inverseMass = 0;
}

PhysicsProperty const PhysicsObject::GetProperties() const
{
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