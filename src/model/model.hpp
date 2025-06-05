#pragma once

#include <string>
#include <vector>
#include <memory>

#include "utils/bounding_box.hpp"
#include "utils/gl_utility.hpp"
#include "utils/transform.hpp"
#include "utils/vertex.hpp"
#include "utils/physics.hpp"
#include "utils/texture2d.hpp"
// 前向声明避免循环引用
class Physics;

class Model {
public:
    Model(const std::string& filepath); // TODO: 这里从文件导入模型要自己实现

    Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    Model(Model&& rhs) noexcept;

    virtual ~Model();

    GLuint getVao() const;

    GLuint getBoundingBoxVao() const;

    size_t getVertexCount() const;

    size_t getFaceCount() const;

    BoundingBox getBoundingBox() const;

    virtual void draw() const;

    virtual void drawBoundingBox() const;

    const std::vector<uint32_t>& getIndices() const {
        return _indices;
    }
    const std::vector<Vertex>& getVertices() const {
        return _vertices;
    }
    const Vertex& getVertex(int i) const {
        return _vertices[i];
    }
    
    // 添加物理组件
    void addPhysics();
    
    // 获取物理组件
    Physics* getPhysics() const;
    
    // 移除物理组件
    void removePhysics();
    
    // 检查是否有物理组件
    bool hasPhysics() const;

public:
    Transform transform;

protected:
    // vertices of the table represented in model's own coordinate
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    // bounding box
    BoundingBox _boundingBox;

    // opengl objects
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;

    GLuint _boxVao = 0;
    GLuint _boxVbo = 0;
    GLuint _boxEbo = 0;

    // 物理组件
    std::unique_ptr<Physics> _physics = nullptr;

    void computeBoundingBox();

    void initGLResources();

    void initBoxGLResources();

    void cleanup();
};

class TexModel : public Model {
public:
    TexModel(const std::string& filepath);

    TexModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    std::shared_ptr<Texture2D> mapKd;
};