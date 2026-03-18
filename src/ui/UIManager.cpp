#include "ui/UIManager.h"
#include "core/Config.h"
#include "core/InputEvents.h"
#include "ui/UIElement.h"
#include <algorithm>
#include <iostream>
#include <memory>

// ===================================================================
// CONSTRUCTION / DESTRUCTION
// ===================================================================

UIManager::UIManager(EventBus &eventBus)
    : InputListener(eventBus),
      m_projection(glm::ortho(0.0f, static_cast<float>(Config::WINDOW_WIDTH),
                              0.0f, static_cast<float>(Config::WINDOW_HEIGHT),
                              -1.0f, 1.0f)) {
  m_clickSub = m_eventBus.Subscribe<MouseClickEvent>(
      [this](const MouseClickEvent &e) { OnMouseClick(e); });
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
  m_elements.erase(std::remove_if(m_elements.begin(), m_elements.end(),
                                  [id](const ElementEntry &entry) {
                                    return entry.id == id;
                                  }),
                   m_elements.end());
}

// ===================================================================
// UPDATE / RENDER
// ===================================================================

void UIManager::Update(float deltaTime) {
  for (auto &entry : m_elements) {
    entry.element->Update(deltaTime);
  }
}

std::vector<UIElement *> UIManager::GetVisibleElements() {
  std::vector<UIElement *> visible;
  for (auto &entry : m_elements) {
    if (entry.element->IsVisible()) {
      visible.push_back(entry.element.get());
    }
  }
  return visible;
}

void UIManager::Render() {
  for (auto *element : GetVisibleElements()) {
    element->Render(m_projection);
  }
}

void UIManager::OnMouseClick(const MouseClickEvent &e) {
  // Iterate in reverse (top-most element first, like z-index)
  for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {

    if (it->element->IsVisible() &&
        it->element->ContainPoint({e.xPos, Config::WINDOW_HEIGHT - e.yPos})) {

      Clickable *clickable = dynamic_cast<Clickable *>(it->element.get());
      if (clickable) {

        clickable->OnClick(e);
      }
      // only the topmost element gets the m_clickSub
      break;
    }
  }
}
