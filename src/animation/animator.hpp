#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>
#include "animation.hpp"
#include "animation_data.hpp"

class Animator {
public:
    Animator(Animation* animation);

    void UpdateAnimation(float dt);
    void PlayAnimation(Animation* pAnimation);
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
    std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }

    // 交互式骨骼控制
    void SetBoneTransform(const std::string& boneName, const glm::mat4& transform);
    void SetBoneRotation(const std::string& boneName, const glm::quat& rotation);
    void SetBonePosition(const std::string& boneName, const glm::vec3& position);
    void SetBoneScale(const std::string& boneName, const glm::vec3& scale);
    
    // 获取骨骼信息
    glm::mat4 GetBoneTransform(const std::string& boneName) const;
    std::vector<std::string> GetBoneNames() const;

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
    
    // 用于交互式控制的骨骼覆盖
    std::map<std::string, glm::mat4> m_BoneOverrides;
};
