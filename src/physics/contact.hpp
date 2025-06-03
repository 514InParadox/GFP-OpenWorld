#ifndef _PHYSICS_CONTACT_H
#define _PHYSICS_CONTACT_H

#include <glm/glm.hpp>
#include <vector>

struct ContactPoint {
    glm::vec3 position;     ///< 世界坐标
    glm::vec3 normal;       ///< 从 B 指向 A
    float     penetration;  ///< 穿透深度
};

struct ContactManifold {
    /* 参与碰撞的两个物体在 CollisionSystem::_objects 中的索引 */
    int       idxA = -1;
    int       idxB = -1;
    std::vector<ContactPoint> points;
};

#endif
