#pragma once

#include <glm/glm.hpp>

// 0 <= stamp <= 1
// return 
class Movement {
public:
    virtual glm::mat4 getTransform(float stamp) = 0;
};

class LinearMovement : public Movement {
public:
    LinearMovement(glm::mat4 startTrans, glm::mat4 endTrans) :
        startTrans_(startTrans), endTrans_(endTrans) {}
    
    glm::mat4 getTransform(float stamp) override;
private:
    glm::mat4 startTrans_, endTrans_;
};