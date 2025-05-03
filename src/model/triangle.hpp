#ifndef _MODEL_TRIANGLE_H
#define _MODEL_TRIANGLE_H

#include <glad/glad.h>
#include <vector>

#include "utils/vertex.hpp"

class Triangle {
public:
    Triangle(const std::vector<Vertex> vertices);

    ~Triangle();

    Triangle(const Triangle& rhs) = delete;

    Triangle(Triangle&& rhs) noexcept;

    void draw();
private:
    std::vector<Vertex> _vertices;
    std::vector<uint32_t> _indices;

    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
};

#endif