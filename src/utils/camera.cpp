#include "camera.hpp"

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
        transform.position, transform.position + transform.getFront(), transform.getUp());
}

PerspectiveCamera::PerspectiveCamera(float fovy, float aspect, float znear, float zfar)
    : fovy(fovy), aspect(aspect), znear(znear), zfar(zfar) {}

glm::mat4 PerspectiveCamera::getProjectionMatrix() const {
    return glm::perspective(fovy, aspect, znear, zfar);
}

Frustum PerspectiveCamera::getFrustum() const {
    Frustum frustum;
    
    const glm::vec3 fv = transform.getFront();  // front vector
    const glm::vec3 rv = transform.getRight();  // right vector  
    const glm::vec3 uv = transform.getUp();     // up vector
    
    // Calculate half dimensions at near and far planes
    const float halfVSide = zfar * tanf(fovy * 0.5f);
    const float halfHSide = halfVSide * aspect;
    const float halfVSideNear = znear * tanf(fovy * 0.5f);
    const float halfHSideNear = halfVSideNear * aspect;
    
    // Front and back face normals point inward
    frustum.planes[Frustum::NearFace] = {transform.position + znear * fv, fv};
    frustum.planes[Frustum::FarFace] = {transform.position + zfar * fv, -fv};
    
    // Side planes: calculate normals for the pyramid sides
    // Left plane
    glm::vec3 leftNormal = glm::normalize(glm::cross(fv - rv * (halfHSide / zfar), uv));
    frustum.planes[Frustum::LeftFace] = {transform.position, leftNormal};
    
    // Right plane  
    glm::vec3 rightNormal = glm::normalize(glm::cross(uv, fv + rv * (halfHSide / zfar)));
    frustum.planes[Frustum::RightFace] = {transform.position, rightNormal};
    
    // Bottom plane
    glm::vec3 bottomNormal = glm::normalize(glm::cross(rv, fv - uv * (halfVSide / zfar)));
    frustum.planes[Frustum::BottomFace] = {transform.position, bottomNormal};
    
    // Top plane
    glm::vec3 topNormal = glm::normalize(glm::cross(fv + uv * (halfVSide / zfar), rv));
    frustum.planes[Frustum::TopFace] = {transform.position, topNormal};

    return frustum;
}

OrthographicCamera::OrthographicCamera(
    float left, float right, float bottom, float top, float znear, float zfar)
    : left(left), right(right), top(top), bottom(bottom), znear(znear), zfar(zfar) {}

glm::mat4 OrthographicCamera::getProjectionMatrix() const {
    return glm::ortho(left, right, bottom, top, znear, zfar);
}

Frustum OrthographicCamera::getFrustum() const {
    Frustum frustum;
    const glm::vec3 fv = transform.getFront();
    const glm::vec3 rv = transform.getRight();
    const glm::vec3 uv = transform.getUp();

    // all of the plane normal points inside the frustum, maybe it's a convention
    frustum.planes[Frustum::NearFace] = {transform.position + znear * fv, fv};
    frustum.planes[Frustum::FarFace] = {transform.position + zfar * fv, -fv};
    frustum.planes[Frustum::LeftFace] = {transform.position - right * rv, rv};
    frustum.planes[Frustum::RightFace] = {transform.position + right * rv, -rv};
    frustum.planes[Frustum::BottomFace] = {transform.position - bottom * uv, uv};
    frustum.planes[Frustum::TopFace] = {transform.position + top * uv, -uv};

    return frustum;
}