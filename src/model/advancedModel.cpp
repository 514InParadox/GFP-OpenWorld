// #include <algorithm>
// #include <iostream>
// #include <limits>
// #include <unordered_map>
// #include <fstream>
// #include <sstream>

// #include <tinyobjloader/tiny_obj_loader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb/stb_image.h>

#include "model/advancedModel.hpp"
#include "utils/physics.hpp"

unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false) {
    std::string filename = std::string(path);
    // filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    std::cout << "filename: " << filename << std::endl;
    
    // Ensure consistent flip behavior for all textures
    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// 这是 .obj 文件顶点、法向、面的导入方式，用以对应文档中了解 .obj 文件格式的部分。后续使用 Assimp 库支持需要使用的高级模型的导入。
AdvancedModel::AdvancedModel(const std::string& filepath) {
    loadModel(filepath);
}


AdvancedModel::~AdvancedModel() {
}

void AdvancedModel::draw() const {
    for (int i = 0; i < _meshes.size(); ++i) {
        _meshes[i].draw();
    }
}

void AdvancedModel::loadModel(const std::string &path) {
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_SplitLargeMeshes);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    std::cout << path << std::endl;
    auto i = path.find_last_of('/');
    auto a = path.substr(0, i);
    directory = a;

    processNode(scene->mRootNode, scene);
}

void AdvancedModel::processNode(aiNode *node, const aiScene *scene) {
    // 处理节点所有的网格（如果有的话）
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        _meshes.push_back(processMesh(mesh, scene));
    }
    // 接下来对它的子节点重复这一过程
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh AdvancedModel::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<AdvancedTexture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        // 处理顶点位置、法线和纹理坐标
        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        if (mesh->mTextureCoords[0])
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        else 
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        vertices.push_back(vertex);
    }
    // 处理索引
    for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }    // 处理材质
    std::cout << "material num: " << mesh->mMaterialIndex << std::endl;
    if(mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<AdvancedTexture> diffuseMaps = loadMaterialTextures(material, 
                                            aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<AdvancedTexture> specularMaps = loadMaterialTextures(material, 
                                            aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<AdvancedTexture> normalMaps = loadMaterialTextures(material, 
                                            aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    return Mesh(vertices, indices, textures);
}

std::vector<AdvancedTexture> AdvancedModel::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
    std::vector<AdvancedTexture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        AdvancedTexture texture;
        texture.id = TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        // texture.path = str;
        textures.push_back(texture);
    }
    return textures;
}

// void AdvancedModel::drawBoundingBox() const {
//     glBindVertexArray(_boxVao);
//     glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
//     glBindVertexArray(0);
// }

// GLuint AdvancedModel::getBoundingBoxVao() const {
//     return _boxVao;
// }

BoundingBox AdvancedModel::getBoundingBox() const {
    return _boundingBox;
}

void AdvancedModel::computeBoundingBox() {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (const auto& m: _meshes) {
        for (const auto& v : m.vertices) {
            minX = std::min(v.position.x, minX);
            minY = std::min(v.position.y, minY);
            minZ = std::min(v.position.z, minZ);
            maxX = std::max(v.position.x, maxX);
            maxY = std::max(v.position.y, maxY);
            maxZ = std::max(v.position.z, maxZ);
        }
    }

    _boundingBox.min = glm::vec3(minX, minY, minZ);
    _boundingBox.max = glm::vec3(maxX, maxY, maxZ);
}

// void AdvancedModel::initBoxGLResources() {
//     std::vector<glm::vec3> boxVertices = {
//         glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.min.z),
//         glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.min.z),
//         glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.min.z),
//         glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.min.z),
//         glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.max.z),
//         glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.max.z),
//         glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.max.z),
//         glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.max.z),
//     };

//     std::vector<uint32_t> boxIndices = {0, 1, 0, 2, 0, 4, 3, 1, 3, 2, 3, 7,
//                                         5, 4, 5, 1, 5, 7, 6, 4, 6, 7, 6, 2};

//     glGenVertexArrays(1, &_boxVao);
//     glGenBuffers(1, &_boxVbo);
//     glGenBuffers(1, &_boxEbo);

//     glBindVertexArray(_boxVao);
//     glBindBuffer(GL_ARRAY_BUFFER, _boxVbo);
//     glBufferData(
//         GL_ARRAY_BUFFER, boxVertices.size() * sizeof(glm::vec3), boxVertices.data(),
//         GL_STATIC_DRAW);

//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boxEbo);
//     glBufferData(
//         GL_ELEMENT_ARRAY_BUFFER, boxIndices.size() * sizeof(uint32_t), boxIndices.data(),
//         GL_STATIC_DRAW);

//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
//     glEnableVertexAttribArray(0);

//     glBindVertexArray(0);
// }

// void AdvancedModel::cleanup() {
//     if (_boxEbo) {
//         glDeleteBuffers(1, &_boxEbo);
//         _boxEbo = 0;
//     }

//     if (_boxVbo) {
//         glDeleteBuffers(1, &_boxVbo);
//         _boxVbo = 0;
//     }

//     if (_boxVao) {
//         glDeleteVertexArrays(1, &_boxVao);
//         _boxVao = 0;
//     }

//     if (_ebo != 0) {
//         glDeleteBuffers(1, &_ebo);
//         _ebo = 0;
//     }

//     if (_vbo != 0) {
//         glDeleteBuffers(1, &_vbo);
//         _vbo = 0;
//     }

//     if (_vao != 0) {
//         glDeleteVertexArrays(1, &_vao);
//         _vao = 0;
//     }
// }

// 添加物理组件
// void AdvancedModel::addPhysics() {
//     if (!_physics) {
//         _physics = std::make_unique<Physics>();
//         _physics->setAdvancedModel(this);
//     }
// }

// // 获取物理组件
// Physics* AdvancedModel::getPhysics() const {
//     return _physics.get();
// }

// // 移除物理组件
// void AdvancedModel::removePhysics() {
//     _physics.reset();
// }

// // 检查是否有物理组件
// bool AdvancedModel::hasPhysics() const {
//     return _physics != nullptr;
// }