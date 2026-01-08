#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

class Transform {
public:
  Transform() : m_model(glm::mat4(1.0f)) {};
  ~Transform() = default;
  const float *GetModelPtr() { return glm::value_ptr(m_model); };
  void Traslate(glm::vec3 translate) {
    m_model = glm::translate(m_model, translate);
  };

  void Scale(glm::vec3 scale) { m_model = glm::scale(m_model, scale); }

  void Roate(float degrees, glm::vec3 direction) {
    m_model = glm::rotate(m_model, glm::radians(degrees), direction);
  }

private:
  glm::mat4 m_model;
};
