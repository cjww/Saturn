#include "pch.h"
#include "SceneCamera.h"

namespace sa {
    void SceneCamera::updateProjection() {
        switch (m_projectionMode)
        {
        case sa::ePerspective:
            m_projMat = glm::perspective(m_fov, m_apectRatio, m_near, m_far);
            break;
        case sa::eOrthographic:
            m_projMat = glm::orthoRH_ZO(m_orthoBounds.left, m_orthoBounds.right, m_orthoBounds.bottom, m_orthoBounds.top, m_near, m_far);
            break;
        default:
            return;
        }
        m_projMat[1][1] *= -1;

    }

    SceneCamera::SceneCamera()
        : m_projMat(1)
        , m_position(0, 0, 0)
        , m_forward(0, 0, 1)
        , m_up(0, 1, 0)
        , m_fov(glm::radians(60.0f))
        , m_apectRatio(1)
        , m_viewport({ {0, 0}, { 1.f, 1.f} })
        , m_projectionMode(ePerspective)
        , m_orthoBounds({-10, 10, -10, 10})
    {
        updateProjection();
    }

    float SceneCamera::getFOVRadians() const {
        return m_fov;
    }

    void SceneCamera::setFOVRadians(float fovRadians) {
        m_fov = fovRadians;
        updateProjection();
    }

    float SceneCamera::getFOVDegrees() const {
        return glm::degrees(m_fov);
    }

    void SceneCamera::setFOVDegrees(float fovDegrees) {
        m_fov = glm::radians(fovDegrees);
        updateProjection();
    }

    Rectf SceneCamera::getViewport() const {
        return m_viewport;
    }

    void SceneCamera::setViewport(Rectf viewport) {
        m_viewport = viewport;
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

    void SceneCamera::lookTo(Vector3 forward) {
        m_forward = glm::normalize(forward);
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

    void SceneCamera::setForward(Vector3 forward) {
        m_forward = glm::normalize(forward);
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

    float SceneCamera::getNear() const {
        return m_near;
    }
    
    float SceneCamera::getFar() const {
        return m_far;
    }

    void SceneCamera::setNear(float value) {
        m_near = value;
        updateProjection();
    }

    void SceneCamera::setFar(float value) {
        m_far = value;
        updateProjection();
    }

    Bounds SceneCamera::getOrthoBounds() const {
        return m_orthoBounds;
    }

    void SceneCamera::setOrthoBounds(Bounds bounds) {
        m_orthoBounds = bounds;
        updateProjection();
    }

    float SceneCamera::getOrthoWidth() const {
        return m_orthoBounds.right - m_orthoBounds.left;
    }

    void SceneCamera::setOrthoWidth(float width) {
        float halfWidth = width * 0.5f;
        m_orthoBounds.left = -halfWidth;
        m_orthoBounds.right = halfWidth;
        float height = width / m_apectRatio;
        float halfHeight = height * 0.5f;

        m_orthoBounds.bottom = -halfHeight;
        m_orthoBounds.top = halfHeight;
        updateProjection();
    }

    ProjectionMode SceneCamera::getProjectionMode() const {
        return m_projectionMode;
    }

    void SceneCamera::setProjectionMode(ProjectionMode projectionMode) {
        m_projectionMode = projectionMode;
        updateProjection();
    }

    void SceneCamera::calculateFrustumBoundsWorldSpace(glm::vec3* pOutPoints) const {
        glm::mat4 inv = glm::inverse(m_projMat * getViewMatrix());
        int i = 0;
        for (uint32_t x = 0; x < 2; x++) {
            for (uint32_t y = 0; y < 2; y++) {
                for (uint32_t z = 0; z < 2; z++) {
                    glm::vec4 point = inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        z,
                        1.0f);
                    point /= point.w;
                    pOutPoints[i] = point;

                    i++;
                }
            }
        }
    }

    bool SceneCamera::intersectsFrustum() const {

        return false;
    }
}