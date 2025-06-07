#include "finalSceneApp.hpp"
#include "player.hpp"

#include <iomanip>
#include <math.h>
#include <chrono>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>

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
    // playerPosition = glm::vec2(-147.0f, -147.0f);
    // playerPosition = glm::vec2(-147.0f, -147.0f);

    // init camera
    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    _camera.reset(new PerspectiveCamera(glm::radians(60.0f), aspect, znear, zfar));
    // _camera->transform.position = glm::vec3(0.0f, 1.8f, 0.0f);
    // _camera->transform.position = glm::vec3(-147.0f, 1.8f, -147.0f);

    // init NPC
    _entity.reset(new AdvancedModel(getAssetFullPath(entityPath)));
    _entity->transform.position = glm::vec3(-10.0f, 0.0f, -10.0f);
    _mita.reset(new AdvancedModel(getAssetFullPath(mitaPath)));
    _mita->transform.scale = glm::vec3(0.1f);
    _mita->transform.position = glm::vec3(mitaCoord.first - 150, 0.0f, mitaCoord.second - 150);    // init gun
    _gun.reset(new AdvancedModel(getAssetFullPath(gunPath)));
    // _gun->transform.scale = glm::vec3(0.1f);

    // init map
    _map.reset(new AdvancedModel(getAssetFullPath(mapPath)));

    // init light
    _light.reset(new AdvancedModel(getAssetFullPath(lightPath)));

    // init skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));    // init shader
    initShader();
    
    // Initialize glow effect
    initGlowFramebuffers();
    initScreenQuad();
    
    // Initialize frame time
    _lastFrameTime = std::chrono::high_resolution_clock::now();

    // Initialize interface
    startInterface.reset(new Interface(getAssetFullPath(startInterfaceImageAddr)));
    loseInterface.reset(new Interface(getAssetFullPath(loseInterfaceImageAddr)));
    winInterface.reset(new Interface(getAssetFullPath(winInterfaceImageAddr)));
}

FinalSceneApp::~FinalSceneApp() {
    cleanupGlowFramebuffers();
}

// camera 在平面上移动
void FinalSceneApp::handleInput() {
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    switch (gameState) {
        case GameState::StartInterface:
            if (_input.keyboard.keyStates[GLFW_KEY_ENTER] != GLFW_RELEASE) {
                gameState = GameState::BeforeMita;
                // playerPosition = glm::vec2(playerCoord.first - 150, playerCoord.second - 150);
                playerPosition = glm::vec2(-242, -160);
                _entityLogic.setEntityPos(glm::vec2(0.0f, 0.0f));
            }
            
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

    // Toggle control mode with P key
    if (_input.keyboard.keyStates[GLFW_KEY_P] == GLFW_PRESS) {
        _controlMode = (_controlMode == ControlMode::Mita) ? ControlMode::Entity : ControlMode::Mita;
        std::cout << "Control mode switched to: " << 
                     ((_controlMode == ControlMode::Mita) ? "Mita" : "Entity") << std::endl;
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

    // std::cout << "playerPosition in handleInput: " << playerPosition.x << ", " << playerPosition.y << std::endl;

    // std::cout << _camera->transform.position.y << std::endl;

    // _camera->transform.position += dbg3D_deltaPosition;
    // playerPosition = glm::vec2(_camera->transform.position.x, _camera->transform.position.z);    // shoot
    if (_input.mouse.press.left && gameState == GameState::AfterMita) {
        // Create ray from camera position in view direction
        glm::vec3 rayOrigin = _camera->transform.position;
        glm::vec3 rayDirection = glm::normalize(_camera->transform.getFront());
        
        // Get entity's world-space AABB
        BoundingBox entityAABB = _entity->getBoundingBox();
        entityAABB.transform(_entity->transform.getLocalMatrix());
        
        // Check ray-AABB intersection
        if (rayIntersectsAABB(rayOrigin, rayDirection, entityAABB)) {
            std::cout << "Hit detected! Entity shot." << std::endl;
            
            // Trigger game state change if in AfterMita state
            gameState = GameState::AfterEntity;
            _mapFinalLattice = std::make_pair(
                (int)floor((playerPosition.x + 150) / 300.0f),
                (int)floor((playerPosition.y + 150) / 300.0f)
            );
            std::cout << "Entity defeated! Proceed to final area." << std::endl;
        }
    }

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
    }    _input.forwardState();
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

    // Use glow rendering pipeline
    if (_enableGlow) {
        // 1. Render scene to framebuffer
        renderSceneToFramebuffer();
        
        // 2. Extract bright colors
        renderBrightExtraction();
        
        // 3. Apply blur to bright colors
        renderBlur();
        
        // 4. Combine original scene with blurred bright colors
        renderFinalComposition();
    } else {
        // Render directly to screen without glow
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        renderSceneToFramebuffer();
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
    _mitaShader->attachVertexShaderFromFile(getAssetFullPath(mitaVertexShaderAddr));
    _mitaShader->attachFragmentShaderFromFile(getAssetFullPath(mitaFragmentShaderAddr));
    _mitaShader->link();

    _gunShader.reset(new GLSLProgram);
    _gunShader->attachVertexShaderFromFile(getAssetFullPath(gunVertexShaderAddr));
    _gunShader->attachFragmentShaderFromFile(getAssetFullPath(gunFragmentShaderAddr));
    _gunShader->link();

    _emissiveShader.reset(new GLSLProgram);
    _emissiveShader->attachVertexShaderFromFile(getAssetFullPath(emissiveVertexShaderAddr));
    _emissiveShader->attachFragmentShaderFromFile(getAssetFullPath(emissiveFragmentShaderAddr));
    _emissiveShader->link();

    // Initialize glow effect shaders
    _extractBrightShader.reset(new GLSLProgram);
    _extractBrightShader->attachVertexShaderFromFile(getAssetFullPath(screenQuadVertexShaderAddr));
    _extractBrightShader->attachFragmentShaderFromFile(getAssetFullPath(extractBrightShaderAddr));
    _extractBrightShader->link();
    
    _blurShader.reset(new GLSLProgram);
    _blurShader->attachVertexShaderFromFile(getAssetFullPath(screenQuadVertexShaderAddr));
    _blurShader->attachFragmentShaderFromFile(getAssetFullPath(blurShaderAddr));
    _blurShader->link();
    
    _combineShader.reset(new GLSLProgram);
    _combineShader->attachVertexShaderFromFile(getAssetFullPath(screenQuadVertexShaderAddr));
    _combineShader->attachFragmentShaderFromFile(getAssetFullPath(combineShaderAddr));
    _combineShader->link();

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
            std::cout << _entityLogic.getEntityPos().x << ", " << _entityLogic.getEntityPos().y << std::endl;
            std::cout << playerPosition.x << ", " << playerPosition.y << std::endl;
            if (distance(_entityLogic.getEntityPos(), playerPosition) < EntityTriggleDist) {
                puts("Lose: BeforeMita");
                // gameState = GameState::LoseInterface;
            }
            mitaPos = glm::vec2(_mita->transform.position.x, _mita->transform.position.z);
            if (distance(mitaPos, playerPosition) < MitaTriggleDist) {
                gameState = GameState::DuringMita;
                // _dialog.Start();
            }
            break;
        case GameState::DuringMita:
            _dialog->proceed(_deltaTime);
            if (_dialog->isFinished()) {
            // if (true) {
                gameState = GameState::AfterMita;
            }
            break;
        case GameState::AfterMita:
            std::cout << _entityLogic.getEntityPos().x << ", " << _entityLogic.getEntityPos().y << std::endl;
            std::cout << playerPosition.x << ", " << playerPosition.y << std::endl;
            if (distance(_entityLogic.getEntityPos(), playerPosition) < EntityTriggleDist) {
                puts("Lose: AfterMita");
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
    _entityLogic.move(playerPosition, _deltaTime);
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

void FinalSceneApp::initGlowFramebuffers() {
    // Scene framebuffer
    glGenFramebuffers(1, &_sceneFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _sceneFramebuffer);
    
    // Scene color texture
    glGenTextures(1, &_sceneColorTexture);
    glBindTexture(GL_TEXTURE_2D, _sceneColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _windowWidth, _windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _sceneColorTexture, 0);
    
    // Scene depth texture
    glGenTextures(1, &_sceneDepthTexture);
    glBindTexture(GL_TEXTURE_2D, _sceneDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _sceneDepthTexture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Scene framebuffer not complete!" << std::endl;
    }
    
    // Bright extraction framebuffer
    glGenFramebuffers(1, &_brightFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _brightFramebuffer);
    
    glGenTextures(1, &_brightColorTexture);
    glBindTexture(GL_TEXTURE_2D, _brightColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _windowWidth, _windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _brightColorTexture, 0);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Bright framebuffer not complete!" << std::endl;
    }
    
    // Blur framebuffers
    glGenFramebuffers(2, _blurFramebuffers);
    glGenTextures(2, _blurColorTextures);
    
    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, _blurFramebuffers[i]);
        glBindTexture(GL_TEXTURE_2D, _blurColorTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _windowWidth, _windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _blurColorTextures[i], 0);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Blur framebuffer " << i << " not complete!" << std::endl;
        }
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FinalSceneApp::initScreenQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &_quadVAO);
    glGenBuffers(1, &_quadVBO);
    glBindVertexArray(_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void FinalSceneApp::renderSceneToFramebuffer() {
    // Bind scene framebuffer (or default framebuffer if glow is disabled)
    if (_enableGlow) {
        glBindFramebuffer(GL_FRAMEBUFFER, _sceneFramebuffer);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glViewport(0, 0, _windowWidth, _windowHeight);
    
    // Clear
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

    // Original positions (commented for testing)
    // _entity->transform.position = glm::vec3(-145, 0.5f, -145);
    // _entity->transform.scale = glm::vec3(0.3f);
    // _mita->transform.position = glm::vec3(mapLattice.first * 300.0f - 150.0f + mitaCoord.first, 0.0f, mapLattice.second * 300.0f - 150.0f + mitaCoord.second);

    // Manual control with IJKL keys (switchable between mita and entity)
    // static glm::vec3 mitaPosition = glm::vec3(mitaCoord.first + mapLattice.first * 300 - 150, 0.0f, mitaCoord.second + mapLattice.second * 300 - 150); // Initial mita position
    glm::vec3 mitaPosition = _mita->transform.position;
    // static glm::vec3 entityPosition(-126.0f, 1.0f, -121.0f); // Initial entity position
    glm::vec3 entityPosition = _entity->transform.position;
    float moveSpeed = 3.0f; // Units per second
    
    // Movement controls (IJKL keys) - controlled object depends on _controlMode
    glm::vec3* controlledPosition = (_controlMode == ControlMode::Mita) ? &mitaPosition : &entityPosition;
    
    if (_controlMode == ControlMode::Entity) {
        if (_input.keyboard.keyStates[GLFW_KEY_I] != GLFW_RELEASE) {
            controlledPosition->z -= moveSpeed * _deltaTime; // Move forward (negative Z)
        }
        if (_input.keyboard.keyStates[GLFW_KEY_K] != GLFW_RELEASE) {
            controlledPosition->z += moveSpeed * _deltaTime; // Move backward (positive Z)
        }
        if (_input.keyboard.keyStates[GLFW_KEY_J] != GLFW_RELEASE) {
            controlledPosition->x -= moveSpeed * _deltaTime; // Move left (negative X)
        }
        if (_input.keyboard.keyStates[GLFW_KEY_L] != GLFW_RELEASE) {
            controlledPosition->x += moveSpeed * _deltaTime; // Move right (positive X)
        }
    } else if (_controlMode == ControlMode::Mita) {
        *controlledPosition = glm::vec3(mapLattice.first * 300 - 150 + mitaCoord.first, 0.0f, mapLattice.second * 300 - 150 + mitaCoord.second);
    }
    
    // Apply positions to models
    _entity->transform.position = entityPosition;
    _entity->transform.scale = glm::vec3(0.3f);
    _mita->transform.position = mitaPosition;

    std::cout << "mita: " << mitaPosition.x << ", " << mitaPosition.z << std::endl;

    // Debug output for mita and entity positions and game state
    static float debugTimer = 0.0f;
    debugTimer += _deltaTime;
    if (debugTimer >= 2.0f) { // Output debug info every 2 seconds
        std::cout << "=== Debug Info ===" << std::endl;
        std::cout << "Control mode: " << ((_controlMode == ControlMode::Mita) ? "Mita" : "Entity") << std::endl;
        
        std::cout << "Mita position: (" << std::fixed << std::setprecision(3) 
                  << _mita->transform.position.x << ", " 
                  << _mita->transform.position.y << ", " 
                  << _mita->transform.position.z << ")" << std::endl;
        
        std::cout << "Entity position: (" << std::fixed << std::setprecision(3) 
                  << _entity->transform.position.x << ", " 
                  << _entity->transform.position.y << ", " 
                  << _entity->transform.position.z << ")" << std::endl;
        
        std::cout << "Camera position: (" << std::fixed << std::setprecision(3) 
                  << _camera->transform.position.x << ", " 
                  << _camera->transform.position.y << ", " 
                  << _camera->transform.position.z << ")" << std::endl;
        
        // Output current game state
        const char* stateNames[] = {"StartInterface", "BeforeMita", "DuringMita", "AfterMita", "AfterEntity", "LoseInterface", "WinInterface"};
        std::cout << "Current game state: " << stateNames[static_cast<int>(gameState)] << std::endl;
          debugTimer = 0.0f;
    }
    
    // Draw map terrain
    _mapShader->use();
    _mapShader->setUniformMat4("projection", projection);
    _mapShader->setUniformMat4("view", view);
    
    // Set map material properties
    _mapShader->setUniformVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    _mapShader->setUniformInt("mapKd", 0); // Bind texture to slot 0

    if (gameState != GameState::AfterEntity) {
        for (int d = 0; d <= 4; ++d) {
            int tx = dx[d] + mapLattice.first,
            ty = dy[d] + mapLattice.second;
            glm::mat4 mapPos = glm::translate(glm::mat4(1.0f), glm::vec3(tx * 300.0f - 150, 0.0f, ty * 300.0f - 150));
            _mapShader->setUniformMat4("model", mapPos);
            _map->draw();
        }
    } else {
        for (int d = 0; d <= 0; ++d) {
            int tx = dx[d] + mapLattice.first,
            ty = dy[d] + mapLattice.second;
            glm::mat4 mapPos = glm::translate(glm::mat4(1.0f), glm::vec3(tx * 300.0f - 150, 0.0f, ty * 300.0f - 150));
            _mapShader->setUniformMat4("model", mapPos);
            _map->draw();
        }
    }

    // draw light (model itself has height, so place at y=0.0f)
    _emissiveShader->use();
    _emissiveShader->setUniformMat4("projection", projection);
    _emissiveShader->setUniformMat4("view", view);
    
    // Set emissive material properties for bright white light
    _emissiveShader->setUniformVec3("material.color", glm::vec3(1.0f, 1.0f, 0.9f)); // Warm white light
    _emissiveShader->setUniformFloat("material.intensity", 1.5f); // Moderate intensity for glow effect
    
    for (int d = 0; d <= 4; ++d) {
        int tx = dx[d] + mapLattice.first,
        ty = dy[d] + mapLattice.second;
        glm::mat4 lightPos = glm::translate(glm::mat4(1.0f), glm::vec3(tx * 300.0f - 150, 0.0f, ty * 300.0f - 150));
        _emissiveShader->setUniformMat4("model", lightPos);
        _light->draw();
    }

    // Update dynamic point lights based on player position and map data
    updateDynamicPointLights();
    
    // Update mita point lights based on mita position and map data
    updateMitaPointLights();

    // draw entity
    if (gameState == GameState::BeforeMita || gameState == GameState::AfterMita) {
        _entityShader->use();
        _entityShader->setUniformMat4("projection", projection);
        _entityShader->setUniformMat4("view", view);
        glm::vec2 entityPos = _entityLogic.getEntityPos();
        _entity->transform.position = glm::vec3(entityPos.x, 0.0f, entityPos.y);
        _entityShader->setUniformMat4("model", _entity->transform.getLocalMatrix());
        
        // Set camera position for lighting calculations
        _entityShader->setUniformVec3("viewPosition", _camera->transform.position);
        
        // Set material properties (reduced reflectivity to prevent over-brightness)
        _entityShader->setUniformVec3("material.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        _entityShader->setUniformVec3("material.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        _entityShader->setUniformVec3("material.specular", glm::vec3(0.2f, 0.2f, 0.2f));
        _entityShader->setUniformVec3("material.color", glm::vec3(0.7f, 0.7f, 0.7f));
        _entityShader->setUniformFloat("material.shininess", 64.0f);
        
        // Set ambient light
        _entityShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f, 1.0f, 1.0f));
        _entityShader->setUniformFloat("ambientLight.intensity", 0.2f);
        
        // Set dynamic point lights
        setPointLightsUniforms(_entityShader.get());
        
        // Debug: Output number of lights being used
        static float lightCountTimer = 0.0f;
        lightCountTimer += _deltaTime;
        if (lightCountTimer >= 2.0f) {
            std::cout << "Setting " << _dynamicPointLights.size() << " point lights to entity shader" << std::endl;
            lightCountTimer = 0.0f;
        }
        
        _entity->draw();
    }    
    
    // draw gun
    if (gameState == GameState::DuringMita) {

    } else if (gameState == GameState::AfterMita || gameState == GameState::BeforeMita) {
        _gunShader->use();
        _gunShader->setUniformMat4("projection", projection);
        _gunShader->setUniformMat4("view", view);
        
        // Calculate gun position relative to camera
        // Place gun at camera's right-front position
        glm::vec3 cameraPos = _camera->transform.position;
        glm::vec3 cameraFront = glm::normalize(_camera->transform.getFront());
        glm::vec3 cameraRight = glm::normalize(_camera->transform.getRight());
        glm::vec3 cameraUp    = glm::normalize(_camera->transform.getUp());
        
        // Offset from camera: slightly right, slightly down, and forward
        glm::vec3 gunOffset = cameraRight * 0.5f +      // Right offset
                             glm::vec3(0, -0.2f, 0) +   // Down offset
                             cameraFront * 1.0f -
                             cameraUp * 0.5f;        // Forward offset
        
        glm::vec3 gunPosition = cameraPos + gunOffset;
        
        // Set gun transform
        _gun->transform.position = gunPosition;
        _gun->transform.scale = glm::vec3(0.015f); // Scale down the gun
          // Rotate gun to align with camera direction, then rotate 180 degrees to face forward
        glm::quat gunRotation180 = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        _gun->transform.rotation = _camera->transform.rotation * gunRotation180;
        
        // Apply model matrix
        _gunShader->setUniformMat4("model", _gun->transform.getLocalMatrix());
        
        // Set camera position for lighting calculations
        _gunShader->setUniformVec3("viewPosition", _camera->transform.position);
        
        // Set material properties for gun
        _gunShader->setUniformVec3("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        _gunShader->setUniformVec3("material.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        _gunShader->setUniformVec3("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));
        _gunShader->setUniformVec3("material.color", glm::vec3(0.3f, 0.3f, 0.3f));
        _gunShader->setUniformFloat("material.shininess", 128.0f);
          // Set ambient light
        _gunShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f, 1.0f, 1.0f));
        _gunShader->setUniformFloat("ambientLight.intensity", 0.3f);        // Set dynamic point lights
        setPointLightsUniforms(_gunShader.get());
        
        // Set default texture uniforms
        _gunShader->setUniformInt("diffuseTexture", 0);
        _gunShader->setUniformInt("normalTexture", 1);
        
        // Draw gun with custom material handling
        bool foundDiffuse = false, foundNormal = false;
        
        for (const auto& mesh : _gun->getMeshes()) {
            // Find and bind appropriate textures
            int diffuseSlot = -1, normalSlot = -1;
            
            for (int i = 0; i < mesh.textures.size(); i++) {
                const auto& texture = mesh.textures[i];
                
                if (texture.type == "texture_diffuse" && !foundDiffuse) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, texture.id);
                    diffuseSlot = 0;
                    foundDiffuse = true;
                } else if (texture.type == "texture_normal" && !foundNormal) {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, texture.id);
                    normalSlot = 1;
                    foundNormal = true;
                }
            }
            
            // Set texture usage flags
            _gunShader->setUniformBool("useDiffuseTexture", foundDiffuse);
            _gunShader->setUniformBool("useNormalTexture", foundNormal);
              // Draw the mesh
            glBindVertexArray(mesh.getVAO());
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        
        // Reset texture state
        glActiveTexture(GL_TEXTURE0);
        
        // Debug output for gun position
        static float gunDebugTimer = 0.0f;
        gunDebugTimer += _deltaTime;
        if (gunDebugTimer >= 3.0f) {
            std::cout << "Gun position: (" << std::fixed << std::setprecision(3) 
                      << gunPosition.x << ", " << gunPosition.y << ", " << gunPosition.z << ")" << std::endl;
            std::cout << "Camera position: (" << std::fixed << std::setprecision(3) 
                      << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
            std::cout << "Gun textures found - Diffuse: " << (foundDiffuse ? "Yes" : "No") 
                      << ", Normal: " << (foundNormal ? "Yes" : "No") << std::endl;
            gunDebugTimer = 0.0f;
        }
    }

    // draw mita
    if (gameState == GameState::BeforeMita || gameState == GameState::DuringMita) {
        _mitaShader->use();
        _mitaShader->setUniformMat4("projection", projection);
        _mitaShader->setUniformMat4("view", view);
        _mitaShader->setUniformMat4("model", _mita->transform.getLocalMatrix());
        
        // Set camera position for lighting calculations
        _mitaShader->setUniformVec3("viewPosition", _camera->transform.position);
        
        // Set material properties for mita (slightly different from entity)
        _mitaShader->setUniformVec3("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
        _mitaShader->setUniformVec3("material.diffuse", glm::vec3(0.6f, 0.6f, 0.6f));
        _mitaShader->setUniformVec3("material.specular", glm::vec3(0.3f, 0.3f, 0.3f));
        _mitaShader->setUniformVec3("material.color", glm::vec3(0.8f, 0.8f, 0.8f));
        _mitaShader->setUniformFloat("material.shininess", 32.0f);
        
        // Set ambient light (increased intensity for better visibility)
        _mitaShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f, 1.0f, 1.0f));
        _mitaShader->setUniformFloat("ambientLight.intensity", 3.2f);
        
        // Set mita's dynamic point lights
        setMitaPointLightsUniforms(_mitaShader.get());
        
        // Enable texture for mita
        _mitaShader->setUniformBool("useTexture", true);
        _mitaShader->setUniformInt("diffuseTexture", 0);
        
        // Debug: Output number of lights being used for mita
        static float mitaLightCountTimer = 0.0f;
        mitaLightCountTimer += _deltaTime;
        if (mitaLightCountTimer >= 2.0f) {
            std::cout << "Setting " << _mitaPointLights.size() << " point lights to mita shader" << std::endl;
            mitaLightCountTimer = 0.0f;
        }
        
        _mita->draw();
    }
}

void FinalSceneApp::renderBrightExtraction() {
    // Bind bright extraction framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, _brightFramebuffer);
    glViewport(0, 0, _windowWidth, _windowHeight);
    glDisable(GL_DEPTH_TEST);
    
    // Clear
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use extract bright shader
    _extractBrightShader->use();
    
    // Bind scene texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _sceneColorTexture);
    _extractBrightShader->setUniformInt("sceneMap", 0);
    
    // Render screen quad
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void FinalSceneApp::renderBlur() {
    bool horizontal = true;
    bool first_iteration = true;
    int amount = 10; // Number of blur passes
    
    _blurShader->use();
    
    for (int i = 0; i < amount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, _blurFramebuffers[horizontal]);
        glViewport(0, 0, _windowWidth, _windowHeight);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        
        _blurShader->setUniformBool("horizontal", horizontal);
        
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        if (first_iteration) {
            glBindTexture(GL_TEXTURE_2D, _brightColorTexture);
        } else {
            glBindTexture(GL_TEXTURE_2D, _blurColorTextures[!horizontal]);
        }
        _blurShader->setUniformInt("image", 0);
        
        // Render screen quad
        glBindVertexArray(_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        
        horizontal = !horizontal;
        if (first_iteration) {
            first_iteration = false;
        }
    }
}

void FinalSceneApp::renderFinalComposition() {
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _windowWidth, _windowHeight);
    glDisable(GL_DEPTH_TEST);
    
    // Clear
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Use combine shader
    _combineShader->use();
    
    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _sceneColorTexture);
    _combineShader->setUniformInt("sceneTexture", 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _blurColorTextures[0]); // Use the final blurred texture
    _combineShader->setUniformInt("bloomTexture", 1);
    
    // Set glow parameters
    _combineShader->setUniformBool("enableGlow", _enableGlow);
    _combineShader->setUniformFloat("glowIntensity", _glowIntensity);
    
    // Render screen quad
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void FinalSceneApp::cleanupGlowFramebuffers() {
    glDeleteFramebuffers(1, &_sceneFramebuffer);
    glDeleteTextures(1, &_sceneColorTexture);
    glDeleteTextures(1, &_sceneDepthTexture);
    
    glDeleteFramebuffers(1, &_brightFramebuffer);
    glDeleteTextures(1, &_brightColorTexture);
    
    glDeleteFramebuffers(2, _blurFramebuffers);
    glDeleteTextures(2, _blurColorTextures);
    
    glDeleteVertexArrays(1, &_quadVAO);
    glDeleteBuffers(1, &_quadVBO);
}

void FinalSceneApp::updateDynamicPointLights() {
    // Clear existing dynamic point lights
    _dynamicPointLights.clear();
    
    // Get current entity position in world coordinates
    glm::vec3 entityPos = _entity->transform.position;
    
    // Convert world position to map coordinates
    // Map coordinate system: world position = (mapX - 150, y, mapZ - 150)
    // So: mapX = worldX + 150, mapZ = worldZ + 150
    int centerMapX = static_cast<int>(entityPos.x + 150);
    int centerMapZ = static_cast<int>(entityPos.z + 150);
    
    // Define search area (11x11 around entity)
    const int searchRadius = 5; // 11x11 area
    
    // Debug output for light detection
    static float lightDebugTimer = 0.0f;
    lightDebugTimer += _deltaTime;
    bool shouldDebug = lightDebugTimer >= 3.0f; // Debug every 3 seconds
    
    if (shouldDebug) {
        std::cout << "=== Dynamic Point Lights Debug ===" << std::endl;
        std::cout << "Entity world position: (" << std::fixed << std::setprecision(1) 
                  << entityPos.x << ", " << entityPos.y << ", " << entityPos.z << ")" << std::endl;
        std::cout << "Entity map position: (" << centerMapX << ", " << centerMapZ << ")" << std::endl;
        lightDebugTimer = 0.0f;
    }
    
    // Scan the area around the player
    for (int dx = -searchRadius; dx <= searchRadius; dx++) {
        for (int dz = -searchRadius; dz <= searchRadius; dz++) {
            int mapX = centerMapX + dx;
            int mapZ = centerMapZ + dz;
            
            // Check bounds
            if (mapX >= 0 && mapX < MAP_LENGTH && mapZ >= 0 && mapZ < MAP_LENGTH) {
                // Check if this position has a light (value == 2)
                if (map[mapZ][mapX] == 2) {
                    // Convert map coordinates back to world coordinates
                    float worldX = static_cast<float>(mapX) - 150.0f;
                    float worldZ = static_cast<float>(mapZ) - 150.0f;
                    float worldY = 3.0f; // Height as specified
                    
                    // Create point light at this position
                    glm::vec3 lightPos(worldX, worldY, worldZ);
                    _dynamicPointLights.emplace_back(lightPos, glm::vec3(1.0f, 1.0f, 0.9f), 2.0f);
                    
                    if (shouldDebug) {
                        std::cout << "Found light at map(" << mapX << ", " << mapZ << ") -> world(" 
                                  << worldX << ", " << worldY << ", " << worldZ << ")" << std::endl;
                    }
                    
                    // Limit number of lights to prevent performance issues
                    if (_dynamicPointLights.size() >= MAX_POINT_LIGHTS) {
                        if (shouldDebug) {
                            std::cout << "Reached maximum point lights limit: " << MAX_POINT_LIGHTS << std::endl;
                        }
                        return;
                    }
                }
            }
        }
    }
    
    if (shouldDebug) {
        std::cout << "Total dynamic point lights: " << _dynamicPointLights.size() << std::endl;
    }
}

void FinalSceneApp::updateMitaPointLights() {
    // Clear existing mita point lights
    _mitaPointLights.clear();
    
    // Get current mita position in world coordinates
    glm::vec3 mitaPos = _mita->transform.position;
    
    // Convert world position to map coordinates
    // Map coordinate system: world position = (mapX - 150, y, mapZ - 150)
    // So: mapX = worldX + 150, mapZ = worldZ + 150
    int centerMapX = static_cast<int>(mitaPos.x + 150);
    int centerMapZ = static_cast<int>(mitaPos.z + 150);
    
    // Define search area (11x11 around mita)
    const int searchRadius = 5; // 11x11 area
    
    // Debug output for light detection
    static float lightDebugTimer = 0.0f;
    lightDebugTimer += _deltaTime;
    bool shouldDebug = lightDebugTimer >= 3.0f; // Debug every 3 seconds
    
    if (shouldDebug) {
        std::cout << "=== Mita Point Lights Debug ===" << std::endl;
        std::cout << "Mita world position: (" << std::fixed << std::setprecision(1) 
                  << mitaPos.x << ", " << mitaPos.y << ", " << mitaPos.z << ")" << std::endl;
        std::cout << "Mita map position: (" << centerMapX << ", " << centerMapZ << ")" << std::endl;
        lightDebugTimer = 0.0f;
    }
    
    // Scan the area around the mita
    for (int dx = -searchRadius; dx <= searchRadius; dx++) {
        for (int dz = -searchRadius; dz <= searchRadius; dz++) {
            int mapX = centerMapX + dx;
            int mapZ = centerMapZ + dz;
            
            // Check bounds
            if (mapX >= 0 && mapX < MAP_LENGTH && mapZ >= 0 && mapZ < MAP_LENGTH) {
                // Check if this position has a light (value == 2)
                if (map[mapZ][mapX] == 2) {
                    // Convert map coordinates back to world coordinates
                    float worldX = static_cast<float>(mapX) - 150.0f;
                    float worldZ = static_cast<float>(mapZ) - 150.0f;
                    float worldY = 3.0f; // Height as specified
                    
                    // Create point light at this position
                    glm::vec3 lightPos(worldX, worldY, worldZ);
                    _mitaPointLights.emplace_back(lightPos, glm::vec3(1.0f, 1.0f, 0.9f), 2.0f);
                    
                    if (shouldDebug) {
                        std::cout << "Found light at map(" << mapX << ", " << mapZ << ") -> world(" 
                                  << worldX << ", " << worldY << ", " << worldZ << ")" << std::endl;
                    }
                    
                    // Limit number of lights to prevent performance issues
                    if (_mitaPointLights.size() >= MAX_POINT_LIGHTS) {
                        if (shouldDebug) {
                            std::cout << "Reached maximum point lights limit for mita: " << MAX_POINT_LIGHTS << std::endl;
                        }
                        return;
                    }
                }
            }
        }
    }
    
    if (shouldDebug) {
        std::cout << "Total mita point lights: " << _mitaPointLights.size() << std::endl;
    }
}

void FinalSceneApp::setPointLightsUniforms(GLSLProgram* shader) {
    // Set number of point lights
    int numLights = static_cast<int>(std::min(_dynamicPointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS)));
    shader->setUniformInt("numPointLights", numLights);
    
    // Set each point light's properties
    for (int i = 0; i < numLights; i++) {
        const auto& light = _dynamicPointLights[i];
        
        std::string baseName = "pointLights[" + std::to_string(i) + "]";
        shader->setUniformVec3(baseName + ".position", light.position);
        shader->setUniformVec3(baseName + ".color", light.color);
        shader->setUniformFloat(baseName + ".intensity", light.intensity);
    }
}

void FinalSceneApp::setMitaPointLightsUniforms(GLSLProgram* shader) {
    // Set number of point lights for mita
    int numLights = static_cast<int>(std::min(_mitaPointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS)));
    shader->setUniformInt("numPointLights", numLights);
    
    // Set each point light's properties
    for (int i = 0; i < numLights; i++) {
        const auto& light = _mitaPointLights[i];
        
        std::string baseName = "pointLights[" + std::to_string(i) + "]";
        shader->setUniformVec3(baseName + ".position", light.position);
        shader->setUniformVec3(baseName + ".color", light.color);
        shader->setUniformFloat(baseName + ".intensity", light.intensity);
    }
}

// Ray-AABB intersection test using the slab method
bool FinalSceneApp::rayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const BoundingBox& aabb) const {
    // Handle edge case where ray direction component is zero
    glm::vec3 invDir = glm::vec3(1.0f);
    for (int i = 0; i < 3; ++i) {
        if (abs(rayDirection[i]) < 1e-8f) {
            // Ray is parallel to the slab
            if (rayOrigin[i] < aabb.min[i] || rayOrigin[i] > aabb.max[i]) {
                return false; // Ray is outside the slab and parallel to it
            }
            invDir[i] = 1e8f; // Use a large value instead of infinity
        } else {
            invDir[i] = 1.0f / rayDirection[i];
        }
    }

    // Calculate t values for each axis
    glm::vec3 t1 = (aabb.min - rayOrigin) * invDir;
    glm::vec3 t2 = (aabb.max - rayOrigin) * invDir;

    // Ensure t1 contains the smaller values
    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);

    // Find the largest tmin and smallest tmax
    float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    // Ray intersects AABB if tNear <= tFar and tFar >= 0
    return tNear <= tFar && tFar >= 0.0f;
}