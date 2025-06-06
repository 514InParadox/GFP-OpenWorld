#pragma once

#include <glm/glm.hpp>
#include <random>
#include "map.hpp"

enum class EntityStatus {
    PATROL = 0,  // 巡逻状态 - 随机游走寻找玩家
    CHASE = 1    // 追逐状态 - 能看到玩家或正在前往最后看见玩家的位置
};

class EntityLogic {
public:
    EntityStatus Status; // Entity status: PATROL or CHASE
    float speed; // Movement speed
    
    EntityLogic() : Status(EntityStatus::PATROL), speed(5.0f), _entityPos(0.0f, 0.0f), _timeSinceStatusZero(0.0f), _lastSeenPlayerPos(0.0f, 0.0f) {} // Constructor to initialize Status and speed
    
    // Move entity based on player position and delta time
    // This function will automatically update Status and position based on line of sight
    void move(const glm::vec2 &playerPos, float deltaTime);
    
    // Entity position getter and setter
    glm::vec2 getEntityPos() const { return _entityPos; }
    void setEntityPos(const glm::vec2 &pos) { _entityPos = pos; }
    
    // Randomly place entity around player without line of sight
    void randomPlace(const glm::vec2 &playerPos);
    
    // Check and handle auto-relocation if needed
    void checkAutoRelocate(const glm::vec2 &playerPos);

private:
    // Time tracking variables
    float _timeSinceStatusZero = 0.0f; // Time since status became PATROL
    
    // Entity position
    glm::vec2 _entityPos;
    
    // Last seen player position (for CHASE behavior when losing sight)
    glm::vec2 _lastSeenPlayerPos;
    
    // Random number generator
    mutable std::random_device _rd;
    mutable std::mt19937 _gen{_rd()};
    
    // Constants
    static constexpr float AUTO_RELOCATE_TIMEOUT = 30.0f; // 30 seconds timeout for auto-relocation
    static constexpr float RANDOM_PLACE_MIN_RADIUS = 10.0f; // Minimum distance from player
    static constexpr float RANDOM_PLACE_MAX_RADIUS = 40.0f; // Maximum distance from player
    static constexpr int MAX_PLACEMENT_ATTEMPTS = 100; // Maximum attempts to find valid position
    static constexpr float ARRIVAL_THRESHOLD = 0.3f; // Distance threshold to consider "arrived" at target
};