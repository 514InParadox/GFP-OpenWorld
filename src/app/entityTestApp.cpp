#include "entityTestApp.hpp"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Entity model paths - using different models to showcase variety
const std::vector<std::string> entityModelPaths = {
    "resource/model/MonkeyHead.obj",
    "resource/model/MonkeyHead.obj", // We'll use the same model but with different materials
    "resource/model/MonkeyHead.obj",
    "resource/model/MonkeyHead.obj",
    "resource/model/MonkeyHead.obj"
};

// Shader file paths - using our new entity shaders
const std::string entityVertexShaderAddr = "shader/vertex/entity.vert";
const std::string entityFragmentShaderAddr = "shader/fragment/entity.frag";

// Skybox textures
const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};

// Entity positioning parameters
const int ENTITY_COUNT = 5;
const float ENTITY_SPACING = 4.0f;
const float ENTITY_CIRCLE_RADIUS = 8.0f;

EntityTestApp::EntityTestApp(const Options &options) : Application(options) {
    // Set input mode to capture mouse
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);    // Initialize camera
    _advCamera.reset(new AdvanceCamera(_windowWidth, _windowHeight));
    _advCamera->getCamera()->transform.position = glm::vec3(0.0f, 5.0f, 15.0f);
    _advCamera->getCamera()->transform.rotation = glm::vec3(-20.0f, 0.0f, 0.0f);

    // Initialize components
    setupMaterials();
    setupEntities();
    initShader();
    initLighting();

    // Initialize skybox
    std::vector<std::string> skyboxFullPaths;
    for (const auto& path : skyboxTexturePaths) {
        skyboxFullPaths.push_back(getAssetFullPath(path));
    }
    _skybox.reset(new SkyBox(skyboxFullPaths));
    
    // Initialize timing
    _lastFrameTime = std::chrono::high_resolution_clock::now();
    
    // Set window title
    glfwSetWindowTitle(_window, "Entity Shader Test - [TAB] Switch Lights, [T] Toggle Texture, [R] Reset Camera");
}

void EntityTestApp::setupMaterials() {
    _materials.clear();
    _materials.reserve(ENTITY_COUNT);

    // Material 1: Shiny red metal
    auto material1 = std::make_unique<Material>();
    material1->ambient = glm::vec3(0.1f, 0.02f, 0.02f);
    material1->diffuse = glm::vec3(0.8f, 0.2f, 0.2f);
    material1->specular = glm::vec3(1.0f, 0.8f, 0.8f);
    material1->materialColor = glm::vec3(0.9f, 0.1f, 0.1f);
    material1->shininess = 64.0f;
    _materials.push_back(std::move(material1));

    // Material 2: Matte green plastic
    auto material2 = std::make_unique<Material>();
    material2->ambient = glm::vec3(0.02f, 0.1f, 0.02f);
    material2->diffuse = glm::vec3(0.3f, 0.8f, 0.3f);
    material2->specular = glm::vec3(0.2f, 0.4f, 0.2f);
    material2->materialColor = glm::vec3(0.1f, 0.8f, 0.1f);
    material2->shininess = 8.0f;
    _materials.push_back(std::move(material2));

    // Material 3: Shiny blue ceramic
    auto material3 = std::make_unique<Material>();
    material3->ambient = glm::vec3(0.02f, 0.02f, 0.1f);
    material3->diffuse = glm::vec3(0.2f, 0.4f, 0.9f);
    material3->specular = glm::vec3(0.9f, 0.9f, 1.0f);
    material3->materialColor = glm::vec3(0.1f, 0.3f, 0.9f);
    material3->shininess = 128.0f;
    _materials.push_back(std::move(material3));

    // Material 4: Gold-like metal
    auto material4 = std::make_unique<Material>();
    material4->ambient = glm::vec3(0.1f, 0.08f, 0.02f);
    material4->diffuse = glm::vec3(0.8f, 0.7f, 0.2f);
    material4->specular = glm::vec3(1.0f, 0.9f, 0.4f);
    material4->materialColor = glm::vec3(0.9f, 0.8f, 0.1f);
    material4->shininess = 96.0f;
    _materials.push_back(std::move(material4));

    // Material 5: Silver-like metal
    auto material5 = std::make_unique<Material>();
    material5->ambient = glm::vec3(0.08f, 0.08f, 0.08f);
    material5->diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
    material5->specular = glm::vec3(0.95f, 0.95f, 0.95f);
    material5->materialColor = glm::vec3(0.8f, 0.8f, 0.8f);
    material5->shininess = 256.0f;
    _materials.push_back(std::move(material5));
}

void EntityTestApp::setupEntities() {
    _entities.clear();
    _entities.reserve(ENTITY_COUNT);

    for (int i = 0; i < ENTITY_COUNT; ++i) {
        // Load model (using the same model for all, but they'll have different materials)
        auto entity = std::make_unique<Model>(getAssetFullPath(entityModelPaths[0]));
        
        // Position entities in a circle
        float angle = (2.0f * M_PI * i) / ENTITY_COUNT;
        float x = ENTITY_CIRCLE_RADIUS * cos(angle);
        float z = ENTITY_CIRCLE_RADIUS * sin(angle);
        
        entity->transform.position = glm::vec3(x, 0.0f, z);
        entity->transform.rotation = glm::vec3(0.0f, glm::degrees(-angle), 0.0f);
        entity->transform.scale = glm::vec3(1.5f, 1.5f, 1.5f);
        
        _entities.push_back(std::move(entity));
    }
}

void EntityTestApp::initShader() {
    _entityShader.reset(new GLSLProgram);
    _entityShader->attachVertexShaderFromFile(getAssetFullPath(entityVertexShaderAddr));
    _entityShader->attachFragmentShaderFromFile(getAssetFullPath(entityFragmentShaderAddr));
    _entityShader->link();
}

void EntityTestApp::initLighting() {
    // Initialize ambient light
    _ambientLight = std::make_unique<AmbientLight>();
    _ambientLight->color = glm::vec3(0.3f, 0.3f, 0.4f);
    _ambientLight->intensity = 0.2f;

    // Initialize different types of lights
    _lights.clear();
    _lights.reserve(3);

    // Point light
    auto pointLight = std::make_unique<PointLight>();
    pointLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
    pointLight->intensity = 1.0f;
    pointLight->transform.position = glm::vec3(0.0f, 8.0f, 0.0f);
    pointLight->kc = 1.0f;
    pointLight->kl = 0.045f;
    pointLight->kq = 0.0075f;
    _lights.push_back(std::move(pointLight));

    // Directional light
    auto dirLight = std::make_unique<DirectionalLight>();
    dirLight->color = glm::vec3(1.0f, 0.95f, 0.8f);
    dirLight->intensity = 0.8f;
    dirLight->transform.rotation = glm::vec3(-45.0f, 30.0f, 0.0f);
    _lights.push_back(std::move(dirLight));

    // Spot light
    auto spotLight = std::make_unique<SpotLight>();
    spotLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLight->intensity = 1.2f;
    spotLight->transform.position = glm::vec3(0.0f, 10.0f, 5.0f);
    spotLight->transform.rotation = glm::vec3(-60.0f, 0.0f, 0.0f);
    spotLight->innerCutOff = glm::cos(glm::radians(15.0f));
    spotLight->cutOff = glm::cos(glm::radians(25.0f));
    spotLight->kc = 1.0f;
    spotLight->kl = 0.045f;
    spotLight->kq = 0.0075f;
    _lights.push_back(std::move(spotLight));

    // Set initial active light
    _activeLight = _lights[_currentLightType].get();
}

void EntityTestApp::handleInput() {
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // Camera controls
    _advCamera->processInput(_input);

    // Light type switching
    if (_input.keyboard.keyStates[GLFW_KEY_TAB] == GLFW_PRESS) {
        switchLightType();
        _input.keyboard.keyStates[GLFW_KEY_TAB] = GLFW_REPEAT; // Prevent immediate re-trigger
    }

    // Texture toggle
    if (_input.keyboard.keyStates[GLFW_KEY_T] == GLFW_PRESS) {
        _enableTexture = !_enableTexture;
        std::cout << "Texture " << (_enableTexture ? "enabled" : "disabled") << std::endl;
        _input.keyboard.keyStates[GLFW_KEY_T] = GLFW_REPEAT;
    }    // Reset camera
    if (_input.keyboard.keyStates[GLFW_KEY_R] == GLFW_PRESS) {
        _advCamera->getCamera()->transform.position = glm::vec3(0.0f, 5.0f, 15.0f);
        _advCamera->getCamera()->transform.rotation = glm::vec3(-20.0f, 0.0f, 0.0f);
        std::cout << "Camera reset" << std::endl;
        _input.keyboard.keyStates[GLFW_KEY_R] = GLFW_REPEAT;
    }

    // Rotation speed control
    if (_input.keyboard.keyStates[GLFW_KEY_UP] != GLFW_RELEASE) {
        _rotationSpeed += 0.5f * _deltaTime;
    }
    if (_input.keyboard.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) {
        _rotationSpeed = std::max(0.0f, _rotationSpeed - 0.5f * _deltaTime);
    }

    _input.forwardState();
}

void EntityTestApp::renderFrame() {
    updateFrameTime();
    updateEntityTransforms();
    updateLighting();

    // Clear screen
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);    // Update camera
    // Note: AdvanceCamera should handle its own updates internally

    // Get camera matrices
    glm::mat4 projection = _advCamera->getCamera()->getProjectionMatrix();
    glm::mat4 view = _advCamera->getCamera()->getViewMatrix();

    // Render entities with the entity shader
    renderEntities();

    // Render skybox last
    _skybox->draw(projection, view);

    // Display information
    displayInfo();
}

void EntityTestApp::renderEntities() {
    glm::mat4 projection = _advCamera->getCamera()->getProjectionMatrix();
    glm::mat4 view = _advCamera->getCamera()->getViewMatrix();
    glm::vec3 viewPos = _advCamera->getCamera()->transform.position;

    _entityShader->use();

    // Set camera uniforms
    _entityShader->setUniformMat4("projection", projection);
    _entityShader->setUniformMat4("view", view);
    _entityShader->setUniformVec3("viewPos", viewPos);

    // Set lighting uniforms
    _entityShader->setUniformVec3("ambientLight.color", _ambientLight->color);
    _entityShader->setUniformFloat("ambientLight.intensity", _ambientLight->intensity);

    // Set active light based on current type
    if (_currentLightType == 0) { // Point Light
        PointLight* pointLight = static_cast<PointLight*>(_activeLight);
        _entityShader->setUniformVec3("pointLight.position", pointLight->transform.position);
        _entityShader->setUniformVec3("pointLight.color", pointLight->color);
        _entityShader->setUniformFloat("pointLight.intensity", pointLight->intensity);
        _entityShader->setUniformFloat("pointLight.kc", pointLight->kc);
        _entityShader->setUniformFloat("pointLight.kl", pointLight->kl);
        _entityShader->setUniformFloat("pointLight.kq", pointLight->kq);
        
        // Disable other lights
        _entityShader->setUniformFloat("dirLight.intensity", 0.0f);
        _entityShader->setUniformFloat("spotLight.intensity", 0.0f);
    }
    else if (_currentLightType == 1) { // Directional Light
        DirectionalLight* dirLight = static_cast<DirectionalLight*>(_activeLight);
        _entityShader->setUniformVec3("dirLight.direction", dirLight->transform.getDefaultFront());
        _entityShader->setUniformVec3("dirLight.color", dirLight->color);
        _entityShader->setUniformFloat("dirLight.intensity", dirLight->intensity);
        
        // Disable other lights
        _entityShader->setUniformFloat("pointLight.intensity", 0.0f);
        _entityShader->setUniformFloat("spotLight.intensity", 0.0f);
    }
    else if (_currentLightType == 2) { // Spot Light
        SpotLight* spotLight = static_cast<SpotLight*>(_activeLight);
        _entityShader->setUniformVec3("spotLight.position", spotLight->transform.position);
        _entityShader->setUniformVec3("spotLight.direction", spotLight->transform.getDefaultFront());
        _entityShader->setUniformVec3("spotLight.color", spotLight->color);
        _entityShader->setUniformFloat("spotLight.intensity", spotLight->intensity);
        _entityShader->setUniformFloat("spotLight.innerCutOff", spotLight->innerCutOff);
        _entityShader->setUniformFloat("spotLight.outerCutOff", spotLight->cutOff);
        _entityShader->setUniformFloat("spotLight.kc", spotLight->kc);
        _entityShader->setUniformFloat("spotLight.kl", spotLight->kl);
        _entityShader->setUniformFloat("spotLight.kq", spotLight->kq);
        
        // Disable other lights
        _entityShader->setUniformFloat("pointLight.intensity", 0.0f);
        _entityShader->setUniformFloat("dirLight.intensity", 0.0f);
    }

    // Set texture usage
    _entityShader->setUniformBool("useTexture", _enableTexture);

    // Render each entity with its material
    for (size_t i = 0; i < _entities.size() && i < _materials.size(); ++i) {
        // Set model matrix
        _entityShader->setUniformMat4("model", _entities[i]->transform.getLocalMatrix());

        // Set material properties
        const Material* material = _materials[i].get();
        _entityShader->setUniformVec3("material.ambient", material->ambient);
        _entityShader->setUniformVec3("material.diffuse", material->diffuse);
        _entityShader->setUniformVec3("material.specular", material->specular);
        _entityShader->setUniformVec3("material.color", material->materialColor);
        _entityShader->setUniformFloat("material.shininess", material->shininess);

        // Draw the entity
        _entities[i]->draw();
    }
}

void EntityTestApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    _totalTime += _deltaTime;
    
    // Cap delta time to avoid large jumps
    if (_deltaTime > 0.05f) {
        _deltaTime = 0.05f;
    }
    
    _lastFrameTime = currentTime;
}

void EntityTestApp::updateEntityTransforms() {
    // Rotate entities around their own axes
    for (size_t i = 0; i < _entities.size(); ++i) {
        float rotationOffset = (2.0f * M_PI * i) / _entities.size();
        _entities[i]->transform.rotation.y += _rotationSpeed * 30.0f * _deltaTime; // 30 degrees per second base speed
        
        // Add some subtle up-down movement
        float bobOffset = sin(_totalTime * 2.0f + rotationOffset) * 0.5f;
        float angle = (2.0f * M_PI * i) / _entities.size();
        float x = ENTITY_CIRCLE_RADIUS * cos(angle);
        float z = ENTITY_CIRCLE_RADIUS * sin(angle);
        _entities[i]->transform.position = glm::vec3(x, bobOffset, z);
    }
}

void EntityTestApp::updateLighting() {
    // Animate light positions slightly for more dynamic lighting
    if (_currentLightType == 0) { // Point light
        PointLight* pointLight = static_cast<PointLight*>(_activeLight);
        pointLight->transform.position.x = 3.0f * sin(_totalTime * 0.5f);
        pointLight->transform.position.z = 3.0f * cos(_totalTime * 0.5f);
    }
    else if (_currentLightType == 2) { // Spot light
        SpotLight* spotLight = static_cast<SpotLight*>(_activeLight);
        spotLight->transform.rotation.y = 15.0f * sin(_totalTime * 0.3f);
    }
}

void EntityTestApp::switchLightType() {
    _currentLightType = (_currentLightType + 1) % 3;
    _activeLight = _lights[_currentLightType].get();
    
    const char* lightTypes[] = {"Point Light", "Directional Light", "Spot Light"};
    std::cout << "Switched to: " << lightTypes[_currentLightType] << std::endl;
}

void EntityTestApp::displayInfo() {
    // This would normally render text on screen, but we'll use console output
    static float infoTimer = 0.0f;
    infoTimer += _deltaTime;
    
    if (infoTimer >= 2.0f) { // Update info every 2 seconds
        const char* lightTypes[] = {"Point Light", "Directional Light", "Spot Light"};
        std::cout << "=== Entity Shader Demo ===" << std::endl;
        std::cout << "Current Light: " << lightTypes[_currentLightType] << std::endl;
        std::cout << "Texture: " << (_enableTexture ? "ON" : "OFF") << std::endl;
        std::cout << "Rotation Speed: " << std::fixed << std::setprecision(1) << _rotationSpeed << std::endl;
        std::cout << "Controls: [TAB] Switch Light, [T] Toggle Texture, [R] Reset Camera" << std::endl;
        std::cout << "          [↑/↓] Adjust Rotation Speed, Mouse: Look Around, WASD: Move" << std::endl;
        std::cout << "=========================" << std::endl;
        infoTimer = 0.0f;
    }
}
