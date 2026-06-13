#pragma once

#include <glm/matrix.hpp>
#include <memory>
#include <vector>

#include "cameras/Camera.h"
#include "core/EventBus.h"
#include "rendering/RenderContext.h"
#include "scene/GameObject.h"

class Scene {
public:
  Scene(EventBus &eventBus);
  ~Scene() = default;
  // Diallow copying
  Scene(const Scene &other) = delete;
  Scene &operator=(const Scene &other) = delete;
  // Allow moving
  Scene(const Scene &&other) noexcept;
  Scene &operator=(const Scene &&other) noexcept;

  // NOTE: this appraoch has a performance downside (using heap memory)
  // For now the project itself if fairly small and I prioritize simplicity over
  // performance. However, when the project grows and the performance concerns
  // arise, consider migrating to ECS/Object pooling for opitimizeation.
  void AddGameObject(std::unique_ptr<GameObject> gameObject);
  // Remove GameObject to Scene
  void RemoveGameObject(GameObject *gameObjectPtr);
  // Add Camera to Scene
  void AddCamera(Camera *camera);
  // Add Lighting to Scene
  // AddLight(Light light)
  void Update(float deltaTime);
  RenderContext GetRenderContext();

private:
  EventBus &m_eventBus;
  std::vector<std::unique_ptr<GameObject>> m_gameObjects = {};
  // NOTE: It could store multiple cameras so that in future it can add features
  // like switching between cameras.
  std::vector<Camera *> m_cameras = {};

  glm::mat4 m_projection =
      glm::perspective(glm::radians(Config::FOV),
                       static_cast<float>(Config::WINDOW_WIDTH) /
                           static_cast<float>(Config::WINDOW_HEIGHT),
                       Config::NEAR_PLANE, Config::FAR_PLANE);
};
