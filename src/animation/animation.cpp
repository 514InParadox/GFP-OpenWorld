#include "animation.hpp"
#include "animated_model.hpp"
#include <iostream>

Animation::Animation(const std::string& animationPath, AnimatedModel* model) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    auto rootNode = scene->mRootNode;
    auto animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    m_Name = animation->mName.C_Str();
    
    // Store the global inverse transformation matrix
    m_GlobalInverseTransform = glm::inverse(AssimpGLMHelpers::ConvertMatrixToGLMFormat(rootNode->mTransformation));
      // Debug: Print root transformation and global inverse
    // std::cout << "\n=== Animation Loading Debug ===" << std::endl;
    // std::cout << "Animation Name: " << m_Name << std::endl;
    // glm::mat4 rootTransform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(rootNode->mTransformation);
    // std::cout << "Root Transform Matrix:" << std::endl;
    // for (int i = 0; i < 4; i++) {
    //     std::cout << "[" << rootTransform[i][0] << ", " << rootTransform[i][1] << ", " << rootTransform[i][2] << ", " << rootTransform[i][3] << "]" << std::endl;
    // }
    // std::cout << "Global Inverse Transform Matrix:" << std::endl;
    // for (int i = 0; i < 4; i++) {
    //     std::cout << "[" << m_GlobalInverseTransform[i][0] << ", " << m_GlobalInverseTransform[i][1] << ", " << m_GlobalInverseTransform[i][2] << ", " << m_GlobalInverseTransform[i][3] << "]" << std::endl;
    // }
    // std::cout << "==============================\n" << std::endl;
    
    ReadHierarchyData(m_RootNode, rootNode);
    ReadMissingBones(animation, *model);
}

Animation::Animation(const std::string& animationPath, AnimatedModel* model, int animationIndex) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);
    assert(animationIndex >= 0 && animationIndex < scene->mNumAnimations);

    auto rootNode = scene->mRootNode;
    auto animation = scene->mAnimations[animationIndex];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    m_Name = animation->mName.C_Str();
    
    // Store the global inverse transformation matrix
    m_GlobalInverseTransform = glm::inverse(AssimpGLMHelpers::ConvertMatrixToGLMFormat(rootNode->mTransformation));
    
    // Debug: Print root transformation and global inverse for indexed animation
    std::cout << "\n=== Animation Loading Debug (Index " << animationIndex << ") ===" << std::endl;
    std::cout << "Animation Name: " << m_Name << std::endl;
    glm::mat4 rootTransform = AssimpGLMHelpers::ConvertMatrixToGLMFormat(rootNode->mTransformation);
    std::cout << "Root Transform Matrix:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "[" << rootTransform[i][0] << ", " << rootTransform[i][1] << ", " << rootTransform[i][2] << ", " << rootTransform[i][3] << "]" << std::endl;
    }
    std::cout << "Global Inverse Transform Matrix:" << std::endl;
    for (int i = 0; i < 4; i++) {
        std::cout << "[" << m_GlobalInverseTransform[i][0] << ", " << m_GlobalInverseTransform[i][1] << ", " << m_GlobalInverseTransform[i][2] << ", " << m_GlobalInverseTransform[i][3] << "]" << std::endl;
    }
    std::cout << "==============================\n" << std::endl;
    
    ReadHierarchyData(m_RootNode, rootNode);
    ReadMissingBones(animation, *model);
}

Bone* Animation::FindBone(const std::string& name) {
    auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
        [&](const Bone& Bone)
        {
            return Bone.GetBoneName() == name;
        }
    );
    if (iter == m_Bones.end()) return nullptr;
    else return &(*iter);
}

void Animation::ReadMissingBones(const aiAnimation* animation, AnimatedModel& model) {
    int size = animation->mNumChannels;

    auto& boneInfoMap = model.GetBoneInfoMap();
    int& boneCount = model.GetBoneCount();

    // reading channels(bones engaged in an animation and their keyframes)
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        m_Bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src) {
    assert(src);

    dest.name = src->mName.data;
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

glm::mat4 LinearMovement::getTransform(float stamp) {
    return stamp * endTrans_ + (1 - stamp) * startTrans_;
}

int Animation::GetAnimationCount(const std::string& animationPath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    if (!scene) {
        return 0;
    }
    return scene->mNumAnimations;
}