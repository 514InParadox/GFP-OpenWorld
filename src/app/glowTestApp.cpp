#include "glowTestApp.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Model and shader paths
const std::string modelPath = "resource/model/MonkeyHead.obj";
const std::string lightModelPath = "resource/model/cube.obj";
const std::string vertexShaderAddr = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/lighting.frag";
const std::string screenQuadVertexShader = "shader/vertex/screen_quad.vert";
const std::string emissiveShaderPath = "shader/fragment/emissive.frag";
const std::string extractBrightShaderPath = "shader/fragment/extract_bright.frag";
const std::string blurShaderPath = "shader/fragment/gaussian_blur.frag";
const std::string combineShaderPath = "shader/fragment/glow_combine.frag";

// Skybox textures
const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};

GlowTestApp::GlowTestApp(const Options &options) : Application(options) {
    // Set input mode to capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // Initialize advanced camera
    _advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));
    _advCamera->setNearPlane(0.1f);
    _advCamera->setFarPlane(100.0f);

    // Setup scene
    setupScene();
    
    // Initialize shaders
    initShaders();
    
    // Initialize framebuffers
    initFramebuffers();
    
    // Initialize screen quad
    initScreenQuad();
    
    // Initialize frame time
    _lastFrameTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "GlowTestApp initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  G - Toggle glow effect" << std::endl;
    std::cout << "  + - Increase light intensity" << std::endl;
    std::cout << "  - - Decrease light intensity" << std::endl;
    std::cout << "  [ - Decrease glow intensity" << std::endl;
    std::cout << "  ] - Increase glow intensity" << std::endl;
    std::cout << "  Arrow keys - Move light source" << std::endl;
    std::cout << "  Page Up/Down - Move light up/down" << std::endl;
}

void GlowTestApp::setupScene() {
    // Load monkey model
    _monkeyModel.reset(new Model(getAssetFullPath(modelPath)));
    _monkeyModel->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    _monkeyModel->transform.scale = glm::vec3(1.0f);
    
    // Load light sphere model
    _lightSphereModel.reset(new Model(getAssetFullPath(lightModelPath)));
    _lightSphereModel->transform.scale = glm::vec3(0.2f); // Small sphere for light
    
    // Initialize lights
    _ambientLight.reset(new AmbientLight());
    _ambientLight->color = glm::vec3(0.1f, 0.1f, 0.1f);
    _ambientLight->intensity = 0.3f;
    
    _pointLight.reset(new PointLight());
    _pointLight->transform.position = glm::vec3(2.0f, 2.0f, 2.0f);
    _pointLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
    _pointLight->intensity = _lightIntensity;
    
    // Initialize material
    _testMaterial.reset(new Material());
    _testMaterial->ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    _testMaterial->diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    _testMaterial->specular = glm::vec3(1.0f, 1.0f, 1.0f);
    _testMaterial->shininess = 32.0f;
    _testMaterial->materialColor = glm::vec3(0.7f, 0.5f, 0.3f); // Brown monkey color
    
    // Initialize light material (emissive)
    _lightMaterial.reset(new Material());
    _lightMaterial->ambient = glm::vec3(1.0f, 1.0f, 1.0f);
    _lightMaterial->diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    _lightMaterial->specular = glm::vec3(1.0f, 1.0f, 1.0f);
    _lightMaterial->shininess = 32.0f;
    _lightMaterial->materialColor = glm::vec3(3.0f, 3.0f, 3.0f); // Bright white for glow effect
    
    // Skip skybox initialization for better glow visibility
}

void GlowTestApp::initShaders() {
    // Scene shader
    _sceneShader.reset(new GLSLProgram);
    _sceneShader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _sceneShader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _sceneShader->link();
    
    // Emissive shader for light source
    _emissiveShader.reset(new GLSLProgram);
    _emissiveShader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _emissiveShader->attachFragmentShaderFromFile(getAssetFullPath(emissiveShaderPath));
    _emissiveShader->link();
    
    // Extract bright shader
    _extractBrightShader.reset(new GLSLProgram);
    _extractBrightShader->attachVertexShaderFromFile(getAssetFullPath(screenQuadVertexShader));
    _extractBrightShader->attachFragmentShaderFromFile(getAssetFullPath(extractBrightShaderPath));
    _extractBrightShader->link();
    
    // Blur shader
    _blurShader.reset(new GLSLProgram);
    _blurShader->attachVertexShaderFromFile(getAssetFullPath(screenQuadVertexShader));
    _blurShader->attachFragmentShaderFromFile(getAssetFullPath(blurShaderPath));
    _blurShader->link();
    
    // Combine shader
    _combineShader.reset(new GLSLProgram);
    _combineShader->attachVertexShaderFromFile(getAssetFullPath(screenQuadVertexShader));
    _combineShader->attachFragmentShaderFromFile(getAssetFullPath(combineShaderPath));
    _combineShader->link();
}

void GlowTestApp::initFramebuffers() {
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

void GlowTestApp::initScreenQuad() {
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

void GlowTestApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    _lastFrameTime = currentTime;
}

void GlowTestApp::handleInput() {
    updateFrameTime();
    
    // Update camera animation if active
    if (_advCamera->isAnimating()) {
        _advCamera->updateAnimation(_deltaTime);
        _input.forwardState();
        return;
    }

    // Exit if ESC is pressed
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // Toggle glow effect
    if (_input.keyboard.keyStates[GLFW_KEY_G] == GLFW_PRESS) {
        _enableGlow = !_enableGlow;
        std::cout << "Glow effect: " << (_enableGlow ? "ON" : "OFF") << std::endl;
        _input.keyboard.keyStates[GLFW_KEY_G] = GLFW_RELEASE;
    }
    
    // Adjust light intensity
    if (_input.keyboard.keyStates[GLFW_KEY_EQUAL] != GLFW_RELEASE) { // + key
        _lightIntensity += 0.1f;
        _pointLight->intensity = _lightIntensity;
        std::cout << "Light intensity: " << _lightIntensity << std::endl;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_MINUS] != GLFW_RELEASE) { // - key
        _lightIntensity = std::max(0.1f, _lightIntensity - 0.1f);
        _pointLight->intensity = _lightIntensity;
        std::cout << "Light intensity: " << _lightIntensity << std::endl;
    }
    
    // Adjust glow intensity
    if (_input.keyboard.keyStates[GLFW_KEY_LEFT_BRACKET] != GLFW_RELEASE) { // [ key
        _glowIntensity = std::max(0.1f, _glowIntensity - 0.1f);
        std::cout << "Glow intensity: " << _glowIntensity << std::endl;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_RIGHT_BRACKET] != GLFW_RELEASE) { // ] key
        _glowIntensity += 0.1f;
        std::cout << "Glow intensity: " << _glowIntensity << std::endl;
    }
    
    // Light position controls
    constexpr float lightMoveSpeed = 3.0f;
    if (_input.keyboard.keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE) {
        _pointLight->transform.position.x -= lightMoveSpeed * _deltaTime;
    }
    if (_input.keyboard.keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE) {
        _pointLight->transform.position.x += lightMoveSpeed * _deltaTime;
    }
    if (_input.keyboard.keyStates[GLFW_KEY_UP] != GLFW_RELEASE) {
        _pointLight->transform.position.z -= lightMoveSpeed * _deltaTime;
    }
    if (_input.keyboard.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) {
        _pointLight->transform.position.z += lightMoveSpeed * _deltaTime;
    }
    if (_input.keyboard.keyStates[GLFW_KEY_PAGE_UP] != GLFW_RELEASE) {
        _pointLight->transform.position.y += lightMoveSpeed * _deltaTime;
    }
    if (_input.keyboard.keyStates[GLFW_KEY_PAGE_DOWN] != GLFW_RELEASE) {
        _pointLight->transform.position.y -= lightMoveSpeed * _deltaTime;
    }
    
    // Camera controls
    constexpr float cameraSpeed = 5.0f;
    constexpr float cameraRotateSpeed = 0.002f;
    
    Camera* camera = _advCamera->getCamera();
    
    // Movement
    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        camera->transform.position += cameraSpeed * _deltaTime * camera->transform.getFront();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        camera->transform.position -= cameraSpeed * _deltaTime * camera->transform.getFront();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        camera->transform.position -= cameraSpeed * _deltaTime * camera->transform.getRight();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        camera->transform.position += cameraSpeed * _deltaTime * camera->transform.getRight();
    }
    
    // Mouse look
    float xOffset = (_input.mouse.move.xNow - _input.mouse.move.xOld) * cameraRotateSpeed;
    float yOffset = (_input.mouse.move.yOld - _input.mouse.move.yNow) * cameraRotateSpeed;
    
    camera->transform.rotation = glm::rotate(camera->transform.rotation, -xOffset, glm::vec3(0.0f, 1.0f, 0.0f));
    camera->transform.rotation = glm::rotate(camera->transform.rotation, -yOffset, camera->transform.getRight());
    
    _input.forwardState();
}

void GlowTestApp::renderFrame() {
    // 1. Render scene to framebuffer
    renderScene();
    
    // 2. Extract bright colors
    renderBrightExtraction();
    
    // 3. Apply blur to bright colors
    renderBlur();
    
    // 4. Combine original scene with blurred bright colors
    renderFinalComposition();
}

void GlowTestApp::renderScene() {
    // Bind scene framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, _sceneFramebuffer);
    glViewport(0, 0, _windowWidth, _windowHeight);
    
    // Clear
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    // Get camera matrices
    Camera* camera = _advCamera->getCamera();
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();
    
    // Render monkey model
    _sceneShader->use();
    
    // Set material uniforms
    _sceneShader->setUniformVec3("material.ambient", _testMaterial->ambient);
    _sceneShader->setUniformVec3("material.diffuse", _testMaterial->diffuse);
    _sceneShader->setUniformVec3("material.specular", _testMaterial->specular);
    _sceneShader->setUniformFloat("material.shininess", _testMaterial->shininess);
    _sceneShader->setUniformVec3("material.color", _testMaterial->materialColor);
    
    // Set light uniforms
    _sceneShader->setUniformVec3("ambientLight.color", _ambientLight->color);
    _sceneShader->setUniformFloat("ambientLight.intensity", _ambientLight->intensity);
    
    _sceneShader->setUniformVec3("pointLights.position", _pointLight->transform.position);
    _sceneShader->setUniformVec3("pointLights.color", _pointLight->color);
    _sceneShader->setUniformFloat("pointLights.intensity", _pointLight->intensity);
    
    // Set camera position
    _sceneShader->setUniformVec3("viewPosition", camera->transform.position);
    
    // Set matrices
    _sceneShader->setUniformMat4("projection", projection);
    _sceneShader->setUniformMat4("view", view);
    _sceneShader->setUniformMat4("model", _monkeyModel->transform.getLocalMatrix());
    
    // Draw model
    _monkeyModel->draw();
    
    // Render light source as a bright sphere using emissive shader
    _lightSphereModel->transform.position = _pointLight->transform.position;
    
    _emissiveShader->use();
    _emissiveShader->setUniformMat4("projection", projection);
    _emissiveShader->setUniformMat4("view", view);
    _emissiveShader->setUniformMat4("model", _lightSphereModel->transform.getLocalMatrix());
    _emissiveShader->setUniformVec3("material.color", _pointLight->color);
    _emissiveShader->setUniformFloat("material.intensity", _pointLight->intensity);
    
    _lightSphereModel->draw();
}

void GlowTestApp::renderBrightExtraction() {
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

void GlowTestApp::renderBlur() {
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

void GlowTestApp::renderFinalComposition() {
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

void GlowTestApp::cleanupFramebuffers() {
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