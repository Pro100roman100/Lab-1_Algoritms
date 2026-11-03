#include "physics_world.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <set>

#include "game_world.h"

namespace {
struct PhysicsObjectPairLess {
    bool operator()(const std::pair<PhysicsObject*, PhysicsObject*>& left,
        const std::pair<PhysicsObject*, PhysicsObject*>& right) const {
        const std::less<PhysicsObject*> less;
        if (left.first != right.first) {
            return less(left.first, right.first);
        }
        return less(left.second, right.second);
    }
};

float GetEffectiveInverseMass(const PhysicsObject* object) {
    const PhysicsProperty properties = object->GetProperties();
    if (properties.type != ObjectType::_dynamic || properties.collider == nullptr) {
        return 0.0f;
    }
    return properties.collider->inverseMass;
}

struct CollisionResult {
    Vector3 normal;
    float penetration;
};

Vector3 GetHalfExtents(const BoxCollider& box) {
    return {
        std::abs(box.size.x) * 0.5f,
        std::abs(box.size.y) * 0.5f,
        std::abs(box.size.z) * 0.5f
    };
}

bool Box_Box(const PhysicsObject& first, const BoxCollider& firstBox,
    const PhysicsObject& second, const BoxCollider& secondBox, CollisionResult& result) {
    const Vector3 firstHalf = GetHalfExtents(firstBox);
    const Vector3 secondHalf = GetHalfExtents(secondBox);
    const Vector3 delta = second.position - first.position;
    const Vector3 overlap = {
        firstHalf.x + secondHalf.x - std::abs(delta.x),
        firstHalf.y + secondHalf.y - std::abs(delta.y),
        firstHalf.z + secondHalf.z - std::abs(delta.z)
    };

    if (overlap.x <= 0.0f || overlap.y <= 0.0f || overlap.z <= 0.0f) {
        return false;
    }

    result.penetration = overlap.x;
    result.normal = { delta.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f };
    if (overlap.y < result.penetration) {
        result.penetration = overlap.y;
        result.normal = { 0.0f, delta.y >= 0.0f ? 1.0f : -1.0f, 0.0f };
    }
    if (overlap.z < result.penetration) {
        result.penetration = overlap.z;
        result.normal = { 0.0f, 0.0f, delta.z >= 0.0f ? 1.0f : -1.0f };
    }

    return true;
}

bool Box_Sphere(const PhysicsObject& boxObject, const BoxCollider& box,
    const PhysicsObject& sphereObject, const SphereCollider& sphere, CollisionResult& result) {
    const Vector3 half = GetHalfExtents(box);
    const Vector3 minimum = boxObject.position - half;
    const Vector3 maximum = boxObject.position + half;
    const Vector3 closest = {
        std::clamp(sphereObject.position.x, minimum.x, maximum.x),
        std::clamp(sphereObject.position.y, minimum.y, maximum.y),
        std::clamp(sphereObject.position.z, minimum.z, maximum.z)
    };
    const Vector3 delta = sphereObject.position - closest;
    const float distanceSquared = Vector3LengthSqr(delta);
    const float radius = std::abs(sphere.radius);
    if (radius <= 0.0f || distanceSquared >= radius * radius) {
        return false;
    }

    if (distanceSquared > 0.000001f) {
        const float distance = std::sqrt(distanceSquared);
        result.normal = Vector3Scale(delta, 1.0f / distance);
        result.penetration = radius - distance;
        return true;
    }

    const float faceDistances[] = {
        sphereObject.position.x - minimum.x,
        maximum.x - sphereObject.position.x,
        sphereObject.position.y - minimum.y,
        maximum.y - sphereObject.position.y,
        sphereObject.position.z - minimum.z,
        maximum.z - sphereObject.position.z
    };
    const Vector3 faceNormals[] = {
        { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
        { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f }
    };
    size_t nearestFace = 0;
    for (size_t i = 1; i < 6; ++i) {
        if (faceDistances[i] < faceDistances[nearestFace]) {
            nearestFace = i;
        }
    }
    result.normal = faceNormals[nearestFace];
    result.penetration = radius + faceDistances[nearestFace];
    return true;
}

bool Sphere_Sphere(const PhysicsObject& first, const SphereCollider& firstSphere,
    const PhysicsObject& second, const SphereCollider& secondSphere, CollisionResult& result) {
    const Vector3 delta = second.position - first.position;
    const float distanceSquared = Vector3LengthSqr(delta);
    const float radiusSum = std::abs(firstSphere.radius) + std::abs(secondSphere.radius);
    if (radiusSum <= 0.0f || distanceSquared >= radiusSum * radiusSum) {
        return false;
    }

    const float distance = std::sqrt(distanceSquared);
    result.normal = distance > 0.000001f
        ? Vector3Scale(delta, 1.0f / distance)
        : Vector3{ 1.0f, 0.0f, 0.0f };
    result.penetration = radiusSum - distance;
    return true;
}

struct ContactConstraint {
    PhysicsObject* first;
    PhysicsObject* second;
    Vector3 normal;
    float restitution;
    float velocityBias;
    float accumulatedNormalImpulse = 0.0f;
};

void ApplyContactImpulse(ContactConstraint& contact, Vector3 impulse) {
    if (GetEffectiveInverseMass(contact.first) > 0.0f) {
        contact.first->AddImpulse(Vector3Scale(impulse, -1.0f));
    }
    if (GetEffectiveInverseMass(contact.second) > 0.0f) {
        contact.second->AddImpulse(impulse);
    }
}
}

void PhysicsWorld::Update(float deltaTime) {
    SCOPE_MARKER(Update, Physics);
    for (auto object : dynamicObjects) {
        auto& properties = object->GetProperties();
        object->AddForce(Vector3(0.f, -9.8f, 0.f) * properties.collider->mass * deltaTime);
        object->AddForce(properties.velocity * -0.01);
        object->ApplyAngularDamping(0.05f);
        
    }

    ApplyForces(deltaTime);
    SolveCollisions();
}

void PhysicsWorld::GenerateGrid(BoundingBox bounds, Vector3 cellSize) {
    grid.Clear();
    grid.GenerateGrid(bounds, cellSize);
}

void PhysicsWorld::SolveCollisions() {
    SCOPE_MARKER(Solve_Collisions, Physics);
    std::set<std::pair<PhysicsObject*, PhysicsObject*>, PhysicsObjectPairLess> pairs;
    for (const GridCell<PhysicsObject>& cell : grid.GetCells()) {
        for (size_t i = 0; i < cell.objects.size(); ++i) {
            for (size_t j = i + 1; j < cell.objects.size(); ++j) {
                PhysicsObject* first = cell.objects[i];
                PhysicsObject* second = cell.objects[j];
                if (first == second || !first->active || !second->active || 
                    (first->GetProperties().type == ObjectType::_static && second->GetProperties().type == ObjectType::_static)) {
                    continue;
                }
                if (std::less<PhysicsObject*>{}(second, first)) {
                    std::swap(first, second);
                }
                pairs.emplace(first, second);
            }
        }
    }

    std::vector<ContactConstraint> contacts;
    contacts.reserve(pairs.size());
    for (const auto& [first, second] : pairs) {
        const float inverseMassFirst = GetEffectiveInverseMass(first);
        const float inverseMassSecond = GetEffectiveInverseMass(second);
        const float inverseMassSum = inverseMassFirst + inverseMassSecond;
        if (inverseMassSum <= 0.0f) {
            continue;
        }

        const Collider* firstCollider = first->GetProperties().collider;
        const Collider* secondCollider = second->GetProperties().collider;
        const auto* firstBox = dynamic_cast<const BoxCollider*>(firstCollider);
        const auto* secondBox = dynamic_cast<const BoxCollider*>(secondCollider);
        const auto* firstSphere = dynamic_cast<const SphereCollider*>(firstCollider);
        const auto* secondSphere = dynamic_cast<const SphereCollider*>(secondCollider);

        CollisionResult collision{};
        bool colliding = false;
        if (firstBox != nullptr && secondBox != nullptr) {
            colliding = Box_Box(*first, *firstBox, *second, *secondBox, collision);
        }
        else if (firstBox != nullptr && secondSphere != nullptr) {
            colliding = Box_Sphere(*first, *firstBox, *second, *secondSphere, collision);
        }
        else if (firstSphere != nullptr && secondBox != nullptr) {
            colliding = Box_Sphere(*second, *secondBox, *first, *firstSphere, collision);
            collision.normal = Vector3Scale(collision.normal, -1.0f);
        }
        else if (firstSphere != nullptr && secondSphere != nullptr) {
            colliding = Sphere_Sphere(*first, *firstSphere, *second, *secondSphere, collision);
        }

        if (!colliding) {
            continue;
        }

        const float correctionMagnitude = std::max(collision.penetration - 0.001f, 0.0f) * 0.8f / inverseMassSum;
        const Vector3 correction = Vector3Scale(collision.normal, correctionMagnitude);
        if (inverseMassFirst > 0.0f) {
            grid.MoveObject(first, Vector3Scale(correction, -inverseMassFirst));
        }
        if (inverseMassSecond > 0.0f) {
            grid.MoveObject(second, Vector3Scale(correction, inverseMassSecond));
        }

        const PhysicsProperty firstProperties = first->GetProperties();
        const PhysicsProperty secondProperties = second->GetProperties();
        const float initialNormalVelocity = Vector3DotProduct(
            secondProperties.velocity - firstProperties.velocity,
            collision.normal);

        ContactConstraint contact{};
        contact.first = first;
        contact.second = second;
        contact.normal = collision.normal;
        contact.restitution = std::min(firstProperties.elasticity, secondProperties.elasticity);
        contact.velocityBias = initialNormalVelocity < -1.0f
            ? -contact.restitution * initialNormalVelocity
            : 0.0f;
        contacts.push_back(contact);
    }

    constexpr int solverIterations = 8;
    for (int iteration = 0; iteration < solverIterations; ++iteration) {
        for (ContactConstraint& contact : contacts) {
            const Vector3 relativeVelocity = contact.second->GetProperties().velocity -
                contact.first->GetProperties().velocity;
            const float inverseMassSum = GetEffectiveInverseMass(contact.first) +
                GetEffectiveInverseMass(contact.second);
            if (inverseMassSum <= 0.000001f) {
                continue;
            }

            const float normalVelocity = Vector3DotProduct(relativeVelocity, contact.normal);
            const float normalImpulseDelta = -(normalVelocity - contact.velocityBias) / inverseMassSum;
            const float previousNormalImpulse = contact.accumulatedNormalImpulse;
            contact.accumulatedNormalImpulse = std::max(
                previousNormalImpulse + normalImpulseDelta, 0.0f);
            const float appliedNormalImpulse = contact.accumulatedNormalImpulse - previousNormalImpulse;
            if (appliedNormalImpulse != 0.0f) {
                ApplyContactImpulse(contact, Vector3Scale(contact.normal, appliedNormalImpulse));
            }
        }
    }
}

void PhysicsWorld::AddObject(PhysicsObject* object) {
    if (object == nullptr) {
        return;
    }
    if (object->GetProperties().type == ObjectType::_dynamic)
        dynamicObjects.push_back(object);
    
    grid.AddObject(object);
}

void PhysicsWorld::MoveObject(PhysicsObject* object, Vector3 transform) {
    grid.MoveObject(object, transform);
}

void PhysicsWorld::RemoveObject(PhysicsObject* object) {
    if (object == nullptr) {
        return;
    }

    if (!grid.GetCells().empty()) {
        grid.RemoveObject(object);
    }
    if (object->GetProperties().type == ObjectType::_dynamic) {
        for (int i = 0; i < dynamicObjects.size(); i++) {
            if (dynamicObjects[i] == object) {
                dynamicObjects[i] = dynamicObjects.back();
                dynamicObjects.pop_back();
                break;
            }
        }
    }
}

void PhysicsWorld::ApplyForces(float deltaTime) {
    SCOPE_MARKER(Apply_Forces, Physics);
    const float step = deltaTime > 0.0f ? deltaTime : 0.0f;
    if (step == 0.0f) {
        return;
    }

    for (PhysicsObject* object : dynamicObjects) {
        if (object == nullptr || !object->active) {
            continue;
        }

        const PhysicsProperty properties = object->GetProperties();
        if (properties.type == ObjectType::_static || properties.collider == nullptr ||
            properties.collider->inverseMass <= 0.0f) {
            continue;
        }

        const Vector3 translation = Vector3Scale(properties.velocity, step);
        if (Vector3LengthSqr(translation) > 0.0f) {
            grid.MoveObject(object, translation);
        }

        const Vector3 angularVelocity = {
            properties.rotationVelocity.x,
            properties.rotationVelocity.y,
            properties.rotationVelocity.z
        };
        const Vector3 localAngularVelocity = Vector3RotateByQuaternion(
            angularVelocity,
            QuaternionInvert(object->rotation));
        const float angularSpeed = Vector3Length(localAngularVelocity);
        if (angularSpeed > 0.000001f) {
            const Quaternion deltaRotation = QuaternionFromAxisAngle(
                Vector3Scale(localAngularVelocity, 1.0f / angularSpeed),
                angularSpeed * step);
            object->RotateObject(deltaRotation);
        }
    }
}