#include "scene/Scene.h"
#include "cameras/Camera.h"
#include "core/EventBus.h"
#include "rendering/RenderContext.h"
#include "scene/GameObject.h"
#include <algorithm>
#include <memory>

Scene::Scene(EventBus &eventBus) : m_eventBus(eventBus) {}
void Scene::AddCamera(Camera *camera) { m_cameras.push_back(camera); }

void Scene::AddGameObject(std::unique_ptr<GameObject> gameObject) {
  m_gameObjects.push_back(std::move(gameObject));
}

void Scene::RemoveGameObject(GameObject *gameObjectPtr) {
  auto it =
      std::find_if(m_gameObjects.begin(), m_gameObjects.end(),
                   [gameObjectPtr](const std::unique_ptr<GameObject> &obj) {
                     return obj.get() == gameObjectPtr;
                   });
  if (it != m_gameObjects.end()) {
    m_gameObjects.erase(it);
  }
}

void Scene::Update(float deltaTime) {

  // NOTE: I might want to keep updating game objects even when a scene does not
  // have a camera attached in future. idk
  if (m_cameras.size() == 0)
    return;

  // NOTE: To make things simple, I will only take care of a single camera for
  // now.
  auto *camera = m_cameras[0];
  RenderContext context{};
  context.SetProjection(m_projection);
  context.SetView(camera->GetCameraView());

  for (auto &gameObject : m_gameObjects) {
    if (!gameObject->IsActive())
      continue;
    gameObject->Update(deltaTime);

    gameObject->Draw(context);
  }
}

RenderContext Scene::GetRenderContext() {
  RenderContext context{};
  if (m_cameras.size() > 0) {
    context.SetProjection(m_projection);
    context.SetView(m_cameras[0]->GetCameraView());
  }
  return context;
}

// TODO: Aspect Ration should be a responsibility of Camera. Refactor it to move
// the projection to Camera.
void Scene::SetViewPortSize(int width, int height) {
  m_projection =
      glm::perspective(glm::radians(Config::FOV),
                       static_cast<float>(width) / static_cast<float>(height),
                       Config::NEAR_PLANE, Config::FAR_PLANE);
}
