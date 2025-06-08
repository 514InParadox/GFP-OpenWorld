#include "player.hpp"

#include <math.h>
#include <iostream>

// 撞墙修正
glm::vec2 getCorrectPos(glm::vec2 playerPos, glm::vec2 deltaPos) {
    if (deltaPos.x == 0 && deltaPos.y == 0)     
        return playerPos;

    glm::vec2 normalDelta = glm::normalize(deltaPos);

    // std::cout << deltaPos.x << " " << deltaPos.y << std::endl;
    // std::cout << normalDelta.x << " " << normalDelta.y << std::endl;

    normalDelta *= 0.5;

    glm::vec2 normalFinalPos = playerPos + normalDelta;
    std::pair<int, int> normalFinalDot = std::make_pair((int)floor(normalFinalPos.x), (int)floor(normalFinalPos.y));
    std::pair<int, int> normalFinalLattice = std::make_pair(((normalFinalDot.first + 150) % 300 + 300) % 300, ((normalFinalDot.second + 150) % 300 + 300) % 300);

    std::pair<int, int> nowDot = std::make_pair((int)floor(playerPos.x), (int)floor(playerPos.y));
    std::pair<int, int> nowLattice = std::make_pair(((nowDot.first + 150) % 300 + 300) % 300, ((nowDot.second + 150) % 300 + 300) % 300);

    if (nowLattice.first != normalFinalLattice.first && nowLattice.second != normalFinalLattice.second) {
        if (map[nowLattice.first][normalFinalLattice.second] == 1)
            deltaPos.y = 0;
        if (map[normalFinalLattice.first][nowLattice.second] == 1)    
            deltaPos.x = 0;
    } else {
        if (nowLattice.first != normalFinalLattice.first && map[normalFinalLattice.first][normalFinalLattice.second] == 1) 
            deltaPos.x = 0;
        if (nowLattice.second != normalFinalLattice.second && map[normalFinalLattice.first][normalFinalLattice.second] == 1)
            deltaPos.y = 0;
    }

    // return playerPos + deltaPos;


    glm::vec2 finalPos = playerPos + deltaPos;
    std::pair<int, int> finalDot = std::make_pair((int)floor(finalPos.x), (int)floor(finalPos.y));
    std::pair<int, int> finalLattice = std::make_pair(((finalDot.first + 150) % 300 + 300) % 300, ((finalDot.second + 150) % 300 + 300) % 300);

    // std::cout << "now: " << nowLattice.first << ", " << nowLattice.second << std::endl;
    // std::cout << "normal: " << normalFinalLattice.first << ", " << normalFinalLattice.second << std::endl;
    // std::cout << "final: " << finalLattice.first << ", " << finalLattice.second << std::endl;

    // std::cout << "final: (" << finalLattice.first << ", " << finalLattice.second << ")" << std::endl;

    if (map[finalLattice.first][finalLattice.second] != 1) 
        return finalPos;
    

    // std::cout << "now: (" << nowLattice.first << ", " << nowLattice.second << ")" << std::endl;

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
        volumeDifLast = std::min(speed - volume, volumeDifLast + 0.0002f);
        // volume += std::min(speed - volume, (speed - volume) / 2 + volumeLast + 0.02f);
        volume += volumeDifLast;
    } else if (speed < volume) {
        volumeDifLast = std::min(volume - speed, volumeDifLast + 0.0002f);
        // volume -= std::min(volume - speed, (volume - speed) / 2 + volumeLast + 0.02f);
        volume -= volumeDifLast;
    }


    cameraHeight = height + volume * sin(timeNow * 4 * pi);
    return glm::vec3(playerPos.x, cameraHeight, playerPos.y);
}