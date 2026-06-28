#pragma once

#include "cameras/Camera.h"
#include "core/Config.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "core/InputListener.h"
#include "scene/Component.h"

class CameraController : public Component, public InputListener {
public:
  CameraController(Camera &camera, EventBus &eventBus);
  ~CameraController() override;

  void Update(float deltaTime) override;

private:
  void OnKey(const KeyEvent &event);
  void OnMouseMove(const MouseMoveEvent &event);
  void OnMouseClick(const MouseClickEvent &event);
  void OnScroll(const ScrollEvent &event);

  Camera &m_camera;
  SubscriptionID m_keySub;
  SubscriptionID m_mouseSub;
  SubscriptionID m_mouseClickSub;
  SubscriptionID m_scrollSub;

  bool m_isDragging = false;
  bool m_firstDragFrame = true;

  glm::vec3 m_moveDirection{0.0f};
  float m_speed = Config::DEFAULT_CAMERA_SPEED;

  float m_scrollDistance = 0.0f;
  float m_zoomOutDistanceBound = 25.0f;
  float m_zoomInDistanceBound = 0.0f;
};
