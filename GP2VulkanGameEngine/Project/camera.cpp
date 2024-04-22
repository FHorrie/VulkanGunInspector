#include "camera.h"

#include <cassert>
#include <limits>

void FH::FHCamera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
    m_ProjectionMatrix = glm::mat4{ 1.0f };
    m_ProjectionMatrix[0][0] = 2.f / (right - left);
    m_ProjectionMatrix[1][1] = 2.f / (bottom - top);
    m_ProjectionMatrix[2][2] = 1.f / (far - near);
    m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
    m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    m_ProjectionMatrix[3][2] = -near / (far - near);
}

void FH::FHCamera::SetPerspectiveProjection(float FOV, float aspect, float near, float far) {
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);

    const float t = glm::tan(FOV / 2.f);

    m_ProjectionMatrix = glm::mat4{ 0.0f };
    m_ProjectionMatrix[0][0] = 1.f / (aspect * t);
    m_ProjectionMatrix[1][1] = 1.f / t;
    m_ProjectionMatrix[2][2] = far / (far - near);
    m_ProjectionMatrix[2][3] = 1.f;
    m_ProjectionMatrix[3][2] = -(far * near) / (far - near);
}

void FH::FHCamera::SetViewDirection(glm::vec3 pos, glm::vec3 direction, glm::vec3 up) {
    const glm::vec3 w{ glm::normalize(direction) };
    const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
    const glm::vec3 v{ glm::cross(w, u) };

    CalculateViewMatrix(u, v, w, pos);
}

void FH::FHCamera::SetViewYXZ(glm::vec3 pos, glm::vec3 rotation) {
    const float cosY = glm::cos(rotation.y);
    const float sinY = glm::sin(rotation.y);
    const float cosX = glm::cos(rotation.x);
    const float sinX = glm::sin(rotation.x);
    const float cosZ = glm::cos(rotation.z);
    const float sinZ = glm::sin(rotation.z);

    const glm::vec3 u{ (cosY * cosZ + sinY * sinX * sinZ), (cosX * sinZ), (cosY * sinX * sinZ - cosZ * sinY) };
    const glm::vec3 v{ (cosZ * sinY * sinX - cosY * sinZ), (cosX * cosZ), (cosY * cosZ * sinX + sinY * sinZ) };
    const glm::vec3 w{ (cosX * sinY), (-sinX), (cosY * cosX) };

    CalculateViewMatrix(u, v, w, pos);
}

//Helper function
void FH::FHCamera::CalculateViewMatrix(const glm::vec3& u, const glm::vec3& v, const glm::vec3& w, const glm::vec3& pos)
{
    m_ViewMatrix = glm::mat4{
        {u.x, v.x, w.x, 0.f},
        {u.y, v.y, w.y, 0.f},
        {u.z, v.z, w.z, 0.f},
        {-glm::dot(u, pos), -glm::dot(v, pos), -glm::dot(w, pos), 1.f}
    };
}