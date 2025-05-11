#define GLM_ENABLE_EXPERIMENTAL
#include "utils/advance_camera.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <iostream>
#include <cmath>   // For std::isfinite

// Constructor
AdvanceCamera::AdvanceCamera(int windowWidth, int windowHeight) {
    // Ensure FOV is in valid range before initializing camera
    if (_fov < _minFOV || _fov > _maxFOV || !std::isfinite(_fov)) {
        _fov = glm::radians(45.0f);
        std::cout << "Initial FOV outside valid range, reset to: " << glm::degrees(_fov) << " degrees" << std::endl;
    }
    
    // Default clipping planes - simplified
    _nearPlane = 0.1f;
    _farPlane = 1000.0f;
    
    // Initialize camera with perspective projection
    const float aspect = 1.0f * windowWidth / windowHeight;
    _camera.reset(new PerspectiveCamera(_fov, aspect, _nearPlane, _farPlane));
    
    // No need for separate animation targets for clipping planes
    _animTarget.fov = _fov;
    
    // Default position
    _camera->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);
    
    // Initialize orbit parameters based on camera position
    _orbitDistance = glm::length(_camera->transform.position - _orbitTarget);
    _orbitHorizontalAngle = 0.0f;  // Initially looking along negative Z
    _orbitVerticalAngle = 0.0f;    // Initially at the equator
}

// Process all input for camera
void AdvanceCamera::processInput(const Input& input) {
    // Skip input processing if camera is animating
    if (_animTarget.active) {
        return;
    }
    
    // Process zoom in all modes
    if (input.mouse.scroll.yOffset != 0.0f) {
        processZoom(input);
    }

    // Process inputs based on the current camera mode
    switch (_mode) {
        case CameraMode::FREE_ROAM:
            // Process rotation
            if (input.mouse.move.xNow != input.mouse.move.xOld || 
                input.mouse.move.yNow != input.mouse.move.yOld) {
                processRotation(input);
            }
            
            // Process movement
            if (input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_Q] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_E] != GLFW_RELEASE) {
                processMovement(input);
            }
            break;
            
        case CameraMode::FIXED:
            // Process rotation only
            if (input.mouse.move.xNow != input.mouse.move.xOld || 
                input.mouse.move.yNow != input.mouse.move.yOld) {
                processRotation(input);
            }
            break;
            
        case CameraMode::PAN:
            // Process panning
            if (input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE ||
                input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE ||
                input.mouse.press.left) {
                processPanning(input);
            }
            break;
            
        case CameraMode::ORBIT:
            // Process orbiting
            if (input.mouse.move.xNow != input.mouse.move.xOld || 
                input.mouse.move.yNow != input.mouse.move.yOld) {
                processOrbiting(input);
            }
            
            // Process orbit distance with keyboard
            if (input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
                // Move closer to target
                _orbitDistance -= _moveSpeed;
                if (_orbitDistance < 0.1f) _orbitDistance = 0.1f;
                updateOrbitPosition();
            }
            if (input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
                // Move away from target
                _orbitDistance += _moveSpeed;
                updateOrbitPosition();
            }
            break;
    }
}

// Zoom To Fit function (simplified clipping plane handling)
void AdvanceCamera::zoomToFit(const BoundingBox& targetBBox, float padding) {
    if (targetBBox.isEmpty()) {
        std::cout << "Warning: Cannot zoom to fit an empty bounding box." << std::endl;
        return;
    }
    
    // Save previous mode before switching to ORBIT
    _previousMode = _mode;
    
    // Switch to ORBIT mode if not already in it
    if (_mode != CameraMode::ORBIT) {
        _mode = CameraMode::ORBIT;
        std::cout << "Switching to ORBIT mode for Zoom To Fit." << std::endl;
    }
    
    // Set the orbit target to the center of the bounding box
    glm::vec3 bboxCenter = (targetBBox.min + targetBBox.max) * 0.5f;
    _orbitTarget = bboxCenter;
    
    // Calculate bounding box dimensions for safety checks
    glm::vec3 dimensions = targetBBox.max - targetBBox.min;
    float maxDimension = std::max(std::max(dimensions.x, dimensions.y), dimensions.z);
    
    // Calculate ideal FOV and distance
    float aspectRatio = 1.0f;
    PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
    if (perspCamera) {
        aspectRatio = perspCamera->aspect;
    }
    
    // Use a reasonable default FOV for viewing
    // Start with current FOV to avoid large jumps
    float targetFOV = _fov;
    // Adjust if outside reasonable range (avoid extremes)
    if (targetFOV < glm::radians(30.0f) || targetFOV > glm::radians(70.0f)) {
        targetFOV = glm::radians(45.0f);
    }

    // Strictly enforce FOV limits
    if (targetFOV < _minFOV) targetFOV = _minFOV;
    if (targetFOV > _maxFOV) targetFOV = _maxFOV;
    
    // Calculate the needed distance to see the entire bounding box
    float targetDistance = calculateDistanceForBoundingBox(targetBBox, targetFOV, aspectRatio, padding);
    
    // Ensure minimum safe distance (prevent camera from being inside model)
    float safeMinDistance = maxDimension * 0.6f;
    targetDistance = std::max(targetDistance, safeMinDistance);
    
    // Set horizontal and vertical orbit angles
    // Use current angles for smoother transition
    float targetHorizontalAngle = _orbitHorizontalAngle;
    float targetVerticalAngle = 0.3f; // Slightly from above
    
    // Calculate target position in spherical coordinates
    float targetPosX = targetDistance * cos(targetVerticalAngle) * sin(targetHorizontalAngle);
    float targetPosY = targetDistance * sin(targetVerticalAngle);
    float targetPosZ = targetDistance * cos(targetVerticalAngle) * cos(targetHorizontalAngle);
    
    // Target position in world space
    glm::vec3 targetPosition = _orbitTarget + glm::vec3(targetPosX, targetPosY, targetPosZ);
    
    // Calculate target rotation (looking at the center)
    glm::vec3 direction = glm::normalize(_orbitTarget - targetPosition);
    glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 up = glm::normalize(glm::cross(right, direction));
    
    glm::mat3 rotMatrix;
    rotMatrix[0] = right;
    rotMatrix[1] = up;
    rotMatrix[2] = -direction;
    glm::quat targetRotation = glm::quat_cast(rotMatrix);
    
    // Setup animation target (simplified)
    _animTarget.position = targetPosition;
    _animTarget.rotation = targetRotation;
    _animTarget.fov = targetFOV;
    _animTarget.active = true;
    _animTarget.elapsed = 0.0f;
    _animTarget.duration = 1.0f / _animationSpeed;
    _animTarget.restorePreviousMode = true;  // Flag to restore previous mode after animation
    
    // Store orbit parameters for after animation
    _orbitDistance = targetDistance;
    _orbitHorizontalAngle = targetHorizontalAngle;
    _orbitVerticalAngle = targetVerticalAngle;
    
    std::cout << "Zoom To Fit animation started." << std::endl;
    std::cout << "Target: (" << bboxCenter.x << ", " << bboxCenter.y << ", " << bboxCenter.z << ")" << std::endl;
    std::cout << "Distance: " << targetDistance << std::endl;
    std::cout << "Current FOV: " << glm::degrees(_fov) << " degrees, Target FOV: " << glm::degrees(targetFOV) << " degrees" << std::endl;
}

// Update animation
void AdvanceCamera::updateAnimation(float deltaTime) {
    if (!_animTarget.active) {
        return;
    }
    
    // Update elapsed time
    _animTarget.elapsed += deltaTime;
    
    // Calculate interpolation factor, strictly limited to [0, 1]
    float rawT = _animTarget.elapsed / _animTarget.duration;
    
    // Force completion once we're close enough to avoid floating point precision issues
    if (rawT >= 0.999f) {
        // Set to exact final values
        _camera->transform.position = _animTarget.position;
        _camera->transform.rotation = _animTarget.rotation;
        
        // Set exact FOV 
        PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
        if (perspCamera) {
            _fov = _animTarget.fov;
            perspCamera->fovy = _fov;
            // No need to handle clipping planes separately
        }
        
        // If we're restoring to previous mode, keep the current position/rotation 
        // but update orbit parameters for future use
        if (_animTarget.restorePreviousMode) {
            // Store the target position and orientation before mode change
            glm::vec3 finalPosition = _camera->transform.position;
            glm::quat finalRotation = _camera->transform.rotation;
            
            // Set the previous mode without updating position
            _mode = _previousMode;
            std::cout << "Restored camera mode: " << getModeString() << std::endl;
            
            // Ensure camera maintains the final position after mode switch
            _camera->transform.position = finalPosition;
            _camera->transform.rotation = finalRotation;
            
            // Update the orbit parameters for future ORBIT mode usage
            _orbitDistance = glm::length(finalPosition - _orbitTarget);
            
            // Calculate angles from final position
            glm::vec3 directionToCamera = finalPosition - _orbitTarget;
            float distance = glm::length(directionToCamera);
            if (distance > 0.001f) {
                _orbitHorizontalAngle = atan2(directionToCamera.x, directionToCamera.z);
                _orbitVerticalAngle = asin(glm::clamp(directionToCamera.y / distance, -1.0f, 1.0f));
            }
        }
        
        // Mark animation as complete
        _animTarget.active = false;
        std::cout << "Zoom To Fit animation completed." << std::endl;
        return;
    }
    
    // Strictly clamp t to [0, 1] to prevent unexpected behaviors
    float t = std::max(0.0f, std::min(rawT, 1.0f));
    
    // Simple cubic ease-in-out for smooth animation
    t = t * t * (3.0f - 2.0f * t);
    
    // Interpolate position
    _camera->transform.position = interpolate(_camera->transform.position, _animTarget.position, t);
    
    // Interpolate rotation (using slerp for quaternions)
    _camera->transform.rotation = glm::slerp(_camera->transform.rotation, _animTarget.rotation, t);
    
    // Interpolate FOV if using perspective camera
    PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
    if (perspCamera) {
        // Direct interpolation between current and target FOV
        _fov = interpolate(_fov, _animTarget.fov, t);
        
        // Ensure FOV is valid
        if (_fov < _minFOV) _fov = _minFOV;
        if (_fov > _maxFOV) _fov = _maxFOV;
        
        // Apply to camera
        perspCamera->fovy = _fov;
    }
}

// Check if camera is currently animating
bool AdvanceCamera::isAnimating() const {
    return _animTarget.active;
}

// Set animation speed
void AdvanceCamera::setAnimationSpeed(float speed) {
    _animationSpeed = std::max(0.1f, speed);
}

// Calculate distance needed to view entire bounding box
float AdvanceCamera::calculateDistanceForBoundingBox(const BoundingBox& bbox, float fov, float aspectRatio, float padding) const {
    // Calculate bounding box dimensions
    glm::vec3 dimensions = bbox.max - bbox.min;
    
    // Consider all dimensions for proper framing
    float boxWidth = dimensions.x;
    float boxHeight = dimensions.y;
    float boxDepth = dimensions.z;
    
    // Calculate the radius of the bounding sphere that contains the box
    float radius = glm::length(dimensions) * 0.5f;
    
    // Determine required distance based on vertical FOV and aspect ratio
    float distance;
    
    if (aspectRatio >= 1.0f) { // Width > Height
        // For wide screens, height is the limiting factor
        distance = radius / sin(fov * 0.5f);
    } else { // Height > Width
        // For tall screens, width is the limiting factor
        float horizontalFov = 2.0f * atan(tan(fov * 0.5f) * aspectRatio);
        distance = radius / sin(horizontalFov * 0.5f);
    }
    
    // Add padding factor
    distance *= padding;
    
    // Ensure minimum viewing distance
    float minDistance = std::max(boxWidth, std::max(boxHeight, boxDepth)) * 0.5f;
    return std::max(distance, minDistance);
}

// Generic linear interpolation between two values
template<typename T>
T AdvanceCamera::interpolate(const T& start, const T& end, float t) const {
    return start * (1.0f - t) + end * t;
}

// Get camera pointer
Camera* AdvanceCamera::getCamera() const {
    return _camera.get();
}

// Set camera mode
void AdvanceCamera::setMode(CameraMode mode) {
    // If already in this mode, do nothing
    if (_mode == mode) {
        return;
    }
    
    CameraMode oldMode = _mode;
    _mode = mode;
    
    // If switching to ORBIT mode, initialize orbit parameters
    if (_mode == CameraMode::ORBIT && oldMode != CameraMode::ORBIT) {
        // Don't modify position here - we'll just update internal orbit tracking
        // to match current camera position
        
        // Calculate distance from camera to target
        float distance = glm::length(_camera->transform.position - _orbitTarget);
        
        // Only update orbit distance if it's not already set
        if (_orbitDistance <= 0.1f || _orbitDistance > 1000.0f) {
            _orbitDistance = distance;
        }
        
        // Calculate initial horizontal and vertical angles
        glm::vec3 directionToCamera = _camera->transform.position - _orbitTarget;
        if (distance > 0.001f) { // Avoid division by zero
            _orbitHorizontalAngle = atan2(directionToCamera.x, directionToCamera.z);
            _orbitVerticalAngle = asin(glm::clamp(directionToCamera.y / distance, -1.0f, 1.0f));
        }
        
        // Output current orbit parameters (for debugging)
        std::cout << "Orbit mode initialized:" << std::endl;
        std::cout << "  Target: (" << _orbitTarget.x << ", " << _orbitTarget.y << ", " << _orbitTarget.z << ")" << std::endl;
        std::cout << "  Distance: " << _orbitDistance << std::endl;
        std::cout << "  Horizontal angle: " << glm::degrees(_orbitHorizontalAngle) << " degrees" << std::endl;
        std::cout << "  Vertical angle: " << glm::degrees(_orbitVerticalAngle) << " degrees" << std::endl;
    }
}

// Get current camera mode
CameraMode AdvanceCamera::getMode() const {
    return _mode;
}

// Get mode name as string
std::string AdvanceCamera::getModeString() const {
    switch (_mode) {
        case CameraMode::FREE_ROAM: return "FREE_ROAM";
        case CameraMode::FIXED: return "FIXED";
        case CameraMode::PAN: return "PAN";
        case CameraMode::ORBIT: return "ORBIT";
        default: return "UNKNOWN";
    }
}

// Set orbit target point
void AdvanceCamera::setOrbitTarget(const glm::vec3& target) {
    // Store old target for calculating position difference
    glm::vec3 oldTarget = _orbitTarget;
    
    // Set new target
    _orbitTarget = target;
    
    // Recalculate distance from camera to new target
    _orbitDistance = glm::length(_camera->transform.position - _orbitTarget);
    
    // Recalculate angles based on new target
    glm::vec3 directionToCamera = _camera->transform.position - _orbitTarget;
    float distance = glm::length(directionToCamera);
    
    if (distance > 0.001f) {
        _orbitHorizontalAngle = atan2(directionToCamera.x, directionToCamera.z);
        _orbitVerticalAngle = asin(glm::clamp(directionToCamera.y / distance, -1.0f, 1.0f));
    }
    
    // Note: We don't call updateOrbitPosition() here to avoid moving the camera
    // The camera should stay in place, only the orbit parameters are updated
    
    std::cout << "Orbit target set to: " 
              << _orbitTarget.x << ", " 
              << _orbitTarget.y << ", " 
              << _orbitTarget.z << std::endl;
}

// Get orbit target point
glm::vec3 AdvanceCamera::getOrbitTarget() const {
    return _orbitTarget;
}

// Set camera movement speed
void AdvanceCamera::setMoveSpeed(float speed) {
    _moveSpeed = speed;
}

// Set camera rotation speed
void AdvanceCamera::setRotateSpeed(float speed) {
    _rotateSpeed = speed;
}

// Set orbit rotation speed
void AdvanceCamera::setOrbitSpeed(float speed) {
    _orbitSpeed = speed;
}

// Set camera zoom speed
void AdvanceCamera::setZoomSpeed(float speed) {
    _zoomSpeed = speed;
}

// Get current field of view
float AdvanceCamera::getFOV() const {
    return _fov;
}

// Set field of view (in radians)
void AdvanceCamera::setFOV(float fov) {
    // Validate FOV
    _fov = glm::clamp(fov, _minFOV, _maxFOV);
    
    // Apply to camera
    PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
    if (perspCamera) {
        perspCamera->fovy = _fov;
        // std::cout << "FOV set to: " << glm::degrees(_fov) << " degrees" << std::endl;
    }
}

// Process camera movement based on keyboard input
void AdvanceCamera::processMovement(const Input& input) {
    // WASD movement
    if (input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position + _moveSpeed * _camera->transform.getFront();
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position - _moveSpeed * _camera->transform.getRight();
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position - _moveSpeed * _camera->transform.getFront();
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position + _moveSpeed * _camera->transform.getRight();
    }
    
    // Up/Down movement
    if (input.keyboard.keyStates[GLFW_KEY_Q] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position - _moveSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_E] != GLFW_RELEASE) {
        _camera->transform.position = _camera->transform.position + _moveSpeed * glm::vec3(0.0f, 1.0f, 0.0f);
    }
}

// Process camera panning (move parallel to view plane)
void AdvanceCamera::processPanning(const Input& input) {
    // Get camera's right and up vectors
    glm::vec3 right = _camera->transform.getRight();
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Create view plane vectors (right and up, both perpendicular to view direction)
    glm::vec3 viewPlaneRight = right;
    glm::vec3 viewPlaneUp = glm::normalize(glm::cross(right, _camera->transform.getFront()));
    
    // Process keyboard input for panning
    if (input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        _camera->transform.position += _moveSpeed * viewPlaneRight;
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        _camera->transform.position -= _moveSpeed * viewPlaneRight;
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        _camera->transform.position += _moveSpeed * viewPlaneUp;
    }
    
    if (input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        _camera->transform.position -= _moveSpeed * viewPlaneUp;
    }
    
    // Process mouse input for panning
    if (input.mouse.press.left) {
        float dx = input.mouse.move.xNow - input.mouse.move.xOld;
        float dy = input.mouse.move.yNow - input.mouse.move.yOld;
        
        // Move camera opposite to mouse direction
        _camera->transform.position -= 0.01f * dx * viewPlaneRight;
        _camera->transform.position += 0.01f * dy * viewPlaneUp;
    }
}

// Process camera orbiting around target point
void AdvanceCamera::processOrbiting(const Input& input) {
    // Reduce rotation speed for smoother control
    float deltaX = _orbitSpeed * 0.5f * (input.mouse.move.xNow - input.mouse.move.xOld);
    float deltaY = _orbitSpeed * 0.5f * (input.mouse.move.yNow - input.mouse.move.yOld);
    
    // Update horizontal and vertical angles
    _orbitHorizontalAngle -= deltaX;
    _orbitVerticalAngle += deltaY;
    
    // Limit vertical angle to avoid flipping (gimbal lock)
    const float maxVerticalAngle = glm::radians(80.0f);
    _orbitVerticalAngle = glm::clamp(_orbitVerticalAngle, -maxVerticalAngle, maxVerticalAngle);
    
    // Update camera position
    updateOrbitPosition();
}

// Update orbit camera position based on angles and distance
void AdvanceCamera::updateOrbitPosition() {
    // Calculate camera position in spherical coordinates
    float posX = _orbitDistance * cos(_orbitVerticalAngle) * sin(_orbitHorizontalAngle);
    float posY = _orbitDistance * sin(_orbitVerticalAngle);
    float posZ = _orbitDistance * cos(_orbitVerticalAngle) * cos(_orbitHorizontalAngle);
    
    // Update camera position
    _camera->transform.position = _orbitTarget + glm::vec3(posX, posY, posZ);
    
    // Make camera look at target
    glm::vec3 direction = glm::normalize(_orbitTarget - _camera->transform.position);
    
    // Calculate orthogonal basis vectors (right, up, forward)
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(direction, worldUp));
    
    // Handle case when camera is directly above or below target
    if (glm::length(right) < 0.001f) {
        right = glm::vec3(1.0f, 0.0f, 0.0f);
    }
    
    glm::vec3 up = glm::normalize(glm::cross(right, direction));
    
    // Create rotation matrix and convert to quaternion
    glm::mat3 rotationMatrix;
    rotationMatrix[0] = right;
    rotationMatrix[1] = up;
    rotationMatrix[2] = -direction;  // Camera looks along negative z axis
    
    _camera->transform.rotation = glm::quat_cast(rotationMatrix);
}

// Process camera rotation based on mouse input
void AdvanceCamera::processRotation(const Input& input) {
    // Horizontal rotation (around Y axis)
    if (input.mouse.move.xNow != input.mouse.move.xOld) {
        float mouseMovementX = input.mouse.move.xNow - input.mouse.move.xOld;
        float deltaX = _rotateSpeed * mouseMovementX;
        
        _camera->transform.rotation = glm::angleAxis(-deltaX, glm::vec3(0.0f, 1.0f, 0.0f)) * _camera->transform.rotation;
    }
    
    // Vertical rotation (around camera's right axis)
    if (input.mouse.move.yNow != input.mouse.move.yOld) {
        float mouseMovementY = input.mouse.move.yNow - input.mouse.move.yOld;
        float deltaY = _rotateSpeed * mouseMovementY;
        
        _camera->transform.rotation = glm::angleAxis(-deltaY, _camera->transform.getRight()) * _camera->transform.rotation;
    }
}

// Process camera zoom based on mouse scroll input
void AdvanceCamera::processZoom(const Input& input) {
    // Adjust FOV based on scroll input
    float zoomAmount = -input.mouse.scroll.yOffset * _zoomSpeed;
    
    if (_mode == CameraMode::ORBIT) {
        // In orbit mode, scrolling changes orbit distance instead of FOV
        _orbitDistance += 0.5f * zoomAmount;
        if (_orbitDistance < 0.1f) _orbitDistance = 0.1f;
        updateOrbitPosition();
        std::cout << "Orbit distance: " << _orbitDistance << std::endl;
    } else {
        // In other modes, scrolling changes FOV
        _fov += glm::radians(zoomAmount);
        // Clamp FOV to min/max values
        _fov = glm::clamp(_fov, _minFOV, _maxFOV);
        
        // Update the camera's projection matrix
        PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
        if (perspCamera) {
            perspCamera->fovy = _fov;
            std::cout << "FOV: " << glm::degrees(_fov) << " degrees" << std::endl;
        }
    }
}

// Update camera projection
void AdvanceCamera::updateProjection(float aspectRatio) {
    PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
    if (perspCamera) {
        perspCamera->fovy = _fov;
        perspCamera->aspect = aspectRatio;
        // No need for separate handling of clipping planes
    }
}

// Set clipping planes directly - simplified approach
void AdvanceCamera::setNearPlane(float nearPlane) {
    // Simple validation with minimum value
    _nearPlane = std::max(0.01f, nearPlane);
    
    // Ensure near < far
    if (_nearPlane >= _farPlane) {
        _nearPlane = _farPlane * 0.01f;
    }
    
    // Apply to camera
    PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
    if (perspCamera) {
        perspCamera->znear = _nearPlane;
    }
}

void AdvanceCamera::setFarPlane(float farPlane) {
    // Simple validation
    _farPlane = std::max(_nearPlane * 2.0f, farPlane);
    
    // Apply to camera
    PerspectiveCamera* perspCamera = dynamic_cast<PerspectiveCamera*>(_camera.get());
    if (perspCamera) {
        perspCamera->zfar = _farPlane;
    }
}

float AdvanceCamera::getNearPlane() const {
    return _nearPlane;
}

float AdvanceCamera::getFarPlane() const {
    return _farPlane;
} 