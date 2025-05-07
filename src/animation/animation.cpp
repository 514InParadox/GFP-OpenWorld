#include "animation.hpp"

glm::mat4 LinearMovement::getTransform(float stamp) {
    return stamp * endTrans_ + (1 - stamp) * startTrans_;
}