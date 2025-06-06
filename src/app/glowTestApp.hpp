#ifndef _APP_GLOWTESTAPP_H
#define _APP_GLOWTESTAPP_H

#include <memory>
#include <chrono>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/advance_camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/light.hpp"

class GlowTestApp : public Application {
public:    
    GlowTestApp(const Options &options);
    ~GlowTestApp() = default;

    void handleInput() override;
    void renderFrame() override;

private:
    // Camera
    std::unique_ptr<AdvanceCamera> _advCamera;
    
    // Models
    std::unique_ptr<Model> _monkeyModel;
    
    // Lights
    std::unique_ptr<AmbientLight> _ambientLight;
    std::unique_ptr<PointLight> _pointLight;
    
    // Materials
    std::unique_ptr<Material> _testMaterial;
    std::unique_ptr<Material> _lightMaterial;
    
    // Shaders
    std::unique_ptr<GLSLProgram> _sceneShader;
    std::unique_ptr<GLSLProgram> _emissiveShader;
    std::unique_ptr<GLSLProgram> _extractBrightShader;
    std::unique_ptr<GLSLProgram> _blurShader;
    std::unique_ptr<GLSLProgram> _combineShader;
    
    // Light sphere model for visualization
    std::unique_ptr<Model> _lightSphereModel;
    
    // Framebuffers and textures for glow effect
    GLuint _sceneFramebuffer;
    GLuint _sceneColorTexture;
    GLuint _sceneDepthTexture;
    
    GLuint _brightFramebuffer;
    GLuint _brightColorTexture;
    
    GLuint _blurFramebuffers[2];
    GLuint _blurColorTextures[2];
    
    // Screen quad for post-processing
    GLuint _quadVAO;
    GLuint _quadVBO;
    
    // Glow control variables
    bool _enableGlow = true;
    float _glowIntensity = 1.0f;
    float _lightIntensity = 2.0f;
    
    // Timer for animation
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;
    
    // Private methods
    void initShaders();
    void initFramebuffers();
    void initScreenQuad();
    void setupScene();
    void updateFrameTime();
    
    void renderScene();
    void renderBrightExtraction();
    void renderBlur();
    void renderFinalComposition();
    
    void cleanupFramebuffers();
};

#endif 