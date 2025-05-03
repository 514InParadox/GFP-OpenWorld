#include "skybox.hpp"

const std::string skyboxVertexShaderPath = "shader/vertex/skybox.vert";
const std::string skyboxFragmentShaderPath = "shader/fragment/skybox.frag";

SkyBox::SkyBox(const std::vector<std::string>& textureFilenames) {
    GLfloat vertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                          1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                          -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                          -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                          1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                          -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                          -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                          1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                          -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                          1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    // create vao and vbo
    glGenVertexArrays(1, &_vao);
    glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glBindVertexArray(0);

    try {
        // init texture
        _texture.reset(new ImageTextureCubemap(textureFilenames));

        const char* vsCode =
            "#version 330 core\n"
            "layout(location = 0) in vec3 aPosition;\n"
            "out vec3 texCoord;\n"
            "uniform mat4 projection;\n"
            "uniform mat4 view;\n"
            "void main() {\n"
            "   texCoord = aPosition;\n"
            "   gl_Position = (projection * view * vec4(aPosition, 1.0f)).xyzz;\n"
            "}\n";

        const char* fsCode =
            "#version 330 core\n"
            "out vec4 color;\n"
            "in vec3 texCoord;\n"
            "uniform samplerCube cubemap;\n"
            "void main() {\n"
            "   color = texture(cubemap, texCoord);\n"
            "}\n";

        _shader.reset(new GLSLProgram);
        // puts("shader");
        _shader->attachVertexShader(vsCode);
        _shader->attachFragmentShader(fsCode);
        // _shader->attachVertexShaderFromFile(skyboxVertexShaderPath);
        // _shader->attachFragmentShaderFromFile(skyboxFragmentShaderPath);
        _shader->link();
        // _shader->use();
    } catch (const std::exception&) {
        cleanup();
        throw;
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::stringstream ss;
        ss << "skybox creation failure, (code " << error << ")";
        cleanup();
        throw std::runtime_error(ss.str());
    }
}

SkyBox::SkyBox(SkyBox&& rhs) noexcept
    : _vao(rhs._vao), _vbo(rhs._vbo), _texture(std::move(rhs._texture)),
      _shader(std::move(rhs._shader)) {
    rhs._vao = 0;
    rhs._vbo = 0;
}

SkyBox::~SkyBox() {
    cleanup();
}

void SkyBox::draw(const glm::mat4& projection, const glm::mat4& view) {
        // TODO:: draw skybox
    // write your code here
    // -----------------------------------------------
    // Disable depth writing to ensure the skybox is always rendered behind other objects
    glDepthFunc(GL_LEQUAL); // Allow skybox to render at the farthest depth
    glDepthMask(GL_FALSE);  // Disable depth writing for the skybox
    
    // Use the shader program
    _shader->use();
    
    // Set the projection and view matrices
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", glm::mat4(glm::mat3(view))); // Remove translation from the view matrix
    
    // Bind the cubemap texture
    _texture->bind(0);
    _shader->setUniformInt("cubemap", 0); // Ensure the uniform location is set
    
    // Bind the VAO and draw the cube
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Re-enable depth writing
    glDepthMask(GL_TRUE);   // Re-enable depth writing
    glDepthFunc(GL_LESS);   // Restore default depth function
    // -----------------------------------------------
}

void SkyBox::cleanup() {
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
        _vbo = 0;
    }
    
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
        _vao = 0;
    }
}