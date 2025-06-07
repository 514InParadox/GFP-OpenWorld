#include "finalSceneApp.hpp"
#include "player.hpp"

#include <iomanip>
#include <math.h>
#include <chrono>

float distance(const glm::vec2 &lhs, const glm::vec2 &rhs) {
    float dx = lhs.x - rhs.x,
          dy = lhs.y - rhs.y;
    return sqrt(dx * dx + dy * dy);
}

FinalSceneApp::FinalSceneApp(const Options &options) : Application(options) {
    // set input mode
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // init player
    playerPosition = glm::vec2(0.0f, 0.0f);

    // init camera
    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    _camera.reset(new PerspectiveCamera(glm::radians(60.0f), aspect, znear, zfar));
    // _camera->transform.position = glm::vec3(0.0f, 1.8f, 0.0f);
    _camera->transform.position = glm::vec3(-147.0f, 1.8f, -147.0f);

    // init NPC
    _entity.reset(new AdvancedModel(getAssetFullPath(entityPath)));
    _mita.reset(new AdvancedModel(getAssetFullPath(mitaPath)));
    _mita->transform.scale = glm::vec3(0.5f);

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

    // Initialize interface
    startInterface.reset(new Interface(getAssetFullPath(startInterfaceImageAddr)));
    loseInterface.reset(new Interface(getAssetFullPath(loseInterfaceImageAddr)));
    winInterface.reset(new Interface(getAssetFullPath(winInterfaceImageAddr)));
}

// camera 在平面上移动
void FinalSceneApp::handleInput() {
    static glm::vec2 playerPosition(-147.0f, -147.0f);
    switch (gameState) {
        case GameState::StartInterface:
            if (_input.mouse.press.left)
                gameState = GameState::AfterMita;
            return;
            // break;
        case GameState::LoseInterface:
            if (_input.mouse.press.left)
                gameState = GameState::StartInterface;
            return;
            // break;
        case GameState::WinInterface:
            if (_input.mouse.press.left)
                gameState = GameState::StartInterface;
            return;
        default:
            break;
    }
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

    // player movement
    glm::vec3 deltaPosition = glm::vec3(0.0f);
    glm::vec3 dbg3D_deltaPosition = glm::vec3(0.0f);
    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        // Project camera front direction onto XZ plane (horizontal plane)
        glm::vec3 front = _camera->transform.getFront();
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        deltaPosition += (cameraMoveSpeed * _deltaTime) * horizontalFront;
        dbg3D_deltaPosition += (cameraMoveSpeed * _deltaTime) * front;
        // _camera->transform.position += (cameraMoveSpeed * _deltaTime) * horizontalFront;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        // Project camera right direction onto XZ plane (horizontal plane)
        glm::vec3 right = _camera->transform.getRight();
        glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
        deltaPosition -= (cameraMoveSpeed * _deltaTime) * horizontalRight;
        dbg3D_deltaPosition -= (cameraMoveSpeed * _deltaTime) * right;
        // _camera->transform.position -= (cameraMoveSpeed * _deltaTime) * horizontalRight;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        // Project camera front direction onto XZ plane (horizontal plane)
        glm::vec3 front = _camera->transform.getFront();
        glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
        deltaPosition -= (cameraMoveSpeed * _deltaTime) * horizontalFront;
        dbg3D_deltaPosition -= (cameraMoveSpeed * _deltaTime) * front;
        // _camera->transform.position -= (cameraMoveSpeed * _deltaTime) * horizontalFront;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        // Project camera right direction onto XZ plane (horizontal plane)
        glm::vec3 right = _camera->transform.getRight();
        glm::vec3 horizontalRight = glm::normalize(glm::vec3(right.x, 0.0f, right.z));
        deltaPosition += (cameraMoveSpeed * _deltaTime) * horizontalRight;
        dbg3D_deltaPosition += (cameraMoveSpeed * _deltaTime) * right;
        // _camera->transform.position += (cameraMoveSpeed * _deltaTime) * horizontalRight;
    }

    playerPosition = getCorrectPos(playerPosition, glm::vec2(deltaPosition.x, deltaPosition.z));
    _camera->transform.position = getCameraPos(playerPosition, glm::vec2(deltaPosition.x, deltaPosition.z), _deltaTime);

    // _camera->transform.position += dbg3D_deltaPosition;

    // view movement
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

    if (gameState == GameState::AfterMita && _input.mouse.press.left) {
        if (false) {
            gameState = GameState::AfterEntity;
            _mapFinalLattice = std::make_pair(
                (int)floor((playerPosition.x + 150) / 300.0f),
                (int)floor((playerPosition.y + 150) / 300.0f)
            );
        }
        // 射击检测，视角射线经过 entity 的碰撞箱
    }

    _input.forwardState();
}

void FinalSceneApp::renderFrame() {
    if (gameState == GameState::StartInterface ||
        gameState == GameState::LoseInterface ||
        gameState == GameState::WinInterface) {
        _interfaceShader->use();
        _interfaceShader->setUniformInt("textureImage", 0);
        switch (gameState) {
            case GameState::StartInterface:
                startInterface->draw();
                break;
            case GameState::LoseInterface:
                loseInterface->draw();
                break;
            case GameState::WinInterface:
               winInterface->draw();
                break;
            default:
                assert(false);
        }
        return;
    }

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

    _entity->transform.position = glm::vec3(-145, 0.5f, -145);
    _entity->transform.scale = glm::vec3(0.3f);

    _mita->transform.position = glm::vec3(mapLattice.first * 300.0f - 150.0f + mitaCoord.first, 0.0f, mapLattice.second * 300.0f - 150.0f + mitaCoord.second);

    _mapShader->use();

    _mapShader->setUniformMat4("projection", projection);
    _mapShader->setUniformMat4("view", view);
    // _mapShader->setUniformMat4("model", _map->transform.getLocalMatrix());

    // _map->draw();

    if (gameState != GameState::AfterEntity) {
        for (int d = 0; d <= 4; ++d) {
            int tx = dx[d] + mapLattice.first,
            ty = dy[d] + mapLattice.second;
            glm::mat4 mapPos = glm::translate(glm::mat4(1.0f), glm::vec3(tx * 300.0f - 150, 3.0f, ty * 300.0f - 150));
            _mapShader->setUniformMat4("model", mapPos);
            _map->draw();
        }
    } else {
        for (int d = 0; d <= 0; ++d) {
            int tx = dx[d] + mapLattice.first,
            ty = dy[d] + mapLattice.second;
            glm::mat4 mapPos = glm::translate(glm::mat4(1.0f), glm::vec3(tx * 300.0f - 150, 3.0f, ty * 300.0f - 150));
            _mapShader->setUniformMat4("model", mapPos);
            _map->draw();
        }
    }

    // draw entity
    if (gameState == GameState::BeforeMita || gameState == GameState::AfterMita) {
        _entityShader->use();
        _entityShader->setUniformMat4("projection", projection);
        _entityShader->setUniformMat4("view", view);
        _entityShader->setUniformMat4("model", _entity->transform.getLocalMatrix());
        _entity->draw();
    }

    // draw mita
    if (gameState == GameState::BeforeMita || gameState == GameState::DuringMita) {
        _mitaShader->use();
        _mitaShader->setUniformMat4("projection", projection);
        _mitaShader->setUniformMat4("view", view);
        _mitaShader->setUniformMat4("model", _mita->transform.getLocalMatrix());
        _mitaShader->setUniformInt("mapKd", 0);
        _mita->draw();
    }
}

void FinalSceneApp::initShader() {
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

    _interfaceShader.reset(new GLSLProgram);
    _interfaceShader->attachVertexShaderFromFile(getAssetFullPath(interfaceVertexShaderAddr));
    _interfaceShader->attachFragmentShaderFromFile(getAssetFullPath(interfaceFragmentShaderAddr));
    _interfaceShader->link();
}

void FinalSceneApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    
    if (_deltaTime > 0.05f) {
        _deltaTime = 0.05f;
    }
    
    _lastFrameTime = currentTime;
}

void FinalSceneApp::updateState() {
    glm::vec2 mitaPos;
    std::pair<int, int> mapLattice;
    switch (gameState) {
        case GameState::BeforeMita:
            if (distance(_entityLogic.getEntityPos(), playerPosition) < EntityTriggleDist) {
                gameState = GameState::LoseInterface;
            }
            mitaPos = glm::vec2(_mita->transform.position.x, _mita->transform.position.z);
            if (distance(mitaPos, playerPosition) < MitaTriggleDist) {
                gameState = GameState::DuringMita;
                // _dialog.Start();
            }
            break;
        case GameState::DuringMita:
            // if (_dialog.IsFinish()) {
            if (false) {
                gameState = GameState::AfterMita;
            }
            break;
        case GameState::AfterMita:
            if (distance(_entityLogic.getEntityPos(), playerPosition) < EntityTriggleDist) {
                gameState = GameState::LoseInterface;
            }
            break;
        case GameState::AfterEntity:
            mapLattice = std::make_pair(
                (int)floor((playerPosition.x + 150) / 300.0f),
                (int)floor((playerPosition.y + 150) / 300.0f)
            );
            if (mapLattice != _mapFinalLattice) {
                gameState = GameState::WinInterface;
            }
            break;
        default:
            break;
    }
}

void FinalSceneApp::run() {
    while (!glfwWindowShouldClose(_window)) {
        updateTime();

        updateState();

        handleInput();

        // 新增：在渲染前做离散碰撞检测
        CollisionSystem::instance().update(_deltaTime);

        renderFrame();

        glfwSwapBuffers(_window);
        glfwPollEvents();
    }
}