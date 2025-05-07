#pragma once

#include <memory>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"

class finalSceneApp : public Application {
public:    
    finalSceneApp(const Options &options);

    ~finalSceneApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    std::unique_ptr<Camera> _camera;
    int activeCameraIndex = 0;

    std::unique_ptr<Model> _model;

    std::unique_ptr<GLSLProgram> _shader;

    std::unique_ptr<SkyBox> _skybox;

    void initShader();
};