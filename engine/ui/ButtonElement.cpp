#include "ui/ButtonElement.h"
#include "core/InputEvents.h"
#include "ui/TextElement.h"
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

// Unit quad: two triangles covering [0,0] to [1,1]
static const float QUAD_VERTICES[] = {
    0.0f, 0.0f, // bottom-left
    1.0f, 0.0f, // bottom-right
    1.0f, 1.0f, // top-right
    0.0f, 0.0f, // bottom-left
    1.0f, 1.0f, // top-right
    0.0f, 1.0f, // top-left
};

// ===================================================================
// CONSTRUCTION / DESTRUCTION
// ===================================================================

ButtonElement::ButtonElement() {
#ifndef UNIT_TEST
  m_shader.emplace("shaders/ui_button.vert.glsl",
                    "shaders/ui_button.frag.glsl");

  glGenVertexArrays(1, &m_vao);
  glGenBuffers(1, &m_vbo);
  glBindVertexArray(m_vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        (void *)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
#endif
}

ButtonElement::~ButtonElement() {
  if (m_vao) {
    glDeleteVertexArrays(1, &m_vao);
  }
  if (m_vbo) {
    glDeleteBuffers(1, &m_vbo);
  }
}

// ===================================================================
// CLICK
// ===================================================================

void ButtonElement::SetOnClick(
    std::function<void(const MouseClickEvent &e)> callback) {
  m_onClick = callback;
}

void ButtonElement::OnClick(const MouseClickEvent &event) {
  if (m_onClick) {
    m_onClick(event);
  }
}

// ===================================================================
// APPEARANCE
// ===================================================================

void ButtonElement::SetColor(glm::vec3 color) { m_color = color; }

glm::vec3 ButtonElement::GetColor() const { return m_color; }

void ButtonElement::SetLabel(const std::string &label) { m_label = label; }

std::string ButtonElement::GetLabel() const { return m_label; }

void ButtonElement::SetLabelColor(glm::vec3 color) { m_labelColor = color; }

glm::vec3 ButtonElement::GetLabelColor() const { return m_labelColor; }

// ===================================================================
// UIElement OVERRIDES
// ===================================================================

void ButtonElement::Render(const glm::mat4 &projection) {
  if (!m_shader)
    return;

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  m_shader->UseProgram();
  m_shader->SetUniformMatrix4FloatPtr("projection",
                                      glm::value_ptr(projection));
  m_shader->SetUniformVec2("uPosition", glm::value_ptr(m_position));
  m_shader->SetUniformVec2("uSize", glm::value_ptr(m_size));
  m_shader->SetUniformVec3("uColor", glm::value_ptr(m_color));

  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  // Render label text if set
  if (!m_label.empty()) {
    if (!m_textElement) {
      m_textElement = std::make_unique<TextElement>();
    }

    float scale = m_size.y / 48.0f * 0.6f;
    m_textElement->SetText(m_label);
    m_textElement->SetColor(m_labelColor);
    m_textElement->SetScale(scale);

    float textWidth = m_textElement->MeasureWidth();
    float textX = m_position.x + (m_size.x - textWidth) / 2.0f;
    float textY = m_position.y + m_size.y * 0.3f;
    m_textElement->SetPosition({textX, textY});
    m_textElement->Render(projection);
  }

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
}

void ButtonElement::Update(float deltaTime) {}
