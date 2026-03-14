#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace {

constexpr float DEFAULT_EPSILON = 0.0001f;

bool FloatApproxEquals(float a, float b, float epsilon = DEFAULT_EPSILON) {
  return std::abs(a - b) < epsilon;
}

bool Vec3ApproxEquals(const glm::vec3 &a, const glm::vec3 &b,
                      float epsilon = DEFAULT_EPSILON) {
  return FloatApproxEquals(a.x, b.x, epsilon) &&
         FloatApproxEquals(a.y, b.y, epsilon) &&
         FloatApproxEquals(a.z, b.z, epsilon);
}

bool Mat4ApproxEquals(const glm::mat4 &a, const glm::mat4 &b,
                      float epsilon = DEFAULT_EPSILON) {
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      if (!FloatApproxEquals(a[i][j], b[i][j], epsilon))
        return false;
  return true;
}

glm::vec3 ExtractPosition(const float *modelPtr) {
  return glm::vec3(modelPtr[12], modelPtr[13], modelPtr[14]);
}

} // anonymous namespace
