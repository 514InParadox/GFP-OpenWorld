#include "finalSceneApp.hpp"
#include <iomanip>
#include <math.h>
#include <chrono>

FinalSceneApp::FinalSceneApp(const Options &options) : Application(options) {
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
    _camera->transform.position = glm::vec3(0.0f, 1.8f, 0.0f);

    // init NPC
    _entity.reset(new AdvancedModel(getAssetFullPath(entityPath)));
    _mita.reset(new AdvancedModel(getAssetFullPath(mitaPath)));

    // init map
    _map.reset(new AdvancedModel(getAssetFullPath(mapPath)));

    // init skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));    // init shader
    initShader();
    
    // Initialize frame time
    _lastFrameTime = std::chrono::high_resolution_clock::now();
}

// camera 在平面上移动
void FinalSceneApp::handleInput() {
    // Update frame time for frame-rate independent movement
    updateFrameTime();
    
    static float cameraMoveSpeed = 10.0f; // Units per second

    if (_input.keyboard.keyStates[GLFW_KEY_LEFT_SHIFT] != GLFW_RELEASE)
        cameraMoveSpeed = playerMoveSpeedFast;
    else    
        cameraMoveSpeed = playerMoveSpeedSlow;

    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }    
      if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        // Project camera front direction onto XZ plane (horizontal plane)
        glm::vec3 front = _camera->transform.getFront();
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        _camera->transform.position += (cameraMoveSpeed * _deltaTime) * horizontalFront;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        // Project camera right direction onto XZ plane (horizontal plane)
        glm::vec3 right = _camera->transform.getRight();
        glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
        _camera->transform.position -= (cameraMoveSpeed * _deltaTime) * horizontalRight;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        // Project camera front direction onto XZ plane (horizontal plane)
        glm::vec3 front = _camera->transform.getFront();
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        _camera->transform.position -= (cameraMoveSpeed * _deltaTime) * horizontalFront;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        // Project camera right direction onto XZ plane (horizontal plane)
        glm::vec3 right = _camera->transform.getRight();
        glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
        _camera->transform.position += (cameraMoveSpeed * _deltaTime) * horizontalRight;
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

void FinalSceneApp::renderFrame() {
    static uint32_t frameCount = 0;
    constexpr uint32_t endurance = 5;
    uint32_t objIdx = frameCount / endurance;

    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = _camera->getProjectionMatrix();
    glm::mat4 view = _camera->getViewMatrix();
    
    _skybox->draw(projection, view);

    glm::vec2 playerPosition = glm::vec2(_camera->transform.position.x, _camera->transform.position.z);

    // draw map

    std::pair<int, int> mapLattice = std::make_pair(
        (int)floor((playerPosition.x + 150) / 300.0f),
        (int)floor((playerPosition.y + 150) / 300.0f)
    );

    _mapShader->use();

    _mapShader->setUniformMat4("projection", projection);
    _mapShader->setUniformMat4("view", view);
    // _mapShader->setUniformMat4("model", _map->transform.getLocalMatrix());

    // _map->draw();

    for (int d = 0; d <= 4; ++d) {
        int tx = dx[d] + mapLattice.first,
            ty = dy[d] + mapLattice.second;
        glm::mat4 mapPos = glm::translate(glm::mat4(1.0f), glm::vec3(tx * 300.0f - 150, 3.0f, ty * 300.0f - 150));
        _mapShader->setUniformMat4("model", mapPos);
        _map->draw();
    }

    // draw entity
    // _entityShader->use();
    // _entityShader->setUniformMat4("projection", projection);
    // _entityShader->setUniformMat4("view", view);
    // _entityShader->setUniformMat4("model", _entity->transform.getLocalMatrix());
    // _entity->draw();

    // draw mita
    // _mitaShader->use();
    // _mitaShader->setUniformMat4("projection", projection);
    // _mitaShader->setUniformMat4("view", view);
    // _mitaShader->setUniformMat4("model", _mita->transform.getLocalMatrix());
    // _mitaShader->setUniformInt("mapKd", 0);
    // _mita->draw();


    frameCount = (frameCount + 1) % (FRAMES * 5);
}

void FinalSceneApp::initShader() {
    // _shader.reset(new GLSLProgram);
    // _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    // _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    // _shader->link();

    _texShader.reset(new GLSLProgram);
    _texShader->attachVertexShaderFromFile(getAssetFullPath(texVertexShaderAddr));
    _texShader->attachFragmentShaderFromFile(getAssetFullPath(texFragmentShaderAddr));
    _texShader->link();

    _entityShader.reset(new GLSLProgram);
    _entityShader->attachVertexShaderFromFile(getAssetFullPath(entityVertexShaderAddr));
    _entityShader->attachFragmentShaderFromFile(getAssetFullPath(entityFragmentShaderAddr));
    _entityShader->link();

    _mapShader.reset(new GLSLProgram);
    _mapShader->attachVertexShaderFromFile(getAssetFullPath(mapVertexShaderAddr));
    _mapShader->attachFragmentShaderFromFile(getAssetFullPath(mapFragmentShaderAddr));
    _mapShader->link();    
    
    _mitaShader.reset(new GLSLProgram);
    _mitaShader->attachVertexShaderFromFile(getAssetFullPath(texVertexShaderAddr));
    _mitaShader->attachFragmentShaderFromFile(getAssetFullPath(texFragmentShaderAddr));
    _mitaShader->link();
}

void FinalSceneApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    
    // Cap delta time to avoid large jumps (same pattern as other apps)
    if (_deltaTime > 0.05f) {
        _deltaTime = 0.05f;
    }
    
    _lastFrameTime = currentTime;
}