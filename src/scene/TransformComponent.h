#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/matrix.hpp"
#include "scene/Component.h"

class TransformComponent : public Component {
public:
  TransformComponent() : m_model(glm::mat4(1.0f)) {}

  const float *GetModelPtr() const { return glm::value_ptr(m_model); }

  void Translate(const glm::vec3 &translation) {
    m_model = glm::translate(m_model, translation);
  }

  void Scale(const glm::vec3 &scale) { m_model = glm::scale(m_model, scale); }

  void Rotate(const float degrees, const glm::vec3 &axis) {
    m_model = glm::rotate(m_model, glm::radians(degrees), axis);
  }

  void SetPosition(const glm::vec3 &position) {
    m_model = glm::translate(glm::mat4(1.0f), position);
  }

  void Reset() { m_model = glm::mat4(1.0f); }

private:
  glm::mat4 m_model;
};
