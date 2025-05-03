#pragma once

// #ifndef _UTILS_SKYBOX_H
// #define _UTILS_SKYBOX_H

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "gl_utility.hpp"
#include "glsl_program.hpp"
#include "texture_cubemap.hpp"

class SkyBox {
public:
    SkyBox(const std::vector<std::string>& textureFilenames);

    SkyBox(SkyBox&& rhs) noexcept;

    ~SkyBox();

    void draw(const glm::mat4& projection, const glm::mat4& view);

private:
    GLuint _vao = 0;
    GLuint _vbo = 0;

    std::unique_ptr<TextureCubemap> _texture;

    std::unique_ptr<GLSLProgram> _shader;

    void cleanup();
};

// #endif