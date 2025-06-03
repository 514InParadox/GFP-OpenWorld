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
    
    // Check if bounding box is empty (has no valid min/max)
    bool isEmpty() const {
        return min.x > max.x || min.y > max.y || min.z > max.z;
    }
    
    // Transform bounding box by a 4x4 matrix
    void transform(const glm::mat4& matrix) {
        // If box is empty, nothing to transform
        if (isEmpty()) {
            return;
        }
        
        // Create 8 corner points of the box
        glm::vec4 corners[8];
        corners[0] = glm::vec4(min.x, min.y, min.z, 1.0f);
        corners[1] = glm::vec4(max.x, min.y, min.z, 1.0f);
        corners[2] = glm::vec4(min.x, max.y, min.z, 1.0f);
        corners[3] = glm::vec4(max.x, max.y, min.z, 1.0f);
        corners[4] = glm::vec4(min.x, min.y, max.z, 1.0f);
        corners[5] = glm::vec4(max.x, min.y, max.z, 1.0f);
        corners[6] = glm::vec4(min.x, max.y, max.z, 1.0f);
        corners[7] = glm::vec4(max.x, max.y, max.z, 1.0f);
        
        // Transform each corner and compute new AABB
        glm::vec3 newMin(std::numeric_limits<float>::max());
        glm::vec3 newMax(-std::numeric_limits<float>::max());
        
        for (int i = 0; i < 8; ++i) {
            glm::vec4 transformedCorner = matrix * corners[i];
            // Divide by w for perspective transformation
            if (transformedCorner.w != 0.0f) {
                transformedCorner /= transformedCorner.w;
            }
            
            newMin = glm::min(newMin, glm::vec3(transformedCorner));
            newMax = glm::max(newMax, glm::vec3(transformedCorner));
        }
        
        min = newMin;
        max = newMax;
    }

    inline glm::vec3 center() const { return 0.5f * (min + max); }

};

#endif