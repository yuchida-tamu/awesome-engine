#include "Camera.h"

Camera::Camera() : m_pitch(0.0f),
                   m_yaw(-90.0f),
                   m_position(glm::vec3(0.0f, 0.0f, 0.0f)),
                   m_front(glm::vec3(0.0f, 0.0f, -1.0f)),
                   m_up(glm::vec3(0.0f, 1.0f, 0.0f))
{
}

Camera::~Camera() {}

/**
 * It takes xOffset and yOffset, which represent mouse movement on screen,
 * and update the direction to which a camera is looking.
 */
void Camera::UpdateFront(float xOffset, float yOffset)
{
    m_yaw += xOffset;
    m_pitch += yOffset;

    if (m_pitch > m_maxRange)
        m_pitch = m_maxRange;
    if (m_pitch < m_minRange)
        m_pitch = m_minRange;

    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    m_front = glm::normalize(direction);
}

void Camera::UpdatePosition(glm::vec3 pos)
{
    m_position = pos;
}

glm::vec3 Camera::GetPosition()
{
    return m_position;
}

glm::vec3 Camera::GetFront()
{
    return m_front;
}

glm::vec3 Camera::GetUp()
{
    return m_up;
}

glm::vec3 Camera::GetRight()
{
    return glm::normalize(glm::cross(m_front, m_up));
}

glm::mat4 Camera::GetCameraView()
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}