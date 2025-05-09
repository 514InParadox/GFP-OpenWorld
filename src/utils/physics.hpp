#ifndef _UTILS_PHYSICS_H
#define _UTILS_PHYSICS_H

#include <glm/glm.hpp>
#include <functional>

// Forward declaration to avoid circular dependency
class Model;

// Physics component that can be attached to models
class Physics {
public:
    // Constructor
    Physics();
    
    // Destructor
    ~Physics() = default;
    
    // Update physics state
    void update(float deltaTime);
    
    // Apply force
    void applyForce(const glm::vec3& force);
    
    // Apply impulse
    void applyImpulse(const glm::vec3& impulse);
    
    // Set object mass (kg)
    void setMass(float mass);
    
    // Get object mass
    float getMass() const;
    
    // Set gravity influence
    void setGravityEnabled(bool enabled);
    
    // Check if gravity is enabled
    bool isGravityEnabled() const;
    
    // Set whether physics properties are fixed (not affected by forces)
    void setStatic(bool isStatic);
    
    // Check if object is static
    bool isStatic() const;
    
    // Set restitution coefficient (0.0-1.0)
    void setRestitution(float restitution);
    
    // Get restitution coefficient
    float getRestitution() const;
    
    // Set friction coefficient (0.0-1.0)
    void setFriction(float friction);
    
    // Get friction coefficient
    float getFriction() const;
    
    // Set gravity acceleration
    void setGravity(const glm::vec3& gravity);
    
    // Get current gravity
    const glm::vec3& getGravity() const;
    
    // Set associated model, establish bidirectional reference
    void setModel(Model* model);
    
    // Get associated model
    Model* getModel() const;
    
    // Get position (from associated model)
    glm::vec3 getPosition() const;
    
    // Set position (update associated model)
    void setPosition(const glm::vec3& position);
    
    // Get velocity
    const glm::vec3& getVelocity() const;
    
    // Set velocity
    void setVelocity(const glm::vec3& velocity);
    
    // Get angular velocity
    const glm::vec3& getAngularVelocity() const;
    
    // Set angular velocity
    void setAngularVelocity(const glm::vec3& angularVelocity);
    
    // Handle bounce based on surface normal vector
    // normal: Surface normal vector (unit vector pointing outward from surface)
    // surfaceFriction: Surface friction coefficient (0-1)
    // minVelocityThreshold: Minimum velocity threshold, velocity will be set to 0 if below this value
    void bounce(const glm::vec3& normal, float surfaceFriction = 0.3f, float minVelocityThreshold = 0.1f);
    
private:
    // Physics properties
    float _mass = 1.0f;                        // Mass (kg)
    float _inverseMass = 1.0f;                 // Inverse mass (for acceleration calculation)
    bool _isStatic = false;                    // Whether object is fixed (not affected by forces)
    bool _gravityEnabled = true;               // Whether affected by gravity
    glm::vec3 _gravity = glm::vec3(0, -9.8f, 0); // Gravity acceleration (m/s^2)
    float _restitution = 0.7f;                 // Restitution coefficient (0.0-1.0)
    float _friction = 0.3f;                    // Friction coefficient (0.0-1.0)
    
    // Motion properties
    glm::vec3 _velocity = glm::vec3(0.0f);        // Linear velocity
    glm::vec3 _angularVelocity = glm::vec3(0.0f); // Angular velocity
    glm::vec3 _force = glm::vec3(0.0f);           // Current net force
    glm::vec3 _torque = glm::vec3(0.0f);          // Current torque
    
    // Associated model
    Model* _model = nullptr;
};

#endif 