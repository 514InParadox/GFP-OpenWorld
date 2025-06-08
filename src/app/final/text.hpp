#pragma once

#include "model/advancedModel.hpp"
//#include "utils/glsl_program.hpp"
#include <glm/glm.hpp>
#include "utils/transform.hpp"

class Text {
public:
    Text(std::unique_ptr<AdvancedModel> text)
        : _text(std::move(text)), _remainingLifeTime(0.0f) {}
    //explicit Text(std::unique_ptr<AdvancedModel> model);

    // update lifetime, return false if should be destroyed
    void setLifeTime(const float &lifeTime);

    bool Life(const float &deltaTime);

    // draw the model, using text shader
    void draw();
    //void draw(GLSLProgram &shader);

    // 获取模型矩阵，供 shader 使用
    glm::mat4 getModelMatrix() const;

    // 直接访问内部模型的变换，用于设置位置等
    Transform& getTransform();
private:
    std::unique_ptr<AdvancedModel> _text;
    float _remainingLifeTime;
};