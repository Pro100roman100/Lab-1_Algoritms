#pragma once

#include <raylib.h>
#include <vector>
#include "physics_object.h"

struct BVHNode {
    BoundingBox bounds;
    std::unique_ptr<BVHNode> left;
    std::unique_ptr<BVHNode> right;
    PhysicsObject* object = nullptr;

    bool IsLeaf() const { return object != nullptr; }
};

class BVH {
public:
    std::unique_ptr<BVHNode> root;

	void Update();

	void AddObject(PhysicsObject* object);
	void RemoveObject(PhysicsObject* object);
    const std::vector<PhysicsObject*>&  GetObjects();

    std::vector<PhysicsObject*> CheckIntersection(const BoundingBox& bounds) const;

    void Clear();

    void Draw() const;
private:
    std::vector<PhysicsObject*> objects;

    std::unique_ptr<BVHNode> BuildRecursive(std::vector<PhysicsObject*>& objs, size_t start, size_t end);
    void QueryRecursive(const BVHNode* node, const BoundingBox& bounds, std::vector<PhysicsObject*>& hits) const;
    
    static BoundingBox CombineBoxes(const BoundingBox& a, const BoundingBox& b);
    static Vector3 GetBoxCenter(const BoundingBox& box);
};