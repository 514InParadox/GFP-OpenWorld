#ifndef _APP_AUDIOTESTAPP_H
#define _APP_AUDIOTESTAPP_H

#include <memory>
#include <chrono>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/advance_camera.hpp"
#include "utils/glsl_program.hpp"

// irrKlang audio library
#include <irrklang/irrKlang.h>
using namespace irrklang;

class AudioTestApp : public Application {
public:    
    AudioTestApp(const Options &options);
    ~AudioTestApp();  // Need custom destructor for audio cleanup

    void handleInput() override;
    void renderFrame() override;

private:
    // Advanced camera for walking simulation
    std::unique_ptr<AdvanceCamera> _advCamera;
    
    // Floor model
    std::unique_ptr<Model> _floorModel;
    
    // Shader program
    std::unique_ptr<GLSLProgram> _shader;
    
    // Timer for animation and footstep timing
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;
    
    // Walking state tracking
    bool _isWalking = false;
    float _walkingSpeed = 0.0f;
    glm::vec3 _lastPosition;
    
    // Footstep timing
    float _footstepTimer = 0.0f;
    const float _footstepInterval = 0.6f; // Time between footsteps in seconds (increased for less frequency)
    bool _leftFoot = true; // Alternate between left and right foot
    
    // Audio control for seamless walking sound
    ISound* _currentWalkingSound = nullptr; // Track current playing walking sound
    bool _wasWalkingLastFrame = false; // Track walking state for seamless transition
    
    // irrKlang audio system
    ISoundEngine* _soundEngine = nullptr;
    
    // Audio methods
    void initAudio();
    void playFootstepSound();
    void updateWalkingState();
    void cleanupAudio();
    
    // Initialization methods
    void initShader();
    void setupScene();
    void updateFrameTime();
    
    // Walking detection
    void detectWalking();
};

#endif 