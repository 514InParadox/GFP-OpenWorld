#include "player.hpp"

#include <math.h>

// 撞墙修正
glm::vec2 getCorrectPos(glm::vec2 playerPos, glm::vec2 deltaPos) {
    glm::vec2 finalPos = playerPos + deltaPos;
    std::pair<int, int> finalDot = std::make_pair((int)floor(finalPos.x), (int)floor(finalPos.y));
    std::pair<int, int> finalLattice = std::make_pair((finalDot.first + 150) % 300, (finalDot.second + 150) % 300);

    if (map[finalLattice.first][finalLattice.second] != 1) 
        return finalPos;
    
    std::pair<int, int> nowDot = std::make_pair((int)floor(playerPos.x), (int)floor(playerPos.y));
    std::pair<int, int> nowLattice = std::make_pair((nowDot.first + 150) % 300, (nowDot.second + 150) % 300);

    if (nowLattice.first != finalLattice.first && nowLattice.second != finalLattice.second) {
        if (map[nowLattice.first][finalLattice.second] == 1)
            deltaPos.y = 0;
        if (map[finalLattice.first][nowLattice.second] == 1)    
            deltaPos.x = 0;
    } else {
        if (nowLattice.first != finalLattice.first) 
            deltaPos.x = 0;
        if (nowLattice.second != finalLattice.second)
            deltaPos.y = 0;
    }
    return playerPos + deltaPos;
}

// 主要用于移动时的镜头摇晃
glm::vec3 getCameraPos(const glm::vec2 &playerPos, const glm::vec2 &deltaPos, const float &deltaTime) {
    constexpr float height = 1.8f;
    static float cameraHeight = height;
    static float volume = 0;
    static float volumeDifLast = 0;
    static float timeNow = 0;

    timeNow += deltaTime;

    float speed = sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y);

    if (speed > volume) {
        volumeDifLast = std::min(speed - volume, volumeDifLast + 0.02f);
        // volume += std::min(speed - volume, (speed - volume) / 2 + volumeLast + 0.02f);
        volume += volumeDifLast;
    } else if (speed < volume) {
        volumeDifLast = std::min(volume - speed, volumeDifLast + 0.02f);
        // volume -= std::min(volume - speed, (volume - speed) / 2 + volumeLast + 0.02f);
        volume -= volumeDifLast;
    }


    cameraHeight = height + volume * sin(timeNow * 4 * pi);
    return glm::vec3(playerPos.x, cameraHeight, playerPos.y);
}