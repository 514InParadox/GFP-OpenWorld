#include "testPhysicsApp.hpp"
#include <sstream>
#include <iomanip>
#include <glm/gtc/matrix_transform.hpp>
#include <utils/collision_system.hpp>

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

// Colors for models
const glm::vec3 floorColor = glm::vec3(0.2f, 0.5f, 0.3f);  // Greenish color for floor
const glm::vec3 objectColor = glm::vec3(0.9f, 0.2f, 0.2f); // Reddish color for falling object

// Initial velocity for projectile motion
const float initialHorizontalSpeed = 3.0f; // Units per second

TestPhysicsApp::TestPhysicsApp(const Options &options) : Application(options) {
    // Set initial camera position to view the scene
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
    glfwSetWindowTitle(_window, "Physics Test - Press SPACE to Begin");
}

void TestPhysicsApp::setupModels() {
    // Create a ground plane
    _floorModel = createPlane(20.0f, 20.0f);
    _floorModel->transform.position = glm::vec3(0.0f, _floorY, 0.0f);
    
    // Create a falling cube
    _fallingObject = createBox(1.0f, 1.0f, 1.0f);
    _fallingObject->transform.position = _initialObjectPosition;
    
    // Add physics component to cube
    _fallingObject->addPhysics();
    Physics* objectPhysics = _fallingObject->getPhysics();

    // 新增，告诉碰撞模型，这个物体要检测碰撞
    CollisionSystem::instance().registerObject(
        _fallingObject.get(), objectPhysics, ColliderShape::AABB);
    // 新增：地板也要参与碰撞（静态刚体），做法相同，不过记得标成静态
    _floorModel->addPhysics();
    Physics* floorPhy = _floorModel->getPhysics();
    floorPhy->setStatic(true);
    CollisionSystem::instance().registerObject(
        _floorModel.get(), floorPhy, ColliderShape::AABB);
    
    // Set physics properties
    objectPhysics->setMass(1.0f);          // 1kg mass
    objectPhysics->setRestitution(0.85f);   // Increase restitution coefficient for better bounce
    objectPhysics->setFriction(0.05f);      // Reduce friction coefficient to maintain horizontal velocity longer
    
    // Set custom gravity value
    objectPhysics->setGravity(glm::vec3(0.0f, -9.8f, -0.0f));  // Default gravity acceleration direction (x, y, z)
    
    std::cout << "Physics simulation setup complete. Press SPACE to start the simulation." << std::endl;
}

std::unique_ptr<Model> TestPhysicsApp::createBox(float width, float height, float depth) {
    // Create box vertices
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

std::unique_ptr<Model> TestPhysicsApp::createPlane(float width, float depth) {
    // Create plane vertices (simple quad)
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Half dimensions
    float hw = width * 0.5f;
    float hd = depth * 0.5f;
    
    // Define the 4 corners of the plane
    glm::vec3 corners[4] = {
        glm::vec3(-hw, 0.0f, -hd),  // left back
        glm::vec3(hw, 0.0f, -hd),   // right back
        glm::vec3(hw, 0.0f, hd),    // right front
        glm::vec3(-hw, 0.0f, hd)    // left front
    };
    
    // All vertices have same normal (facing up)
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    
    // Define texture coordinates
    glm::vec2 texCoords[4] = {
        glm::vec2(0, 0),  // bottom-left
        glm::vec2(1, 0),  // bottom-right
        glm::vec2(1, 1),  // top-right
        glm::vec2(0, 1)   // top-left
    };
    
    // Add vertices
    for (int i = 0; i < 4; ++i) {
        vertices.push_back(Vertex(corners[i], normal, texCoords[i]));
    }
    
    // Add indices for two triangles
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(3);
    
    return std::make_unique<Model>(vertices, indices);
}

void TestPhysicsApp::handleInput() { 
    // Update frame time
    updateFrameTime();

    // Exit if ESC is pressed
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }
    
    // Start simulation when SPACE is pressed
    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS && !_simulationStarted) {
        startSimulation();
        
        // Reset the key state
        _input.keyboard.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
    }
    
    // Reset simulation with R key
    if (_input.keyboard.keyStates[GLFW_KEY_R] == GLFW_PRESS) {
        resetSimulation();
        
        // Reset the key state
        _input.keyboard.keyStates[GLFW_KEY_R] = GLFW_RELEASE;
    }
    
    // Update physics state if simulation is running
    if (_simulationStarted) {
        updatePhysics();
    }

    // Update input state
    _input.forwardState();
}

void TestPhysicsApp::resetSimulation() {
    // Reset object position
    _fallingObject->transform.position = _initialObjectPosition;
    
    // Reset physics component
    Physics* objectPhysics = _fallingObject->getPhysics();
    objectPhysics->setVelocity(glm::vec3(0.0f));
    objectPhysics->setAngularVelocity(glm::vec3(0.0f));
    
    _simulationStarted = false;
    
    std::cout << "Physics simulation reset. Press SPACE to start again." << std::endl;
    glfwSetWindowTitle(_window, "Physics Test - Press SPACE to Begin");
}

void TestPhysicsApp::startSimulation() {
    _simulationStarted = true;
    
    // Set initial velocity (horizontal right direction)
    Physics* objectPhysics = _fallingObject->getPhysics();
    objectPhysics->setVelocity(glm::vec3(initialHorizontalSpeed, 0.0f, 0.0f));
    
    std::cout << "Physics simulation started with projectile motion!" << std::endl;
    glfwSetWindowTitle(_window, "Physics Test - Simulation Running");
}

void TestPhysicsApp::updatePhysics() {
    // Get and update physics component
    Physics* physics = _fallingObject->getPhysics();
    
    // Check if velocity is very small, stop simulation if true
    if (glm::length(physics->getVelocity()) < MIN_VELOCITY_THRESHOLD) {
        // Only output stop message when object still has velocity (avoid repeated output)
        if (glm::length(physics->getVelocity()) > 0.0f) {
            std::cout << "Object has come to rest. Stopping simulation." << std::endl;
        }
        
        // Completely stop the object
        physics->setVelocity(glm::vec3(0.0f));
        return;
    }
    
    // Output current frame start velocity
    glm::vec3 vel = physics->getVelocity();
    // std::cout << "Frame start - Velocity: (" 
    //           << vel.x << ", " << vel.y << ", " << vel.z 
    //           << "), Speed: " << glm::length(vel) << std::endl;
    
    // Update velocity and position (excluding collision detection)
    physics->update(_deltaTime);
    
    // Handle collision detection and response at application level
    checkBoundaryCollisions();
    
    // Output current frame end velocity
    vel = physics->getVelocity();
    // std::cout << "Frame end - Velocity: (" 
    //           << vel.x << ", " << vel.y << ", " << vel.z 
    //           << "), Speed: " << glm::length(vel) << std::endl;
}

void TestPhysicsApp::checkBoundaryCollisions() {
    Physics* physics = _fallingObject->getPhysics();
    glm::vec3 position = _fallingObject->transform.position;
    float friction = physics->getFriction();
    
    // Output position and velocity before collision detection
    glm::vec3 vel = physics->getVelocity();
    // std::cout << "Collision check start - Position: (" 
    //           << position.x << ", " << position.y << ", " << position.z 
    //           << "), Speed: " << glm::length(vel) << std::endl;
    
    // Get cube dimensions (assuming 1x1x1)
    float halfSize = 0.5f;
    
    // Track if collision occurred to avoid multiple collisions in one frame
    bool collisionHandled = false;
    
    // Check floor collision (Y-axis)
    if (position.y - halfSize <= _floorY && !collisionHandled) {
        // Place object on floor, ensure no floor penetration
        position.y = _floorY + halfSize + 0.001f;  // Reduce offset for more precise collision
        
        // Handle floor bounce using upward normal vector
        glm::vec3 floorNormal(0.0f, 1.0f, 0.0f);
        physics->bounce(floorNormal, friction, MIN_VELOCITY_THRESHOLD);  // Directly pass final threshold
        
        // Update object position
        _fallingObject->transform.position = position;
        
        collisionHandled = true;
    }
    
    // Check left/right wall collisions (X-axis)
    if (position.x - halfSize <= _leftWallX && !collisionHandled) {
        position.x = _leftWallX + halfSize + 0.001f;
        glm::vec3 leftWallNormal(1.0f, 0.0f, 0.0f);
        physics->bounce(leftWallNormal, friction, MIN_VELOCITY_THRESHOLD);
        _fallingObject->transform.position = position;
        collisionHandled = true;
    } 
    else if (position.x + halfSize >= _rightWallX && !collisionHandled) {
        position.x = _rightWallX - halfSize - 0.001f;
        glm::vec3 rightWallNormal(-1.0f, 0.0f, 0.0f);
        physics->bounce(rightWallNormal, friction, MIN_VELOCITY_THRESHOLD);
        _fallingObject->transform.position = position;
        collisionHandled = true;
    }
    
    // Check front/back wall collisions (Z-axis)
    if (position.z - halfSize <= _backWallZ && !collisionHandled) {
        position.z = _backWallZ + halfSize + 0.001f;
        glm::vec3 backWallNormal(0.0f, 0.0f, 1.0f);
        physics->bounce(backWallNormal, friction, MIN_VELOCITY_THRESHOLD);
        _fallingObject->transform.position = position;
        collisionHandled = true;
    } 
    else if (position.z + halfSize >= _frontWallZ && !collisionHandled) {
        position.z = _frontWallZ - halfSize - 0.001f;
        glm::vec3 frontWallNormal(0.0f, 0.0f, -1.0f);
        physics->bounce(frontWallNormal, friction, MIN_VELOCITY_THRESHOLD);
        _fallingObject->transform.position = position;
        collisionHandled = true;
    }
    
    // Check velocity after collision detection
    vel = physics->getVelocity();
    std::cout << "Collision check end - Speed: " << glm::length(vel) 
              << (collisionHandled ? " (Collision handled)" : " (No collision)") << std::endl;
}

void TestPhysicsApp::renderFrame() {   
    // Clear the screen
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    // Create fixed camera matrices
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                          (float)_windowWidth / (float)_windowHeight, 
                                          0.1f, 100.0f);
    
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 15.0f),  // Camera position
        glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
    );

    // Render all models
    _shader->use();
    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);

    // Draw floor with green color
    _shader->setUniformMat4("model", _floorModel->transform.getLocalMatrix());
    _shader->setUniformVec3("objectColor", floorColor);
    _floorModel->draw();
    
    // Draw falling object with red color
    _shader->setUniformMat4("model", _fallingObject->transform.getLocalMatrix());
    _shader->setUniformVec3("objectColor", objectColor);
    _fallingObject->draw();
    
    // Render skybox
    _skybox->draw(projection, view);
}

void TestPhysicsApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}

void TestPhysicsApp::updateFrameTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    _deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - _lastFrameTime).count();
    
    // Cap delta time to avoid large jumps
    if (_deltaTime > 0.05f) {
        _deltaTime = 0.05f;
    }
    
    _lastFrameTime = currentTime;
} 