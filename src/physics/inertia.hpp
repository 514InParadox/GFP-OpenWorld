#ifndef _PHYSICS_INERTIA_H
#define _PHYSICS_INERTIA_H

#include <glm/glm.hpp>

/* -- 体心坐标系下的惯性张量(对角) ------------------------- */
inline glm::mat3 boxInertia(float m, const glm::vec3& halfExt) {
    float x2 = 4.f * halfExt.x * halfExt.x;
    float y2 = 4.f * halfExt.y * halfExt.y;
    float z2 = 4.f * halfExt.z * halfExt.z;
    return glm::mat3(
        1.f/12.f * m * (y2 + z2), 0, 0,
        0, 1.f/12.f * m * (x2 + z2), 0,
        0, 0, 1.f/12.f * m * (x2 + y2)
    );
}

inline glm::mat3 sphereInertia(float m, float r) {
    float diag = 2.f/5.f * m * r * r;
    return glm::mat3(diag);
}

#endif
