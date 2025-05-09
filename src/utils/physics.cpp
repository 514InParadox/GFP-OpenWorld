#include "utils/physics.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>
#include "model/model.hpp"

Physics::Physics() : 
    _mass(1.0f),
    _inverseMass(1.0f),
    _isStatic(false),
    _gravityEnabled(true),
    _gravity(0.0f, -9.8f, 0.0f),
    _restitution(0.85f),
    _friction(0.2f),
    _velocity(0.0f),
    _angularVelocity(0.0f),
    _force(0.0f),
    _torque(0.0f),
    _model(nullptr)
{
}

void Physics::update(float deltaTime) {
    // Static objects don't update physics state
    if (_isStatic || _model == nullptr) {
        return;
    }
    
    // Apply gravity
    if (_gravityEnabled) {
        _force += _mass * _gravity;
    }
    
    // Calculate acceleration (F = ma, a = F/m)
    glm::vec3 acceleration = _force * _inverseMass;
    
    // Update velocity (v = v0 + a*t)
    _velocity += acceleration * deltaTime;
    
    // Truncate very small velocities to prevent jittering
    const float velocityThreshold = 0.01f;
    if (glm::length(_velocity) < velocityThreshold) {
        _velocity = glm::vec3(0.0f);
    }
    
    // Update position (p = p0 + v*t)
    glm::vec3 newPosition = _model->transform.position + _velocity * deltaTime;
    _model->transform.position = newPosition;
    
    // Update rotation (simplified version, only considers rotation around local coordinate system)
    if (glm::length(_angularVelocity) > 0.0001f) {
        // Convert angular velocity to rotation quaternion
        glm::quat rotationDelta = glm::quat(0.0f, _angularVelocity * deltaTime);
        rotationDelta = glm::normalize(rotationDelta);
        
        // Apply rotation
        _model->transform.rotation = glm::normalize(_model->transform.rotation * rotationDelta);
    }
    
    // Reset forces and torques for next frame
    _force = glm::vec3(0.0f);
    _torque = glm::vec3(0.0f);
}

void Physics::bounce(const glm::vec3& normal, float surfaceFriction, float minVelocityThreshold) {
    if (_isStatic) return;  // Static objects don't bounce
    
    // Ensure normal is a unit vector
    glm::vec3 unitNormal = glm::normalize(normal);
    
    // Calculate velocity component along normal
    float velocityAlongNormal = glm::dot(_velocity, unitNormal);
    
    // Only bounce when object is moving towards surface (negative velocity projection on normal)
    if (velocityAlongNormal < 0) {
        // Debug output - before bounce
        // std::cout << "Before bounce - Velocity: (" 
        //         << _velocity.x << ", " << _velocity.y << ", " << _velocity.z 
        //         << "), Speed: " << glm::length(_velocity) << std::endl;
        
        // Calculate normal and tangent components of velocity
        glm::vec3 normalVelocity = velocityAlongNormal * unitNormal;
        glm::vec3 tangentVelocity = _velocity - normalVelocity;
        
        // Bounce: reverse normal velocity and apply restitution coefficient
        glm::vec3 reflectedNormalVelocity = -normalVelocity * _restitution;
        
        // Apply friction to reduce tangent velocity
        tangentVelocity *= (1.0f - surfaceFriction);
        
        // Combine reflected normal velocity and reduced tangent velocity
        _velocity = reflectedNormalVelocity + tangentVelocity;
        
        // Debug output - immediately after calculation
        // std::cout << "After reflection calculation - Velocity: (" 
        //         << _velocity.x << ", " << _velocity.y << ", " << _velocity.z 
        //         << "), Speed: " << glm::length(_velocity) << std::endl;
        
        // Only completely stop when total velocity is very small
        float totalSpeed = glm::length(_velocity);
        if (totalSpeed < minVelocityThreshold) {
            _velocity = glm::vec3(0.0f);
            // std::cout << "Speed too low, stopping motion" << std::endl;
        }
        
        // Debug output - final after bounce
        // std::cout << "Final velocity after bounce: (" 
        //         << _velocity.x << ", " << _velocity.y << ", " << _velocity.z 
        //         << "), Speed: " << glm::length(_velocity) << std::endl;
    }
}

void Physics::applyForce(const glm::vec3& force) {
    if (_isStatic) return;
    _force += force;
}

void Physics::applyImpulse(const glm::vec3& impulse) {
    if (_isStatic) return;
    _velocity += impulse * _inverseMass;
}

void Physics::setMass(float mass) {
    if (mass <= 0.0f) {
        // Objects with mass 0 are treated as static
        _mass = 0.0f;
        _inverseMass = 0.0f;
        _isStatic = true;
    } else {
        _mass = mass;
        _inverseMass = 1.0f / mass;
    }
}

float Physics::getMass() const {
    return _mass;
}

void Physics::setGravityEnabled(bool enabled) {
    _gravityEnabled = enabled;
}

bool Physics::isGravityEnabled() const {
    return _gravityEnabled;
}

void Physics::setStatic(bool isStatic) {
    _isStatic = isStatic;
    if (isStatic) {
        // Static objects have zero velocity and acceleration
        _velocity = glm::vec3(0.0f);
        _angularVelocity = glm::vec3(0.0f);
        _force = glm::vec3(0.0f);
        _torque = glm::vec3(0.0f);
        
        // Static objects have infinite mass, inverse is 0
        _inverseMass = 0.0f;
    } else if (_mass > 0.0f) {
        // If switching from static to dynamic, restore inverse mass
        _inverseMass = 1.0f / _mass;
    } else {
        // If mass is 0, set a default value
        _mass = 1.0f;
        _inverseMass = 1.0f;
    }
}

bool Physics::isStatic() const {
    return _isStatic;
}

void Physics::setRestitution(float restitution) {
    _restitution = std::min(std::max(restitution, 0.0f), 1.0f);
}

float Physics::getRestitution() const {
    return _restitution;
}

void Physics::setFriction(float friction) {
    _friction = std::min(std::max(friction, 0.0f), 1.0f);
}

float Physics::getFriction() const {
    return _friction;
}

void Physics::setGravity(const glm::vec3& gravity) {
    _gravity = gravity;
}

const glm::vec3& Physics::getGravity() const {
    return _gravity;
}

void Physics::setModel(Model* model) {
    _model = model;
}

Model* Physics::getModel() const {
    return _model;
}

glm::vec3 Physics::getPosition() const {
    if (_model) {
        return _model->transform.position;
    }
    return glm::vec3(0.0f);
}

void Physics::setPosition(const glm::vec3& position) {
    if (_model) {
        _model->transform.position = position;
    }
}

const glm::vec3& Physics::getVelocity() const {
    return _velocity;
}

void Physics::setVelocity(const glm::vec3& velocity) {
    if (!_isStatic) {
        _velocity = velocity;
    }
}

const glm::vec3& Physics::getAngularVelocity() const {
    return _angularVelocity;
}

void Physics::setAngularVelocity(const glm::vec3& angularVelocity) {
    if (!_isStatic) {
        _angularVelocity = angularVelocity;
    }
} 