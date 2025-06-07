#include "animator.hpp"
#include <iostream>
#include <glm/gtx/quaternion.hpp>

Animator::Animator(Animation* animation) {
    m_CurrentTime = 0.0;
    m_CurrentAnimation = animation;

    m_FinalBoneMatrices.reserve(100);

    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::UpdateAnimation(float dt) {
    m_DeltaTime = dt;
    if (m_CurrentAnimation) {
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
    }
}

void Animator::PlayAnimation(Animation* pAnimation) {
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

    if (Bone) {
        Bone->Update(m_CurrentTime);
        nodeTransform = Bone->GetLocalTransform();
    }

    // 检查是否有骨骼覆盖
    if (m_BoneOverrides.find(nodeName) != m_BoneOverrides.end()) {
        nodeTransform = m_BoneOverrides[nodeName];
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}

void Animator::SetBoneTransform(const std::string& boneName, const glm::mat4& transform) {
    m_BoneOverrides[boneName] = transform;
}

void Animator::SetBoneRotation(const std::string& boneName, const glm::quat& rotation) {
    glm::mat4 rotMat = glm::toMat4(rotation);
    // 如果已经有覆盖，保持其他变换
    if (m_BoneOverrides.find(boneName) != m_BoneOverrides.end()) {
        glm::mat4& current = m_BoneOverrides[boneName];
        glm::vec3 pos = glm::vec3(current[3]);
        glm::vec3 scale = glm::vec3(glm::length(current[0]), glm::length(current[1]), glm::length(current[2]));
        m_BoneOverrides[boneName] = glm::translate(glm::mat4(1.0f), pos) * rotMat * glm::scale(glm::mat4(1.0f), scale);
    } else {
        m_BoneOverrides[boneName] = rotMat;
    }
}

void Animator::SetBonePosition(const std::string& boneName, const glm::vec3& position) {
    if (m_BoneOverrides.find(boneName) != m_BoneOverrides.end()) {
        m_BoneOverrides[boneName][3] = glm::vec4(position, 1.0f);
    } else {
        m_BoneOverrides[boneName] = glm::translate(glm::mat4(1.0f), position);
    }
}

void Animator::SetBoneScale(const std::string& boneName, const glm::vec3& scale) {
    if (m_BoneOverrides.find(boneName) != m_BoneOverrides.end()) {
        glm::mat4& current = m_BoneOverrides[boneName];
        glm::vec3 pos = glm::vec3(current[3]);
        glm::mat3 rotScale = glm::mat3(current);
        glm::mat3 rot = glm::mat3(
            glm::normalize(rotScale[0]),
            glm::normalize(rotScale[1]),
            glm::normalize(rotScale[2])
        );
        m_BoneOverrides[boneName] = glm::translate(glm::mat4(1.0f), pos) * glm::mat4(rot) * glm::scale(glm::mat4(1.0f), scale);
    } else {
        m_BoneOverrides[boneName] = glm::scale(glm::mat4(1.0f), scale);
    }
}

glm::mat4 Animator::GetBoneTransform(const std::string& boneName) const {
    auto it = m_BoneOverrides.find(boneName);
    if (it != m_BoneOverrides.end()) {
        return it->second;
    }
    return glm::mat4(1.0f);
}

std::vector<std::string> Animator::GetBoneNames() const {
    std::vector<std::string> names;
    if (m_CurrentAnimation) {
        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        for (const auto& pair : boneInfoMap) {
            names.push_back(pair.first);
        }
    }
    return names;
}
