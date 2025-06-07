#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct BoneInfo {
    // id is index in finalBoneMatrices
    int id;
    // offset matrix transforms vertex from model space to bone space
    glm::mat4 offset;
};

struct AssimpNodeData {
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};
