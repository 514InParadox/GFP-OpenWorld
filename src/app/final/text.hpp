#pragma once

#include "model/advancedModel.hpp"
#include "utils/glsl_program.hpp"

class Text {
public:
    Text(std::unique_ptr<AdvancedModel> text) : _text(std::move(text)) {}

    // update lifetime, return false if should be destroyed
    void setLifeTime(const float &lifeTime);

    bool Life(const float &deltaTime);

    // draw the model, using text shader
    // void draw();
    void draw(GLSLProgram &shader);
private:
    std::unique_ptr<AdvancedModel> _text;
    float _remainingLifeTime;
};