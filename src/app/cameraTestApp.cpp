#include "cameraTestApp.hpp"
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

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

// Number of models to create
const int MODEL_COUNT = 7;
// Space between models
const float MODEL_SPACING = 3.0f;
 
CameraTestApp::CameraTestApp(const Options &options) : Application(options) {
    // Set input mode to capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // Initialize advanced camera
    _advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));

    // Initialize clipping planes with camera's current values
    _nearPlane = _advCamera->getNearClippingPlane();
    _farPlane = _advCamera->getFarClippingPlane();

    // Setup models in a row
    setupModels();

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

void CameraTestApp::setupModels() {
    // Clear any existing models
    _models.clear();
    
    // Create model instances and position them in a row
    float startX = -(MODEL_COUNT - 1) * MODEL_SPACING / 2.0f;  // Center the row
    
    for (int i = 0; i < MODEL_COUNT; ++i) {
        // Create new model
        auto model = std::make_unique<Model>(getAssetFullPath(modelPath));
        
        // Position model in a row
        float x = startX + i * MODEL_SPACING;
        model->transform.position = glm::vec3(x, 0.0f, 0.0f);
        
        // Add some variation to models
        // Scale models differently (smaller to larger)
        float scale = 0.5f + (0.5f * i / (MODEL_COUNT - 1));
        model->transform.scale = glm::vec3(scale);
        
        // Rotate every other model slightly for variation
        if (i % 2 == 1) {
            model->transform.rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        
        // Add model to collection
        _models.push_back(std::move(model));
    }
    
    std::cout << "Created " << MODEL_COUNT << " models in a row" << std::endl;
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
            // Set orbit target to the center of the model row
            glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);
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
            // Create a combined bounding box for all models
            BoundingBox combinedBBox;
            
            for (const auto& model : _models) {
                // Get model's bounding box
                BoundingBox modelBBox = model->getBoundingBox();
                
                // Apply model transform to the bounding box
                glm::mat4 modelMatrix = model->transform.getLocalMatrix();
                modelBBox.transform(modelMatrix);
                
                // Expand the combined bounding box
                if (combinedBBox.isEmpty()) {
                    combinedBBox = modelBBox;
                } else {
                    combinedBBox += modelBBox;  // Use the += operator instead of expand method
                }
            }
            
            // Start zoom to fit animation - use a slightly larger padding for safety
            _advCamera->zoomToFit(combinedBBox, 1.5f);
            
            std::cout << "Zoom To Fit initiated. Press F to fit all models in view." << std::endl;
        } else {
            std::cout << "Camera is already animating. Please wait for animation to complete." << std::endl;
        }
        
        // Reset key state
        _input.keyboard.keyStates[GLFW_KEY_F] = GLFW_RELEASE;
    }

    // Clipping Plane Adjustment Keys

    // Z/X: Adjust near clipping plane
    if (_input.keyboard.keyStates[GLFW_KEY_Z] != GLFW_RELEASE) {
        // Decrease near plane (move it closer to camera)
        _nearPlane = std::max(0.01f, _nearPlane - _planeAdjustSpeed * _deltaTime);
        _advCamera->setNearClippingPlane(_nearPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_X] != GLFW_RELEASE) {
        // Increase near plane (move it farther from camera)
        _nearPlane += _planeAdjustSpeed * _deltaTime;
        _advCamera->setNearClippingPlane(_nearPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    // C/V: Adjust far clipping plane
    if (_input.keyboard.keyStates[GLFW_KEY_C] != GLFW_RELEASE) {
        // Decrease far plane (move it closer to camera)
        _farPlane = std::max(_nearPlane * 1.1f, _farPlane - _planeAdjustSpeed * 10.0f * _deltaTime);
        _advCamera->setFarClippingPlane(_farPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_V] != GLFW_RELEASE) {
        // Increase far plane (move it farther from camera)
        _farPlane += _planeAdjustSpeed * 10.0f * _deltaTime;
        _advCamera->setFarClippingPlane(_farPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }

    // R: Reset clipping planes to default values
    if (_input.keyboard.keyStates[GLFW_KEY_R] == GLFW_PRESS) {
        _nearPlane = 1.0f;
        _farPlane = 15.0f;
        _advCamera->setNearClippingPlane(_nearPlane);
        _advCamera->setFarClippingPlane(_farPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
        
        std::cout << "Reset clipping planes to default values" << std::endl;
        _input.keyboard.keyStates[GLFW_KEY_R] = GLFW_RELEASE;
    }
    
    // Update info display time
    if (_showClippingInfo) {
        _clippingInfoDisplayTime += _deltaTime;
        if (_clippingInfoDisplayTime > _clippingInfoTimeout) {
            _showClippingInfo = false;
        }
    }

    // Process all camera input through the advanced camera
    _advCamera->processInput(_input);
    
    // Update window title to show camera mode
    std::string modeStr = "Camera Mode: " + _advCamera->getModeString();
    if (_advCamera->isAnimating()) {
        modeStr += " (Animating)";
    }
    glfwSetWindowTitle(_window, (modeStr + " - Use Z/X to adjust near plane, C/V for far plane, R to reset").c_str());

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

    // Render all models
    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);

    for (const auto& model : _models) {
        _shader->setUniformMat4("model", model->transform.getLocalMatrix());
        model->draw();
    }

    // Render skybox with a custom projection matrix (fixed clipping planes)
    // Extract field of view from the camera for the skybox projection
    float fov = _advCamera->getFOV();
    float aspect = (float)_windowWidth / (float)_windowHeight;
    
    // Create a skybox projection matrix with fixed near/far planes
    // This ensures skybox is always rendered at maximum distance
    glm::mat4 skyboxProjection = glm::perspective(fov, aspect, 0.1f, 1000.0f);
    
    // Remove translation from view matrix (only rotation for skybox)
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    
    // Render skybox with fixed projection matrix
    _skybox->draw(skyboxProjection, skyboxView);
    
    // Display clipping plane info on screen if needed
    if (_showClippingInfo) {
        displayClippingPlaneInfo();
    }
}

void CameraTestApp::displayClippingPlaneInfo() {
    // This is a placeholder for actual on-screen display
    // In a real implementation, you would render text or UI elements
    // Since we don't have text rendering set up, we'll just print to console
    
    // Format the message with proper precision
    std::stringstream ss;
    ss << "Clipping Planes: Near = " << std::fixed << std::setprecision(2) << _nearPlane 
       << ", Far = " << std::fixed << std::setprecision(2) << _farPlane;
    
    // Print to console - in a real app, this would be rendered as text on screen
    static std::string lastMessage = "";
    if (ss.str() != lastMessage) {
        std::cout << ss.str() << std::endl;
        lastMessage = ss.str();
    }
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