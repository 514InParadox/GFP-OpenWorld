#pragma once

#include <glm/glm.hpp>

#define MAX_BONE_INFLUENCE 4

struct AnimatedVertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;

    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];

    AnimatedVertex() {
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            m_BoneIDs[i] = -1;
            m_Weights[i] = 0.0f;
        }
    }
};
