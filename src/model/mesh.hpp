#pragma once

// 供 NPC 等复杂模型使用

#include <vector>
#include "utils/vertex.hpp"
#include "model/advancedTexture.hpp"
#include "utils/glsl_program.hpp"


class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<AdvancedTexture> textures;
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<AdvancedTexture> textures);
        void draw() const;
        // void draw(GLSLProgram &shader) const;
    private:
        unsigned int VAO, VBO, EBO;
        void setupMesh();
};  