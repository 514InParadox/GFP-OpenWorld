#include "lightingTestApp2.hpp"
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

#define MAX_LIGHT 3

// Model path
const std::string modelPath = "resource/model/MonkeyHead.obj";

// Shader file paths
const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
//const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";
const std::string fragmentShaderAddr = "shader/fragment/lighting.frag";

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
 
lightingTestApp2::lightingTestApp2(const Options &options) : Application(options) {
    // Set input mode to capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // Initialize advanced camera
    _advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));

    // Initialize clipping planes with camera's current values
    _nearPlane = _advCamera->getNearPlane();
    _farPlane = _advCamera->getFarPlane();
    
    // Set initial clipping planes to match the values in the header file
    // This ensures consistency between initial and reset values
    _advCamera->setNearPlane(1.0f);  // Match the value used in reset
    _advCamera->setFarPlane(15.0f);  // Match the value used in reset
    _nearPlane = 1.0f;  // Update our local tracking variables
    _farPlane = 15.0f;

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
    
    // Initialize light
    _activeLight = nullptr;
    initLight();

    // Initialize testing material
    _testMaterial = std::make_unique<Material>();
    
    // Initialize frame time
    _lastFrameTime = std::chrono::high_resolution_clock::now();
}

void lightingTestApp2::setupModels() {
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

void lightingTestApp2::handleInput() { 
    constexpr float lightMoveSpeed = 2.0f;
    constexpr float intensity_speed = 0.02f;
    constexpr float cameraRotateSpeed = 0.002f;
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
        _advCamera->setNearPlane(_nearPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_X] != GLFW_RELEASE) {
        // Increase near plane (move it farther from camera)
        _nearPlane += _planeAdjustSpeed * _deltaTime;
        _advCamera->setNearPlane(_nearPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    // C/V: Adjust far clipping plane
    if (_input.keyboard.keyStates[GLFW_KEY_C] != GLFW_RELEASE) {
        // Decrease far plane (move it closer to camera)
        _farPlane = std::max(_nearPlane * 1.1f, _farPlane - _planeAdjustSpeed * 10.0f * _deltaTime);
        _advCamera->setFarPlane(_farPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_V] != GLFW_RELEASE) {
        // Increase far plane (move it farther from camera)
        _farPlane += _planeAdjustSpeed * 10.0f * _deltaTime;
        _advCamera->setFarPlane(_farPlane);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }

    // N/M: Adjust field of view (FOV)
    if (_input.keyboard.keyStates[GLFW_KEY_N] != GLFW_RELEASE) {
        // Decrease FOV (zoom in)
        float currentFOV = _advCamera->getFOV();
        float newFOV = currentFOV - glm::radians(_fovAdjustSpeed * _deltaTime);
        _advCamera->setFOV(newFOV);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_M] != GLFW_RELEASE) {
        // Increase FOV (zoom out)
        float currentFOV = _advCamera->getFOV();
        float newFOV = currentFOV + glm::radians(_fovAdjustSpeed * _deltaTime);
        _advCamera->setFOV(newFOV);
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
    }

    // R: Reset clipping planes to default values
    if (_input.keyboard.keyStates[GLFW_KEY_R] == GLFW_PRESS) {
        _nearPlane = 1.0f;
        _farPlane = 15.0f;
        _advCamera->setNearPlane(_nearPlane);
        _advCamera->setFarPlane(_farPlane);
        
        // Reset FOV to default (45 degrees)
        _advCamera->setFOV(glm::radians(45.0f));
        
        _showClippingInfo = true;
        _clippingInfoDisplayTime = 0.0f;
        
        std::cout << "Reset camera parameters to default values: Near = " << _nearPlane 
                  << ", Far = " << _farPlane 
                  << ", FOV = 45" << std::endl;
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
    glfwSetWindowTitle(_window, (modeStr + " - Use Z/X to adjust near plane, C/V for far plane, N/M for FOV, R to reset").c_str());

    // Update input state
    _input.forwardState();

    // _lights[0] = std::make_unique<PointLight>();
    // _lights[1] = std::make_unique<DirectionalLight>();
    // _lights[2] = std::make_unique<SpotLight>();

    // I/O/P Change the light that is using (direction-punctual-spotlight)
    if(_input.keyboard.keyStates[GLFW_KEY_I] != GLFW_RELEASE){  // Change to directional light
        _activeLight = _lights[1].get();
        std::cerr << "Switch to control directional light" << std::endl;
    }
    if(_input.keyboard.keyStates[GLFW_KEY_O] != GLFW_RELEASE){  // Change to point light
        _activeLight = _lights[0].get();
        std::cerr << "Switch to control point light" << std::endl;
    }
    if(_input.keyboard.keyStates[GLFW_KEY_P] != GLFW_RELEASE){  // Change to spotlight
        _activeLight = _lights[2].get();
        std::cerr << "Switch to control spot light" << std::endl;
    }

    // K/L: Light intensity adjustment
    if(_input.keyboard.keyStates[GLFW_KEY_K] != GLFW_RELEASE){  // 
        _activeLight->intensity += intensity_speed;
        if(_activeLight->intensity > 1.0f) _activeLight->intensity = 1.0f;
    }
    if(_input.keyboard.keyStates[GLFW_KEY_L] != GLFW_RELEASE){  // Change to spotlight
        _activeLight->intensity -= intensity_speed;
        if(_activeLight->intensity < 0.0f) _activeLight->intensity = 0.0f;
    }

    // ARROW_KEYS/,/. : (For Point/Spotlight)Light position adjustment. 
    if (_input.keyboard.keyStates[GLFW_KEY_UP] != GLFW_RELEASE) {
        _activeLight->transform.position = _activeLight->transform.position + lightMoveSpeed * _activeLight->transform.getDefaultUp();
        std::cerr << "Light move up to (" 
        << _activeLight->transform.position.x << ", " 
        << _activeLight->transform.position.y << ", "
        << _activeLight->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE) {
        _activeLight->transform.position = _activeLight->transform.position - lightMoveSpeed * _activeLight->transform.getDefaultRight();
        std::cerr << "Light move left to (" 
        << _activeLight->transform.position.x << ", " 
        << _activeLight->transform.position.y << ", "
        << _activeLight->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) {
        _activeLight->transform.position = _activeLight->transform.position - lightMoveSpeed * _activeLight->transform.getDefaultUp();
        std::cerr << "Light move down to (" 
        << _activeLight->transform.position.x << ", " 
        << _activeLight->transform.position.y << ", "
        << _activeLight->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE) {
        _activeLight->transform.position = _activeLight->transform.position + lightMoveSpeed * _activeLight->transform.getDefaultRight();
        std::cerr << "Light move right to (" 
        << _activeLight->transform.position.x << ", " 
        << _activeLight->transform.position.y << ", "
        << _activeLight->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_COMMA] != GLFW_RELEASE) {
        _activeLight->transform.position = _activeLight->transform.position - lightMoveSpeed * _activeLight->transform.getDefaultFront();
        std::cerr << "Light move nearar to (" 
        << _activeLight->transform.position.x << ", " 
        << _activeLight->transform.position.y << ", "
        << _activeLight->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_PERIOD] != GLFW_RELEASE) {
        _activeLight->transform.position = _activeLight->transform.position + lightMoveSpeed * _activeLight->transform.getDefaultFront();
        std::cerr << "Light move farther to (" 
        << _activeLight->transform.position.x << ", " 
        << _activeLight->transform.position.y << ", "
        << _activeLight->transform.position.z << ")"
        << std::endl;
    }

    // Y/G/H/J: (For Directional/Spot Light) angle adjustment
    if (_input.keyboard.keyStates[GLFW_KEY_Y] != GLFW_RELEASE) {
        _activeLight->transform.rotation = glm::rotate(_activeLight->transform.rotation, cameraRotateSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 eulerAngles = glm::eulerAngles( _activeLight->transform.rotation);
        std::cerr << "Light angle move upwards to (" 
        << eulerAngles.x << ", " 
        << eulerAngles.y << ", "
        << eulerAngles.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_H] != GLFW_RELEASE) {
        _activeLight->transform.rotation = glm::rotate(_activeLight->transform.rotation, -cameraRotateSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 eulerAngles = glm::eulerAngles( _activeLight->transform.rotation);
        std::cerr << "Light angle move downwards to (" 
        << eulerAngles.x << ", " 
        << eulerAngles.y << ", "
        << eulerAngles.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_G] != GLFW_RELEASE) {
        _activeLight->transform.rotation = glm::rotate(_activeLight->transform.rotation, -cameraRotateSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 eulerAngles = glm::eulerAngles( _activeLight->transform.rotation);
        std::cerr << "Light angle move leftwards to (" 
        << eulerAngles.x << ", " 
        << eulerAngles.y << ", "
        << eulerAngles.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_J] != GLFW_RELEASE) {
        _activeLight->transform.rotation = glm::rotate(_activeLight->transform.rotation, cameraRotateSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 eulerAngles = glm::eulerAngles( _activeLight->transform.rotation);
        std::cerr << "Light angle move rightwards to (" 
        << eulerAngles.x << ", " 
        << eulerAngles.y << ", "
        << eulerAngles.z << ")"
        << std::endl;
    }


}

void lightingTestApp2::renderFrame() {   
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

    // material set
    _shader->setUniformVec3("material.ambient",_testMaterial->ambient);
    _shader->setUniformVec3("material.diffuse",_testMaterial->diffuse);
    _shader->setUniformVec3("material.specular",_testMaterial->specular);
    _shader->setUniformFloat("material.shininess",_testMaterial->shininess);
    _shader->setUniformVec3("material.color",_testMaterial->materialColor);
    
    updateLight();
    
    _shader->setUniformVec3("viewPosition",camera->transform.position);
    
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

void lightingTestApp2::displayClippingPlaneInfo() {
    // This is a placeholder for actual on-screen display
    // In a real implementation, you would render text or UI elements
    // Since we don't have text rendering set up, we'll just print to console
    
    // Format the message with proper precision
    std::stringstream ss;
    ss << "Clipping Planes: Near = " << std::fixed << std::setprecision(2) << _nearPlane 
       << ", Far = " << std::fixed << std::setprecision(2) << _farPlane
       << " | FOV = " << std::fixed << std::setprecision(1) << glm::degrees(_advCamera->getFOV());
    
    // Print to console - in a real app, this would be rendered as text on screen
    static std::string lastMessage = "";
    if (ss.str() != lastMessage) {
        std::cout << ss.str() << std::endl;
        lastMessage = ss.str();
    }
}

void lightingTestApp2::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}

void lightingTestApp2::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    _lastFrameTime = currentTime;
} 

void lightingTestApp2::initLight(){
    _ambientLight = std::make_unique<AmbientLight>();

    _lights.resize(MAX_LIGHT); // MAX_LIGHT = 3;

    _lights[0] = std::make_unique<PointLight>();
    _lights[1] = std::make_unique<DirectionalLight>();
    _lights[2] = std::make_unique<SpotLight>();

    _activeLight = _lights[0].get(); // 默认激活 point light

    updateLight();   
}

void lightingTestApp2::updateLight(){

    // update ambient light, default on
    _shader->setUniformVec3("ambientLight.color",_ambientLight->color);
    _shader->setUniformFloat("ambientLight.intensity",_ambientLight->intensity);

    // The following three type of lights are switched by key

    for(const auto& lightPtr : _lights){
        // update pointLight
        if (PointLight* _pointLight = dynamic_cast<PointLight*>(lightPtr.get())){
            _shader->setUniformVec3("pointLights.position",_pointLight->transform.position);
            _shader->setUniformVec3("pointLights.color",_pointLight->color);
            _shader->setUniformFloat("pointLights.intensity",_pointLight->intensity);
            // _shader->setUniformFloat("pointLights.kc",_pointLight->kc);
            // _shader->setUniformFloat("pointLights.kq",_pointLight->kq);
            // _shader->setUniformFloat("pointLights.kl",_pointLight->kl);
            // std::cerr << "pointlight updated" << std::endl;
        }
        
        // update dirLight
        if (DirectionalLight* _dirLight = dynamic_cast<DirectionalLight*>(lightPtr.get())){
            _shader->setUniformVec3("dirLights.direction",_dirLight->transform.getFront());
            _shader->setUniformVec3("dirLights.color",_dirLight->color);
            _shader->setUniformFloat("dirLights.intensity",_dirLight->intensity);
            // std::cerr << "dirlight updated" << std::endl;
        }
        
        // update spotLight
        if (SpotLight* _spotLight = dynamic_cast<SpotLight*>(lightPtr.get())){
            _shader->setUniformVec3("spotLights.position",_spotLight->transform.position);
            _shader->setUniformVec3("spotLights.direction",_spotLight->transform.getFront());
            _shader->setUniformVec3("spotLights.color",_spotLight->color);
            _shader->setUniformFloat("spotLights.intensity",_spotLight->intensity);
            _shader->setUniformFloat("spotLights.cutOff",_spotLight->cutOff);
            // _shader->setUniformFloat("spotLights.innerCutOff",_spotLight->innerCutOff);
            // _shader->setUniformFloat("spotLights.kc",_spotLight->kc);
            // _shader->setUniformFloat("spotLights.kq",_spotLight->kq);
            // _shader->setUniformFloat("spotLights.kl",_spotLight->kl);
            // std::cerr << "spotlight updated" << std::endl;
        }

    }

}

