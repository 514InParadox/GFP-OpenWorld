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
    // _camera->transform.position = glm::vec3(-147.0f, 1.8f, -147.0f);    // init NPC - Animated models
    initAnimatedModels();// init gun
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

    // Init dialog
    _dialog.reset(new Dialog("resource/text"));

    // Init sound engine
    _audioManager.reset(new AudioManager());
    _audioManager->init(); // 加载音频文件
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
                _entityLogic.Status = EntityStatus::PATROL;
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

    // P key control mode switching disabled - Entity now uses AI logic    

    // player movement
    glm::vec3 deltaPosition = glm::vec3(0.0f);
    glm::vec3 dbg3D_deltaPosition = glm::vec3(0.0f);
    glm::vec2 playerPreviousPosition = playerPosition;
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
        BoundingBox entityAABB = _animatedEntity->getBoundingBox();
        entityAABB.transform(_animatedEntity->transform.getLocalMatrix());
        
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
        }    }

    // view movement
    if (_input.mouse.move.xNow != _input.mouse.move.xOld) {
        float mouse_movement_in_x_direction = _input.mouse.move.xNow - _input.mouse.move.xOld;
        float delta = cameraRotateSpeed * mouse_movement_in_x_direction;

        _camera->transform.rotation = glm::angleAxis(-delta, glm::vec3(0.0f, 1.0f, 0.0f)) * _camera->transform.rotation;
    }

    if (_input.mouse.move.yNow != _input.mouse.move.yOld) {
        float mouse_movement_in_y_direction = _input.mouse.move.yNow - _input.mouse.move.yOld;
        float delta = -cameraRotateSpeed * mouse_movement_in_y_direction;

        // Limit vertical angle to prevent looking too far up or down
        static float currentPitch = 0.0f;  // Track current pitch angle
        const float maxPitchAngle = glm::radians(80.0f);  // Maximum pitch angle (80 degrees)
        
        // Calculate new pitch angle
        float newPitch = currentPitch - delta;
        
        // Clamp pitch angle to prevent camera flipping
        newPitch = glm::clamp(newPitch, -maxPitchAngle, maxPitchAngle);
        
        // Only apply rotation if within limits
        if (newPitch != currentPitch) {
            float actualDelta = currentPitch - newPitch;
            currentPitch = newPitch;
            _camera->transform.rotation = glm::angleAxis(actualDelta, _camera->transform.getRight()) * _camera->transform.rotation;
        }
    }

    _input.forwardState();

    // update audio state
    _audioManager->updateEntitySound(_entityLogic, playerPosition);
    _audioManager->updateListenerPosition(_camera->transform.position, 
                                          _camera->transform.getFront(),
                                          _camera->transform.getUp());
    _audioManager->updatePlayerFootsteps(playerPosition,
                                         playerPreviousPosition,
                                         cameraMoveSpeed == playerMoveSpeedFast,
                                         0.8f,
                                         _deltaTime);
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
    _gunShader->attachFragmentShaderFromFile(getAssetFullPath(entityFragmentShaderAddr));
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
    static float during_debug_remain_time;
    switch (gameState) {
        case GameState::BeforeMita:
            // Entity collision detection
            if (distance(_entityLogic.getEntityPos(), playerPosition) < EntityTriggleDist) {
                // puts("Lose: BeforeMita");
                gameState = GameState::LoseInterface;
            }
            mitaPos = glm::vec2(_animatedMita->transform.position.x, _animatedMita->transform.position.z);
            if (distance(mitaPos, playerPosition) < MitaTriggleDist) {
                gameState = GameState::DuringMita;
                if (_dialog) {
                    _dialog->resetDialog();
                    glm::vec3 dialogPos = _animatedMita->transform.position + glm::vec3(0.0f, 1.8f, 0.0f);
                    _dialog->setBasePosition(dialogPos);
                }
                during_debug_remain_time = 5.0f;
            }
            break;
        case GameState::DuringMita:
        if (during_debug_remain_time > 0) {
            during_debug_remain_time -= _deltaTime;
        } else {
                _entityLogic.setEntityPos(glm::vec2(10.0f, -10.0f));
                _entityLogic.Status = EntityStatus::PATROL;
                gameState = GameState::AfterMita;
            }
            break;
        case GameState::AfterMita:
            // Entity collision detection
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
    // Entity AI logic movement
    _entityLogic.move(playerPosition, _deltaTime);
}

void FinalSceneApp::run() {    
    while (!glfwWindowShouldClose(_window)) {
        updateTime();

        updateState();

        // Update animations
        updateAnimations();

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
    // _mita->transform.position = glm::vec3(mapLattice.first * 300.0f - 150.0f + mitaCoord.first, 0.0f, mapLattice.second * 300.0f - 150.0f + mitaCoord.second);    // Manual control with IJKL keys (switchable between mita and entity)
    // glm::vec3 mitaPosition = _animatedMita ? _animatedMita->transform.position : _mita->transform.position;
    // glm::vec3 entityPosition = _animatedEntity ? _animatedEntity->transform.position : _entity->transform.position;
    // Mita position based on map coordinates
    glm::vec3 mitaPosition = glm::vec3(mapLattice.first * 300.0f - 150.0f + mitaCoord.first, 0.0f, mapLattice.second * 300.0f - 150.0f + mitaCoord.second);
    
    // Entity position controlled by AI logic
    glm::vec2 entityPos = _entityLogic.getEntityPos();
    glm::vec3 entityPosition = glm::vec3(entityPos.x, 0.3f, entityPos.y);
    // IJKL key controls removed - Entity now uses AI logic, Mita uses fixed position
    // mitaPosition = glm::vec3(mapLattice.first * 300 - 150 + mitaCoord.first, 0.0f, mapLattice.second * 300 - 150 + mitaCoord.second);
      // Apply positions to models (both regular and animated models for compatibility)
    if (_animatedEntity) {
        _animatedEntity->transform.position = entityPosition;
        _animatedEntity->transform.scale = glm::vec3(0.4f);
    }
    // if (_entity) {
    //     _entity->transform.position = entityPosition;
    //     _entity->transform.scale = glm::vec3(0.3f);
    // }
    
    if (_animatedMita) {
        _animatedMita->transform.position = mitaPosition;
        _animatedMita->transform.scale = glm::vec3(0.5f); // Use original scale for mita
    }
    // if (_mita) {
    //     _mita->transform.position = mitaPosition;
    //     _mita->transform.scale = glm::vec3(50.0f); // Increased scale for better visibility
    // }

    // TEST: Disable mita position output
    // std::cout << "mita: " << mitaPosition.x << ", " << mitaPosition.z << std::endl;    // Debug output for mita and entity positions and game state
    static float debugTimer = 0.0f;
            debugTimer += _deltaTime;
        if (debugTimer >= 2.0f) { // Output debug info every 2 seconds
            // TEST: Disable general debug info to focus on entity lighting
            // std::cout << "=== Debug Info ===" << std::endl;
            // std::cout << "Control mode: " << ((_controlMode == ControlMode::Mita) ? "Mita" : "Entity") << std::endl;
            
            // Use animated models for position if available
            // glm::vec3 currentMitaPos = _animatedMita ? _animatedMita->transform.position : _mita->transform.position;
            // glm::vec3 currentEntityPos = _animatedEntity ? _animatedEntity->transform.position : _entity->transform.position;
            // glm::vec3 currentMitaPos = _animatedMita->transform.position;
            // glm::vec3 currentEntityPos = _animatedEntity->transform.position;
            
            // std::cout << "Mita position: (" << std::fixed << std::setprecision(3) 
            //           << currentMitaPos.x << ", " 
            //           << currentMitaPos.y << ", " 
            //           << currentMitaPos.z << ")" << std::endl;
            
            // std::cout << "Entity position: (" << std::fixed << std::setprecision(3) 
            //           << currentEntityPos.x << ", " 
            //           << currentEntityPos.y << ", " 
            //           << currentEntityPos.z << ")" << std::endl;
            
            // std::cout << "Camera position: (" << std::fixed << std::setprecision(3) 
            //           << _camera->transform.position.x << ", " 
            //           << _camera->transform.position.y << ", " 
            //           << _camera->transform.position.z << ")" << std::endl;
            
            // Output current game state
            // const char* stateNames[] = {"StartInterface", "BeforeMita", "DuringMita", "AfterMita", "AfterEntity", "LoseInterface", "WinInterface"};
            // std::cout << "Current game state: " << stateNames[static_cast<int>(gameState)] << std::endl;
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
    
    // Update camera point lights based on camera position and map data
    updateCameraPointLights();    // draw entity
    if (gameState == GameState::BeforeMita || gameState == GameState::AfterMita) {
        if (_animatedEntity && _entityAnimator) {
            _entityShader->use();
            _entityShader->setUniformMat4("projection", projection);
            _entityShader->setUniformMat4("view", view);
            
            // Entity position is now set by AI logic above
              // Make entity face towards player (camera position)
            glm::vec3 playerPosition = _camera->transform.position;
            glm::vec3 entityToPlayer = playerPosition - _animatedEntity->transform.position;
            
            // Only rotate around Y axis (yaw) to keep entity upright
            if (glm::length(entityToPlayer) > 0.001f) { // Avoid zero-length vector
                entityToPlayer.y = 0.0f; // Remove vertical component for Y-axis only rotation
                entityToPlayer = glm::normalize(entityToPlayer);
                
                // Calculate rotation to face player with 90-degree offset for model orientation
                float angle = atan2(entityToPlayer.x, entityToPlayer.z); // Calculate Y-axis rotation angle
                angle -= glm::radians(90.0f); // Add 90-degree offset to correct model orientation
                _animatedEntity->transform.rotation = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            
            _entityShader->setUniformMat4("model", _animatedEntity->transform.getLocalMatrix());
            
            // Set camera position for lighting calculations
            _entityShader->setUniformVec3("viewPosition", _camera->transform.position);
            
            // Set material properties (enhanced for better light visibility)
            _entityShader->setUniformVec3("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));   // Slightly higher ambient
            _entityShader->setUniformVec3("material.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));   // Much higher diffuse
            _entityShader->setUniformVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));  // Higher specular
            _entityShader->setUniformVec3("material.color", glm::vec3(0.9f, 0.9f, 0.9f));     // Brighter base color
            _entityShader->setUniformFloat("material.shininess", 32.0f);                       // Lower shininess for broader highlights
            
            // Set ambient light (reduced to make point lights more visible)
            _entityShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f, 1.0f, 1.0f));
            _entityShader->setUniformFloat("ambientLight.intensity", 0.05f);                   // Much lower ambient
            
            // Set dynamic point lights
            setPointLightsUniforms(_entityShader.get());
            
            // Set bone matrices for skeletal animation
            auto transforms = _entityAnimator->GetFinalBoneMatrices();
            for (int i = 0; i < transforms.size(); ++i) {
                _entityShader->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            }
            
            // Debug: Output entity position and lighting info
            static float lightCountTimer = 0.0f;
            lightCountTimer += _deltaTime;
            if (lightCountTimer >= 2.0f) {
                std::cout << "=== Entity Debug Info ===" << std::endl;
                std::cout << "Entity position: (" << std::fixed << std::setprecision(2) 
                          << _animatedEntity->transform.position.x << ", " 
                          << _animatedEntity->transform.position.y << ", " 
                          << _animatedEntity->transform.position.z << ")" << std::endl;
                std::cout << "Player position: (" << std::fixed << std::setprecision(2) 
                          << _camera->transform.position.x << ", " 
                          << _camera->transform.position.y << ", " 
                          << _camera->transform.position.z << ")" << std::endl;
                
                // Output entity status
                EntityStatus currentStatus = _entityLogic.getStatus();
                std::string statusStr;
                switch (currentStatus) {
                    case EntityStatus::PATROL:
                        statusStr = "PATROL";
                        break;
                    case EntityStatus::CHASE:
                        statusStr = "CHASE";
                        break;
                    default:
                        statusStr = "UNKNOWN";
                        break;
                }
                std::cout << "Entity status: " << statusStr << std::endl;
                
                // Calculate distance to player
                glm::vec2 entityPos2D = _entityLogic.getEntityPos();
                glm::vec2 playerPos2D = glm::vec2(_camera->transform.position.x, _camera->transform.position.z);
                float distanceToPlayer = glm::length(entityPos2D - playerPos2D);
                std::cout << "Distance to player: " << std::fixed << std::setprecision(1) << distanceToPlayer << std::endl;
                
                lightCountTimer = 0.0f;
            }
            
            _animatedEntity->Draw(*_entityShader);
        }
    }
    
    // draw gun
    if (gameState == GameState::DuringMita) {

    } else if (gameState == GameState::AfterMita || gameState == GameState::AfterEntity) {
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
        
        // Set material properties for gun (dark gray/black tactical look)
        _gunShader->setUniformVec3("material.ambient", glm::vec3(0.6f, 0.6f, 0.6f));   // High ambient for brightness
        _gunShader->setUniformVec3("material.diffuse", glm::vec3(1.5f, 1.5f, 1.5f));   // Very high diffuse
        _gunShader->setUniformVec3("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));  // Moderate specular for matte finish
        _gunShader->setUniformVec3("material.color", glm::vec3(0.3f, 0.3f, 0.3f));     // Dark gray/black gun color
        _gunShader->setUniformFloat("material.shininess", 32.0f);                       // Lower shininess for tactical matte look
          // Set ambient light (very high for gun visibility in dark areas)
        _gunShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f, 1.0f, 1.0f));
        _gunShader->setUniformFloat("ambientLight.intensity", 2.4f);                    // Very high ambient light for dark visibility        // Set camera-based dynamic point lights
        setCameraPointLightsUniforms(_gunShader.get());
        
        // Set default directional light (disabled)
        _gunShader->setUniformVec3("dirLights.direction", glm::vec3(0.0f, -1.0f, 0.0f));
        _gunShader->setUniformVec3("dirLights.color", glm::vec3(1.0f, 1.0f, 1.0f));
        _gunShader->setUniformFloat("dirLights.intensity", 0.0f); // Disabled
        
        // Set default spotlight (disabled)
        _gunShader->setUniformVec3("spotLights.position", glm::vec3(0.0f, 0.0f, 0.0f));
        _gunShader->setUniformVec3("spotLights.direction", glm::vec3(0.0f, -1.0f, 0.0f));
        _gunShader->setUniformVec3("spotLights.color", glm::vec3(1.0f, 1.0f, 1.0f));
        _gunShader->setUniformFloat("spotLights.cutOff", cos(glm::radians(12.5f)));
        _gunShader->setUniformFloat("spotLights.intensity", 0.0f); // Disabled
        
        // Set default texture uniforms for entity.frag
        _gunShader->setUniformInt("diffuseTexture", 0);
        _gunShader->setUniformInt("normalTexture", 1);
        
        // Draw gun - force material color display (ignore textures for now)
        std::cout << "Gun mesh count: " << _gun->getMeshes().size() << std::endl;
        
        for (const auto& mesh : _gun->getMeshes()) {
            std::cout << "Gun mesh texture count: " << mesh.textures.size() << std::endl;
            
            // Force disable textures to show material color
            _gunShader->setUniformBool("useTexture", false);
            _gunShader->setUniformBool("useNormalTexture", false);
            
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
            std::cout << "=== Gun Debug Info ===" << std::endl;
            std::cout << "Gun position: (" << std::fixed << std::setprecision(3) 
                      << gunPosition.x << ", " << gunPosition.y << ", " << gunPosition.z << ")" << std::endl;
            std::cout << "Camera position: (" << std::fixed << std::setprecision(3) 
                      << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
            std::cout << "Gun: Dark gray tactical appearance with enhanced visibility" << std::endl;
            gunDebugTimer = 0.0f;
        }
    }    // draw mita
    if (gameState == GameState::BeforeMita || gameState == GameState::DuringMita) {
        if (_animatedMita && _mitaAnimator) {
            _mitaShader->use();
            _mitaShader->setUniformMat4("projection", projection);
            _mitaShader->setUniformMat4("view", view);            // Make mita face towards player (camera position) with smooth rotation
            glm::vec3 playerPosition = _camera->transform.position;
            glm::vec3 mitaToPlayer = playerPosition - _animatedMita->transform.position;
            
            // Only rotate around Y axis (yaw) to keep mita upright
            if (glm::length(mitaToPlayer) > 0.001f) { // Avoid zero-length vector
                mitaToPlayer.y = 0.0f; // Remove vertical component for Y-axis only rotation
                mitaToPlayer = glm::normalize(mitaToPlayer);
                
                // Calculate target angle to face player
                float targetAngle = atan2(mitaToPlayer.x, mitaToPlayer.z); // Calculate Y-axis rotation angle
                
                // Use static variable to maintain smooth rotation state
                static float currentMitaAngle = 0.0f;
                static bool mitaAngleInitialized = false;
                
                // Initialize the angle on first run
                if (!mitaAngleInitialized) {
                    glm::vec3 currentForward = _animatedMita->transform.getFront();
                    currentForward.y = 0.0f; // Remove vertical component
                    if (glm::length(currentForward) > 0.001f) {
                        currentForward = glm::normalize(currentForward);
                        currentMitaAngle = atan2(currentForward.x, currentForward.z);
                    } else {
                        currentMitaAngle = targetAngle; // Fallback to target angle
                    }
                    mitaAngleInitialized = true;
                }
                
                // Handle angle wrapping to ensure shortest path rotation
                float angleDiff = targetAngle - currentMitaAngle;
                if (angleDiff > glm::pi<float>()) {
                    angleDiff -= 2.0f * glm::pi<float>();
                }
                if (angleDiff < -glm::pi<float>()) {
                    angleDiff += 2.0f * glm::pi<float>();
                }
                
                // Smooth interpolation with exponential decay
                currentMitaAngle += angleDiff * 0.1f; // Slower interpolation (10% per frame)
                
                // Apply the rotation
                _animatedMita->transform.rotation = glm::angleAxis(currentMitaAngle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            
            _mitaShader->setUniformMat4("model", _animatedMita->transform.getLocalMatrix());
              // Set camera position for lighting calculations
            _mitaShader->setUniformVec3("viewPosition", _camera->transform.position);
              // Set material properties for mita (enhanced for much better visibility - brighter)
            _mitaShader->setUniformVec3("material.ambient", glm::vec3(0.4f, 0.4f, 0.4f));   // Increased from 0.2f
            _mitaShader->setUniformVec3("material.diffuse", glm::vec3(1.2f, 1.2f, 1.2f));   // Increased from 0.8f
            _mitaShader->setUniformVec3("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));  // Increased from 0.5f
            _mitaShader->setUniformVec3("material.color", glm::vec3(1.4f, 1.4f, 1.4f));     // Increased from 1.0f
            _mitaShader->setUniformFloat("material.shininess", 64.0f);
              // Set ambient light (increased intensity for better mita visibility)
            _mitaShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f, 1.0f, 1.0f));
            _mitaShader->setUniformFloat("ambientLight.intensity", 1.2f);                     // Increased from 0.8f
            
            // Set mita's dynamic point lights
            setMitaPointLightsUniforms(_mitaShader.get());
              // Enable texture for mita
            _mitaShader->setUniformBool("useTexture", true);
            _mitaShader->setUniformInt("diffuseTexture", 0);
            
            // Set bone matrices for skeletal animation
            auto transforms = _mitaAnimator->GetFinalBoneMatrices();
            for (int i = 0; i < transforms.size(); ++i) {
                _mitaShader->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            }
            
            // TEST: Disable mita transformation debug output
            // static float mitaDebugTimer = 0.0f;
            // mitaDebugTimer += _deltaTime;
            // if (mitaDebugTimer >= 3.0f) {
            //     glm::mat4 mitaMatrix = _animatedMita->transform.getLocalMatrix();
            //     std::cout << "Mita transform matrix position: (" 
            //               << mitaMatrix[3][0] << ", " << mitaMatrix[3][1] << ", " << mitaMatrix[3][2] << ")" << std::endl;
            //     std::cout << "Mita scale: " << _animatedMita->transform.scale.x << std::endl;
            //     mitaDebugTimer = 0.0f;
            // }
            
            // TEST: Disable mita light debug output
            // static float mitaLightCountTimer = 0.0f;
            // mitaLightCountTimer += _deltaTime;
            // if (mitaLightCountTimer >= 2.0f) {
            //     std::cout << "Setting " << _mitaPointLights.size() << " point lights to mita shader" << std::endl;
            //     mitaLightCountTimer = 0.0f;
            // }
            
            _animatedMita->Draw(*_mitaShader);
            // TEST: Disable mita position output to see entity debug info clearly
            // std::cout << "draw mita at" << _animatedMita->transform.position.x << ", " << _animatedMita->transform.position.z << std::endl;
        }
    }

    // Render dialog during DuringMita state
    if (!_dialog) {
        std::cout << "123" << std::endl;
    }
    if (gameState == GameState::DuringMita && _dialog) {
        // Use entity shader for dialog text rendering (it supports basic 3D model rendering)
        _entityShader->use();
        _entityShader->setUniformMat4("projection", projection);
        _entityShader->setUniformMat4("view", view);
        
        // Set basic material properties for text
        _entityShader->setUniformVec3("material.ambient", glm::vec3(0.3f));
        _entityShader->setUniformVec3("material.diffuse", glm::vec3(1.0f));
        _entityShader->setUniformVec3("material.specular", glm::vec3(0.5f));
        _entityShader->setUniformVec3("material.color", glm::vec3(1.0f));
        _entityShader->setUniformFloat("material.shininess", 32.0f);
        
        // Set lighting for dialog text
        _entityShader->setUniformVec3("ambientLight.color", glm::vec3(1.0f));
        _entityShader->setUniformFloat("ambientLight.intensity", 0.8f);
        _entityShader->setUniformVec3("viewPosition", _camera->transform.position);
        
        // Disable texture for text models
        _entityShader->setUniformBool("useTexture", false);
        
        // Get camera position and rotation for billboard effect
        glm::vec3 cameraPos = _camera->transform.position;
        glm::quat cameraRot = _camera->transform.rotation;
        
        // Draw dialog with camera-facing billboard effect
        _dialog->draw(_deltaTime, _entityShader.get(), cameraPos, cameraRot);
        
        std::cout << "Rendered dialog during DuringMita state" << std::endl;
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
    glm::vec3 entityPos = _animatedEntity->transform.position;
    
    // Convert world position to map coordinates
    // First, map world coordinates to [-150, 150] range (tiled map)
    float wrappedX = fmod(entityPos.x + 150.0f, 300.0f) - 150.0f;
    float wrappedZ = fmod(entityPos.z + 150.0f, 300.0f) - 150.0f;
    if (wrappedX < -150.0f) wrappedX += 300.0f;
    if (wrappedZ < -150.0f) wrappedZ += 300.0f;
    
    // Then convert to array indices [0, 299]
    int centerMapX = static_cast<int>(wrappedX + 150);
    int centerMapZ = static_cast<int>(wrappedZ + 150);
    
    // Clamp to valid array bounds (should not be needed now, but for safety)
    centerMapX = std::max(0, std::min(centerMapX, MAP_LENGTH - 1));
    centerMapZ = std::max(0, std::min(centerMapZ, MAP_LENGTH - 1));
    
    // Define search area (11x11 around entity)
    const int searchRadius = 5; // 11x11 area
    
    // Debug output disabled for light detection
    bool shouldDebug = false; // Disable light debug output
    
    // Scan the area around the player
    for (int dx = -searchRadius; dx <= searchRadius; dx++) {
        for (int dz = -searchRadius; dz <= searchRadius; dz++) {
            int mapX = centerMapX + dx;
            int mapZ = centerMapZ + dz;
            
            // Check bounds
            if (mapX >= 0 && mapX < MAP_LENGTH && mapZ >= 0 && mapZ < MAP_LENGTH) {
                // Check if this position has a light (value == 2)
                if (map[mapX][mapZ] == 2) {
                    // Convert map coordinates back to world coordinates
                    // First get the standard map coordinate
                    float standardWorldX = static_cast<float>(mapX) - 150.0f;
                    float standardWorldZ = static_cast<float>(mapZ) - 150.0f;
                    
                    // Then adjust to the same "tile" as the entity
                    // Calculate which 300x300 tile the entity is in
                    int entityTileX = static_cast<int>(floor((entityPos.x + 150.0f) / 300.0f));
                    int entityTileZ = static_cast<int>(floor((entityPos.z + 150.0f) / 300.0f));
                    
                    // Place light in the same tile as the entity
                    float worldX = standardWorldX + entityTileX * 300.0f;
                    float worldZ = standardWorldZ + entityTileZ * 300.0f;
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
    glm::vec3 mitaPos = _animatedMita->transform.position;
    
    // Convert world position to map coordinates
    // First, map world coordinates to [-150, 150] range (tiled map)
    float wrappedX = fmod(mitaPos.x + 150.0f, 300.0f) - 150.0f;
    float wrappedZ = fmod(mitaPos.z + 150.0f, 300.0f) - 150.0f;
    if (wrappedX < -150.0f) wrappedX += 300.0f;
    if (wrappedZ < -150.0f) wrappedZ += 300.0f;
    
    // Then convert to array indices [0, 299]
    int centerMapX = static_cast<int>(wrappedX + 150);
    int centerMapZ = static_cast<int>(wrappedZ + 150);
    
    // Clamp to valid array bounds (should not be needed now, but for safety)
    centerMapX = std::max(0, std::min(centerMapX, MAP_LENGTH - 1));
    centerMapZ = std::max(0, std::min(centerMapZ, MAP_LENGTH - 1));
    
    // Define search area (11x11 around mita)
    const int searchRadius = 5; // 11x11 area
    
    // Debug output disabled for mita light detection
    bool shouldDebug = false; // Disable mita light debug output
    
    // Scan the area around the mita
    for (int dx = -searchRadius; dx <= searchRadius; dx++) {
        for (int dz = -searchRadius; dz <= searchRadius; dz++) {
            int mapX = centerMapX + dx;
            int mapZ = centerMapZ + dz;
            
            // Check bounds
            if (mapX >= 0 && mapX < MAP_LENGTH && mapZ >= 0 && mapZ < MAP_LENGTH) {
                // Check if this position has a light (value == 2)
                if (map[mapX][mapZ] == 2) {
                    // Convert map coordinates back to world coordinates
                    // First get the standard map coordinate
                    float standardWorldX = static_cast<float>(mapX) - 150.0f;
                    float standardWorldZ = static_cast<float>(mapZ) - 150.0f;
                    
                    // Then adjust to the same "tile" as the mita
                    // Calculate which 300x300 tile the mita is in
                    int mitaTileX = static_cast<int>(floor((mitaPos.x + 150.0f) / 300.0f));
                    int mitaTileZ = static_cast<int>(floor((mitaPos.z + 150.0f) / 300.0f));
                    
                    // Place light in the same tile as the mita
                    float worldX = standardWorldX + mitaTileX * 300.0f;
                    float worldZ = standardWorldZ + mitaTileZ * 300.0f;
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
        for (int i = 0; i < _mitaPointLights.size(); i++) {
            const auto& light = _mitaPointLights[i];
            std::cout << "  Mita Light " << i << ": world(" << std::fixed << std::setprecision(1)
                      << light.position.x << ", " << light.position.y << ", " << light.position.z 
                      << ") intensity=" << light.intensity << std::endl;
        }
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

// Animation system implementation
void FinalSceneApp::initAnimatedModels() {
    try {
        // Initialize animated entity model
        _animatedEntity.reset(new AnimatedModel(getAssetFullPath(animatedEntityPath)));
        _animatedEntity->transform.position = glm::vec3(-10.0f, 0.3f, -10.0f);
        
        // Initialize animated mita model
        _animatedMita.reset(new AnimatedModel(getAssetFullPath(animatedMitaPath)));
        _animatedMita->transform.scale = glm::vec3(0.5f);
        _animatedMita->transform.position = glm::vec3(mitaCoord.first - 150, 0.0f, mitaCoord.second - 150);
        
        // Setup animations for both models
        setupAnimations();
        
        std::cout << "Animated models initialized successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize animated models: " << e.what() << std::endl;
    }
}

void FinalSceneApp::setupAnimations() {
    if (!_animatedEntity || !_animatedMita) {
        std::cerr << "Animated models not initialized, cannot setup animations" << std::endl;
        return;
    }
    
    try {
        // Setup entity animations
        int entityAnimationCount = Animation::GetAnimationCount(getAssetFullPath(animatedEntityPath));
        for (int i = 0; i < entityAnimationCount; ++i) {
            auto animation = std::make_unique<Animation>(getAssetFullPath(animatedEntityPath), _animatedEntity.get(), i);
            _entityAnimations.push_back(std::move(animation));
        }
        
        // Setup mita animations
        int mitaAnimationCount = Animation::GetAnimationCount(getAssetFullPath(animatedMitaPath));
        for (int i = 0; i < mitaAnimationCount; ++i) {
            auto animation = std::make_unique<Animation>(getAssetFullPath(animatedMitaPath), _animatedMita.get(), i);
            _mitaAnimations.push_back(std::move(animation));
        }
        
        // Create animators with first animation
        if (!_entityAnimations.empty()) {
            _entityAnimator.reset(new Animator(_entityAnimations[0].get()));
            _currentEntityAnimationIndex = 0;
        }
        
        if (!_mitaAnimations.empty()) {
            _mitaAnimator.reset(new Animator(_mitaAnimations[0].get()));
            _currentMitaAnimationIndex = 0;
        }
        
        std::cout << "Animations setup completed!" << std::endl;
        std::cout << "Entity animations: " << _entityAnimations.size() << std::endl;
        std::cout << "Mita animations: " << _mitaAnimations.size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to setup animations: " << e.what() << std::endl;
    }
}

void FinalSceneApp::updateAnimations() {
    // Calculate delta time for animations
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - _lastFrameTime).count();
    _lastFrameTime = currentTime;
    
    // Update entity animation based on EntityStatus
    updateEntityAnimation();
    
    // Update mita animation (always moving)
    updateMitaAnimation();
    
    // Update animators
    if (_entityAnimator) {
        _entityAnimator->UpdateAnimation(deltaTime * 5);
    }
    
    if (_mitaAnimator) {
        _mitaAnimator->UpdateAnimation(deltaTime * 5);
    }
}

void FinalSceneApp::updateEntityAnimation() {
    if (_entityAnimations.empty() || !_entityAnimator) return;
    
    // Get entity status from EntityLogic
    EntityStatus currentStatus = _entityLogic.getStatus();
    
    // Determine which animation to play based on status
    int targetAnimationIndex = _currentEntityAnimationIndex;
    
    switch (currentStatus) {
        case EntityStatus::PATROL:
            // Use first animation for patrol (usually idle/walk)
            targetAnimationIndex = 0;
            break;
        case EntityStatus::CHASE:
            // Use second animation for chase (usually run) if available
            targetAnimationIndex = _entityAnimations.size() > 1 ? 1 : 0;
            break;
        default:
            targetAnimationIndex = 0;
            break;
    }
    
    // Switch animation if needed
    if (targetAnimationIndex != _currentEntityAnimationIndex) {
        switchEntityAnimation(targetAnimationIndex);
    }
}

void FinalSceneApp::updateMitaAnimation() {
    // Mita always uses the first animation (movement)
    if (_mitaAnimations.empty() || !_mitaAnimator) return;
    
    // Keep using the first animation for now
    if (_currentMitaAnimationIndex != 0) {
        switchMitaAnimation(0);
    }
}

void FinalSceneApp::switchEntityAnimation(int animationIndex) {
    if (animationIndex >= 0 && animationIndex < _entityAnimations.size() && _entityAnimator) {
        _currentEntityAnimationIndex = animationIndex;
        _entityAnimator->PlayAnimation(_entityAnimations[animationIndex].get());
        std::cout << "Switched entity to animation " << animationIndex << std::endl;
    }
}

void FinalSceneApp::switchMitaAnimation(int animationIndex) {
    if (animationIndex >= 0 && animationIndex < _mitaAnimations.size() && _mitaAnimator) {
        _currentMitaAnimationIndex = animationIndex;
        _mitaAnimator->PlayAnimation(_mitaAnimations[animationIndex].get());
        std::cout << "Switched mita to animation " << animationIndex << std::endl;
    }
}

void FinalSceneApp::updateCameraPointLights() {
    // Clear existing camera point lights
    _cameraPointLights.clear();
    
    // Get current camera position in world coordinates
    glm::vec3 cameraPos = _camera->transform.position;
    
    // Convert world position to map coordinates
    // First, map world coordinates to [-150, 150] range (tiled map)
    float wrappedX = fmod(cameraPos.x + 150.0f, 300.0f) - 150.0f;
    float wrappedZ = fmod(cameraPos.z + 150.0f, 300.0f) - 150.0f;
    if (wrappedX < -150.0f) wrappedX += 300.0f;
    if (wrappedZ < -150.0f) wrappedZ += 300.0f;
    
    // Then convert to array indices [0, 299]
    int centerMapX = static_cast<int>(wrappedX + 150);
    int centerMapZ = static_cast<int>(wrappedZ + 150);
    
    // Clamp to valid array bounds (should not be needed now, but for safety)
    centerMapX = std::max(0, std::min(centerMapX, MAP_LENGTH - 1));
    centerMapZ = std::max(0, std::min(centerMapZ, MAP_LENGTH - 1));
    
    // Define search area (11x11 around camera)
    const int searchRadius = 5; // 11x11 area
    
    // Debug output disabled for camera light detection
    bool shouldDebug = false; // Disable camera light debug output
    
    if (shouldDebug) {
        std::cout << "=== Camera Point Lights Debug ===" << std::endl;
        std::cout << "Camera world position: (" << std::fixed << std::setprecision(1) 
                  << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
        std::cout << "Camera wrapped position: (" << std::fixed << std::setprecision(1) 
                  << wrappedX << ", " << cameraPos.y << ", " << wrappedZ << ")" << std::endl;
        std::cout << "Camera map position: (" << centerMapX << ", " << centerMapZ << ")" << std::endl;
        std::cout << "Search area: map(" << (centerMapX - searchRadius) << "," << (centerMapZ - searchRadius) 
                  << ") to map(" << (centerMapX + searchRadius) << "," << (centerMapZ + searchRadius) << ")" << std::endl;
        
        // Print 11x11 map grid
        std::cout << "11x11 Map grid around camera:" << std::endl;
        std::cout << "   ";
        for (int dx = -searchRadius; dx <= searchRadius; dx++) {
            std::cout << std::setw(2) << (centerMapX + dx) % 100 << " ";
        }
        std::cout << std::endl;
        
        for (int dz = -searchRadius; dz <= searchRadius; dz++) {
            int mapZ = centerMapZ + dz;
            std::cout << std::setw(2) << mapZ % 100 << ":";
            for (int dx = -searchRadius; dx <= searchRadius; dx++) {
                int mapX = centerMapX + dx;
                if (mapX >= 0 && mapX < MAP_LENGTH && mapZ >= 0 && mapZ < MAP_LENGTH) {
                    std::cout << std::setw(2) << static_cast<int>(map[mapX][mapZ]) << " ";
                } else {
                    std::cout << " X ";
                }
            }
            std::cout << std::endl;
        }
        
        // lightDebugTimer = 0.0f;
    }
    
    // Scan the area around the camera
    for (int dx = -searchRadius; dx <= searchRadius; dx++) {
        for (int dz = -searchRadius; dz <= searchRadius; dz++) {
            int mapX = centerMapX + dx;
            int mapZ = centerMapZ + dz;
            
            // Check bounds
            if (mapX >= 0 && mapX < MAP_LENGTH && mapZ >= 0 && mapZ < MAP_LENGTH) {
                // Check if this position has a light (value == 2)
                if (map[mapX][mapZ] == 2) {
                    // Convert map coordinates back to world coordinates
                    // First get the standard map coordinate
                    float standardWorldX = static_cast<float>(mapX) - 150.0f;
                    float standardWorldZ = static_cast<float>(mapZ) - 150.0f;
                    
                    // Then adjust to the same "tile" as the camera
                    // Calculate which 300x300 tile the camera is in
                    int cameraTileX = static_cast<int>(floor((cameraPos.x + 150.0f) / 300.0f));
                    int cameraTileZ = static_cast<int>(floor((cameraPos.z + 150.0f) / 300.0f));
                    
                    // Place light in the same tile as the camera
                    float worldX = standardWorldX + cameraTileX * 300.0f;
                    float worldZ = standardWorldZ + cameraTileZ * 300.0f;
                    float worldY = 3.0f; // Height as specified
                    
                    // Create point light at this position
                    glm::vec3 lightPos(worldX, worldY, worldZ);
                    _cameraPointLights.emplace_back(lightPos, glm::vec3(1.0f, 1.0f, 0.9f), 2.0f);
                    
                    if (shouldDebug) {
                        std::cout << "Found light at map(" << mapX << ", " << mapZ << ") -> world(" 
                                  << worldX << ", " << worldY << ", " << worldZ << ")" << std::endl;
                    }
                    
                    // Limit number of lights to prevent performance issues
                    if (_cameraPointLights.size() >= MAX_POINT_LIGHTS) {
                        if (shouldDebug) {
                            std::cout << "Reached maximum point lights limit for camera: " << MAX_POINT_LIGHTS << std::endl;
                        }
                        return;
                    }
                }
            }
        }
    }
    
    if (shouldDebug) {
        std::cout << "Total camera point lights: " << _cameraPointLights.size() << std::endl;
        for (int i = 0; i < _cameraPointLights.size(); i++) {
            const auto& light = _cameraPointLights[i];
            std::cout << "  Camera Light " << i << ": world(" << std::fixed << std::setprecision(1)
                      << light.position.x << ", " << light.position.y << ", " << light.position.z 
                      << ") intensity=" << light.intensity << std::endl;
        }
    }
}

void FinalSceneApp::setCameraPointLightsUniforms(GLSLProgram* shader) {
    // Set number of point lights for camera
    int numLights = static_cast<int>(std::min(_cameraPointLights.size(), static_cast<size_t>(MAX_POINT_LIGHTS)));
    shader->setUniformInt("numPointLights", numLights);
    
    // Set each point light's properties
    for (int i = 0; i < numLights; i++) {
        const auto& light = _cameraPointLights[i];
        
        std::string baseName = "pointLights[" + std::to_string(i) + "]";
        shader->setUniformVec3(baseName + ".position", light.position);
        shader->setUniformVec3(baseName + ".color", light.color);
        shader->setUniformFloat(baseName + ".intensity", light.intensity);
    }
}