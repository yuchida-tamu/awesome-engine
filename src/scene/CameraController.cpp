#include "scene/CameraController.h"
#include "cameras/Camera.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "rendering/PostProcessing.h"
#include <GLFW/glfw3.h>

CameraController::CameraController(Camera &camera, EventBus &eventBus)
    : m_camera(camera), m_eventBus(eventBus) {
  m_keySub =
      m_eventBus.Subscribe<KeyEvent>([this](const KeyEvent &e) { OnKey(e); });
  m_mouseSub = m_eventBus.Subscribe<MouseMoveEvent>(
      [this](const MouseMoveEvent &e) { OnMouseMove(e); });
}

CameraController::~CameraController() {
  m_eventBus.Unsubscribe(m_keySub);
  m_eventBus.Unsubscribe(m_mouseSub);
}

void CameraController::OnKey(const KeyEvent &event) {
  if (event.action != KeyAction::Held)
    return;

  if (event.key == GLFW_KEY_W)
    m_moveDirection += m_camera.GetFront();
  if (event.key == GLFW_KEY_S)
    m_moveDirection -= m_camera.GetFront();
  if (event.key == GLFW_KEY_A)
    m_moveDirection -= m_camera.GetRight();
  if (event.key == GLFW_KEY_D)
    m_moveDirection += m_camera.GetRight();
}

void CameraController::OnMouseMove(const MouseMoveEvent &event) {
  m_camera.UpdateFront(event.xOffset, event.yOffset);
}

void CameraController::Update(float deltaTime) {
  if (glm::length(m_moveDirection) > 0.0f) {
    glm::vec3 newPos = m_camera.GetPosition() +
                       glm::normalize(m_moveDirection) * m_speed * deltaTime;
    m_camera.UpdatePosition(newPos);
    m_moveDirection = glm::vec3(0.0);
  }
}
