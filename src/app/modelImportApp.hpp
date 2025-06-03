#pragma once

#include "application.hpp"
#include "model/model.hpp"
#include "utils/camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/texture2d.hpp"

#define FRAMES 10

class TexModel : public Model {
public:
    TexModel(const std::string& filepath);

    TexModel(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    std::shared_ptr<Texture2D> mapKd;
};

class ModelImportApp : public Application {
public:
    ModelImportApp(const Options &options);

    ~ModelImportApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    std::unique_ptr<Camera> _camera;
    
    std::unique_ptr<TexModel> _texModel;

    std::unique_ptr<Model> _models[FRAMES];
    // std::array<std::unique_ptr<Model>, FRAMES> _models;

    std::unique_ptr<GLSLProgram> _shader;
    std::unique_ptr<GLSLProgram> _texShader;

    std::unique_ptr<SkyBox> _skybox;

    void initShader();
};