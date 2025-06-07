#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/animated_vertex.hpp"
#include "utils/glsl_program.hpp"
#include "utils/texture2d.hpp"

class AnimatedMesh {
public:
    // mesh data
    std::vector<AnimatedVertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::shared_ptr<Texture2D>> textures;

    AnimatedMesh(std::vector<AnimatedVertex> vertices, std::vector<unsigned int> indices, 
                 std::vector<std::shared_ptr<Texture2D>> textures);
    void Draw(GLSLProgram& shader);

private:
    // render data
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};
