#include "pch.h"
#include "Camera.h"
namespace sa {

    Camera::Camera()
        : m_projMat(1)
        , m_position(0, 0, 0)
        , m_forward(0, 0, 1)
        , m_up(0, 1, 0)
        , m_fov(glm::radians(60.0f))
        , m_apectRatio(1)
    {
        m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);
    }

    Camera::Camera(const RenderWindow* pWindow)
        : Camera(Vector2(pWindow->getCurrentExtent()))
    {
    }

    Camera::Camera(Vector2 windowExtent)
        : Camera()
    {
        m_viewport.setPosition(Vector2(0, 0));
        m_viewport.setSize(windowExtent);
        m_apectRatio = m_viewport.getSize().x / m_viewport.getSize().y;
        m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);

    }

    void Camera::setFOVRadians(float fovRadians) {
        m_fov = fovRadians;
        m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);
    }

    void Camera::setFOVDegrees(float fovDegrees) {
        m_fov = glm::radians(fovDegrees);
        m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);
    }

    void Camera::setViewport(Rect viewport) {
        m_viewport = viewport;
        m_apectRatio = m_viewport.getSize().x / m_viewport.getSize().y;
        m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);
    }

    void Camera::lookAt(Vector3 target) {
        m_forward = target - m_position;
        m_forward = glm::normalize(m_forward);
    }

    void Camera::setPosition(Vector3 position) {
        m_position = position;
    }

    Vector3 Camera::getPosition() const {
        return m_position;
    }

    void Camera::rotate(float angle, Vector3 axis) {
        m_forward = glm::rotate(m_forward, angle, axis);
    }

    Vector3 Camera::getForward() const {
        return m_forward;
    }

    Vector3 Camera::getUp() const {
        return -m_up;
    }

    Vector3 Camera::getRight() const {
        return glm::cross(m_forward, m_up);
    }


    Matrix4x4 Camera::getViewMatrix() const {
        return glm::lookAt(m_position, m_position + m_forward, m_up);
    }

    Matrix4x4 Camera::getProjectionMatrix() const {
        return m_projMat;
    }

    Rect Camera::getViewport() const {
        return m_viewport;
    }
}