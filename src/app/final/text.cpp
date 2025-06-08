#include "text.hpp"
#include <glm/glm.hpp>
#include "utils/transform.hpp"

//Text::Text(std::unique_ptr<AdvancedModel> model)
//    :_text(std::move(model)), _remainingLifeTime(0.0f){}

void Text::setLifeTime(const float &lifeTime) {
    _remainingLifeTime = lifeTime;
}

bool Text::Life(const float &deltaTime) {
    // 减少剩余生命时间
    _remainingLifeTime -= deltaTime;
    
    // 如果生命时间还大于0，返回true表示文本仍然存活
    return _remainingLifeTime > 0.0f;
}

void Text::draw() {
    // 绘制文本模型
    if (_text) {
        _text->draw();
    }
}

glm::mat4 Text::getModelMatrix() const {
    if (_text) {
        return _text->transform.getLocalMatrix();
    }
    return glm::mat4(1.0f);
}

Transform& Text::getTransform() {
    return _text->transform;
}