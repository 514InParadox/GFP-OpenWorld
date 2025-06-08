#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <cassert>
#include "bone.hpp"
#include "animation_data.hpp"
#include "utils/assimp_glm_helpers.hpp"

// Forward declaration
class AnimatedModel;

class Animation {
public:
    Animation() = default;
    Animation(const std::string& animationPath, AnimatedModel* model);
    Animation(const std::string& animationPath, AnimatedModel* model, int animationIndex);
    ~Animation() = default;

    Bone* FindBone(const std::string& name);    inline float GetTicksPerSecond() { return m_TicksPerSecond; }
    inline float GetDuration() { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
    inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }
    inline const std::string& GetName() const { return m_Name; }
    inline const glm::mat4& GetGlobalInverseTransform() const { return m_GlobalInverseTransform; }

    // Static method to get animation count from file
    static int GetAnimationCount(const std::string& animationPath);

private:
    void ReadMissingBones(const aiAnimation* animation, AnimatedModel& model);
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);    std::string m_Name;
    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::map<std::string, BoneInfo> m_BoneInfoMap;
    glm::mat4 m_GlobalInverseTransform;
};

// 0 <= stamp <= 1
// return 
class Movement {
public:
    virtual glm::mat4 getTransform(float stamp) = 0;
};

class LinearMovement : public Movement {
public:
    LinearMovement(glm::mat4 startTrans, glm::mat4 endTrans) :
        startTrans_(startTrans), endTrans_(endTrans) {}
    
    glm::mat4 getTransform(float stamp) override;
private:
    glm::mat4 startTrans_, endTrans_;
};