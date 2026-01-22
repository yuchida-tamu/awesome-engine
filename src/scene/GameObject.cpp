#include "scene/GameObject.h"
#include "rendering/RenderContext.h"
#include "scene/TransformComponent.h"

void GameObject::Update(float deltaTime) {
  if (!m_active)
    return;

  for (auto &component : m_components) {
    component->Update(deltaTime);
  }
}

void GameObject::Draw(RenderContext &context) {
  if (!m_active)
    return;

  TransformComponent *transformPtr = GetComponent<TransformComponent>();

  if (transformPtr == nullptr)
    return;

  for (auto &component : m_components) {
    if (component->IsRenderable()) {

      component->Draw(context, transformPtr->GetModelPtr());
    }
  }
}
