#include "entity.hpp"
#include "player.hpp"

#include <glm/glm.hpp>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function to handle proper modulo for negative numbers
int properMod(int a, int b) {
    return ((a % b) + b) % b;
}

// Convert world coordinates to map indices, handling periodic boundary conditions
std::pair<int, int> worldToMapCoords(const glm::vec2 &worldPos) {
    int worldX = (int)floor(worldPos.x);
    int worldY = (int)floor(worldPos.y);
    
    // Convert to map coordinates using proper modulo
    // World range is conceptually infinite, but map repeats every 300 units
    // Map indices are 0-299, corresponding to world coordinates -150 to 149
    int mapX = properMod(worldX + 150, 300);
    int mapY = properMod(worldY + 150, 300);
    
    return std::make_pair(mapX, mapY);
}

// Check if there's a clear line of sight between two positions using computational geometry
bool hasLineOfSight(const glm::vec2 &pos1, const glm::vec2 &pos2) {
    // Calculate direction and distance
    glm::vec2 direction = pos2 - pos1;
    float distance = glm::length(direction);
    
    // If positions are the same, they can see each other
    if (distance < 0.001f) {
        return true;
    }
    
    // Normalize direction vector
    glm::vec2 normalizedDir = direction / distance;
    
    // Sample points along the line with small step size for precision
    float stepSize = 0.1f; // Sample every 0.1 units
    int numSteps = (int)(distance / stepSize) + 1;
    
    for (int i = 0; i <= numSteps; i++) {
        float t = (i == numSteps) ? distance : i * stepSize;
        glm::vec2 currentPos = pos1 + normalizedDir * t;
        
        // Convert world coordinates to map indices using proper modulo
        auto mapCoords = worldToMapCoords(currentPos);
        int mapX = mapCoords.first;
        int mapY = mapCoords.second;
        
        // Check if current position intersects with a wall
        if (map[mapX][mapY] == 1) {
            return false; // Wall blocks line of sight
        }
    }
    
    return true; // Clear line of sight
}

// Check if a position is valid (not in a wall)
bool isValidPosition(const glm::vec2 &pos) {
    // Convert world coordinates to map indices using proper modulo
    auto mapCoords = worldToMapCoords(pos);
    int mapX = mapCoords.first;
    int mapY = mapCoords.second;
    
    // Check if position is not in a wall
    return map[mapX][mapY] != 1;
}

// Generate a random direction vector
glm::vec2 generateRandomDirection(std::mt19937 &gen) {
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    float angle = angleDist(gen);
    return glm::vec2(cos(angle), sin(angle));
}

// Move entity based on player position and delta time
// This function automatically updates Status and position based on line of sight
void EntityLogic::move(const glm::vec2 &playerPos, float deltaTime) {
    // Check if entity and player can see each other
    bool canSeePlayer = hasLineOfSight(_entityPos, playerPos);
    
    glm::vec2 moveDirection(0.0f, 0.0f);
    
    if (canSeePlayer) {
        // Entity can see player - set status to CHASE
        EntityStatus previousStatus = Status;
        Status = EntityStatus::CHASE;
        _lastSeenPlayerPos = playerPos; // Update last seen player position
        
        // If status changed from PATROL to CHASE, we don't need to track patrol time anymore
        if (previousStatus == EntityStatus::PATROL) {
            _timeSinceStatusZero = 0.0f; // Reset patrol timer
        }
        
        // Calculate direction vector towards player
        glm::vec2 direction = playerPos - _entityPos;
        
        // Set move direction if positions are different
        if (glm::length(direction) > 0.0f) {
            moveDirection = glm::normalize(direction);
        }
    } else {
        // Entity cannot see player
        if (Status == EntityStatus::CHASE) {
            // Status is CHASE but can't see player - move towards last seen player position
            glm::vec2 directionToLastSeen = _lastSeenPlayerPos - _entityPos;
            float distanceToLastSeen = glm::length(directionToLastSeen);
            
            // Check if entity has arrived at the last seen position
            if (distanceToLastSeen <= ARRIVAL_THRESHOLD) {
                // Arrived at last seen position, change status to PATROL immediately
                Status = EntityStatus::PATROL;
                _timeSinceStatusZero = 0.0f; // Start tracking patrol time
                // Move randomly since we've arrived but player isn't here
                moveDirection = generateRandomDirection(_gen);
            } else {
                // Move towards last seen player position
                moveDirection = glm::normalize(directionToLastSeen);
            }
        } else if (Status == EntityStatus::PATROL) {
            // Status is PATROL - random walk
            // Update time since status became PATROL
            _timeSinceStatusZero += deltaTime;
            
            // Move in random direction
            moveDirection = generateRandomDirection(_gen);
        }
    }
    
    // Apply movement based on direction, speed, and deltaTime
    if (glm::length(moveDirection) > 0.0f) {
        glm::vec2 deltaPos = moveDirection * speed * deltaTime;
        
        // Use getCorrectPos from player.hpp for wall collision correction
        _entityPos = getCorrectPos(_entityPos, deltaPos);
    }
    
    // Check and handle auto-relocation if needed (called automatically after each move)
    checkAutoRelocate(playerPos);
}

// Check and handle auto-relocation if needed
void EntityLogic::checkAutoRelocate(const glm::vec2 &playerPos) {
    // Only check for auto-relocation if status is PATROL
    if (Status == EntityStatus::PATROL) {
        // If entity has been in PATROL status for more than 30 seconds, relocate
        if (_timeSinceStatusZero >= AUTO_RELOCATE_TIMEOUT) {
            randomPlace(playerPos);
            _timeSinceStatusZero = 0.0f; // Reset the timer after relocation
        }
    }
}

// Randomly place entity around player without line of sight
void EntityLogic::randomPlace(const glm::vec2 &playerPos) {
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(RANDOM_PLACE_MIN_RADIUS, RANDOM_PLACE_MAX_RADIUS);
    
    for (int attempt = 0; attempt < MAX_PLACEMENT_ATTEMPTS; ++attempt) {
        // Generate random angle and radius
        float angle = angleDist(_gen);
        float radius = radiusDist(_gen);
        
        // Calculate potential position
        glm::vec2 potentialPos = playerPos + glm::vec2(
            radius * cos(angle),
            radius * sin(angle)
        );
        
        // Check if position is valid (not in wall) and has no line of sight to player
        if (isValidPosition(potentialPos) && !hasLineOfSight(potentialPos, playerPos)) {
            _entityPos = potentialPos;
            return; // Successfully placed entity
        }
    }
    
    // If we couldn't find a valid position after MAX_PLACEMENT_ATTEMPTS,
    // place entity at a default position (this shouldn't happen often)
    _entityPos = playerPos + glm::vec2(RANDOM_PLACE_MAX_RADIUS, 0.0f);
}
