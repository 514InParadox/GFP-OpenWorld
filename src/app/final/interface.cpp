#include "interface.hpp"

Interface::Interface(const std::string &imagePath) {
    initGLResources(imagePath);
}

void Interface::initGLResources(const std::string &imagePath) {
    constexpr float vertices[] = {
        // 位置              // 纹理坐标
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // 左下
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  // 右下
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  // 右上
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f   // 左上
    };
    constexpr unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // 加载纹理
    int width, height, nrChannels;
    unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);
    
    if (!data) {
        throw std::runtime_error("Failed to load texture: " + imagePath);
    }

    // 生成并绑定纹理
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 上传纹理数据
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // 释放图片数据
    stbi_image_free(data);

    // create a vertex array object
    glGenVertexArrays(1, &_vao);
    // create a vertex buffer object
    glGenBuffers(1, &_vbo);
    // create a element array buffer
    glGenBuffers(1, &_ebo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 位置属性 (location = 0)
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性 (location = 1)
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    
    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cleanup();
        throw std::runtime_error("OpenGL Error in Interface initialization: " + std::to_string(error));
    }
}



Interface::~Interface() {
    cleanup();
}

void Interface::draw() const {
    // 启用透明度混合（如果需要）
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 使用着色器程序
    // _shader->use();
    
    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);
    
    // 绘制
    glBindVertexArray(_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    // 禁用混合
    // glDisable(GL_BLEND);
}

void Interface::cleanup() {
    if (_texture) {
        glDeleteTextures(1, &_texture);
        _texture = 0;
    }
    
    if (_ebo) {
        glDeleteBuffers(1, &_ebo);
        _ebo = 0;
    }

    if (_vbo) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }

    if (_vao) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}