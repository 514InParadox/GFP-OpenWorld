#pragma once

#include <string>
#include <vector>
#include <memory>

#include <assimp/scene.h>

#include "utils/bounding_box.hpp"
#include "utils/gl_utility.hpp"
#include "utils/transform.hpp"
#include "utils/vertex.hpp"
#include "utils/physics.hpp"

#include "mesh.hpp"
// 前向声明避免循环引用
class Physics;

class AdvancedModel {
public:
    AdvancedModel(const std::string& filepath); // TODO: 这里从文件导入模型要自己实现

    // AdvancedModel(AdvancedModel&& rhs) noexcept;

    virtual ~AdvancedModel();

    // GLuint getBoundingBoxVao() const;

    BoundingBox getBoundingBox() const;

    // virtual void drawBoundingBox() const;
    
    virtual void draw() const;

    const std::vector<Mesh>& getMeshes() const {
        return _meshes;
    }
    const Mesh& getMesh(int i) const {
        return _meshes[i];
    }
    

    // --------- Physics ---------
    // 添加物理组件
    void addPhysics();
    
    // 获取物理组件
    Physics* getPhysics() const;
    
    // 移除物理组件
    void removePhysics();
    
    // 检查是否有物理组件
    bool hasPhysics() const;
    // --------- Physics ---------

    void loadModel(const std::string &path);

    void processNode(aiNode *node, const aiScene *scene);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<AdvancedTexture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, 
                                            std::string typeName);

public:
    Transform transform;

protected:
    // vertices of the table represented in model's own coordinate
    // std::vector<Vertex> _vertices;
    // std::vector<uint32_t> _indices;
    std::vector<Mesh> _meshes;

    std::string directory;

    // 物理组件
    std::unique_ptr<Physics> _physics = nullptr;

    void computeBoundingBox();

    BoundingBox _boundingBox;

    // void initGLResources();

    // void initBoxGLResources();

    // void cleanup();
};