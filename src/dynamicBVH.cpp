#include "dynamicBVH.h"
#include <algorithm>

float DynamicBVH::SurfaceArea(const BoundingBox& b) {
    const float dx = b.max.x - b.min.x;
    const float dy = b.max.y - b.min.y;
    const float dz = b.max.z - b.min.z;
    return 2.0f * (dx * dy + dy * dz + dz * dx);
}

bool DynamicBVH::Contains(const BoundingBox& outer, const BoundingBox& inner) {
    return outer.min.x <= inner.min.x && outer.min.y <= inner.min.y && outer.min.z <= inner.min.z &&
        outer.max.x >= inner.max.x && outer.max.y >= inner.max.y && outer.max.z >= inner.max.z;
}

BoundingBox DynamicBVH::CombineBoxes(const BoundingBox& a, const BoundingBox& b) {
    return BoundingBox{
        Vector3{ std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y), std::min(a.min.z, b.min.z) },
        Vector3{ std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y), std::max(a.max.z, b.max.z) }
    };
}

BoundingBox DynamicBVH::MakeFat(const BoundingBox& tight) {
    const float k = FatMargin * 0.5f;
    const Vector3 ext = Vector3Subtract(tight.max, tight.min);
    const Vector3 m = {
        std::max(ext.x * k, MinMargin),
        std::max(ext.y * k, MinMargin),
        std::max(ext.z * k, MinMargin)
    };
    return BoundingBox{ Vector3Subtract(tight.min, m), Vector3Add(tight.max, m) };
}

int DynamicBVH::AllocateNode() {
    if (!freeNodes.empty()) {
        const int index = freeNodes.back();
        freeNodes.pop_back();
        nodes[index] = DynamicBVHNode{};
        return index;
    }
    nodes.emplace_back();
    return static_cast<int>(nodes.size()) - 1;
}

void DynamicBVH::FreeNode(int index) {
    nodes[index] = DynamicBVHNode{};
    freeNodes.push_back(index);
}

void DynamicBVH::AddObject(PhysicsObject* object) {
    if (!object || leaves.find(object) != leaves.end())
        return;

    objects.push_back(object);

    const int leaf = AllocateNode();
    nodes[leaf].object = object;
    nodes[leaf].bounds = MakeFat(object->GetBounds());
    leaves[object] = leaf;

    InsertLeaf(leaf);
}

void DynamicBVH::RemoveObject(PhysicsObject* object) {
    const auto it = leaves.find(object);
    if (it == leaves.end())
        return;

    const int leaf = it->second;
    RemoveLeaf(leaf);
    FreeNode(leaf);
    leaves.erase(it);

    const auto objIt = std::find(objects.begin(), objects.end(), object);
    if (objIt != objects.end()) {
        *objIt = objects.back();
        objects.pop_back();
    }
}

const std::vector<PhysicsObject*>& DynamicBVH::GetObjects() const {
    return objects;
}

void DynamicBVH::Clear() {
    objects.clear();
    leaves.clear();
    nodes.clear();
    freeNodes.clear();
    root = BVH_NULL_NODE;
}

void DynamicBVH::Update() {
    for (PhysicsObject* object : objects) {
        const int leaf = leaves[object];
        const BoundingBox tight = object->GetBounds();

        if (Contains(nodes[leaf].bounds, tight))
            continue;

        RemoveLeaf(leaf);
        nodes[leaf].bounds = MakeFat(tight);
        InsertLeaf(leaf);
    }
}

void DynamicBVH::InsertLeaf(int leaf) {
    if (root == BVH_NULL_NODE) {
        root = leaf;
        nodes[root].parent = BVH_NULL_NODE;
        return;
    }

    const BoundingBox leafBox = nodes[leaf].bounds;
    int index = root;
    while (!nodes[index].IsLeaf()) {
        const int child1 = nodes[index].left;
        const int child2 = nodes[index].right;

        const float area = SurfaceArea(nodes[index].bounds);
        const float combinedArea = SurfaceArea(CombineBoxes(nodes[index].bounds, leafBox));

        const float cost = 2.0f * combinedArea;
        const float inheritanceCost = 2.0f * (combinedArea - area);

        auto childCost = [&](int child) {
            const float merged = SurfaceArea(CombineBoxes(leafBox, nodes[child].bounds));
            if (nodes[child].IsLeaf())
                return merged + inheritanceCost;
            return merged - SurfaceArea(nodes[child].bounds) + inheritanceCost;
            };

        const float cost1 = childCost(child1);
        const float cost2 = childCost(child2);

        if (cost < cost1 && cost < cost2)
            break;

        index = (cost1 < cost2) ? child1 : child2;
    }

    const int sibling = index;
    const int oldParent = nodes[sibling].parent;
    const int newParent = AllocateNode();

    nodes[newParent].parent = oldParent;
    nodes[newParent].bounds = CombineBoxes(leafBox, nodes[sibling].bounds);
    nodes[newParent].height = nodes[sibling].height + 1;
    nodes[newParent].left = sibling;
    nodes[newParent].right = leaf;

    if (oldParent != BVH_NULL_NODE) {
        if (nodes[oldParent].left == sibling)
            nodes[oldParent].left = newParent;
        else
            nodes[oldParent].right = newParent;
    }
    else {
        root = newParent;
    }

    nodes[sibling].parent = newParent;
    nodes[leaf].parent = newParent;

    RefitUpwards(newParent);
}

void DynamicBVH::RemoveLeaf(int leaf) {
    if (leaf == root) {
        root = BVH_NULL_NODE;
        return;
    }

    const int parent = nodes[leaf].parent;
    const int grandParent = nodes[parent].parent;
    const int sibling = (nodes[parent].left == leaf) ? nodes[parent].right : nodes[parent].left;

    if (grandParent != BVH_NULL_NODE) {
        if (nodes[grandParent].left == parent)
            nodes[grandParent].left = sibling;
        else
            nodes[grandParent].right = sibling;

        nodes[sibling].parent = grandParent;
        FreeNode(parent);
        RefitUpwards(grandParent);
    }
    else {
        root = sibling;
        nodes[sibling].parent = BVH_NULL_NODE;
        FreeNode(parent);
    }

    nodes[leaf].parent = BVH_NULL_NODE;
}

void DynamicBVH::RefitUpwards(int index) {
    while (index != BVH_NULL_NODE) {
        index = Balance(index);

        const int l = nodes[index].left;
        const int r = nodes[index].right;
        nodes[index].height = 1 + std::max(nodes[l].height, nodes[r].height);
        nodes[index].bounds = CombineBoxes(nodes[l].bounds, nodes[r].bounds);

        index = nodes[index].parent;
    }
}

int DynamicBVH::Balance(int iA) {
    DynamicBVHNode& A = nodes[iA];
    if (A.IsLeaf() || A.height < 2)
        return iA;

    const int iB = A.left;
    const int iC = A.right;
    DynamicBVHNode& B = nodes[iB];
    DynamicBVHNode& C = nodes[iC];

    const int balance = C.height - B.height;

    if (balance > 1) {
        const int iF = C.left;
        const int iG = C.right;
        DynamicBVHNode& F = nodes[iF];
        DynamicBVHNode& G = nodes[iG];

        C.left = iA;
        C.parent = A.parent;
        A.parent = iC;

        if (C.parent != BVH_NULL_NODE) {
            if (nodes[C.parent].left == iA) nodes[C.parent].left = iC;
            else                            nodes[C.parent].right = iC;
        }
        else {
            root = iC;
        }

        if (F.height > G.height) {
            C.right = iF;
            A.right = iG;
            G.parent = iA;
            A.bounds = CombineBoxes(B.bounds, G.bounds);
            C.bounds = CombineBoxes(A.bounds, F.bounds);
            A.height = 1 + std::max(B.height, G.height);
            C.height = 1 + std::max(A.height, F.height);
        }
        else {
            C.right = iG;
            A.right = iF;
            F.parent = iA;
            A.bounds = CombineBoxes(B.bounds, F.bounds);
            C.bounds = CombineBoxes(A.bounds, G.bounds);
            A.height = 1 + std::max(B.height, F.height);
            C.height = 1 + std::max(A.height, G.height);
        }
        return iC;
    }

    if (balance < -1) {
        const int iD = B.left;
        const int iE = B.right;
        DynamicBVHNode& D = nodes[iD];
        DynamicBVHNode& E = nodes[iE];

        B.left = iA;
        B.parent = A.parent;
        A.parent = iB;

        if (B.parent != BVH_NULL_NODE) {
            if (nodes[B.parent].left == iA) nodes[B.parent].left = iB;
            else                            nodes[B.parent].right = iB;
        }
        else {
            root = iB;
        }

        if (D.height > E.height) {
            B.right = iD;
            A.left = iE;
            E.parent = iA;
            A.bounds = CombineBoxes(C.bounds, E.bounds);
            B.bounds = CombineBoxes(A.bounds, D.bounds);
            A.height = 1 + std::max(C.height, E.height);
            B.height = 1 + std::max(A.height, D.height);
        }
        else {
            B.right = iE;
            A.left = iD;
            D.parent = iA;
            A.bounds = CombineBoxes(C.bounds, D.bounds);
            B.bounds = CombineBoxes(A.bounds, E.bounds);
            A.height = 1 + std::max(C.height, D.height);
            B.height = 1 + std::max(A.height, E.height);
        }
        return iB;
    }

    return iA;
}

std::vector<PhysicsObject*> DynamicBVH::CheckIntersection(const BoundingBox& bounds) const {
    std::vector<PhysicsObject*> hits;
    if (root == BVH_NULL_NODE)
        return hits;

    std::vector<int> stack;
    stack.reserve(64);
    stack.push_back(root);

    while (!stack.empty()) {
        const int index = stack.back();
        stack.pop_back();

        const DynamicBVHNode& node = nodes[index];
        if (!CheckCollisionBoxes(node.bounds, bounds))
            continue;

        if (node.IsLeaf()) {
            if (CheckCollisionBoxes(node.object->GetBounds(), bounds))
                hits.push_back(node.object);
        }
        else {
            stack.push_back(node.left);
            stack.push_back(node.right);
        }
    }
    return hits;
}

void DynamicBVH::Draw() const {
    if (root == BVH_NULL_NODE)
        return;

    struct StackItem { int node; int depth; };
    std::vector<StackItem> stack;
    stack.reserve(64);
    stack.push_back({ root, 0 });

    while (!stack.empty()) {
        const StackItem current = stack.back();
        stack.pop_back();

        const DynamicBVHNode& node = nodes[current.node];
        const unsigned char brightness =
            (unsigned char)std::max(40, 255 - (current.depth * 25));

        const Color boxColor = node.IsLeaf()
            ? Color{ brightness, 0, 0, 255 }
        : Color{ 0, brightness, 0, 255 };

        DrawBoundingBox(node.bounds, boxColor);

        if (!node.IsLeaf()) {
            stack.push_back({ node.right, current.depth + 1 });
            stack.push_back({ node.left, current.depth + 1 });
        }
    }
}

std::vector<PhysicsObject*> DynamicBVH::Raycast(Ray ray) const {
    std::vector<PhysicsObject*> hits;
    RaycastRecursive(root, ray, hits);
    return hits;
}

void DynamicBVH::RaycastRecursive(int nodeIndex, Ray ray, std::vector<PhysicsObject*>& hits) const {
    if (nodeIndex == BVH_NULL_NODE)
        return;

    const DynamicBVHNode& node = nodes[nodeIndex];
    RayCollision boxCollision = GetRayCollisionBox(ray, node.bounds);

    if (!boxCollision.hit) {
        return;
    }

    if (node.IsLeaf()) {
        hits.push_back(node.object);
    }
    else {
        RaycastRecursive(node.left, ray, hits);
        RaycastRecursive(node.right, ray, hits);
    }
}
