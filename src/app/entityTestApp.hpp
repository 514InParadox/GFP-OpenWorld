#ifndef _APP_ENTITYTESTAPP_H
#define _APP_ENTITYTESTAPP_H

#include <memory>
#include <vector>
#include <chrono>

#include "application.hpp"
#include "model/model.hpp"
#include "utils/advance_camera.hpp"
#include "utils/glsl_program.hpp"
#include "utils/skybox.hpp"
#include "utils/light.hpp"

class EntityTestApp : public Application {
public:    
    EntityTestApp(const Options &options);

    ~EntityTestApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    // Camera system
    std::unique_ptr<AdvanceCamera> _advCamera;

    // Entity models with different materials
    std::vector<std::unique_ptr<Model>> _entities;
    std::vector<std::unique_ptr<Material>> _materials;

    // Shader program for entities
    std::unique_ptr<GLSLProgram> _entityShader;

    // Lighting system
    std::unique_ptr<AmbientLight> _ambientLight;
    std::vector<std::unique_ptr<Light>> _lights;
    Light* _activeLight;

    // Skybox for environment
    std::unique_ptr<SkyBox> _skybox;
    
    // Timer for animations
    std::chrono::time_point<std::chrono::high_resolution_clock> _lastFrameTime;
    float _deltaTime = 0.0f;

    // Entity animation parameters
    float _rotationSpeed = 1.0f;
    float _totalTime = 0.0f;

    // Control flags
    bool _enableTexture = false;
    int _currentLightType = 0; // 0: Point, 1: Directional, 2: Spot

    // Private methods
    void initShader();
    void setupEntities();
    void setupMaterials();
    void initLighting();
    void updateFrameTime();
    void updateEntityTransforms();
    void renderEntities();
    void updateLighting();
    void switchLightType();
    void displayInfo();
};

#endif
