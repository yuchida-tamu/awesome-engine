#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera();
    ~Camera();
    void UpdateFront(float xOffset, float yOffset);
    void UpdatePosition(glm::vec3 pos);
    void SetTarget(glm::vec3 targetPos);
    glm::vec3 GetPosition();
    glm::vec3 GetFront();
    glm::vec3 GetUp();
    glm::vec3 GetRight();
    glm::mat4 GetCameraView();

private:
    const float m_maxRange = 89.0f;
    const float m_minRange = -89.0f;
    float m_pitch, m_yaw;
    glm::vec3 m_front, m_up, m_position, m_target;
};