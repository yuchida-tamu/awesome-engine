#pragma once

#include "cameras/Camera.h"
#include "core/Config.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "scene/Component.h"

class CameraController : public Component {
public:
  CameraController(Camera &camera, EventBus &eventBus);
  ~CameraController();

  void Update(float deltaTime) override;

private:
  void OnKey(const KeyEvent &event);
  void OnMouseMove(const MouseMoveEvent &event);

  Camera &m_camera;
  EventBus &m_eventBus;
  SubscriptionID m_keySub;
  SubscriptionID m_mouseSub;

  glm::vec3 m_moveDirection{0.0f};
  float m_speed = Config::DEFAULT_CAMERA_SPEED;
};
