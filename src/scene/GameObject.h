#pragma once

#include <iterator>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "Component.h"
#include "rendering/RenderContext.h"
#include "rendering/Shader.h"

class GameObject {
public:
  GameObject() {};
  template <typename T, typename... Args> T *AddComponent(Args &&...args) {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    auto component = std::make_unique<T>(std::forward<Args>(args)...);
    T *rawPtr = component.get();
    component->SetOwner(this);

    m_components.push_back(std::move(component));
    m_componentMap[std::type_index(typeid(T))] = rawPtr;

    return rawPtr;
  }

  template <typename T> T *GetComponent() const {
    static_assert(std::is_base_of<Component, T>::value,
                  "T must derive from Component");

    auto iterator = m_componentMap.find(std::type_index(typeid(T)));
    if (iterator != m_componentMap.end()) {
      return static_cast<T *>(iterator->second);
    }

    return nullptr;
  }

  template <typename T> bool HasComponent() const {
    return GetComponent<T>() != nullptr;
  }

  void Update(float deltaTime);
  void Draw(RenderContext &context);

  void SetIsActive(bool active) { m_active = active; }
  bool IsActive() { return m_active; }

private:
  std::vector<std::unique_ptr<Component>> m_components;
  std::unordered_map<std::type_index, Component *> m_componentMap;
  bool m_active = true;
};
