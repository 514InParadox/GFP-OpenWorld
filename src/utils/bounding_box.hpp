#ifndef _UTILS_BOUNDING_BOX_H
#define _UTILS_BOUNDING_BOX_H

#include <glm/glm.hpp>
#include <limits>

struct BoundingBox {
    glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 max = -glm::vec3(std::numeric_limits<float>::max());

    BoundingBox& operator+=(const BoundingBox& rhs) {
        min = glm::min(min, rhs.min);
        max = glm::max(max, rhs.max);

        return *this;
    }
};

#endif