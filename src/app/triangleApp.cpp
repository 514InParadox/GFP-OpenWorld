#include "triangleApp.hpp"
#include "utils/vertex.hpp"

const std::string vertexShaderAddr   = "shader/vertex/triangleApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/triangleApp.frag";

const std::string textureAddr = "resource/texture/woodBox.jpg";

TriangleApp::TriangleApp(const Options &options) : Application(options) {
    _triangle.reset(new Triangle({
        Vertex(glm::vec3(-0.6f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(0.6f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(0.0f, 0.8f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f))
    }));

    initShader();
}

void TriangleApp::handleInput() {
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }
}

void TriangleApp::renderFrame() {
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    _shader->use();

    static const float pi = (float) acos(-1);
    _color.r = (float) (sin(twicks / 250.0 + pi / 4) / 2 + 0.5);
    _color.g = (float) (sin(twicks / 625.0 + pi / 3) / 2 + 0.5);
    _color.b = (float) (sin(twicks / 450.0 + pi / 6) / 2 + 0.5);

    _shader->setUniformVec4("inputColor", glm::vec4(_color, 1.0f));

    _triangle->draw();

    ++twicks;
    // TODO: get real time instead of app twicks
}

void TriangleApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}