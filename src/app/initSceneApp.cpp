#include "initSceneApp.hpp"

const std::string modelPath = "resource/model/MonkeyHead.obj";

const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";

const std::vector<std::string> skyboxTexturePaths = {
    "resource/skybox/default/right.jpg",
    "resource/skybox/default/left.jpg",
    "resource/skybox/default/top.jpg",
    "resource/skybox/default/bottom.jpg",
    "resource/skybox/default/front.jpg",
    "resource/skybox/default/back.jpg"
};
 
InitSceneApp::InitSceneApp(const Options &options) : Application(options) {
    // set input mode
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // init cameras
    _cameras.resize(2);

    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    // perspective camera
    _cameras[0].reset(new PerspectiveCamera(glm::radians(60.0f), aspect, 0.1f, 10000.0f));
    _cameras[0]->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // orthographic camera
    _cameras[1].reset(
        new OrthographicCamera(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f, znear, zfar));
    _cameras[1]->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // init model
    _model.reset(new Model(getAssetFullPath(modelPath)));

    // init shader
    initShader();

    // init skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));
}

void InitSceneApp::handleInput() { 
    constexpr float cameraMoveSpeed = 0.5f;
    constexpr float cameraRotateSpeed = 0.002f;

    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS) {
        activeCameraIndex = (activeCameraIndex + 1) % _cameras.size();
        _input.keyboard.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
        return;
    }

    Camera* camera = _cameras[activeCameraIndex].get();

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

void InitSceneApp::renderFrame() {   
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = _cameras[activeCameraIndex]->getProjectionMatrix();
    glm::mat4 view = _cameras[activeCameraIndex]->getViewMatrix();

    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);
    _shader->setUniformMat4("model", _model->transform.getLocalMatrix());

    _model->draw();

    _skybox->draw(projection, view);
}

void InitSceneApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}