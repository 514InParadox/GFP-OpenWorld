#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <optional>
#include <iostream>

#include "utils/collision_system.hpp"
#include "utils/bvh.hpp"
#include "physics/contact.hpp"
#include "utils/physics.hpp"
#include "model/model.hpp"
#include "utils/transform.hpp"

/* =========================================================================
   内部工具：AABB / Sphere 基础几何
   =========================================================================*/
namespace {

/* --- BoundingBox 中心点 ------------------------------------------------ */
inline glm::vec3 center(const BoundingBox& bb)
{
    return 0.5f * (bb.min + bb.max);
}

/* --- AABB ∩ AABB ------------------------------------------------------- */
bool aabbIntersect(const BoundingBox& a, const BoundingBox& b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
           (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
           (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

/* --- 离散 Sphere ∩ AABB ------------------------------------------------ */
bool sphereAabbIntersect(const BoundingSphere& s, const BoundingBox& b,
                         glm::vec3& normal, float& penetration)
{
    glm::vec3 closest = glm::clamp(s.center, b.min, b.max);
    glm::vec3 diff    = closest - s.center;
    float     dist2   = glm::length2(diff);
    if (dist2 > s.radius * s.radius) return false;

    float dist = std::sqrt(dist2);
    normal      = dist > 0.f ? -diff / dist : glm::vec3(1,0,0);
    penetration = s.radius - dist;
    return true;
}

} // namespace
/* ====================================================================== */

/* ---------------- 单例 ---------------- */
CollisionSystem& CollisionSystem::instance()
{
    static CollisionSystem inst;
    return inst;
}

/* ---------------- 注册 / 清空 ---------------- */
void CollisionSystem::registerObject(Model* m, Physics* p, ColliderShape s)
{
    _objects.push_back({m, p, s});
}
void CollisionSystem::clear()
{
    _objects.clear();
    _manifolds.clear();
}

/* ======================================================================
                               主更新流程
   ======================================================================*/
void CollisionSystem::update(float dt)
{
    _manifolds.clear();

    /* 1) 广相 BVH ------------------------------------------------------- */
    buildBVH();
    auto pairs = gatherCandidatePairs();

    /* 2) 窄相：生成接触流形 ------------------------------------------- */
    for (auto [ia, ib] : pairs)
        if (auto m = narrowPhase(ia, ib))
            _manifolds.push_back(*m);

    /* 3) 求解：线 + 角冲量 -------------------------------------------- */
    for (ContactManifold& m : _manifolds)
    {
        Physics* pa = _objects[m.idxA].physics;
        Physics* pb = _objects[m.idxB].physics;
        if (!pa || !pb) continue;

        float eRest = std::min(pa->getRestitution(), pb->getRestitution());

        /* 每个接触点单独求冲量 */
        for (ContactPoint& cp : m.points)
        {
            glm::vec3 n = cp.normal;

            glm::vec3 ca = _objects[m.idxA].model->transform.position;
            glm::vec3 cb = _objects[m.idxB].model->transform.position;
            glm::vec3 ra = cp.position - ca;
            glm::vec3 rb = cp.position - cb;

            glm::vec3 va = pa->getVelocity() + glm::cross(pa->getAngVel(), ra);
            glm::vec3 vb = pb->getVelocity() + glm::cross(pb->getAngVel(), rb);
            glm::vec3 dv = va - vb;
            float vRel   = glm::dot(dv, n);
            if (vRel > 0.f) continue;               // 已分离

            glm::vec3 raXn = glm::cross(ra, n);
            glm::vec3 rbXn = glm::cross(rb, n);

            float denom =
                pa->invMass() + pb->invMass() +
                glm::dot(raXn, pa->invInertiaWorld() * raXn) +
                glm::dot(rbXn, pb->invInertiaWorld() * rbXn);

            if (denom < 1e-6f) continue;

            float j = -(1.f + eRest) * vRel / denom;
            glm::vec3 impulse = j * n;

            pa->applyImpulseAtPoint( impulse, cp.position );
            pb->applyImpulseAtPoint(-impulse, cp.position );
        }
    }

    /* 4) 整帧积分位置/旋转 ------------------------------------------- */
    for (auto& o : _objects)
        if (o.physics && !o.physics->isStatic())
            o.physics->integrate(dt);
}

/* ======================================================================
                               Broad Phase
   ======================================================================*/
void CollisionSystem::buildBVH()
{
    _cachedAabbs.clear();
    _cachedAabbs.reserve(_objects.size());
    for (auto& o : _objects) _cachedAabbs.push_back(getWorldAabb(o));
    _bvh = BVHTree(_cachedAabbs);
}
std::vector<std::pair<int,int>> CollisionSystem::gatherCandidatePairs()
{
    return _bvh.queryPairs();
}

/* ======================================================================
                               Narrow Phase
   ======================================================================*/
std::optional<ContactManifold>
CollisionSystem::narrowPhase(int ia, int ib)
{
    const CollisionObject& A = _objects[ia];
    const CollisionObject& B = _objects[ib];
    ContactManifold mani; mani.idxA = ia; mani.idxB = ib;

    /* --- Sphere–Sphere ------------------------------------------------ */
    if (A.shape==ColliderShape::Sphere && B.shape==ColliderShape::Sphere)
    {
        auto sa = getWorldSphere(A);
        auto sb = getWorldSphere(B);
        glm::vec3 diff = sb.center - sa.center;
        float dist2 = glm::length2(diff);
        float rSum  = sa.radius + sb.radius;
        if (dist2 > rSum*rSum) return std::nullopt;

        float dist = std::sqrt(dist2);
        glm::vec3 n = dist>1e-6f ? diff/dist : glm::vec3(1,0,0);
        float pen = rSum - dist;

        mani.points.push_back({
            sa.center + n * (sa.radius - pen*0.5f),  // pos
            n, pen
        });
        return mani;
    }

    /* --- Sphere–AABB -------------------------------------------------- */
    if (A.shape==ColliderShape::Sphere && B.shape==ColliderShape::AABB)
    {
        auto sa = getWorldSphere(A);
        auto bb = getWorldAabb  (B);
        glm::vec3 n; float pen;
        if (!sphereAabbIntersect(sa, bb, n, pen)) return std::nullopt;

        mani.points.push_back({
            sa.center + n * (sa.radius - pen*0.5f),
            n, pen
        });
        return mani;
    }
    if (A.shape==ColliderShape::AABB && B.shape==ColliderShape::Sphere)
    {
        if (auto rev = narrowPhase(ib, ia))
        {
            for (auto& p : rev->points) p.normal = -p.normal;
            return rev;
        }
        return std::nullopt;
    }

    /* --- AABB–AABB (离散) -------------------------------------------- */
    auto aa = getWorldAabb(A);
    auto bb = getWorldAabb(B);
    if (!aabbIntersect(aa, bb)) return std::nullopt;

    glm::vec3 overlap(
        std::min(aa.max.x, bb.max.x) - std::max(aa.min.x, bb.min.x),
        std::min(aa.max.y, bb.max.y) - std::max(aa.min.y, bb.min.y),
        std::min(aa.max.z, bb.max.z) - std::max(aa.min.z, bb.min.z)
    );
    int axis = 0;
    if (overlap.y < overlap.x) axis = 1;
    if (overlap.z < overlap[axis]) axis = 2;

    glm::vec3 n(0);
    n[axis] = (center(aa)[axis] < center(bb)[axis]) ? -1.f : 1.f;
    float pen = overlap[axis];

    mani.points.push_back({
        0.5f*(center(aa)+center(bb)), n, pen
    });
    return mani;
}

/* ======================================================================
                        包围体 (对象空间→世界空间)
   ======================================================================*/
BoundingBox CollisionSystem::getWorldAabb(const CollisionObject& o) const
{
    BoundingBox bb = o.model->getBoundingBox();
    bb.transform(o.model->transform.getLocalMatrix());
    return bb;
}
BoundingSphere CollisionSystem::getWorldSphere(const CollisionObject& o) const
{
    BoundingBox local = o.model->getBoundingBox();
    glm::vec3 diag = local.max - local.min;
    float r = 0.5f * glm::length(diag);

    BoundingSphere s;
    s.center = glm::vec3(o.model->transform.getLocalMatrix() *
                         glm::vec4(center(local), 1.f));
    s.radius = r * o.model->transform.scale.x;   // 近似处理等比缩放
    return s;
}

/* ======================================================================
                              结果访问
   ======================================================================*/
const std::vector<ContactManifold>&
CollisionSystem::getManifolds() const { return _manifolds; }
