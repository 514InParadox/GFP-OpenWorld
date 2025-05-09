#include "cameraTestApp.hpp"

// Model path
const std::string modelPath = "resource/model/MonkeyHead.obj";

// Shader file paths
const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";

// Skybox textures
const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};
 
CameraTestApp::CameraTestApp(const Options &options) : Application(options) {
    // Set input mode to capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // Initialize advanced camera
    _advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));

    // Initialize the model
    _model.reset(new Model(getAssetFullPath(modelPath)));

    // Initialize shader
    initShader();

    // Initialize skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));
    
    // Initialize frame time
    _lastFrameTime = std::chrono::high_resolution_clock::now();
}

void CameraTestApp::handleInput() { 
    // Update frame time for animation
    updateFrameTime();
    
    // Update camera animation if active
    if (_advCamera->isAnimating()) {
        _advCamera->updateAnimation(_deltaTime);
        // Skip other input processing while animating
        _input.forwardState();
        return;
    }

    // Exit if ESC is pressed
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // Switch camera mode using number keys
    // Key 1: FREE_ROAM mode
    if (_input.keyboard.keyStates[GLFW_KEY_1] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::FREE_ROAM) {
            _advCamera->setMode(CameraMode::FREE_ROAM);
            std::cout << "Switched to FREE_ROAM mode - movement and rotation" << std::endl;
            // Reset key state
            _input.keyboard.keyStates[GLFW_KEY_1] = GLFW_RELEASE;
        }
    }
    
    // Key 2: FIXED mode
    if (_input.keyboard.keyStates[GLFW_KEY_2] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::FIXED) {
            _advCamera->setMode(CameraMode::FIXED);
            std::cout << "Switched to FIXED mode - rotation only" << std::endl;
            // Reset key state
            _input.keyboard.keyStates[GLFW_KEY_2] = GLFW_RELEASE;
        }
    }
    
    // Key 3: PAN mode
    if (_input.keyboard.keyStates[GLFW_KEY_3] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::PAN) {
            _advCamera->setMode(CameraMode::PAN);
            std::cout << "Switched to PAN mode - parallel movement only" << std::endl;
            // Reset key state
            _input.keyboard.keyStates[GLFW_KEY_3] = GLFW_RELEASE;
        }
    }
    
    // Key 4: ORBIT mode
    if (_input.keyboard.keyStates[GLFW_KEY_4] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::ORBIT) {
            // First set orbit target to model position (origin for the monkey head model)
            glm::vec3 targetPosition = _model->transform.position;
            _advCamera->setOrbitTarget(targetPosition);
            
            // Then switch to ORBIT mode
            _advCamera->setMode(CameraMode::ORBIT);
            std::cout << "Switched to ORBIT mode - rotate around the model" << std::endl;
            
            // Update orbit camera position and orientation
            _advCamera->updateOrbitPosition();
            
            // Reset key state
            _input.keyboard.keyStates[GLFW_KEY_4] = GLFW_RELEASE;
        }
    }
    
    // Key F: Zoom To Fit
    if (_input.keyboard.keyStates[GLFW_KEY_F] == GLFW_PRESS) {
        // Only process if camera is not already animating
        if (!_advCamera->isAnimating()) {
            // Get model's bounding box
            BoundingBox modelBBox = _model->getBoundingBox();
            
            // Apply model transform to the bounding box
            glm::mat4 modelMatrix = _model->transform.getLocalMatrix();
            modelBBox.transform(modelMatrix);
            
            // Start zoom to fit animation - use a slightly larger padding for safety
            _advCamera->zoomToFit(modelBBox, 1.5f);
            
            std::cout << "Zoom To Fit initiated. Press F to fit model in view." << std::endl;
        } else {
            std::cout << "Camera is already animating. Please wait for animation to complete." << std::endl;
        }
        
        // Reset key state
        _input.keyboard.keyStates[GLFW_KEY_F] = GLFW_RELEASE;
    }

    // Process all camera input through the advanced camera
    _advCamera->processInput(_input);
    
    // Update window title to show camera mode
    std::string modeStr = "Camera Mode: " + _advCamera->getModeString();
    if (_advCamera->isAnimating()) {
        modeStr += " (Animating)";
    }
    glfwSetWindowTitle(_window, (modeStr + " - Press 1/2/3/4 to switch modes, F to Zoom To Fit").c_str());

    // Update input state
    _input.forwardState();
}

void CameraTestApp::renderFrame() {   
    // Clear the screen
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get camera matrices from the advanced camera
    Camera* camera = _advCamera->getCamera();
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();

    // Render model
    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);
    _shader->setUniformMat4("model", _model->transform.getLocalMatrix());

    _model->draw();

    // Render skybox
    _skybox->draw(projection, view);
}

void CameraTestApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}

void CameraTestApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    _lastFrameTime = currentTime;
} 