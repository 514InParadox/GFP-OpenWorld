#ifndef _APP_TESTPHYSICSAPP_H
#define _APP_TESTPHYSICSAPP_H

#include <memory>
#include <chrono>
#include <vector>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/physics.hpp"

class TestPhysicsApp : public Application {
public:    
    TestPhysicsApp(const Options &options);

    ~TestPhysicsApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    // Constants
    const float MIN_VELOCITY_THRESHOLD = 0.01f;
    
    // Physics related
    bool _simulationStarted = false;
    float _deltaTime = 0.0f;
    
    // Boundary parameters
    float _floorY = -5.0f;        // Floor Y coordinate
    float _leftWallX = -10.0f;    // Left wall X coordinate
    float _rightWallX = 10.0f;    // Right wall X coordinate
    float _frontWallZ = 10.0f;    // Front wall Z coordinate
    float _backWallZ = -10.0f;    // Back wall Z coordinate
    
    // Initial position (for reset) - Ensure enough height to avoid initial penetration
    glm::vec3 _initialObjectPosition = glm::vec3(-8.0f, 6.0f, 0.0f);
    
    // Models
    std::unique_ptr<Model> _floorModel;      // Ground plane
    std::unique_ptr<Model> _fallingObject;   // Falling object

    // Shader program
    std::unique_ptr<GLSLProgram> _shader;

    // Skybox
    std::unique_ptr<SkyBox> _skybox;
    
    // Timer for time calculations
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;

    void initShader();
    
    void updateFrameTime();

    // Create models
    void setupModels();
    
    // Create box model
    std::unique_ptr<Model> createBox(float width, float height, float depth);
    
    // Create plane model
    std::unique_ptr<Model> createPlane(float width, float depth);
    
    // Reset physics simulation
    void resetSimulation();
    
    // Start physics simulation
    void startSimulation();
    
    // Check boundary collisions and handle bounces
    void checkBoundaryCollisions();
    
    // Update physics state
    void updatePhysics();
};

#endif 