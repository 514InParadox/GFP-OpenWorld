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
    
public:
    FinalSceneApp(const Options &options);

    ~FinalSceneApp() = default;

    void handleInput() override;

    void renderFrame() override;
    
    void run() override;
    
    void updateState();
private:
    GameState gameState{GameState::BeforeMita};

    glm::vec2 playerPosition;

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
                                 _gunShader;    

    std::unique_ptr<GLSLProgram> _interfaceShader;

    EntityLogic _entityLogic;
    // Dialog _dialog;
    std::unique_ptr<Dialog> _dialog;

    std::pair<int, int> _mapFinalLattice;

    std::unique_ptr<Interface> startInterface, loseInterface, winInterface;

    std::unique_ptr<SkyBox> _skybox;

    // Time management for frame-rate independent movement
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;

    void initShader();
    
    // Update frame time calculation
    void updateFrameTime();
};