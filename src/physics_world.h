#pragma once
#include <vector>
#include <set>
#include <utility>
#include <raylib.h>
#include "physics_object.h"
#include "uniform_grid.h"
#include "scope_marker.h"
#include "BVH.h"

struct RaycastHit {
    PhysicsObject* object = nullptr;
    Vector3 point = { 0 };
    Vector3 normal = { 0 };
    float distance = 0.0f;
};

class PhysicsWorld {
public:
    void Update(float deltaTime);

    void AddObject(PhysicsObject* object);
    void MoveObject(PhysicsObject* object, Vector3 transform);
    void RemoveObject(PhysicsObject* object);

    bool Raycast(Ray ray, RaycastHit& hitResult, float maxDistance = 1000.0f);

private:
    void SolveCollisions();
    void ApplyForces(float deltaTime);

    bool CheckCollision(PhysicsObject* objA, PhysicsObject* objB, CollisionResult& result);
    bool BoxBoxCollision(PhysicsObject* objA, BoxCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionResult& result);
    bool SphereBoxCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionResult& result);
    bool SphereSphereCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, SphereCollider* b, CollisionResult& result);

    void ResolveContact(CollisionResult& contact);

    std::vector<PhysicsObject*> dynamicObjects;
    bool isUpdated = true;
    BVH staticObjects;

    using CollisionPair = std::pair<PhysicsObject*, PhysicsObject*>;
    std::set<CollisionPair> previousCollisions;
};