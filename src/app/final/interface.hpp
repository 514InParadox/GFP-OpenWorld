#pragma once

#include <string>
#include <memory>

#include "utils/gl_utility.hpp"
#include "utils/glsl_program.hpp"
#include "stb/stb_image.h"

// 界面

class Interface {
public:
    Interface(const std::string &imagePath);
    
    ~Interface();
    
    void draw() const;
    
    // 禁用拷贝构造和赋值
    Interface(const Interface&) = delete;
    Interface& operator=(const Interface&) = delete;
    Interface& operator=(Interface&&) = delete;

private:
    void initGLResources(const std::string &imagePath);
    void cleanup();
    
    // OpenGL 资源
    GLuint _vao = 0;
    GLuint _vbo = 0;
    GLuint _ebo = 0;
    GLuint _texture = 0;
};