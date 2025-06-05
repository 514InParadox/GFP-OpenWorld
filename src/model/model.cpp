#include <algorithm>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <fstream>
#include <sstream>

#include <tinyobjloader/tiny_obj_loader.h>

#include "model/model.hpp"
#include "utils/physics.hpp"

TexModel::TexModel(const std::string& filepath) : Model(filepath) {}

TexModel::TexModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : Model(vertices, indices) {}

// 这是 .obj 文件顶点、法向、面的导入方式，用以对应文档中了解 .obj 文件格式的部分。后续使用 Assimp 库支持需要使用的高级模型的导入。
Model::Model(const std::string& filepath) {
    // 仅支持文件中的 v, vn, vt, f 行
    std::vector<glm::vec3> fvertices;
    std::vector<glm::vec3> fnormals;
    std::vector<glm::vec2> ftexCoords;
    std::vector<uint32_t> findices;

    // 将 file 中分 v, vn, vt 存储的顶点以 f 聚合
    std::vector<Vertex> vertices; 
    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    std::vector<uint32_t> indices;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            fvertices.push_back(glm::vec3(x, y, z));
        } else if (type == "vn") {
            float xn, yn, zn;
            iss >> xn >> yn >> zn;
            fnormals.push_back(glm::vec3(xn, yn, zn));
        } else if (type == "vt") {
            float xt, yt;
            iss >> xt >> yt;
            ftexCoords.push_back(glm::vec2(xt, yt));
        } else if (type == "f") {
            std::string vertexStr;
            uint32_t tPoint, lPoint, vcnt = 0;
            while (iss >> vertexStr) {
                std::istringstream viss(vertexStr);
                std::string v, vt, vn;
                
                std::getline(viss, v, '/');
                std::getline(viss, vt, '/');
                std::getline(viss, vn, '/');

                Vertex vert = Vertex(
                    v.empty() ? glm::vec3() : fvertices[std::stoi(v) - 1],
                    vn.empty() ? glm::vec3() : fnormals[std::stoi(vn) - 1],
                    vt.empty() ? glm::vec2() : ftexCoords[std::stoi(vt) - 1]
                );

                if (uniqueVertices.count(vert) == 0) {
                    uniqueVertices[vert] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vert);
                }

                ++vcnt;
                if (vcnt >= 3) {
                    indices.push_back(tPoint);
                    indices.push_back(lPoint);
                    indices.push_back(uniqueVertices[vert]);
                }
                lPoint = uniqueVertices[vert];
                if (vcnt == 1) tPoint = lPoint;
            }
        }
    }

    _vertices = vertices;
    _indices = indices;

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Model::Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    : _vertices(vertices), _indices(indices) {

    computeBoundingBox();

    initGLResources();

    initBoxGLResources();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error: " + std::to_string(error));
    }
}

Model::Model(Model&& rhs) noexcept
    : _vertices(std::move(rhs._vertices)), _indices(std::move(rhs._indices)),
      _boundingBox(std::move(rhs._boundingBox)), _vao(rhs._vao), _vbo(rhs._vbo), _ebo(rhs._ebo),
      _boxVao(rhs._boxVao), _boxVbo(rhs._boxVbo), _boxEbo(rhs._boxEbo) {
    _vao = 0;
    _vbo = 0;
    _ebo = 0;
    _boxVao = 0;
    _boxVbo = 0;
    _boxEbo = 0;
}

Model::~Model() {
    cleanup();
}

BoundingBox Model::getBoundingBox() const {
    return _boundingBox;
}

void Model::draw() const {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Model::drawBoundingBox() const {
    glBindVertexArray(_boxVao);
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

GLuint Model::getVao() const {
    return _vao;
}

GLuint Model::getBoundingBoxVao() const {
    return _boxVao;
}

size_t Model::getVertexCount() const {
    return _vertices.size();
}

size_t Model::getFaceCount() const {
    return _indices.size() / 3;
}

void Model::initGLResources() {
    // create a vertex array object
    glGenVertexArrays(1, &_vao);
    // create a vertex buffer object
    glGenBuffers(1, &_vbo);
    // create a element array buffer
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t), _indices.data(),
        GL_STATIC_DRAW);

    // specify layout, size of a vertex, data type, normalize, sizeof vertex array, offset of the
    // attribute
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Model::computeBoundingBox() {
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for (const auto& v : _vertices) {
        minX = std::min(v.position.x, minX);
        minY = std::min(v.position.y, minY);
        minZ = std::min(v.position.z, minZ);
        maxX = std::max(v.position.x, maxX);
        maxY = std::max(v.position.y, maxY);
        maxZ = std::max(v.position.z, maxZ);
    }

    _boundingBox.min = glm::vec3(minX, minY, minZ);
    _boundingBox.max = glm::vec3(maxX, maxY, maxZ);
}

void Model::initBoxGLResources() {
    std::vector<glm::vec3> boxVertices = {
        glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.min.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.min.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.min.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.min.x, _boundingBox.max.y, _boundingBox.max.z),
        glm::vec3(_boundingBox.max.x, _boundingBox.max.y, _boundingBox.max.z),
    };

    std::vector<uint32_t> boxIndices = {0, 1, 0, 2, 0, 4, 3, 1, 3, 2, 3, 7,
                                        5, 4, 5, 1, 5, 7, 6, 4, 6, 7, 6, 2};

    glGenVertexArrays(1, &_boxVao);
    glGenBuffers(1, &_boxVbo);
    glGenBuffers(1, &_boxEbo);

    glBindVertexArray(_boxVao);
    glBindBuffer(GL_ARRAY_BUFFER, _boxVbo);
    glBufferData(
        GL_ARRAY_BUFFER, boxVertices.size() * sizeof(glm::vec3), boxVertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _boxEbo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, boxIndices.size() * sizeof(uint32_t), boxIndices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Model::cleanup() {
    if (_boxEbo) {
        glDeleteBuffers(1, &_boxEbo);
        _boxEbo = 0;
    }

    if (_boxVbo) {
        glDeleteBuffers(1, &_boxVbo);
        _boxVbo = 0;
    }

    if (_boxVao) {
        glDeleteVertexArrays(1, &_boxVao);
        _boxVao = 0;
    }

    if (_ebo != 0) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }

    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}

// 添加物理组件
void Model::addPhysics() {
    if (!_physics) {
        _physics = std::make_unique<Physics>();
        _physics->setModel(this);
    }
}

// 获取物理组件
Physics* Model::getPhysics() const {
    return _physics.get();
}

// 移除物理组件
void Model::removePhysics() {
    _physics.reset();
}

// 检查是否有物理组件
bool Model::hasPhysics() const {
    return _physics != nullptr;
}