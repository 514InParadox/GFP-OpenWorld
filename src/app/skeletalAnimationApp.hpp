#pragma once

#include "app/application.hpp"
#include "utils/camera.hpp"
#include "utils/skybox.hpp"
#include "animation/animated_model.hpp"
#include "animation/animation.hpp"
#include "animation/animator.hpp"
#include "utils/glsl_program.hpp"
#include <memory>

class SkeletalAnimationApp : public Application {
public:
    SkeletalAnimationApp(const Options& options);
    ~SkeletalAnimationApp();

private:
    void handleInput() override;
    void renderFrame() override;
    void initShader();
    void initModel();
    void setupLighting();
    void setupAnimations();
    void updateAnimation();    void loadTestAnimations();

    // Camera
    std::unique_ptr<PerspectiveCamera> _camera;

    // Shaders
    std::unique_ptr<GLSLProgram> _animatedShader;

    // Models and Animation
    std::unique_ptr<AnimatedModel> _animatedModel;
    std::vector<std::unique_ptr<Animation>> _animations;
    std::unique_ptr<Animator> _animator;

    // Skybox
    std::unique_ptr<SkyBox> _skybox;    // Animation control
    int _currentAnimationIndex = 0;
    bool _animationPaused = false;
    float _animationSpeed = 1.0f;
    
    // Lighting
    glm::vec3 _lightPosition = glm::vec3(2.0f, 4.0f, 2.0f);
    glm::vec3 _lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    float _lightIntensity = 1.0f;    // Model transform
    glm::vec3 _modelPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _modelRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 _modelScale = glm::vec3(1.0f, 1.0f, 1.0f);
    
    // Debug options
    bool _debugUV = false;
};
