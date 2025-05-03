#include "triangle.hpp"

Triangle::Triangle(const std::vector<Vertex> vertices):
    _vertices(vertices), _indices({0, 1, 2})
    {
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(),
        _vertices.data(),
        GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t),
        _indices.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position)
    );    

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

Triangle::~Triangle() {
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

Triangle::Triangle(Triangle&& rhs) noexcept {
    _vertices = std::move(rhs._vertices);
    _indices = std::move(rhs._indices);

    _vao = rhs._vao;
    _vbo = rhs._vbo;
    _ebo = rhs._ebo;

    rhs._vao = 0;
    rhs._vbo = 0;
    rhs._ebo = 0;
}

void Triangle::draw() {
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}