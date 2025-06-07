#pragma once

#include "app/application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/texture2d.hpp"
#include "model/advancedModel.hpp"

#include "srcDef.hpp"
#include "entity.hpp"
#include "dialog.hpp"
#include "interface.hpp"
#include "map.hpp"

class FinalSceneApp : public Application {

    enum class GameState {
        StartInterface,
        BeforeMita,
        DuringMita,
        AfterMita,
        AfterEntity,
        LoseInterface,
        WinInterface,
/*
        StartInterface
              |
          BeforeMita ---- LoseInterface
              |
          DuringMita
              |
           AfterMita ---- LoseInterface
              |
          AfterEntity
              |
        WinInterface
*/
    };

    // Point light structure for dynamic lighting
    struct DynamicPointLight {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
        
        DynamicPointLight(const glm::vec3& pos, const glm::vec3& col = glm::vec3(1.0f, 1.0f, 0.9f), float intens = 2.0f)
            : position(pos), color(col), intensity(intens) {}
    };
    
public:
    FinalSceneApp(const Options &options);

    ~FinalSceneApp();

    void handleInput() override;

    void renderFrame() override;
    
    void run() override;
    
    void updateState();
private:
    // Glow effect methods
    void initGlowFramebuffers();
    void initScreenQuad();
    void renderSceneToFramebuffer();
    void renderBrightExtraction();
    void renderBlur();
    void renderFinalComposition();
    void cleanupGlowFramebuffers();
    
    // Dynamic point light management
    void updateDynamicPointLights();
    void updateMitaPointLights();
    void setPointLightsUniforms(GLSLProgram* shader);
    void setMitaPointLightsUniforms(GLSLProgram* shader);
    
    GameState gameState{GameState::StartInterface}; // Start directly in game, not interface

    // Control mode for IJKL keys
    enum class ControlMode {
        Mita,
        Entity
    };
    ControlMode _controlMode{ControlMode::Mita}; // Start controlling mita

    std::unique_ptr<Camera> _camera;
    
    std::unique_ptr<TexModel> _texModel;

    std::unique_ptr<GLSLProgram> _texShader;

    std::unique_ptr<AdvancedModel> _entity,
                                   _mita,
                                   _map,
                                   _gun,
                                   _light;


    std::unique_ptr<GLSLProgram> _entityShader,
                                 _mitaShader,
                                 _mapShader,
                                 _gunShader,
                                 _emissiveShader;    

    std::unique_ptr<GLSLProgram> _interfaceShader;

    // Glow effect shaders
    std::unique_ptr<GLSLProgram> _extractBrightShader;
    std::unique_ptr<GLSLProgram> _blurShader;
    std::unique_ptr<GLSLProgram> _combineShader;

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
    float _glowIntensity = 0.02f;

    // Dynamic point lights
    std::vector<DynamicPointLight> _dynamicPointLights;    std::vector<DynamicPointLight> _mitaPointLights;
    static const int MAX_POINT_LIGHTS = 32;

    EntityLogic _entityLogic;
    // Dialog _dialog;
    std::unique_ptr<Dialog> _dialog;

    std::pair<int, int> _mapFinalLattice;

    std::unique_ptr<Interface> startInterface, loseInterface, winInterface;

    std::unique_ptr<SkyBox> _skybox;

    // Time management for frame-rate independent movement
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;

    glm::vec2 playerPosition;    void initShader();
    
    // Ray-AABB intersection test for shooting
    bool rayIntersectsAABB(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const BoundingBox& aabb) const;
    
    // Update frame time calculation
    void updateFrameTime();
};