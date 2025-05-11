#ifndef _APP_CAMERATESTAPP_H
#define _APP_CAMERATESTAPP_H

#include <memory>
#include <chrono>
#include <vector>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/advance_camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"

class CameraTestApp : public Application {
public:    
    CameraTestApp(const Options &options);

    ~CameraTestApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    // Advanced camera for testing
    std::unique_ptr<AdvanceCamera> _advCamera;

    // Multiple models for visualization
    std::vector<std::unique_ptr<Model>> _models;

    // Shader program
    std::unique_ptr<GLSLProgram> _shader;

    // Optional skybox for better visualization
    std::unique_ptr<SkyBox> _skybox;
    
    // Timer for animation
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;

    // Clipping plane variables
    float _nearPlane = 1.0f;
    float _farPlane = 15.0f;
    float _planeAdjustSpeed = 0.5f;
    
    // FOV adjustment speed
    float _fovAdjustSpeed = 5.0f;  // Degrees per second
    
    bool _showClippingInfo = true;
    float _clippingInfoDisplayTime = 0.0f;
    const float _clippingInfoTimeout = 3.0f;

    void initShader();
    
    void updateFrameTime();

    void displayClippingPlaneInfo();

    // Create multiple models in a row
    void setupModels();
};

#endif 