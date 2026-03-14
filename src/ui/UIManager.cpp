#include "ui/UIManager.h"
#include "rendering/Shader.h"
#include "ui/UIElement.h"
#include <algorithm>

// ===================================================================
// CONSTRUCTION / DESTRUCTION
// ===================================================================

UIManager::UIManager(EventBus &eventBus)
    : InputListener(eventBus),
      m_projection(glm::ortho(0.0f, static_cast<float>(Config::WINDOW_WIDTH),
                              0.0f, static_cast<float>(Config::WINDOW_HEIGHT),
                              -1.0f, 1.0f)) {}

UIManager::UIManager(EventBus &eventBus, Shader shader)
    : InputListener(eventBus),
      m_projection(glm::ortho(0.0f, static_cast<float>(Config::WINDOW_WIDTH),
                              0.0f, static_cast<float>(Config::WINDOW_HEIGHT),
                              -1.0f, 1.0f)),
      m_shader(std::move(shader)) {}

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
  if (!m_shader.has_value()) {
    return;
  }

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_shader->UseProgram();
  m_shader->SetUniformMatrix4FloatPtr("projection",
                                      glm::value_ptr(m_projection));

  for (auto *element : GetVisibleElements()) {
    element->Render(*m_shader);
  }

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}
