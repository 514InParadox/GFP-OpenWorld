#include "finalSceneApp.hpp"

const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";

const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};

finalSceneApp::finalSceneApp(const Options &options) : Application(options) {
    // input mode, set curser
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // init perspective camera
    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    _camera.reset(new PerspectiveCamera(glm::radians(60.0f), aspect, 0.1f, 10000.0f));
    _camera->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // init skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));
}

void finalSceneApp::handleInput() {
    constexpr float cameraMoveSpeed = 0.5f;
    constexpr float cameraRotateSpeed = 0.002f;

    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    Camera* camera = _camera.get();

    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position + cameraMoveSpeed * camera->transform.getFront();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position - cameraMoveSpeed * camera->transform.getRight();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position - cameraMoveSpeed * camera->transform.getFront();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position + cameraMoveSpeed * camera->transform.getRight();
    }

    if (_input.mouse.move.xNow != _input.mouse.move.xOld) {
        float mouse_movement_in_x_direction = _input.mouse.move.xNow - _input.mouse.move.xOld;
        float delta = cameraRotateSpeed * mouse_movement_in_x_direction;

        camera->transform.rotation = glm::angleAxis(-delta, glm::vec3(0.0f, 1.0f, 0.0f)) * camera->transform.rotation;
    }

    if (_input.mouse.move.yNow != _input.mouse.move.yOld) {
        float mouse_movement_in_y_direction = _input.mouse.move.yNow - _input.mouse.move.yOld;
        float delta = cameraRotateSpeed * mouse_movement_in_y_direction;

        camera->transform.rotation = glm::angleAxis(-delta, camera->transform.getRight()) * camera->transform.rotation;
    }



    _input.forwardState();
}

void finalSceneApp::renderFrame() {   
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = _camera->getProjectionMatrix();
    glm::mat4 view = _camera->getViewMatrix();

    // _shader->use();

    // TODO_hy: add other components

    _skybox->draw(projection, view);
}

void finalSceneApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}