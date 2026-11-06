#pragma once

#include <raylib.h>
#include <vector>
#include <unordered_map>
#include "physics_object.h"

constexpr int BVH_NULL_NODE = -1;

struct DynamicBVHNode {
    BoundingBox bounds = {};
    int parent = BVH_NULL_NODE;
    int left = BVH_NULL_NODE;
    int right = BVH_NULL_NODE;
    int height = 0;
    PhysicsObject* object = nullptr;

    bool IsLeaf() const { return left == BVH_NULL_NODE; }
};

class DynamicBVH {
public:
    static constexpr float FatMargin = 0.15f;
    static constexpr float MinMargin = 0.05f;

    void Update();

    void AddObject(PhysicsObject* object);
    void RemoveObject(PhysicsObject* object);
    const std::vector<PhysicsObject*>& GetObjects() const;

    std::vector<PhysicsObject*> CheckIntersection(const BoundingBox& bounds) const;

    void Clear();
    void Draw() const;

private:
    std::vector<DynamicBVHNode> nodes;
    std::vector<int> freeNodes;
    int root = BVH_NULL_NODE;

    std::vector<PhysicsObject*> objects;
    std::unordered_map<PhysicsObject*, int> leaves;

    int AllocateNode();
    void FreeNode(int index);
    void InsertLeaf(int leaf);
    void RemoveLeaf(int leaf);
    int Balance(int index);
    void RefitUpwards(int index);

    static BoundingBox CombineBoxes(const BoundingBox& a, const BoundingBox& b);
    static BoundingBox MakeFat(const BoundingBox& tight);

    static float SurfaceArea(const BoundingBox& b);
    static bool Contains(const BoundingBox& outer, const BoundingBox& inner);
};