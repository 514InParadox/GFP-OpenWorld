#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <optional>
#include <vector>
#include "physics/contact.hpp"
#include "utils/bounding_box.hpp"
#include "utils/bounding_sphere.hpp"
#include "utils/bvh.hpp"

class Model;
class Physics;

/* ---------- 基础对象 ---------- */
enum class ColliderShape { AABB, Sphere };
struct CollisionObject {
    Model*        model   = nullptr;
    Physics*      physics = nullptr;
    ColliderShape shape   = ColliderShape::AABB;
};

/* ---------- 主系统 ---------- */
class CollisionSystem {
public:
    static CollisionSystem& instance();

    /* 注册 / 场景切换 */
    void registerObject(Model* m, Physics* p,
                        ColliderShape shape = ColliderShape::AABB);
    void clear();

    /* 每帧调用 (在 render 前) */
    void update(float dt);

    /* 调试访问 */
    const std::vector<ContactManifold>& getManifolds() const;

private:
    CollisionSystem()  = default;
    ~CollisionSystem() = default;
    CollisionSystem(const CollisionSystem&)            = delete;
    CollisionSystem& operator=(const CollisionSystem&) = delete;

    /* ---- Broad Phase ---- */
    void buildBVH();
    std::vector<std::pair<int,int>> gatherCandidatePairs();

    /* ---- Narrow Phase ---- */
    std::optional<ContactManifold> narrowPhase(int idxA, int idxB);

    /* ---- 工具 ---- */
    BoundingBox    getWorldAabb  (const CollisionObject& o) const;
    BoundingSphere getWorldSphere(const CollisionObject& o) const;

private:
    std::vector<CollisionObject>   _objects;
    std::vector<BoundingBox>       _cachedAabbs;
    BVHTree                        _bvh   = BVHTree(_cachedAabbs);
    std::vector<ContactManifold>   _manifolds;
};

