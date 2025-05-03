#ifndef _APP_INITSCENEAPP_H
#define _APP_INITSCENEAPP_H

#include <memory>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"

class InitSceneApp : public Application {
public:    
    InitSceneApp(const Options &options);

    ~InitSceneApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    std::vector<std::unique_ptr<Camera> > _cameras;
    int activeCameraIndex = 0;

    std::unique_ptr<Model> _model;

    std::unique_ptr<GLSLProgram> _shader;

    std::unique_ptr<SkyBox> _skybox;

    void initShader();
};

#endif