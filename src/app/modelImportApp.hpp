#pragma once

#include "application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/texture2d.hpp"

class ModelImportApp : public Application {
public:
    ModelImportApp(const Options &options);

    ~ModelImportApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    std::unique_ptr<Camera> _camera;
    
    std::unique_ptr<TexModel> _texModel;
    
    std::unique_ptr<GLSLProgram> _texShader;

    std::unique_ptr<SkyBox> _skybox;

    void initShader();
};