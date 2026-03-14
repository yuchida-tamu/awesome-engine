#include "doctest.h"
#include "TestHelpers.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/RenderContext.h"

TEST_CASE("RenderContext - SetProjection/GetProjectionPtr round-trip") {
  RenderContext context;
  glm::mat4 proj =
      glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

  context.SetProjection(proj);

  const float *ptr = context.GetProjectionPtr();
  glm::mat4 retrieved = glm::make_mat4(ptr);

  CHECK(Mat4ApproxEquals(proj, retrieved));
}

TEST_CASE("RenderContext - SetView/GetViewPtr round-trip") {
  RenderContext context;
  glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
                                glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));

  context.SetView(view);

  const float *ptr = context.GetViewPtr();
  glm::mat4 retrieved = glm::make_mat4(ptr);

  CHECK(Mat4ApproxEquals(view, retrieved));
}
