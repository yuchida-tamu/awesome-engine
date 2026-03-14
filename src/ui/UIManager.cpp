#include "ui/UIManager.h"
#include <algorithm>

// ===================================================================
// CONSTRUCTION / DESTRUCTION
// ===================================================================

UIManager::UIManager(EventBus &eventBus) : InputListener(eventBus) {
  // TODO: Subscribe to any events the UI system needs.
  //   For example, subscribe to a WindowResizeEvent to update
  //   m_screenWidth / m_screenHeight for the orthographic projection.
}

UIManager::~UIManager() {
  // TODO: Unsubscribe from any EventBus subscriptions.
}

// ===================================================================
// ELEMENT MANAGEMENT
// ===================================================================

int UIManager::Register(std::unique_ptr<UIElement> element) {
  int id = m_nextId++;
  m_elements.push_back({id, std::move(element)});
  return id;
}

void UIManager::Deregister(int id) {
  // TODO: Remove the element with the matching ID.
  //   Use std::remove_if + erase idiom on m_elements.
  //   Safe to call with an ID that doesn't exist (just does nothing).
}

// ===================================================================
// UPDATE / RENDER
// ===================================================================

void UIManager::Update(float deltaTime) {
  for (auto &entry : m_elements) {
    entry.element->Update(deltaTime);
  }
}

void UIManager::Render() {
  // TODO: Render all visible UI elements.
  //
  // Steps:
  //   1. glDisable(GL_DEPTH_TEST);
  //   2. glEnable(GL_BLEND);
  //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //   3. glm::mat4 projection = glm::ortho(0.0f, m_screenWidth, 0.0f,
  //   m_screenHeight);
  //   4. Activate text shader, set "projection" uniform.
  //   5. for (auto &entry : m_elements) {
  //        if (entry.element->IsVisible()) {
  //          entry.element->Render(shader);
  //        }
  //      }
  //   6. glEnable(GL_DEPTH_TEST);
  //      glDisable(GL_BLEND);
}
