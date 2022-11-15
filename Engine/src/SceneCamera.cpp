#include "pch.h"
#include "SceneCamera.h"

namespace sa {
    void SceneCamera::updateProjection() {
        m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);
        m_projMat[1][1] *= -1;
    }

    SceneCamera::SceneCamera()
        : m_projMat(1)
        , m_position(0, 0, 0)
        , m_forward(0, 0, 1)
        , m_up(0, 1, 0)
        , m_fov(glm::radians(60.0f))
        , m_apectRatio(1)
        , m_viewport({ {0, 0}, { 128, 128 } })
    {
        updateProjection();
    }

    SceneCamera::SceneCamera(const Window* pWindow)
        : SceneCamera(pWindow->getCurrentExtent())
    {
    }

    SceneCamera::SceneCamera(Extent windowExtent)
        : SceneCamera()
    {
        m_viewport.offset = { 0, 0 };
        m_viewport.extent = windowExtent;
        m_apectRatio = (float)m_viewport.extent.width / m_viewport.extent.height;
        updateProjection();

    }

    void SceneCamera::setFOVRadians(float fovRadians) {
        m_fov = fovRadians;
        updateProjection();
    }

    void SceneCamera::setFOVDegrees(float fovDegrees) {
        m_fov = glm::radians(fovDegrees);
        updateProjection();
    }

    void SceneCamera::setViewport(Rect viewport) {
        m_viewport = viewport;
        m_apectRatio = (float)m_viewport.extent.width / m_viewport.extent.height;
        updateProjection();
    }

    void SceneCamera::setAspectRatio(float aspectRatio) {
        m_apectRatio = aspectRatio;
        updateProjection();
    }

    void SceneCamera::lookAt(Vector3 target) {
        m_forward = target - m_position;
        m_forward = glm::normalize(m_forward);
    }

    void SceneCamera::setPosition(Vector3 position) {
        m_position = position;
    }

    Vector3 SceneCamera::getPosition() const {
        return m_position;
    }

    void SceneCamera::rotate(float angle, Vector3 axis) {
        m_forward = glm::rotate(m_forward, angle, axis);
    }

    Vector3 SceneCamera::getForward() const {
        return m_forward;
    }

    Vector3 SceneCamera::getUp() const {
        return -m_up;
    }

    Vector3 SceneCamera::getRight() const {
        return glm::normalize(glm::cross(m_forward, m_up));
    }

    Matrix4x4 SceneCamera::getViewMatrix() const {
        return glm::lookAt(m_position, m_position + m_forward, m_up);
    }

    Matrix4x4 SceneCamera::getProjectionMatrix() const {
        return m_projMat;
    }

    Rect SceneCamera::getViewport() const {
        return m_viewport;
    }

    float SceneCamera::getNear() const {
        return m_near;
    }
    
    float SceneCamera::getFar() const {
        return m_far;
    }
}