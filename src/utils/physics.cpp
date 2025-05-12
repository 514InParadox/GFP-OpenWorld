#include "utils/physics.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>
#include "model/model.hpp"
#include <glm/gtx/quaternion.hpp>   // 新增

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
    _invInertiaLocal = glm::mat3(1.0f) * (5.0f / (2.0f * _mass * 0.5f * 0.5f)); // 均匀球逆惯性
    _invInertiaWorld = _invInertiaLocal;   // 初始朝向与对象空间一致

}

void Physics::update(float deltaTime) {
    // Static objects don't update physics state
    if (_isStatic || _model == nullptr) {
        return;
    }
    // 每帧把局部逆惯性张量旋到世界空间：R * I_body^{-1} * R^T
    glm::mat3 R = glm::mat3_cast(_model->transform.rotation);
    _invInertiaWorld = R * _invInertiaLocal * glm::transpose(R);

    
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
    // 更新，正确写法
    if (glm::length(_angularVelocity) > 1e-6f)
    {
        float angle = glm::length(_angularVelocity) * deltaTime;
        glm::vec3 axis = _angularVelocity / glm::length(_angularVelocity);
        glm::quat dq = glm::angleAxis(angle, axis);
        _model->transform.rotation = glm::normalize(dq * _model->transform.rotation);
    }
    
    // Reset forces and torques for next frame
    _force = glm::vec3(0.0f);
    _torque = glm::vec3(0.0f);
}

void Physics::integratePosition(float dt)
{
    if (_isStatic || _model == nullptr) return;
    _model->transform.position += _velocity * dt;
}

void Physics::applyImpulseAtPoint(const glm::vec3& impulse,
                                  const glm::vec3& worldPoint)
{
    if (_isStatic) return;
    // 线速度
    _velocity += impulse * _inverseMass;

    // 角速度：Δω = I^{-1} · (r × J)
    glm::vec3 r = worldPoint - _model->transform.position;   // 向量 r = P - C
    glm::vec3 dL = glm::cross(r, impulse);                   // 角动量增量
    _angularVelocity += _invInertiaWorld * dL;
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

Physics::Physics(Model* owner): _owner(owner)
{
    /* 默认均匀球体（半径 0.5）逆惯性 */
    _invInertiaLocal = glm::inverse(sphereInertia(_mass, 0.5f));
    updateInertiaTensor();
}

/* ---------- integrate(float dt) ---------- */
void Physics::integrate(float dt)
{
    if (_isStatic || !_owner) return;

    /* 更新世界逆惯性(随旋转) */
    updateInertiaTensor();

    /* 线速度 -> 位置 */
    _owner->transform.position += _linearV * dt;

    /* 角速度 -> 旋转四元数 */
    float wLen = glm::length(_angularV);
    if (wLen > 1e-6f) {
        float  angle = wLen * dt;
        glm::vec3 axis  = _angularV / wLen;
        _owner->transform.rotation =
            glm::normalize(glm::angleAxis(angle, axis) *
                           _owner->transform.rotation);
    }
}

void Physics::updateInertiaTensor()
{
    if (_isStatic) { _invInertiaWorld = glm::mat3(0); return; }
    glm::mat3 R = glm::mat3_cast(_owner->transform.rotation);
    _invInertiaWorld = R * _invInertiaLocal * glm::transpose(R);
}