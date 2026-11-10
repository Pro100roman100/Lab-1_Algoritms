#pragma once
#include <vector>
#include <set>
#include <utility>
#include <raylib.h>
#include <unordered_set>

#include "physics_object.h"
#include "uniform_grid.h"
#include "scope_marker.h"

#include "BVH.h"
#include "dynamicBVH.h"

#define USE_DYNAMIC_BVH

class PhysicsWorld {
public:
    void UpdateBVH();
    void Update(float deltaTime);
    void DebugDraw();

    void AddObject(PhysicsObject* object);
    void MoveObject(PhysicsObject* object, Vector3 transform);
    void RemoveObject(PhysicsObject* object);

    bool Raycast(Ray ray, RaycastHit& hitResult, float maxDistance = 1000.0f);

private:
    void CheckCollisions();
    void BroadPhase();
    void NarrowPhase();
    void CollisionEvents();
    void SolveCollisions();

    void ApplyForces(float deltaTime);

    bool CheckCollision(PhysicsObject* objA, PhysicsObject* objB, CollisionInfo& result);
    bool BoxBoxCollision(PhysicsObject* objA, BoxCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionInfo& result);
    bool SphereBoxCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, BoxCollider* b, CollisionInfo& result);
    bool SphereSphereCollision(PhysicsObject* objA, SphereCollider* a, PhysicsObject* objB, SphereCollider* b, CollisionInfo& result);

    bool RaycastObject(PhysicsObject* object, Ray ray, RaycastHit& hitResult, float maxDistance = 1000.0f);

#ifdef USE_DYNAMIC_BVH
    DynamicBVH dynamicObjects;
#else
    std::vector<PhysicsObject*> dynamicObjects;
#endif // USE_DYNAMIC_BVH

    bool isUpdated = true;
    BVH staticObjects;

    using CollisionPair = std::pair<PhysicsObject*, PhysicsObject*>;
    struct CollisionPairHash {
        std::size_t operator()(const CollisionPair& pair) const {
            auto h1 = std::hash<PhysicsObject*>{}(pair.first);
            auto h2 = std::hash<PhysicsObject*>{}(pair.second);

            return h1 ^ h2;
        }
    };
    std::vector<CollisionPair> broadPhasePairs;
    std::vector<CollisionInfo> collisions;
    std::unordered_set<CollisionPair, CollisionPairHash> previousCollisions;
    std::unordered_set<CollisionPair, CollisionPairHash> currentCollisions;
};