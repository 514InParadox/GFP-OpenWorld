#pragma once

#include <glm/glm.hpp>

#include "map.hpp"
#include "srcDef.hpp"

// 撞墙修正
glm::vec2 getCorrectPos(glm::vec2 playerPos, glm::vec2 deltaPos);

// 主要用于移动时的镜头摇晃
glm::vec3 getCameraPos(const glm::vec2 &playerPos, const glm::vec2 &deltaPos, const float &deltaTime);