#include "skeletalAnimationApp.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Test model and animation paths
const std::string modelPath = "resource/model/Howler_motion.fbx";
const std::string animationPath = "resource/model/Howler_motion.fbx";

// Alternative test models (if vampire model is not available)
const std::vector<std::string> alternativeModels = {
    "resource/model/animated/character.fbx",
    "resource/model/animated/character.dae",
    "resource/model/test_animated.fbx"
};

// Shader paths
const std::string animatedVertexShaderAddr = "shader/vertex/animated_model.vert";
const std::string animatedFragmentShaderAddr = "shader/fragment/animated_model.frag";

// Skybox textures
const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};

SkeletalAnimationApp::SkeletalAnimationApp(const Options& options) : Application(options) {
    // Set input mode for mouse look
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // Initialize camera
    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 100.0f;
    
    _camera.reset(new PerspectiveCamera(glm::radians(45.0f), aspect, znear, zfar));
    _camera->transform.position = glm::vec3(0, 2, 5);    // Initialize resources
    initShader();
    initModel();
    setupAnimations();

    // Initialize skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

SkeletalAnimationApp::~SkeletalAnimationApp() {
}

void SkeletalAnimationApp::handleInput() {
    constexpr float cameraMoveSpeed = 0.1f;
    constexpr float cameraRotateSpeed = 0.002f;

    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // Camera movement
    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        _camera->transform.position += cameraMoveSpeed * _camera->transform.getFront();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        _camera->transform.position -= cameraMoveSpeed * _camera->transform.getFront();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        _camera->transform.position -= cameraMoveSpeed * _camera->transform.getRight();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        _camera->transform.position += cameraMoveSpeed * _camera->transform.getRight();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_Q] != GLFW_RELEASE) {
        _camera->transform.position += cameraMoveSpeed * _camera->transform.getUp();
    }
    if (_input.keyboard.keyStates[GLFW_KEY_E] != GLFW_RELEASE) {
        _camera->transform.position -= cameraMoveSpeed * _camera->transform.getUp();
    }

    // Animation controls
    static bool spacePressed = false;
    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] != GLFW_RELEASE && !spacePressed) {
        _animationPaused = !_animationPaused;
        spacePressed = true;
    } else if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_RELEASE) {
        spacePressed = false;
    }    // Animation speed controls
    if (_input.keyboard.keyStates[GLFW_KEY_UP] != GLFW_RELEASE) {
        _animationSpeed = std::min(_animationSpeed + 0.01f, 3.0f);
    }
    if (_input.keyboard.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) {
        _animationSpeed = std::max(_animationSpeed - 0.01f, 0.0f);
    }

    // Model transform controls
    static bool rPressed = false;
    if (_input.keyboard.keyStates[GLFW_KEY_R] != GLFW_RELEASE && !rPressed) {
        // Reset model position and rotation
        _modelPosition = glm::vec3(0.0f, 0.0f, 0.0f);
        _modelRotation = glm::vec3(0.0f, 0.0f, 0.0f);
        _modelScale = glm::vec3(1.0f, 1.0f, 1.0f);
        rPressed = true;
    } else if (_input.keyboard.keyStates[GLFW_KEY_R] == GLFW_RELEASE) {
        rPressed = false;
    }

    // Model rotation controls (with Left Shift held)
    if (_input.keyboard.keyStates[GLFW_KEY_LEFT_SHIFT] != GLFW_RELEASE) {
        if (_input.keyboard.keyStates[GLFW_KEY_I] != GLFW_RELEASE) {
            _modelRotation.x += 1.0f;
        }
        if (_input.keyboard.keyStates[GLFW_KEY_K] != GLFW_RELEASE) {
            _modelRotation.x -= 1.0f;
        }
        if (_input.keyboard.keyStates[GLFW_KEY_J] != GLFW_RELEASE) {
            _modelRotation.y -= 1.0f;
        }
        if (_input.keyboard.keyStates[GLFW_KEY_L] != GLFW_RELEASE) {
            _modelRotation.y += 1.0f;
        }
    }    // Print controls info (with F1)
    static bool f1Pressed = false;
    if (_input.keyboard.keyStates[GLFW_KEY_F1] != GLFW_RELEASE && !f1Pressed) {
        std::cout << "\n=== Skeletal Animation Controls ===" << std::endl;
        std::cout << "Camera: WASD - Move, QE - Up/Down, Mouse - Look" << std::endl;
        std::cout << "Animation: SPACE - Pause/Resume, UP/DOWN - Speed, S - Switch Animation" << std::endl;
        std::cout << "Model: R - Reset, SHIFT+IJKL - Rotate" << std::endl;
        std::cout << "Debug: U - Toggle UV Debug Mode" << std::endl;
        std::cout << "F1 - Show this help, ESC - Exit" << std::endl;
        std::cout << "Current Speed: " << _animationSpeed << std::endl;
        std::cout << "Paused: " << (_animationPaused ? "Yes" : "No") << std::endl;
        std::cout << "UV Debug: " << (_debugUV ? "ON" : "OFF") << std::endl;
        if (!_animations.empty()) {
            std::cout << "Current Animation: " << _animations[_currentAnimationIndex]->GetName() 
                     << " (" << (_currentAnimationIndex + 1) << "/" << _animations.size() << ")" << std::endl;
        }
        std::cout << "=================================\n" << std::endl;
        f1Pressed = true;
    } else if (_input.keyboard.keyStates[GLFW_KEY_F1] == GLFW_RELEASE) {
        f1Pressed = false;
    }

    // UV Debug toggle (with U key)
    static bool uPressed = false;
    if (_input.keyboard.keyStates[GLFW_KEY_U] != GLFW_RELEASE && !uPressed) {
        _debugUV = !_debugUV;
        std::cout << "UV Debug Mode: " << (_debugUV ? "ON - Displaying UV coordinates as colors" : "OFF - Normal rendering") << std::endl;
        uPressed = true;
    } else if (_input.keyboard.keyStates[GLFW_KEY_U] == GLFW_RELEASE) {
        uPressed = false;
    }

    // Animation switching (with S key)
    static bool sPressed = false;
    if (_input.keyboard.keyStates[GLFW_KEY_J] != GLFW_RELEASE && !sPressed && !_animations.empty()) {
        // Only switch if S is not being used for camera movement (no other movement keys pressed)
        if (_input.keyboard.keyStates[GLFW_KEY_W] == GLFW_RELEASE && 
            _input.keyboard.keyStates[GLFW_KEY_A] == GLFW_RELEASE && 
            _input.keyboard.keyStates[GLFW_KEY_D] == GLFW_RELEASE) {
              _currentAnimationIndex = (_currentAnimationIndex + 1) % _animations.size();
            _animator->PlayAnimation(_animations[_currentAnimationIndex].get());
            
            // std::cout << "Switched to animation: " << _animations[_currentAnimationIndex]->GetName() 
            //          << " (" << (_currentAnimationIndex + 1) << "/" << _animations.size() << ")" << std::endl;
        }
        sPressed = true;
    } else if (_input.keyboard.keyStates[GLFW_KEY_J] == GLFW_RELEASE) {
        sPressed = false;
    }

    // Mouse look
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

void SkeletalAnimationApp::renderFrame() {
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Update animation
    if (!_animationPaused) {
        updateAnimation();
    }

    // Get matrices
    glm::mat4 projection = _camera->getProjectionMatrix();
    glm::mat4 view = _camera->getViewMatrix();

    // Update model transform
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, _modelPosition);
    model = glm::rotate(model, glm::radians(_modelRotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(_modelRotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(_modelRotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, _modelScale);

    _animatedModel->transform.setFromTRS(model);    // Render animated model
    if (_animatedModel && _animator) {
        _animatedShader->use();
        _animatedShader->setUniformMat4("projection", projection);
        _animatedShader->setUniformMat4("view", view);
        _animatedShader->setUniformMat4("model", _animatedModel->transform.getLocalMatrix());

        // Set debug uniforms
        _animatedShader->setUniformBool("debugUV", _debugUV);

        // Set bone matrices
        auto transforms = _animator->GetFinalBoneMatrices();
        for (int i = 0; i < transforms.size(); ++i) {
            _animatedShader->setUniformMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
        }

        // Set lighting uniforms (only if not in debug mode)
        if (!_debugUV) {
            setupLighting();
        }

        _animatedModel->Draw(*_animatedShader);
    }

    // Render skybox
    _skybox->draw(projection, view);
}

void SkeletalAnimationApp::initShader() {
    _animatedShader.reset(new GLSLProgram);
    try {        _animatedShader->attachVertexShaderFromFile(getAssetFullPath(animatedVertexShaderAddr));
        _animatedShader->attachFragmentShaderFromFile(getAssetFullPath(animatedFragmentShaderAddr));
        _animatedShader->link();
        // std::cout << "Animated shader compiled successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Animated shader compilation failed: " << e.what() << std::endl;
    }
}

void SkeletalAnimationApp::initModel() {    try {
        _animatedModel.reset(new AnimatedModel(getAssetFullPath(modelPath)));
        // std::cout << "Animated model loaded successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load animated model: " << e.what() << std::endl;
        
        // Try alternative models
        for (const auto& altPath : alternativeModels) {            try {
                _animatedModel.reset(new AnimatedModel(getAssetFullPath(altPath)));
                // std::cout << "Loaded alternative model: " << altPath << std::endl;
                break;
            } catch (const std::exception& e2) {
                std::cerr << "Alternative model " << altPath << " also failed: " << e2.what() << std::endl;
            }
        }
        
        if (!_animatedModel) {
            std::cerr << "No animated model could be loaded!" << std::endl;
        }
    }
}

void SkeletalAnimationApp::setupLighting() {
    if (!_animatedShader) return;
    
    _animatedShader->setUniformVec3("viewPos", _camera->transform.position);
    
    // Directional light
    _animatedShader->setUniformVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
    _animatedShader->setUniformVec3("dirLight.ambient", glm::vec3(0.3f, 0.3f, 0.3f));
    _animatedShader->setUniformVec3("dirLight.diffuse", _lightColor * _lightIntensity);
    _animatedShader->setUniformVec3("dirLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    // Point lights (set all 4 slots, but only activate first one)
    for (int i = 0; i < 4; i++) {
        std::string baseName = "pointLights[" + std::to_string(i) + "]";
        if (i == 0) {
            // Main point light
            _animatedShader->setUniformVec3(baseName + ".position", _lightPosition);
            _animatedShader->setUniformVec3(baseName + ".ambient", glm::vec3(0.2f, 0.2f, 0.2f));
            _animatedShader->setUniformVec3(baseName + ".diffuse", _lightColor * _lightIntensity);
            _animatedShader->setUniformVec3(baseName + ".specular", glm::vec3(1.0f, 1.0f, 1.0f));
            _animatedShader->setUniformFloat(baseName + ".constant", 1.0f);
            _animatedShader->setUniformFloat(baseName + ".linear", 0.09f);
            _animatedShader->setUniformFloat(baseName + ".quadratic", 0.032f);
        } else {
            // Disable other point lights
            _animatedShader->setUniformVec3(baseName + ".position", glm::vec3(0.0f));
            _animatedShader->setUniformVec3(baseName + ".ambient", glm::vec3(0.0f));
            _animatedShader->setUniformVec3(baseName + ".diffuse", glm::vec3(0.0f));
            _animatedShader->setUniformVec3(baseName + ".specular", glm::vec3(0.0f));
            _animatedShader->setUniformFloat(baseName + ".constant", 1.0f);
            _animatedShader->setUniformFloat(baseName + ".linear", 0.0f);
            _animatedShader->setUniformFloat(baseName + ".quadratic", 1.0f);
        }
    }

    // Spot light (disabled by default)
    _animatedShader->setUniformVec3("spotLight.position", glm::vec3(0.0f, 10.0f, 0.0f));
    _animatedShader->setUniformVec3("spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
    _animatedShader->setUniformFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    _animatedShader->setUniformFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
    _animatedShader->setUniformVec3("spotLight.ambient", glm::vec3(0.0f)); // Disabled
    _animatedShader->setUniformVec3("spotLight.diffuse", glm::vec3(0.0f)); // Disabled
    _animatedShader->setUniformVec3("spotLight.specular", glm::vec3(0.0f)); // Disabled
    _animatedShader->setUniformFloat("spotLight.constant", 1.0f);
    _animatedShader->setUniformFloat("spotLight.linear", 0.09f);
    _animatedShader->setUniformFloat("spotLight.quadratic", 0.032f);

    // Material properties (also set in AnimatedMesh::Draw, but ensure it's set)
    _animatedShader->setUniformFloat("material.shininess", 64.0f);
}

void SkeletalAnimationApp::setupAnimations() {
    if (!_animatedModel) return;

    try {        // Get total number of animations in the file
        int animationCount = Animation::GetAnimationCount(getAssetFullPath(animationPath));
        // std::cout << "Found " << animationCount << " animations in file" << std::endl;
          // Load all animations
        for (int i = 0; i < animationCount; ++i) {
            auto animation = std::make_unique<Animation>(getAssetFullPath(animationPath), _animatedModel.get(), i);
            // std::cout << "Loaded animation " << i << ": " << animation->GetName() << std::endl;
            _animations.push_back(std::move(animation));
        }
          // Create animator with first animation
        if (!_animations.empty()) {
            _animator.reset(new Animator(_animations[0].get()));
            _currentAnimationIndex = 0;
            // std::cout << "Animation setup completed! Currently playing: " << _animations[0]->GetName() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to setup animations: " << e.what() << std::endl;
    }
}

void SkeletalAnimationApp::updateAnimation() {
    if (_animator && !_animations.empty()) {
        _animator->UpdateAnimation(_deltaTime * _animationSpeed);
    }
}
