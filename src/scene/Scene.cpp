#include "scene/Scene.h"
#include "cameras/Camera.h"
#include "rendering/RenderContext.h"
#include <memory>

void Scene::AddCamera(Camera *camera) { m_cameras.push_back(camera); }

void Scene::AddGameObject(std::unique_ptr<GameObject> gameObject) {
  m_gameObjects.push_back(std::move(gameObject));
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

  // NOTE: hmmm, I have to update Input somehow, so that it updates camera
  // positions and other controls to make a scene interactive and reactive.

  for (auto &gameObject : m_gameObjects) {
    if (!gameObject->IsActive())
      return;
    gameObject->Update(deltaTime);

    gameObject->Draw(context);
  }
}
