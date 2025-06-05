#pragma once

#include "app/application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/texture2d.hpp"
#include "model/advancedModel.hpp"

#include "srcDef.hpp"

#define FRAMES 10

class FinalSceneApp : public Application {

    enum class GameState {
        StartInterface,
        BeforeMita,
        DuringMita,
        AfterMita,
        LoseInterface,
        WinInterface,
/*
        StartInterface
              |
          BeforeMita ---- LoseInterface
              |
          DuringMita
              |
           AfterMita
           /       \
          /         \
    WinInterface  LoseInterface
*/
    };
    
public:
    FinalSceneApp(const Options &options);

    ~FinalSceneApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    GameState gameState{GameState::StartInterface};

    std::unique_ptr<Camera> _camera;
    
    std::unique_ptr<TexModel> _texModel;

    std::unique_ptr<GLSLProgram> _texShader;

    std::unique_ptr<AdvancedModel> _entity,
                                   _mita,
                                   _map,
                                   _gun;


    std::unique_ptr<GLSLProgram> _entityShader,
                                 _mitaShader,
                                 _mapShader,
                                 _gunShader;    std::unique_ptr<SkyBox> _skybox;

    // Time management for frame-rate independent movement
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;

    void initShader();
    
    // Update frame time calculation
    void updateFrameTime();
};