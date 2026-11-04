#include "BVH.h"

BoundingBox BVH::CombineBoxes(const BoundingBox& a, const BoundingBox& b) {
    return BoundingBox{
        Vector3{ std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z) },
        Vector3{ std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z) }
    };
}

Vector3 BVH::GetBoxCenter(const BoundingBox& box) {
    return Vector3{
        (box.min.x + box.max.x) * 0.5f,
        (box.min.y + box.max.y) * 0.5f,
        (box.min.z + box.max.z) * 0.5f
    };
}

void BVH::AddObject(PhysicsObject* object) {
    if (!object) 
        return;

    if (std::find(objects.begin(), objects.end(), object) == objects.end()) {
        objects.push_back(object);
    }
}

void BVH::RemoveObject(PhysicsObject* object) {
    for (int i = 0; i < objects.size(); i++) {
        if (objects[i] == object) {
            objects[i] = objects.back();
            objects.pop_back();
        }
    }
}

std::vector<PhysicsObject*>& const BVH::GetObjects()
{
    return objects;
}

void BVH::Clear() {
    objects.clear();
    root.reset();
}

void BVH::Draw() const {
    if (!root) 
        return;

    struct StackItem {
        const BVHNode* node;
        int depth;
    };

    std::vector<StackItem> stack;
    stack.reserve(64);

    stack.push_back({ root.get(), 0 });

    while (!stack.empty()) {
        StackItem current = stack.back();
        stack.pop_back();

        const BVHNode* node = current.node;
        int depth = current.depth;

        unsigned char brightness = (unsigned char)std::max(40, 255 - (depth * 25));

        Color boxColor;
        if (node->IsLeaf()) {
            boxColor = Color{ brightness, 0, 0, 255 };
        }
        else {
            boxColor = Color{ 0, brightness, 0, 255 };
        }

        DrawBoundingBox(node->bounds, boxColor);

        if (node->right) {
            stack.push_back({ node->right.get(), depth + 1 });
        }
        if (node->left) {
            stack.push_back({ node->left.get(), depth + 1 });
        }
    }
}

void BVH::Update() {
    root.reset();
    if (objects.empty()) 
        return;

    std::vector<PhysicsObject*> buildList = objects;
    root = BuildRecursive(buildList, 0, buildList.size());
}

std::unique_ptr<BVHNode> BVH::BuildRecursive(std::vector<PhysicsObject*>& objs, size_t start, size_t end) {
    if (start >= end) 
        return nullptr;

    auto node = std::make_unique<BVHNode>();
    size_t count = end - start;

    node->bounds = objs[start]->GetBounds();
    for (size_t i = start + 1; i < end; ++i) {
        node->bounds = CombineBoxes(node->bounds, objs[i]->GetBounds());
    }

    if (count == 1) {
        node->object = objs[start];
        return node;
    }

    Vector3 extent = Vector3Subtract(node->bounds.max, node->bounds.min);
    int axis = 0;
    if (extent.y > extent.x && extent.y > extent.z) 
        axis = 1;
    else if (extent.z > extent.x && extent.z > extent.y) 
        axis = 2;

    float splitPos = 0.0f;
    if (axis == 0) 
        splitPos = node->bounds.min.x + (extent.x * 0.5f);
    else if (axis == 1) 
        splitPos = node->bounds.min.y + (extent.y * 0.5f);
    else if (axis == 2) 
        splitPos = node->bounds.min.z + (extent.z * 0.5f);

    size_t mid = start;
    for (size_t i = start; i < end; ++i) {
        Vector3 center = GetBoxCenter(objs[i]->GetBounds());
        float objPos = (axis == 0) ? center.x : ((axis == 1) ? center.y : center.z);

        if (objPos < splitPos) {
            std::swap(objs[mid], objs[i]);
            mid++;
        }
    }

    if (mid == start || mid == end) {
        mid = start + count / 2;
    }

    // Build children
    node->left = BuildRecursive(objs, start, mid);
    node->right = BuildRecursive(objs, mid, end);

    return node;
}
std::vector<PhysicsObject*> BVH::CheckIntersection(const BoundingBox& bounds) const {
    std::vector<PhysicsObject*> hits;
    if (root) {
        QueryRecursive(root.get(), bounds, hits);
    }
    return hits;
}

void BVH::QueryRecursive(const BVHNode* node, const BoundingBox& bounds, std::vector<PhysicsObject*>& hits) const {
    if (!node || !CheckCollisionBoxes(node->bounds, bounds)) {
        return;
    }

    if (node->IsLeaf()) {
        hits.push_back(node->object);
    }
    else {
        QueryRecursive(node->left.get(), bounds, hits);
        QueryRecursive(node->right.get(), bounds, hits);
    }
}