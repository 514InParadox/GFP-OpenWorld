#ifndef _APP_CAMERATESTAPP_H
#define _APP_CAMERATESTAPP_H

#include <memory>
#include <chrono>

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

    // Central model for visualization
    std::unique_ptr<Model> _model;

    // Shader program
    std::unique_ptr<GLSLProgram> _shader;

    // Optional skybox for better visualization
    std::unique_ptr<SkyBox> _skybox;
    
    // Timer for animation
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;

    void initShader();
    
    void updateFrameTime();
};

#endif 