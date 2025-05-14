#ifndef _APP_LIGHTINGTESTAPP_H
#define _APP_LIGHTINGTESTAPP_H

#include <memory>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"

#include "utils/light.hpp"

class lightingTestApp : public Application {
public:    
    lightingTestApp(const Options &options);

    ~lightingTestApp() = default;

    void handleInput() override;

    void renderFrame() override;

    void setMaterial();

    void setLight();

private:
    // A material for testing.
    std::unique_ptr<Material> _testMaterial;

    // Light for test
    std::unique_ptr<Light> _light;
    std::vector<std::unique_ptr<Camera> > _cameras;
    int activeCameraIndex = 0;

    std::unique_ptr<Model> _model;

    std::unique_ptr<GLSLProgram> _shader;

    std::unique_ptr<SkyBox> _skybox;

    void initShader();
};

#endif