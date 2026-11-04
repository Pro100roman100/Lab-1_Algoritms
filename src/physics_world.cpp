#include "physics_world.h"
#include <algorithm>
#include <cmath>
#include "game_world.h"

void PhysicsWorld::Update(float deltaTime) {
    SCOPE_MARKER(Update, Physics);
    
    if (isUpdated) {
        staticObjects.Update();
        isUpdated = false;
    }

    for (auto object : dynamicObjects) {
        if (!object || !object->active) continue;

        auto& properties = object->GetProperties();

        object->AddForce(Vector3Scale(Vector3{ 0.f, -9.8f, 0.f }, properties.collider->mass * deltaTime));
        object->AddForce(Vector3Scale(properties.velocity, -0.05f * deltaTime));
        object->ApplyAngularDamping(0.05f);
    }

    ApplyForces(deltaTime);
    SolveCollisions();
}

void PhysicsWorld::AddObject(PhysicsObject* object) {
    if (object == nullptr) return;

    if (object->GetProperties().type == ObjectType::_static) {
        isUpdated = true;
        staticObjects.AddObject(object);
    }
    else {
        dynamicObjects.push_back(object);
    }
}

void PhysicsWorld::MoveObject(PhysicsObject* object, Vector3 transform) {
    if (!object) return;

    object->position = Vector3Add(object->position, transform);

    if (object->GetProperties().type == ObjectType::_static) {
        staticObjects.Update();
    }
}

void PhysicsWorld::RemoveObject(PhysicsObject* object) {
    if (object == nullptr) return;

    for (auto it = previousCollisions.begin(); it != previousCollisions.end(); ) {
        if (it->first == object || it->second == object) {
            PhysicsObject* other = (it->first == object) ? it->second : it->first;
            other->OnCollisionExit(object);
            it = previousCollisions.erase(it);
        }
        else {
            ++it;
        }
    }

    if (object->GetProperties().type == ObjectType::_static) {
        isUpdated = true;
        staticObjects.RemoveObject(object);
    }
    else {
        auto it = std::find(dynamicObjects.begin(), dynamicObjects.end(), object);
        if (it != dynamicObjects.end()) {
            *it = dynamicObjects.back();
            dynamicObjects.pop_back();
        }
    }
}

void PhysicsWorld::ApplyForces(float deltaTime) {
    SCOPE_MARKER(ApplyForces, Physics);
    const float step = std::min(deltaTime, 0.02f);

    for (PhysicsObject* object : dynamicObjects) {
        if (object == nullptr || !object->active) continue;

        const PhysicsProperty properties = object->GetProperties();
        if (properties.type != ObjectType::_dynamic || properties.collider == nullptr ||
            properties.collider->inverseMass <= 0.0f) {
            continue;
        }

        const Vector3 translation = Vector3Scale(properties.velocity, step);
        if (Vector3LengthSqr(translation) > 0.0f) {
            GameWorld::GetInstance().MoveObject(object, translation);
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

void PhysicsWorld::SolveCollisions() {
    SCOPE_MARKER(SolveCollisions, Physics);
    std::vector<CollisionResult> contacts;
    std::set<CollisionPair> currentCollisions;

    // 1. Широка фаза
    {
        SCOPE_MARKER(BroadPhase, Physics);
        for (size_t i = 0; i < dynamicObjects.size(); ++i) {
            PhysicsObject* objA = dynamicObjects[i];
            if (!objA || !objA->active) continue;

            // Динамічні vs Статичні
            std::vector<PhysicsObject*> staticHits = staticObjects.CheckIntersection(objA->GetBounds());
            for (PhysicsObject* objB : staticHits) {
                CollisionResult result;
                if (CheckCollision(objA, objB, result)) {
                    contacts.push_back(result);

                    CollisionPair pair = (objA < objB) ? std::make_pair(objA, objB) : std::make_pair(objB, objA);
                    currentCollisions.insert(pair);

                    if (previousCollisions.find(pair) == previousCollisions.end()) {
                        objA->OnCollisionEnter(objB, result);
                        objB->OnCollisionEnter(objA, result);
                    }
                    else {
                        objA->OnCollisionStay(objB, result);
                        objB->OnCollisionStay(objA, result);
                    }
                }
            }

            // Динамічні vs Динамічні
            for (size_t j = i + 1; j < dynamicObjects.size(); ++j) {
                PhysicsObject* objB = dynamicObjects[j];
                if (!objB || !objB->active) continue;
                if (CheckCollisionBoxes(objA->GetBounds(), objB->GetBounds())) {
                    CollisionResult result;
                    if (CheckCollision(objA, objB, result)) {
                        contacts.push_back(result);

                        CollisionPair pair = (objA < objB) ? std::make_pair(objA, objB) : std::make_pair(objB, objA);
                        currentCollisions.insert(pair);

                        if (previousCollisions.find(pair) == previousCollisions.end()) {
                            objA->OnCollisionEnter(objB, result);
                            objB->OnCollisionEnter(objA, result);
                        }
                        else {
                            objA->OnCollisionStay(objB, result);
                            objB->OnCollisionStay(objA, result);
                        }
                    }
                }
            }
        }
    }
    

    for (const auto& pair : previousCollisions) {
        if (currentCollisions.find(pair) == currentCollisions.end()) {
            pair.first->OnCollisionExit(pair.second);
            pair.second->OnCollisionExit(pair.first);
        }
    }

    previousCollisions = currentCollisions;

    {
        SCOPE_MARKER(NarrowPhase, Physics);
        const int solverIterations = 1;
        for (int i = 0; i < solverIterations; ++i) {
            for (auto& contact : contacts) {
                ResolveContact(contact);
            }
        }
    }
}

bool PhysicsWorld::CheckCollision(PhysicsObject* objA, PhysicsObject* objB, CollisionResult& result) {
    Collider* colA = objA->GetProperties().collider;
    Collider* colB = objB->GetProperties().collider;

    BoxCollider* boxA = dynamic_cast<BoxCollider*>(colA);
    SphereCollider* sphereA = dynamic_cast<SphereCollider*>(colA);

    BoxCollider* boxB = dynamic_cast<BoxCollider*>(colB);
    SphereCollider* sphereB = dynamic_cast<SphereCollider*>(colB);

    if (boxA && boxB) 
        return BoxBoxCollision(objA, boxA, objB, boxB, result);
    if (sphereA && sphereB) 
        return SphereSphereCollision(objA, sphereA, objB, sphereB, result);
    if (sphereA && boxB) 
        return SphereBoxCollision(objA, sphereA, objB, boxB, result);
    if (boxA && sphereB) {
        bool hit = SphereBoxCollision(objB, sphereB, objA, boxA, result);
        if (hit) {
            result.normal = Vector3Scale(result.normal, -1.0f);
            result.objA = objA;
            result.objB = objB;
        }
        return hit;
    }
    return false;
}

bool PhysicsWorld::SphereSphereCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, SphereCollider* b, CollisionResult& result) {
    Vector3 diff = Vector3Subtract(objB->position, objA->position);
    float distSqr = Vector3LengthSqr(diff);
    float radiusSum = a->radius + b->radius;

    if (distSqr >= radiusSum * radiusSum) return false;

    float dist = sqrtf(distSqr);
    result.objA = objA;
    result.objB = objB;

    if (dist == 0.0f) {
        result.normal = { 0.0f, 1.0f, 0.0f };
        result.penetration = a->radius;
    }
    else {
        result.normal = Vector3Scale(diff, 1.0f / dist); // Нормаль від A до B
        result.penetration = radiusSum - dist;
    }
    result.position = Vector3Add(objA->position, Vector3Scale(result.normal, a->radius));
    return true;
}
bool PhysicsWorld::BoxBoxCollision(PhysicsObject* objA, BoxCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionResult& result) {
    BoundingBox boxA = objA->GetBounds();
    BoundingBox boxB = objB->GetBounds();

    Vector3 centerA = { (boxA.min.x + boxA.max.x) * 0.5f, (boxA.min.y + boxA.max.y) * 0.5f, (boxA.min.z + boxA.max.z) * 0.5f };
    Vector3 centerB = { (boxB.min.x + boxB.max.x) * 0.5f, (boxB.min.y + boxB.max.y) * 0.5f, (boxB.min.z + boxB.max.z) * 0.5f };

    Vector3 halfExtentsA = { (boxA.max.x - boxA.min.x) * 0.5f, (boxA.max.y - boxA.min.y) * 0.5f, (boxA.max.z - boxA.min.z) * 0.5f };
    Vector3 halfExtentsB = { (boxB.max.x - boxB.min.x) * 0.5f, (boxB.max.y - boxB.min.y) * 0.5f, (boxB.max.z - boxB.min.z) * 0.5f };

    Vector3 distance = Vector3Subtract(centerB, centerA);

    float overlapX = (halfExtentsA.x + halfExtentsB.x) - std::abs(distance.x);
    if (overlapX <= 0.0f) return false;

    float overlapY = (halfExtentsA.y + halfExtentsB.y) - std::abs(distance.y);
    if (overlapY <= 0.0f) return false;

    float overlapZ = (halfExtentsA.z + halfExtentsB.z) - std::abs(distance.z);
    if (overlapZ <= 0.0f) return false;

    result.objA = objA;
    result.objB = objB;

    if (overlapX < overlapY && overlapX < overlapZ) {
        result.penetration = overlapX;
        result.normal = (distance.x < 0) ? Vector3{ -1, 0, 0 } : Vector3{ 1, 0, 0 };
    }
    else if (overlapY < overlapZ) {
        result.penetration = overlapY;
        result.normal = (distance.y < 0) ? Vector3{ 0, -1, 0 } : Vector3{ 0, 1, 0 };
    }
    else {
        result.penetration = overlapZ;
        result.normal = (distance.z < 0) ? Vector3{ 0, 0, -1 } : Vector3{ 0, 0, 1 };
    }

    result.position = Vector3Add(centerA, Vector3Scale(result.normal, halfExtentsA.x)); // Орієнтовна точка зіткнення
    return true;
}
bool PhysicsWorld::SphereBoxCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionResult& result) {
    BoundingBox box = objB->GetBounds();
    Vector3 spherePos = objA->position;

    Vector3 closestPoint = {
        std::clamp(spherePos.x, box.min.x, box.max.x),
        std::clamp(spherePos.y, box.min.y, box.max.y),
        std::clamp(spherePos.z, box.min.z, box.max.z)
    };

    Vector3 diff = Vector3Subtract(spherePos, closestPoint);
    float distSqr = Vector3LengthSqr(diff);

    if (distSqr > a->radius * a->radius) return false;

    result.objA = objA;
    result.objB = objB;
    float dist = sqrtf(distSqr);

    if (dist == 0.0f) {
        result.normal = { 0, 1, 0 };
        result.penetration = a->radius;
        result.position = closestPoint;
    }
    else {
        result.penetration = a->radius - dist;
        result.normal = Vector3Scale(diff, -1.0f / dist); // Нормаль від Сфери (A) до Коробки (B)
        result.position = closestPoint;
    }
    return true;
}

void PhysicsWorld::ResolveContact(CollisionResult& contact) {
    PhysicsObject* objA = contact.objA;
    PhysicsObject* objB = contact.objB;

    float invMassA = objA->GetProperties().collider->inverseMass;
    float invMassB = objB->GetProperties().collider->inverseMass;

    if (invMassA == 0.0f && invMassB == 0.0f) 
        return;

    Vector3 relVel = Vector3Subtract(objB->GetProperties().velocity, objA->GetProperties().velocity);
    float velAlongNormal = Vector3DotProduct(relVel, contact.normal);

    if (velAlongNormal > 0.0f) 
        return;

    float e = objA->GetProperties().elasticity * objB->GetProperties().elasticity;

    float j = -(1.0f + e) * velAlongNormal;
    j /= (invMassA + invMassB);

    Vector3 impulse = Vector3Scale(contact.normal, j);

    if (invMassA > 0.0f) 
        objA->AddImpulse(Vector3Scale(impulse, -1.0f));
    if (invMassB > 0.0f) 
        objB->AddImpulse(impulse);

    const float percent = 0.8f;
    const float slop = 0.05f;
    Vector3 correction = Vector3Scale(contact.normal, (std::max(contact.penetration - slop, 0.0f) / (invMassA + invMassB)) * percent);

    if (invMassA > 0.0f) {
        Vector3 moveA = Vector3Scale(correction, -invMassA);
        GameWorld::GetInstance().MoveObject(objA, moveA);
    }
    if (invMassB > 0.0f) {
        Vector3 moveB = Vector3Scale(correction, invMassB);
        GameWorld::GetInstance().MoveObject(objB, moveB);
    }
}

bool PhysicsWorld::Raycast(Ray ray, RaycastHit& hitResult, float maxDistance) {
    bool hit = false;
    hitResult.distance = maxDistance;

    // Динамічні об'єкти
    for (auto obj : dynamicObjects) {
        if (!obj || !obj->active) continue;
        RayCollision col = GetRayCollisionBox(ray, obj->GetBounds());
        if (col.hit && col.distance < hitResult.distance) {
            hitResult.distance = col.distance;
            hitResult.normal = col.normal;
            hitResult.point = col.point;
            hitResult.object = obj;
            hit = true;
        }
    }

    // Статичні об'єкти
    BoundingBox rayBox;
    Vector3 endPos = Vector3Add(ray.position, Vector3Scale(ray.direction, maxDistance));
    rayBox.min = { std::min(ray.position.x, endPos.x), std::min(ray.position.y, endPos.y), std::min(ray.position.z, endPos.z) };
    rayBox.max = { std::max(ray.position.x, endPos.x), std::max(ray.position.y, endPos.y), std::max(ray.position.z, endPos.z) };

    std::vector<PhysicsObject*> potentialStatics = staticObjects.CheckIntersection(rayBox);
    for (auto obj : potentialStatics) {
        RayCollision col = GetRayCollisionBox(ray, obj->GetBounds());
        if (col.hit && col.distance < hitResult.distance) {
            hitResult.distance = col.distance;
            hitResult.normal = col.normal;
            hitResult.point = col.point;
            hitResult.object = obj;
            hit = true;
        }
    }

    return hit;
}