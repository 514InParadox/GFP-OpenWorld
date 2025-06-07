#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

#include "animated_mesh.hpp"
#include "animation_data.hpp"
#include "utils/assimp_glm_helpers.hpp"
#include "utils/glsl_program.hpp"
#include "utils/transform.hpp"
#include "utils/texture2d.hpp"

class AnimatedModel {
public:
    AnimatedModel(const std::string& path, bool gamma = false);
    ~AnimatedModel();

    void Draw(GLSLProgram& shader);
    
    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

public:
    Transform transform;

private:
    // model data
    std::vector<std::unique_ptr<AnimatedMesh>> meshes;
    std::string directory;
    bool gammaCorrection;

    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    std::unique_ptr<AnimatedMesh> processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<std::shared_ptr<Texture2D>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, 
                                                                  std::string typeName);
    void SetVertexBoneDataToDefault(AnimatedVertex& vertex);
    void SetVertexBoneData(AnimatedVertex& vertex, int boneID, float weight);
    void ExtractBoneWeightForVertices(std::vector<AnimatedVertex>& vertices, aiMesh* mesh, const aiScene* scene);
};
