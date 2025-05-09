#ifndef _UTILS_ADVANCE_CAMERA_HPP
#define _UTILS_ADVANCE_CAMERA_HPP

#include <memory>
#include <chrono>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "utils/camera.hpp"
#include "utils/input.hpp"
#include "utils/bounding_box.hpp"

// Camera mode enumeration
enum class CameraMode {
    FREE_ROAM,  // Camera can move and rotate
    FIXED,      // Camera can only rotate but not move
    PAN,        // Camera can only move parallel to view plane
    ORBIT       // Camera orbits around a target point
};

class AdvanceCamera {
public:
    // Constructor
    AdvanceCamera(int windowWidth, int windowHeight);
    
    // Destructor
    ~AdvanceCamera() = default;
    
    // Process input and update camera state
    void processInput(const Input& input);
    
    // Get camera reference
    Camera* getCamera() const;
    
    // Set camera mode directly
    void setMode(CameraMode mode);
    
    // Get current camera mode
    CameraMode getMode() const;
    
    // Set camera movement speed
    void setMoveSpeed(float speed);
    
    // Set camera rotation speed
    void setRotateSpeed(float speed);
    
    // Set orbit rotation speed
    void setOrbitSpeed(float speed);
    
    // Set zoom speed
    void setZoomSpeed(float speed);
    
    // Set animation speed for smooth transitions
    void setAnimationSpeed(float speed);
    
    // Get current field of view (in radians)
    float getFOV() const;
    
    // Get mode name as string
    std::string getModeString() const;
    
    // Set orbit target point
    void setOrbitTarget(const glm::vec3& target);
    
    // Get orbit target point
    glm::vec3 getOrbitTarget() const;
    
    // Update orbit camera position based on angles and distance
    void updateOrbitPosition();
    
    // Zoom To Fit - Automatically adjust camera to fit target object
    void zoomToFit(const BoundingBox& targetBBox, float padding = 1.2f);
    
    // Update camera position and rotation during animation
    void updateAnimation(float deltaTime);
    
    // Check if camera is currently animating
    bool isAnimating() const;

    // Get near clipping plane distance
    float getNearClippingPlane() const;
    
    // Set near clipping plane distance
    void setNearClippingPlane(float nearPlane);
    
    // Get far clipping plane distance
    float getFarClippingPlane() const;
    
    // Set far clipping plane distance
    void setFarClippingPlane(float farPlane);

private:
    // The camera instance
    std::unique_ptr<Camera> _camera;
    
    // Current camera mode
    CameraMode _mode = CameraMode::FREE_ROAM;
    
    // Previous camera mode (before Zoom To Fit)
    CameraMode _previousMode = CameraMode::FREE_ROAM;
    
    // Camera movement speed
    float _moveSpeed = 0.1f;
    
    // Camera rotation speed
    float _rotateSpeed = 0.002f;
    
    // Camera orbit speed
    float _orbitSpeed = 0.002f;
    
    // Camera zoom speed
    float _zoomSpeed = 2.0f;
    
    // Animation speed for smooth transitions
    float _animationSpeed = 2.0f;
    
    // Field of view limits (in radians)
    float _minFOV = glm::radians(5.0f);  // Zoomed in (narrow FOV)
    float _maxFOV = glm::radians(120.0f);  // Zoomed out (wide FOV)
    
    // Current field of view (in radians)
    float _fov = glm::radians(45.0f);
    
    // Orbit target point
    glm::vec3 _orbitTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    
    // Orbit distance
    float _orbitDistance = 15.0f;
    
    // Orbit horizontal angle (around Y-axis)
    float _orbitHorizontalAngle = 0.0f;
    
    // Orbit vertical angle (from XZ-plane)
    float _orbitVerticalAngle = 0.0f;
    
    // Animation target values
    struct AnimationTarget {
        glm::vec3 position;
        glm::quat rotation;
        float fov;
        float nearPlane;
        float farPlane;
        bool active = false;
        float duration = 1.0f;
        float elapsed = 0.0f;
        bool restorePreviousMode = false;
    } _animTarget;
    
    // Process camera movement in FREE_ROAM mode
    void processMovement(const Input& input);
    
    // Process camera rotation
    void processRotation(const Input& input);
    
    // Process camera panning (parallel to view plane)
    void processPanning(const Input& input);
    
    // Process camera orbiting around target point
    void processOrbiting(const Input& input);
    
    // Process camera zoom
    void processZoom(const Input& input);
    
    // Calculate required distance to view bounding box
    float calculateDistanceForBoundingBox(const BoundingBox& bbox, float fov, float aspectRatio, float padding) const;
    
    // Interpolate between two values
    template<typename T>
    T interpolate(const T& start, const T& end, float t) const;
    
    // Update camera projection after FOV change
    void updateProjection(float aspectRatio);
};

#endif // _UTILS_ADVANCE_CAMERA_HPP 