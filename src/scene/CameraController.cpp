#include "scene/CameraController.h"
#include "cameras/Camera.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "rendering/PostProcessing.h"
#include <GLFW/glfw3.h>

CameraController::CameraController(Camera &camera, EventBus &eventBus)
    : InputListener(eventBus), m_camera(camera) {
  m_keySub =
      m_eventBus.Subscribe<KeyEvent>([this](const KeyEvent &e) { OnKey(e); });
  m_mouseSub = m_eventBus.Subscribe<MouseMoveEvent>(
      [this](const MouseMoveEvent &e) { OnMouseMove(e); });
  m_mouseClickSub = m_eventBus.Subscribe<MouseClickEvent>(
      [this](const MouseClickEvent &e) { OnMouseClick(e); });
}

CameraController::~CameraController() {
  m_eventBus.Unsubscribe(m_keySub);
  m_eventBus.Unsubscribe(m_mouseSub);
  m_eventBus.Unsubscribe(m_mouseClickSub);
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

void CameraController::OnMouseClick(const MouseClickEvent &event) {
  if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.key == KeyAction::Down) {
    m_isDragging = true;
    m_firstDragFrame = true;
  }
  if (event.button == GLFW_MOUSE_BUTTON_LEFT && event.key == KeyAction::Up) {
    m_isDragging = false;
  }
}

void CameraController::OnMouseMove(const MouseMoveEvent &event) {
  if (!m_isDragging)
    return;

  if (m_firstDragFrame) {
    m_firstDragFrame = false;
    return;
  }

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
