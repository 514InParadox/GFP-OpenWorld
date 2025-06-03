#ifndef _UTILS_BOUNDING_SPHERE_H
#define _UTILS_BOUNDING_SPHERE_H

#include <glm/glm.hpp>

// @brief 世界空间下的包围球
struct BoundingSphere{
  glm::vec3 center {0.0f};
  float     radius {0.0f};
};

#endif