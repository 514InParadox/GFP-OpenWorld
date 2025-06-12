#include "animated_model.hpp"
#include <iostream>
#include <fstream>
#include <limits>

AnimatedModel::AnimatedModel(const std::string& path, bool gamma) : gammaCorrection(gamma) {
    loadModel(path);
}

AnimatedModel::~AnimatedModel() {
    // Cleanup is handled by unique_ptr automatically
}

void AnimatedModel::Draw(GLSLProgram& shader) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i]->Draw(shader);
}

BoundingBox AnimatedModel::getBoundingBox() const {
    return _boundingBox;
}

void AnimatedModel::loadModel(const std::string& path) {    Assimp::Importer importer;
    // Remove aiProcess_FlipUVs to match Blender's UV coordinates
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | 
                                                   aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices |
                                                   aiProcess_ValidateDataStructure);    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        // std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }    directory = path.substr(0, path.find_last_of('/'));
    // std::cout << "Loading model from: " << path << std::endl;
    // std::cout << "Directory: " << directory << std::endl;

    processNode(scene->mRootNode, scene);
    
    // Compute bounding box after loading all meshes
    computeBoundingBox();
}

void AnimatedModel::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

std::unique_ptr<AnimatedMesh> AnimatedModel::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<AnimatedVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture2D>> textures;    // std::cout << "Processing mesh with " << mesh->mNumVertices << " vertices" << std::endl;
    
    // Check if mesh has texture coordinates
    bool hasTexCoords = mesh->HasTextureCoords(0);
    // std::cout << "Mesh has texture coordinates: " << (hasTexCoords ? "YES" : "NO") << std::endl;
    
    // if (hasTexCoords) {
    //     std::cout << "Texture coordinate components: " << mesh->mNumUVComponents[0] << std::endl;
    // }

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        AnimatedVertex vertex;
        SetVertexBoneDataToDefault(vertex);

        vertex.Position = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
        
        if (mesh->mNormals) {
            vertex.Normal = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);
        } else {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f); // Default up normal
        }

        // Handle texture coordinates more carefully
        if (hasTexCoords) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
              // Debug: Print UV coordinates for first few vertices
            // if (i < 5) {
            //     std::cout << "Vertex " << i << " UV: (" << vec.x << ", " << vec.y << ")" << std::endl;
            // }
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // std::cout << "Warning: No texture coordinates for vertex " << i << std::endl;
        }

        // Handle tangents and bitangents if available
        if (mesh->mTangents) {
            vertex.Tangent = AssimpGLMHelpers::GetGLMVec(mesh->mTangents[i]);
        } else {
            vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f); // Default tangent
        }
        
        if (mesh->mBitangents) {
            vertex.Bitangent = AssimpGLMHelpers::GetGLMVec(mesh->mBitangents[i]);
        } else {
            vertex.Bitangent = glm::vec3(0.0f, 0.0f, 1.0f); // Default bitangent
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Load different texture types
    std::vector<std::shared_ptr<Texture2D>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    std::vector<std::shared_ptr<Texture2D>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    std::vector<std::shared_ptr<Texture2D>> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    std::vector<std::shared_ptr<Texture2D>> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    
    // Debug output
    std::cout << "Mesh loaded with " << textures.size() << " textures" << std::endl;
    if (textures.empty()) {
        std::cout << "Warning: No textures found for mesh. Model may appear black without fallback handling." << std::endl;
    }

    ExtractBoneWeightForVertices(vertices, mesh, scene);

    return std::make_unique<AnimatedMesh>(vertices, indices, textures);
}

void AnimatedModel::SetVertexBoneDataToDefault(AnimatedVertex& vertex) {
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void AnimatedModel::SetVertexBoneData(AnimatedVertex& vertex, int boneID, float weight) {
    // Validate bone ID
    if (boneID < 0 || boneID >= MAX_BONES) {  // Use proper MAX_BONES constant
        std::cout << "Warning: Invalid bone ID " << boneID << " (MAX_BONES = " << MAX_BONES << ")" << std::endl;
        return;
    }
    
    // Find an empty slot for the bone data
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_BoneIDs[i] < 0) {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            return;
        }
    }
    
    // If no empty slot found, replace the one with smallest weight
    int minWeightIndex = 0;
    for (int i = 1; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_Weights[i] < vertex.m_Weights[minWeightIndex]) {
            minWeightIndex = i;
        }
    }
    
    // Only replace if the new weight is larger
    if (weight > vertex.m_Weights[minWeightIndex]) {
        vertex.m_Weights[minWeightIndex] = weight;
        vertex.m_BoneIDs[minWeightIndex] = boneID;
    }
}

void AnimatedModel::ExtractBoneWeightForVertices(std::vector<AnimatedVertex>& vertices, aiMesh* mesh, const aiScene* scene) {
    auto& boneInfoMap = m_BoneInfoMap;
    int& boneCount = m_BoneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        } else {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            
            // Proper bounds checking to prevent vector out of range
            if (vertexId >= 0 && vertexId < vertices.size()) {
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            } else {
                std::cout << "Warning: Invalid vertex ID " << vertexId << " for bone " << boneName 
                          << ". Vertices size: " << vertices.size() << std::endl;
            }
        }
    }
    
    // Normalize bone weights for all vertices
    std::cout << "Normalizing bone weights for " << vertices.size() << " vertices..." << std::endl;
    int verticesWithoutBones = 0;
    int verticesWithNormalizedWeights = 0;
    
    for (auto& vertex : vertices) {
        float totalWeight = 0.0f;
        int activeBones = 0;
        
        // Calculate total weight
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            if (vertex.m_BoneIDs[i] >= 0 && vertex.m_Weights[i] > 0.0f) {
                totalWeight += vertex.m_Weights[i];
                activeBones++;
            }
        }
        
        if (activeBones == 0) {
            verticesWithoutBones++;
            // Vertex has no bone influence - this is normal for some models
            continue;
        }
        
        // Normalize weights if total is not 1.0
        if (totalWeight > 0.0f && abs(totalWeight - 1.0f) > 0.001f) {
            for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
                if (vertex.m_BoneIDs[i] >= 0 && vertex.m_Weights[i] > 0.0f) {
                    vertex.m_Weights[i] /= totalWeight;
                }
            }
            verticesWithNormalizedWeights++;
        }
    }
    
    std::cout << "Vertices without bone influence: " << verticesWithoutBones << std::endl;
    std::cout << "Vertices with normalized weights: " << verticesWithNormalizedWeights << std::endl;
}

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

std::vector<std::shared_ptr<Texture2D>> AnimatedModel::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
    std::vector<std::shared_ptr<Texture2D>> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        
        std::string filename = std::string(str.C_Str());
        std::string texturePath = filename;
        
        // Helper function to extract filename from path
        auto extractFilename = [](const std::string& path) -> std::string {
            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                return path.substr(lastSlash + 1);
            }
            return path;
        };
        
        // Extract just the filename without path
        std::string baseFilename = extractFilename(filename);
        
        // List of paths to try in order
        std::vector<std::string> pathsToTry = {
            texturePath,                                    // Original path
            directory + "/" + baseFilename,                 // Model directory + filename
            directory + "\\" + baseFilename,                // Model directory + filename (Windows)
            "resource/texture/" + baseFilename,             // Common texture directory
            "resource/texture/ChibiMita.png",               // Specific fallback for Mita
            baseFilename                                    // Just the filename
        };
        
        bool textureLoaded = false;
        for (const auto& pathToTry : pathsToTry) {
            try {
                auto texture = std::make_shared<ImageTexture2D>(pathToTry);
                textures.push_back(texture);
                std::cout << "Successfully loaded texture: " << pathToTry << std::endl;
                textureLoaded = true;
                break;
            } catch (const std::exception& e) {
                // Continue to next path
                std::cout << "Failed to load texture at path: " << pathToTry << " - " << e.what() << std::endl;
            }
        }
        
        if (!textureLoaded) {
            std::cout << "All texture loading attempts failed for: " << filename << std::endl;
        }
    }
    
    if (textures.empty() && mat->GetTextureCount(type) > 0) {
        std::cout << "Warning: Expected " << mat->GetTextureCount(type) << " textures of type " << typeName 
                  << " but loaded 0" << std::endl;
    }
    return textures;
}

void AnimatedModel::computeBoundingBox() {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh->vertices) {
            minX = std::min(vertex.Position.x, minX);
            minY = std::min(vertex.Position.y, minY);
            minZ = std::min(vertex.Position.z, minZ);
            maxX = std::max(vertex.Position.x, maxX);
            maxY = std::max(vertex.Position.y, maxY);
            maxZ = std::max(vertex.Position.z, maxZ);
        }
    }

    _boundingBox.min = glm::vec3(minX, minY, minZ);
    _boundingBox.max = glm::vec3(maxX, maxY, maxZ);
}
