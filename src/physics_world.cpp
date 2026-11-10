#include "physics_world.h"
#include <algorithm>
#include <cmath>
#include "game_world.h"

void PhysicsWorld::UpdateBVH() {
    staticObjects.Update();
    dynamicObjects.Update();
}

void PhysicsWorld::Update(float deltaTime) {
    SCOPE_MARKER(Update, Physics);

    if (isUpdated) {
        staticObjects.Update();
        isUpdated = false;
    }

    for (auto object : dynamicObjects.GetObjects()) {
        if (!object || !object->active) 
            continue;

        auto& properties = object->GetProperties();

        object->AddForce(properties.collider->mass * Vector3{ 0.f, -9.8f, 0.f });
        object->AddForce(properties.drag * properties.velocity * -properties.collider->mass);
    }

    ApplyForces(deltaTime);
    dynamicObjects.Update();
    CheckCollisions();
}

void PhysicsWorld::DebugDraw() {
    staticObjects.Draw();
}

void PhysicsWorld::AddObject(PhysicsObject* object) {
    if (object == nullptr) return;

    if (object->GetProperties().type == ObjectType::_static) {
        isUpdated = true;
        staticObjects.AddObject(object);
    }
    else {
        dynamicObjects.AddObject(object);
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
        dynamicObjects.RemoveObject(object);
    }
}

void PhysicsWorld::ApplyForces(float deltaTime) {
    SCOPE_MARKER(ApplyForces, Physics);
    deltaTime = std::min(deltaTime, 0.02f);

    for (PhysicsObject* object : dynamicObjects.GetObjects()) {
        if (object == nullptr || !object->active) 
            continue;

        const PhysicsProperty& properties = object->GetProperties();
        if (properties.type != ObjectType::_dynamic || properties.collider == nullptr || properties.collider->inverseMass <= 0.0f) {
            continue;
        }

        object->ApplyAcceleration(deltaTime);
        const Vector3 translation = Vector3Scale(properties.velocity, deltaTime);
        if (Vector3LengthSqr(translation) > 0.0f) {
            GameWorld::GetInstance().MoveObject(object, translation);
        }
    }
}

void PhysicsWorld::CheckCollisions() {
    SCOPE_MARKER(CheckCollisions, Physics);

    broadPhasePairs.clear();
    currentCollisions.clear();
    collisions.clear();

    BroadPhase();
    NarrowPhase();
    CollisionEvents();
    SolveCollisions();
}

void PhysicsWorld::BroadPhase() {
    SCOPE_MARKER(BroadPhase, Physics);

    const auto& objs = dynamicObjects.GetObjects();
    for (size_t i = 0; i < objs.size(); ++i) {
        PhysicsObject* objA = objs[i];

        if (!objA || !objA->active)
            continue;

        const BoundingBox boundsA = objA->GetBounds();

        // Dynamic vs Static
        for (PhysicsObject* objB : staticObjects.CheckIntersection(boundsA)) {
            if (!objB || !objB->active || objA == objB)
                continue;
            broadPhasePairs.emplace_back(objA, objB);
        }

        // Dynamic vs Dynamic
        for (PhysicsObject* objB : dynamicObjects.CheckIntersection(boundsA)) {
            if (!objB || !objB->active || objA >= objB)
                continue;
            broadPhasePairs.emplace_back(objA, objB);
        }
    }
}

void PhysicsWorld::NarrowPhase() {
    SCOPE_MARKER(NarrowPhase, Physics);

    for (const auto& pair : broadPhasePairs) {
        CollisionInfo collision;

        if (!CheckCollision(pair.first, pair.second, collision))
            continue;

        collisions.push_back(collision);
        currentCollisions.insert(pair);
    }
}

void PhysicsWorld::CollisionEvents() {
    SCOPE_MARKER(CollisionEvents, Physics);

    // Enter / Stay
    for (const CollisionInfo& collision : collisions) {
        PhysicsObject* objA = collision.a;
        PhysicsObject* objB = collision.b;

        CollisionPair pair = std::make_pair(objA, objB);

        if (previousCollisions.contains(pair)) {
            objA->OnCollisionStay(objB, collision);
            objB->OnCollisionStay(objA, collision);
        }
        else {
            objA->OnCollisionEnter(objB, collision);
            objB->OnCollisionEnter(objA, collision);
        }
    }

    // Exit
    for (const auto& pair : previousCollisions) {
        if (!currentCollisions.contains(pair)) {
            pair.first->OnCollisionExit(pair.second);
            pair.second->OnCollisionExit(pair.first);
        }
    }

    previousCollisions = std::move(currentCollisions);
}

void PhysicsWorld::SolveCollisions() {
    SCOPE_MARKER(SolveCollisions, Physics);

    for (const auto& collision : collisions) {
        if (!collision.a || !collision.b)
            continue;

        const PhysicsProperty& propA = collision.a->GetProperties();
        const PhysicsProperty& propB = collision.b->GetProperties();

        const float invA = propA.collider->inverseMass;
        const float invB = propB.collider->inverseMass;
        const float totalInvMass = invA + invB;
        if (totalInvMass <= 0.0f)
            continue;

        const Vector3 normal = collision.point.normal;

        const Vector3 relativeVelocity = propB.velocity - propA.velocity;
        const float velAlongNormal = Vector3DotProduct(relativeVelocity, normal);

        if (velAlongNormal >= 0.0f)
            continue;

        float elasticity = propA.elasticity * propB.elasticity;
        if (-velAlongNormal < 1e-5f)
            elasticity = 0.0f;

        const float j = -(1.0f + elasticity) * velAlongNormal / totalInvMass;
        const Vector3 impulse = normal * j;

        collision.a->AddImpulse(impulse * -1.0f);
        collision.b->AddImpulse(impulse);

        const float depth = std::max(collision.point.penetration, 0.0f);
        if (depth <= 0.0f)
            continue;

        const Vector3 correction = collision.point.normal * depth / totalInvMass;

        if (invA > 0.0f)
            GameWorld::GetInstance().MoveObject(collision.a, correction * -invA);
        if (invB > 0.0f)
            GameWorld::GetInstance().MoveObject(collision.b, correction * invB);
    }
}

bool PhysicsWorld::CheckCollision(PhysicsObject* objA, PhysicsObject* objB, CollisionInfo& result) {
    Collider* colA = objA->GetProperties().collider;
    Collider* colB = objB->GetProperties().collider;
    result.a = objA;
    result.b = objB;

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
            result.point.normal = result.point.normal * -1.f;
            std::swap(result.point.localA, result.point.localB);
            result.a = objA;
            result.b = objB;
        }
        return hit;
    }
    return false;
}
bool PhysicsWorld::SphereSphereCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, SphereCollider* b, CollisionInfo& m) {
    const float radii = a->radius + b->radius;
    const Vector3 delta = objB->position - objA->position;
    const float distSqr = Vector3LengthSqr(delta);

    if (distSqr >= radii * radii)
        return false;

    const float dist = std::sqrt(distSqr);
    const Vector3 normal = dist > 0 ? delta * (1.0f / dist) : Vector3{ 0.f, 1.f, 0.f };

    m.AddContactPoint(normal * a->radius, normal * -b->radius, normal, radii - dist);
    return true;
}
bool PhysicsWorld::BoxBoxCollision(PhysicsObject* objA, BoxCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionInfo& m) {
    static const Vector3 faces[6] = {
        Vector3{ 1.f, 0.f, 0.f }, Vector3{-1.f, 0.f, 0.f },
        Vector3{ 0.f, 1.f, 0.f }, Vector3{ 0.f,-1.f, 0.f },
        Vector3{ 0.f, 0.f, 1.f }, Vector3{ 0.f, 0.f,-1.f },
    };

    const BoundingBox A = objA->GetBounds();
    const BoundingBox B = objB->GetBounds();

    const float depths[6] = {
        A.max.x - B.min.x,
        B.max.x - A.min.x,
        A.max.y - B.min.y,
        B.max.y - A.min.y,
        A.max.z - B.min.z,
        B.max.z - A.min.z,
    };

    float penetration = FLT_MAX;
    int best = 0;
    for (int i = 0; i < 6; ++i) {
        if (depths[i] <= 0.0f)
            return false;
        if (depths[i] < penetration) {
            penetration = depths[i];
            best = i;
        }
    }

    m.AddContactPoint(Vector3{}, Vector3{}, faces[best], penetration);
    return true;
}
bool PhysicsWorld::SphereBoxCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionInfo& m) {
    const BoundingBox box = objB->GetBounds();
    const Vector3 center = objA->position;

    const Vector3 closest = Vector3Clamp(center, box.min, box.max);
    const Vector3 diff = center - closest;
    const float distSqr = Vector3LengthSqr(diff);

    if (distSqr >= a->radius * a->radius)
        return false;

    if (distSqr > 0.f) {
        const float dist = std::sqrt(distSqr);
        const Vector3 normal = diff * (-1.0f / dist);
        m.AddContactPoint(normal * a->radius, closest - objB->position, normal, a->radius - dist);
        return true;
    }

    const float d[6] = {
        center.x - box.min.x, box.max.x - center.x,
        center.y - box.min.y, box.max.y - center.y,
        center.z - box.min.z, box.max.z - center.z,
    };
    static const Vector3 outward[6] = {
        Vector3{-1.f, 0.f, 0.f}, Vector3{ 1.f, 0.f, 0.f},
        Vector3{ 0.f,-1.f, 0.f}, Vector3{ 0.f, 1.f, 0.f},
        Vector3{ 0.f, 0.f,-1.f}, Vector3{ 0.f, 0.f, 1.f},
    };
    int best = 0;
    for (int i = 1; i < 6; ++i)
        if (d[i] < d[best]) best = i;

    const Vector3 normal = outward[best] * -1.0f;
    m.AddContactPoint(normal * a->radius, Vector3{}, normal, a->radius + d[best]);
    return true;
}


bool PhysicsWorld::Raycast(Ray ray, RaycastHit& hitResult, float maxDistance) {
    bool hit = false;
    hitResult = RaycastHit{};
    hitResult.distance = maxDistance;

    for (auto obj : dynamicObjects.Raycast(ray)) {
        if (RaycastObject(obj, ray, hitResult, maxDistance)) {
            hit = true;
        }
    }

    for (auto obj : staticObjects.Raycast(ray)) {
        if (RaycastObject(obj, ray, hitResult, maxDistance)) {
            hit = true;
        }
    }

    return hit;
}

bool PhysicsWorld::RaycastObject(PhysicsObject* object, Ray ray, RaycastHit& hitResult, float maxDistance) {
    RayCollision col = object->Raycast(ray);
    if (col.hit && col.distance < hitResult.distance && col.distance <= maxDistance) {
        hitResult.distance = col.distance;
        hitResult.normal = col.normal;
        hitResult.point = col.point;
        hitResult.object = object;
        return true;
    }
    return false;
}
