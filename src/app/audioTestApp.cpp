#include "audioTestApp.hpp"
#include <iostream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

// Model and shader paths
const std::string floorModelPath = "resource/model/cube.obj"; // Use cube as floor
const std::string vertexShaderAddr = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";

AudioTestApp::AudioTestApp(const Options &options) : Application(options) {
    // Set input mode to capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // Initialize advanced camera
    _advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));
    _advCamera->setNearPlane(0.1f);
    _advCamera->setFarPlane(100.0f);
    
    // Set camera to FREE_ROAM mode for walking simulation
    _advCamera->setMode(CameraMode::FREE_ROAM);

    // Setup scene
    setupScene();
    
    // Initialize shader
    initShader();
    
    // Initialize audio system
    initAudio();
    
    // Initialize frame time and position tracking
    _lastFrameTime = std::chrono::high_resolution_clock::now();
    _lastPosition = _advCamera->getCamera()->transform.position;
    
    std::cout << "AudioTestApp initialized successfully!" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move around (mode dependent)" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  1 - FREE_ROAM mode (full movement)" << std::endl;
    std::cout << "  2 - FIXED mode (rotation only)" << std::endl;
    std::cout << "  3 - PAN mode (parallel movement)" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "Listen for footstep sounds while walking!" << std::endl;
}

AudioTestApp::~AudioTestApp() {
    cleanupAudio();
}

void AudioTestApp::setupScene() {
    // Create a large floor using the cube model
    _floorModel.reset(new Model(getAssetFullPath(floorModelPath)));
    
    // Scale the cube to make it a large flat floor
    _floorModel->transform.position = glm::vec3(0.0f, -1.0f, 0.0f); // Below the camera
    _floorModel->transform.scale = glm::vec3(20.0f, 0.1f, 20.0f); // Wide, thin floor
    
    std::cout << "Floor created: 20x20 units" << std::endl;
}

void AudioTestApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}

void AudioTestApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    _lastFrameTime = currentTime;
}

void AudioTestApp::initAudio() {
    // Initialize irrKlang sound engine
    _soundEngine = createIrrKlangDevice();
    
    if (!_soundEngine) {
        std::cerr << "Error: Could not initialize irrKlang audio engine!" << std::endl;
        std::cout << "Falling back to console simulation mode." << std::endl;
        return;
    }
    
    std::cout << "irrKlang audio engine initialized successfully!" << std::endl;
    
    // Optionally play background music
    // _soundEngine->play2D("resource/audio/background.mp3", true); // true for loop
}

void AudioTestApp::cleanupAudio() {
    // Stop and clean up any playing walking sound
    if (_currentWalkingSound) {
        if (!_currentWalkingSound->isFinished()) {
            _currentWalkingSound->stop();
        }
        _currentWalkingSound->drop();
        _currentWalkingSound = nullptr;
    }
    
    // Clean up sound engine
    if (_soundEngine) {
        _soundEngine->drop(); // Release the sound engine
        _soundEngine = nullptr;
        std::cout << "irrKlang audio engine cleaned up." << std::endl;
    }
}

void AudioTestApp::handleInput() {
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

    // Switch camera mode using number keys (like cameraTestApp)
    if (_input.keyboard.keyStates[GLFW_KEY_1] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::FREE_ROAM) {
            _advCamera->setMode(CameraMode::FREE_ROAM);
            std::cout << "Switched to FREE_ROAM mode - full movement and rotation" << std::endl;
            _input.keyboard.keyStates[GLFW_KEY_1] = GLFW_RELEASE;
        }
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_2] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::FIXED) {
            _advCamera->setMode(CameraMode::FIXED);
            std::cout << "Switched to FIXED mode - rotation only" << std::endl;
            _input.keyboard.keyStates[GLFW_KEY_2] = GLFW_RELEASE;
        }
    }
    
    if (_input.keyboard.keyStates[GLFW_KEY_3] == GLFW_PRESS) {
        if (_advCamera->getMode() != CameraMode::PAN) {
            _advCamera->setMode(CameraMode::PAN);
            std::cout << "Switched to PAN mode - parallel movement only" << std::endl;
            _input.keyboard.keyStates[GLFW_KEY_3] = GLFW_RELEASE;
        }
    }

    // Store old position for walking detection
    Camera* camera = _advCamera->getCamera();
    glm::vec3 oldPosition = camera->transform.position;
    
    // Process all camera input through the advanced camera system
    _advCamera->processInput(_input);
    
    // Update walking state and footstep audio based on position change
    detectWalking();
    updateWalkingState();
    
    // Update window title to show camera mode
    std::string modeStr = "Audio Test - Camera Mode: " + _advCamera->getModeString();
    if (_advCamera->isAnimating()) {
        modeStr += " (Animating)";
    }
    glfwSetWindowTitle(_window, (modeStr + " - Use 1/2/3 to switch camera modes").c_str());
    
    _input.forwardState();
}

void AudioTestApp::detectWalking() {
    Camera* camera = _advCamera->getCamera();
    glm::vec3 currentPosition = camera->transform.position;
    
    // Calculate movement distance (only horizontal movement for walking)
    glm::vec3 horizontalMovement = currentPosition - _lastPosition;
    horizontalMovement.y = 0.0f; // Ignore vertical movement
    
    float movementDistance = glm::length(horizontalMovement);
    _walkingSpeed = movementDistance / _deltaTime;
    
    // Consider walking if moving faster than a threshold
    const float walkingThreshold = 0.5f; // Minimum speed to be considered walking
    _isWalking = (_walkingSpeed > walkingThreshold);
    
    _lastPosition = currentPosition;
}

void AudioTestApp::updateWalkingState() {
    if (_isWalking) {
        // Start walking sound if not already playing or if we just started walking
        if (!_wasWalkingLastFrame || !_currentWalkingSound || _currentWalkingSound->isFinished()) {
            playFootstepSound();
        }
        
        // Adjust volume based on walking speed
        if (_currentWalkingSound && !_currentWalkingSound->isFinished()) {
            float volume = glm::clamp(_walkingSpeed / 10.0f, 0.3f, 1.0f);
            _currentWalkingSound->setVolume(volume);
        }
        
        _wasWalkingLastFrame = true;
    } else {
        // Stop walking sound when we stop walking
        if (_wasWalkingLastFrame && _currentWalkingSound && !_currentWalkingSound->isFinished()) {
            _currentWalkingSound->stop();
            _currentWalkingSound->drop();
            _currentWalkingSound = nullptr;
            std::cout << "🛑 Stopped walking sound" << std::endl;
        }
        
        _wasWalkingLastFrame = false;
    }
}

void AudioTestApp::playFootstepSound() {
    if (!_soundEngine) {
        // Fallback to console simulation if audio engine failed to initialize
        std::cout << "🚶 Walking simulation (Speed: " << std::fixed << std::setprecision(1) << _walkingSpeed << ")" << std::endl;
        return;
    }
    
    // Stop any existing walking sound before starting a new one
    if (_currentWalkingSound && !_currentWalkingSound->isFinished()) {
        _currentWalkingSound->stop();
        _currentWalkingSound->drop();
        _currentWalkingSound = nullptr;
    }
    
    const char* walkingSound = "audio/walking-sound.wav";
    
    // Play the walking sound in a loop for seamless walking
    _currentWalkingSound = _soundEngine->play2D(walkingSound, true, false, true); // true = loop
    
    if (_currentWalkingSound) {
        // Set initial volume based on walking speed
        float volume = glm::clamp(_walkingSpeed / 10.0f, 0.3f, 1.0f);
        _currentWalkingSound->setVolume(volume);
        
        std::cout << "🚶 Started walking sound (Speed: " << std::fixed << std::setprecision(1) << _walkingSpeed 
                  << ", Volume: " << std::setprecision(2) << volume << ")" << std::endl;
    } else {
        std::cout << "Failed to play walking sound: " << walkingSound << std::endl;
    }
}

void AudioTestApp::renderFrame() {
    // Clear the screen
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get camera matrices from the advanced camera
    Camera* camera = _advCamera->getCamera();
    glm::mat4 projection = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();

    // Render the floor
    _shader->use();
    
    // Set matrices
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);
    _shader->setUniformMat4("model", _floorModel->transform.getLocalMatrix());
    
    // Draw floor
    _floorModel->draw();
} 