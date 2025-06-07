#pragma once

#include <vector>
#include <string>
#include <assimp/anim.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "utils/assimp_glm_helpers.hpp"
#include <glm/gtx/quaternion.hpp>

struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

class Bone {
private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

public:
    // Reads keyframes from aiNodeAnim
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    // Interpolates between positions, rotations & scaling keys based on the current time 
    // of the animation and prepares the local transformation matrix by combining all keys transformations
    void Update(float animationTime);

    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }

    // Gets the current index on mKeyPositions to interpolate to based on the current animation time
    int GetPositionIndex(float animationTime);
    // Gets the current index on mKeyRotations to interpolate to based on the current animation time
    int GetRotationIndex(float animationTime);
    // Gets the current index on mKeyScalings to interpolate to based on the current animation time
    int GetScaleIndex(float animationTime);

    // 新增：交互式骨骼控制
    void SetPosition(const glm::vec3& position) { m_LocalTransform[3] = glm::vec4(position, 1.0f); }
    void SetRotation(const glm::quat& rotation) { 
        glm::mat4 rotMat = glm::toMat4(rotation);
        // 保持原有的位置和缩放
        glm::vec3 pos = glm::vec3(m_LocalTransform[3]);
        glm::vec3 scale = glm::vec3(glm::length(m_LocalTransform[0]), glm::length(m_LocalTransform[1]), glm::length(m_LocalTransform[2]));
        m_LocalTransform = glm::translate(glm::mat4(1.0f), pos) * rotMat * glm::scale(glm::mat4(1.0f), scale);
    }
    void SetScale(const glm::vec3& scale) {
        // 保持原有的位置和旋转
        glm::vec3 pos = glm::vec3(m_LocalTransform[3]);
        glm::mat3 rotScale = glm::mat3(m_LocalTransform);
        glm::mat3 rot = glm::mat3(
            glm::normalize(rotScale[0]),
            glm::normalize(rotScale[1]),
            glm::normalize(rotScale[2])
        );
        m_LocalTransform = glm::translate(glm::mat4(1.0f), pos) * glm::mat4(rot) * glm::scale(glm::mat4(1.0f), scale);
    }

private:
    // Gets normalized value for Lerp & Slerp
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    // Figures out which position keys to interpolate b/w and performs the interpolation 
    // and returns the translation matrix
    glm::mat4 InterpolatePosition(float animationTime);

    // Figures out which rotations keys to interpolate b/w and performs the interpolation 
    // and returns the rotation matrix
    glm::mat4 InterpolateRotation(float animationTime);

    // Figures out which scaling keys to interpolate b/w and performs the interpolation 
    // and returns the scale matrix
    glm::mat4 InterpolateScaling(float animationTime);
};
