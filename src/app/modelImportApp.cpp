#include "modelImportApp.hpp"
#include <iomanip>
#include <math.h>

constexpr float pi = 3.141592653589793;
constexpr float a2r = pi / 180;

const std::string modelPath = "resource/model/liquid";

const std::string modelTexPath = "resource/model/RawCube.obj";

const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";
const std::string texVertexShaderAddr   = "shader/vertex/oneTexture_diffuseLight.vert";
const std::string texFragmentShaderAddr = "shader/fragment/oneTexture_diffuseLight.frag";

const std::string texturePath = "resource/texture/CubeTexture.png";

const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};

TexModel::TexModel(const std::string& filepath) : Model(filepath) {}

TexModel::TexModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : Model(vertices, indices) {}

ModelImportApp::ModelImportApp(const Options &options) : Application(options) {
    // set input mode
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // init cameras
    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    _camera.reset(new PerspectiveCamera(glm::radians(60.0f), aspect, znear, zfar));
    _camera->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // init model
    for (int i = 0; i < FRAMES; ++i) {
        std::ostringstream oss;
        oss << std::setw(4) << std::setfill('0') << i + 1;
        try {
            _models[i].reset(new Model(getAssetFullPath(modelPath + oss.str() + ".obj")));
        } catch (const std::exception &e) {
            std::cerr << "Failed to load model for frame " << i << ": " << e.what() << std::endl;
            _models[i] = nullptr;
        }
    }

    _texModel.reset(new TexModel(getAssetFullPath(modelTexPath)));
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, 45 * a2r, glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, 45 * a2r, glm::vec3(1.0f, 0.0f, 0.0f));
    _texModel->transform.setFromTRS(transform);

    // init textures
    std::shared_ptr<Texture2D> cubeTexture =
    std::make_shared<ImageTexture2D>(getAssetFullPath(texturePath));
    _texModel->mapKd = cubeTexture;

    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));

    // init shader
    initShader();
}

void ModelImportApp::handleInput() {
    constexpr float cameraMoveSpeed = 0.3f;
    constexpr float cameraRotateSpeed = 0.002f;

    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position + cameraMoveSpeed * _camera->transform.getFront();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position - cameraMoveSpeed * _camera->transform.getRight();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position - cameraMoveSpeed * _camera->transform.getFront();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position + cameraMoveSpeed * _camera->transform.getRight();
    }

    if (_input.mouse.move.xNow != _input.mouse.move.xOld) {
        float mouse_movement_in_x_direction = _input.mouse.move.xNow - _input.mouse.move.xOld;
        float delta = cameraRotateSpeed * mouse_movement_in_x_direction;

        _camera->transform.rotation = glm::angleAxis(-delta, glm::vec3(0.0f, 1.0f, 0.0f)) * _camera->transform.rotation;
    }

    if (_input.mouse.move.yNow != _input.mouse.move.yOld) {
        float mouse_movement_in_y_direction = _input.mouse.move.yNow - _input.mouse.move.yOld;
        float delta = cameraRotateSpeed * mouse_movement_in_y_direction;

        _camera->transform.rotation = glm::angleAxis(-delta, _camera->transform.getRight()) * _camera->transform.rotation;
    }

    _input.forwardState();
}

void ModelImportApp::renderFrame() {
    static uint32_t frameCount = 0;
    constexpr uint32_t endurance = 5;
    uint32_t objIdx = frameCount / endurance;

    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = _camera->getProjectionMatrix();
    glm::mat4 view = _camera->getViewMatrix();

    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);
    _shader->setUniformMat4("model", _models[objIdx]->transform.getLocalMatrix());

    if (_models[objIdx]) {
        _models[objIdx]->draw();
    } else {
        // Handle the case where the model is not initialized
        std::cerr << "Model at frame " << frameCount << " is not initialized!" << std::endl;
    }
    
    _texShader->use();
    _texShader->setUniformMat4("projection", projection);
    _texShader->setUniformMat4("view", view);
    _texShader->setUniformMat4("model", _texModel->transform.getLocalMatrix());
    _texModel->mapKd->bind(0);
    _texShader->setUniformInt("mapKd", 0);
    
    _texModel->draw();
    
    _skybox->draw(projection, view);


    frameCount = (frameCount + 1) % (FRAMES * 5);
}

void ModelImportApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();

    _texShader.reset(new GLSLProgram);
    _texShader->attachVertexShaderFromFile(getAssetFullPath(texVertexShaderAddr));
    _texShader->attachFragmentShaderFromFile(getAssetFullPath(texFragmentShaderAddr));
    _texShader->link();
}