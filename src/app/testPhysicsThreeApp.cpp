#include "testPhysicsThreeApp.hpp"
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>

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

// Initial angular velocity value
const float initialAngularVelocityY = 1.0f;
// Angular velocity change increment
const float angularVelocityDelta = 0.2f;

TestPhysicsThreeApp::TestPhysicsThreeApp(const Options &options) : Application(options) {
    // Set initial background color
    _clearColor = glm::vec4(0.1f, 0.1f, 0.2f, 1.0f);
    
    // Initialize models
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
    
    // Set window title
    glfwSetWindowTitle(_window, "Angular Velocity Test - Press 1 to speed up, 2 to slow down");
    
    // Start simulation
    startSimulation();
}

void TestPhysicsThreeApp::setupModels() {
    // Create center pillar
    _centerPillar = createBox(1.0f, 5.0f, 1.0f);
    _centerPillar->transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Create rotating object (cube)
    _rotatingObject = createBox(1.0f, 1.0f, 1.0f);
    // Position the object 5 units away from center
    _rotatingObject->transform.position = glm::vec3(5.0f, 0.0f, 0.0f);
    
    // Add physics component
    _rotatingObject->addPhysics();
    Physics* objectPhysics = _rotatingObject->getPhysics();
    
    // Set physics properties
    objectPhysics->setMass(1.0f);
    objectPhysics->setRestitution(0.6f);
    objectPhysics->setFriction(0.2f);
    
    // Disable gravity
    objectPhysics->setGravityEnabled(false);
}

std::unique_ptr<Model> TestPhysicsThreeApp::createBox(float width, float height, float depth) {
    // Create cube vertices
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Half dimensions
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float hd = depth * 0.5f;
    
    // Define the 8 corners of the box
    glm::vec3 corners[8] = {
        glm::vec3(-hw, -hh, -hd),  // 0: left bottom back
        glm::vec3(hw, -hh, -hd),   // 1: right bottom back
        glm::vec3(hw, hh, -hd),    // 2: right top back
        glm::vec3(-hw, hh, -hd),   // 3: left top back
        glm::vec3(-hw, -hh, hd),   // 4: left bottom front
        glm::vec3(hw, -hh, hd),    // 5: right bottom front
        glm::vec3(hw, hh, hd),     // 6: right top front
        glm::vec3(-hw, hh, hd)     // 7: left top front
    };
    
    // Define the 6 face normals
    glm::vec3 normals[6] = {
        glm::vec3(0, 0, -1),  // back
        glm::vec3(0, 0, 1),   // front
        glm::vec3(-1, 0, 0),  // left
        glm::vec3(1, 0, 0),   // right
        glm::vec3(0, -1, 0),  // bottom
        glm::vec3(0, 1, 0)    // top
    };
    
    // Define texture coordinates
    glm::vec2 texCoords[4] = {
        glm::vec2(0, 0),  // bottom-left
        glm::vec2(1, 0),  // bottom-right
        glm::vec2(1, 1),  // top-right
        glm::vec2(0, 1)   // top-left
    };
    
    // Add vertices for each face (6 faces, 4 vertices each)
    // Back face
    vertices.push_back(Vertex(corners[0], normals[0], texCoords[0]));
    vertices.push_back(Vertex(corners[1], normals[0], texCoords[1]));
    vertices.push_back(Vertex(corners[2], normals[0], texCoords[2]));
    vertices.push_back(Vertex(corners[3], normals[0], texCoords[3]));
    
    // Front face
    vertices.push_back(Vertex(corners[4], normals[1], texCoords[0]));
    vertices.push_back(Vertex(corners[5], normals[1], texCoords[1]));
    vertices.push_back(Vertex(corners[6], normals[1], texCoords[2]));
    vertices.push_back(Vertex(corners[7], normals[1], texCoords[3]));
    
    // Left face
    vertices.push_back(Vertex(corners[0], normals[2], texCoords[0]));
    vertices.push_back(Vertex(corners[3], normals[2], texCoords[1]));
    vertices.push_back(Vertex(corners[7], normals[2], texCoords[2]));
    vertices.push_back(Vertex(corners[4], normals[2], texCoords[3]));
    
    // Right face
    vertices.push_back(Vertex(corners[1], normals[3], texCoords[0]));
    vertices.push_back(Vertex(corners[5], normals[3], texCoords[1]));
    vertices.push_back(Vertex(corners[6], normals[3], texCoords[2]));
    vertices.push_back(Vertex(corners[2], normals[3], texCoords[3]));
    
    // Bottom face
    vertices.push_back(Vertex(corners[0], normals[4], texCoords[0]));
    vertices.push_back(Vertex(corners[4], normals[4], texCoords[1]));
    vertices.push_back(Vertex(corners[5], normals[4], texCoords[2]));
    vertices.push_back(Vertex(corners[1], normals[4], texCoords[3]));
    
    // Top face
    vertices.push_back(Vertex(corners[3], normals[5], texCoords[0]));
    vertices.push_back(Vertex(corners[2], normals[5], texCoords[1]));
    vertices.push_back(Vertex(corners[6], normals[5], texCoords[2]));
    vertices.push_back(Vertex(corners[7], normals[5], texCoords[3]));
    
    // Define indices for each face (6 faces, 2 triangles each, 3 indices per triangle)
    for (int i = 0; i < 6; ++i) {
        int baseIndex = i * 4;
        
        // First triangle
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        
        // Second triangle
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
    }
    
    return std::make_unique<Model>(vertices, indices);
}

void TestPhysicsThreeApp::handleInput() { 
    // Update frame time
    updateFrameTime();

    // Exit on ESC key
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }
    
    // Increase angular velocity with key 1
    if (_input.keyboard.keyStates[GLFW_KEY_1] == GLFW_PRESS) {
        increaseAngularVelocity();
        _input.keyboard.keyStates[GLFW_KEY_1] = GLFW_RELEASE;
    }
    
    // Decrease angular velocity with key 2
    if (_input.keyboard.keyStates[GLFW_KEY_2] == GLFW_PRESS) {
        decreaseAngularVelocity();
        _input.keyboard.keyStates[GLFW_KEY_2] = GLFW_RELEASE;
    }
    
    // Update physics state
    updatePhysics();

    // Update input state
    _input.forwardState();
}

void TestPhysicsThreeApp::startSimulation() {
    // Set initial angular velocity (Y-axis rotation only)
    updateObjectPositionAndVelocity();
}

void TestPhysicsThreeApp::increaseAngularVelocity() {
    _currentAngularVelocity += angularVelocityDelta;
    updateObjectPositionAndVelocity();
}

void TestPhysicsThreeApp::decreaseAngularVelocity() {
    _currentAngularVelocity -= angularVelocityDelta;
    if (_currentAngularVelocity < 0.0f) {
        _currentAngularVelocity = 0.0f;
    }
    updateObjectPositionAndVelocity();
}

void TestPhysicsThreeApp::updateObjectPositionAndVelocity() {
    // Calculate linear velocity using angular velocity and radius
    float radius = 5.0f; // Distance from center
    float linearVelocity = _currentAngularVelocity * radius;
    
    // Calculate tangent direction based on current position
    glm::vec3 position = _rotatingObject->transform.position;
    glm::vec3 toCenter = glm::vec3(0.0f, 0.0f, 0.0f) - position;
    glm::vec3 tangent = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(toCenter));
    
    // Set velocity in tangent direction
    Physics* physics = _rotatingObject->getPhysics();
    physics->setVelocity(tangent * linearVelocity);
    
    // Update window title with current angular velocity
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Angular Velocity Test - Current angular velocity: " << _currentAngularVelocity << " rad/s";
    glfwSetWindowTitle(_window, ss.str().c_str());
}

void TestPhysicsThreeApp::updatePhysics() {
    Physics* physics = _rotatingObject->getPhysics();
    
    // Update physics state
    physics->update(_deltaTime);
    
    // Keep object moving in a fixed radius circle
    glm::vec3 position = _rotatingObject->transform.position;
    glm::vec3 toCenter = glm::vec3(0.0f, 0.0f, 0.0f) - position;
    float radius = 5.0f;
    
    // Correct distance to maintain specified radius
    if (glm::length(toCenter) != radius) {
        position = -glm::normalize(toCenter) * radius;
        _rotatingObject->transform.position = position;
    }
    
    // Update tangential velocity
    updateObjectPositionAndVelocity();
}

void TestPhysicsThreeApp::renderFrame() {   
    // Clear screen
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    // Create camera matrices
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                          (float)_windowWidth / (float)_windowHeight, 
                                          0.1f, 100.0f);
    
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 10.0f, 20.0f),  // Top-down angle view
        glm::vec3(0.0f, 0.0f, 0.0f),    // Look at center
        glm::vec3(0.0f, 1.0f, 0.0f)     // Up vector
    );

    // Render all models
    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);

    // Draw center pillar (gray)
    _shader->setUniformMat4("model", _centerPillar->transform.getLocalMatrix());
    _shader->setUniformVec3("objectColor", glm::vec3(0.5f, 0.5f, 0.5f));
    _centerPillar->draw();
    
    // Draw rotating object (red)
    _shader->setUniformMat4("model", _rotatingObject->transform.getLocalMatrix());
    _shader->setUniformVec3("objectColor", glm::vec3(0.9f, 0.2f, 0.2f));
    _rotatingObject->draw();
    
    // Render skybox
    _skybox->draw(projection, view);
}

void TestPhysicsThreeApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}

void TestPhysicsThreeApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    
    // Limit delta time to avoid large jumps
    if (_deltaTime > 0.05f) {
        _deltaTime = 0.05f;
    }
    
    _lastFrameTime = currentTime;
} 