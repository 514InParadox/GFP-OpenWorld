#ifndef _MODEL_TRIANGLE_H
#define _MODEL_TRIANGLE_H

#include <glad/glad.h>
#include <vector>

#include "utils/vertex.hpp"
#include "model.hpp"

class Triangle : public Model {
public:
    Triangle(const std::vector<Vertex> vertices);

    ~Triangle();

    Triangle(const Triangle& rhs) = delete;

    Triangle(Triangle&& rhs) noexcept;
private:
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
};

#endif