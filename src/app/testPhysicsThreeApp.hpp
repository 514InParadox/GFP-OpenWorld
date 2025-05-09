#ifndef _APP_TESTPHYSICSTHREEAPP_H
#define _APP_TESTPHYSICSTHREEAPP_H

#include <memory>
#include <chrono>
#include <vector>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/physics.hpp"

class TestPhysicsThreeApp : public Application {
public:    
    TestPhysicsThreeApp(const Options &options);

    ~TestPhysicsThreeApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    // Current angular velocity value
    float _currentAngularVelocity = 1.0f;
    
    // Frame time calculation
    float _deltaTime = 0.0f;
    
    // Models
    std::unique_ptr<Model> _centerPillar;    // Center pillar
    std::unique_ptr<Model> _rotatingObject;  // Rotating object

    // Shader
    std::unique_ptr<GLSLProgram> _shader;

    // Skybox
    std::unique_ptr<SkyBox> _skybox;
    
    // Timer for frame time calculations
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;

    // Initialize shader
    void initShader();
    
    // Update frame time
    void updateFrameTime();

    // Create models
    void setupModels();
    
    // Create box model
    std::unique_ptr<Model> createBox(float width, float height, float depth);
    
    // Start simulation
    void startSimulation();
    
    // Increase angular velocity
    void increaseAngularVelocity();
    
    // Decrease angular velocity
    void decreaseAngularVelocity();
    
    // Update object position and velocity
    void updateObjectPositionAndVelocity();
    
    // Update physics state
    void updatePhysics();
};

#endif 