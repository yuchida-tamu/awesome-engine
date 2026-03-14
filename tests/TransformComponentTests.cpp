#include "doctest.h"
#include "TestHelpers.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene/TransformComponent.h"

// ===================================================================
// CONSTRUCTOR TESTS
// ===================================================================

TEST_CASE("TransformComponent - Default constructor creates identity matrix") {
  TransformComponent tc;
  glm::mat4 identity(1.0f);
  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());

  CHECK(Mat4ApproxEquals(model, identity));
}

// ===================================================================
// TRANSLATE TESTS
// ===================================================================

TEST_CASE("TransformComponent - Translate moves position") {
  TransformComponent tc;
  tc.Translate(glm::vec3(1.0f, 2.0f, 3.0f));

  glm::vec3 pos = ExtractPosition(tc.GetModelPtr());
  CHECK(pos.x == doctest::Approx(1.0f));
  CHECK(pos.y == doctest::Approx(2.0f));
  CHECK(pos.z == doctest::Approx(3.0f));
}

TEST_CASE("TransformComponent - Multiple translates accumulate") {
  TransformComponent tc;
  tc.Translate(glm::vec3(1.0f, 0.0f, 0.0f));
  tc.Translate(glm::vec3(0.0f, 2.0f, 0.0f));
  tc.Translate(glm::vec3(0.0f, 0.0f, 3.0f));

  glm::vec3 pos = ExtractPosition(tc.GetModelPtr());
  CHECK(pos.x == doctest::Approx(1.0f));
  CHECK(pos.y == doctest::Approx(2.0f));
  CHECK(pos.z == doctest::Approx(3.0f));
}

// ===================================================================
// SCALE TESTS
// ===================================================================

TEST_CASE("TransformComponent - Scale modifies matrix diagonal") {
  TransformComponent tc;
  tc.Scale(glm::vec3(2.0f, 3.0f, 4.0f));

  glm::mat4 expected = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 4.0f));
  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());

  CHECK(Mat4ApproxEquals(model, expected));
}

TEST_CASE("TransformComponent - Uniform scale") {
  TransformComponent tc;
  tc.Scale(glm::vec3(5.0f));

  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());
  CHECK(model[0][0] == doctest::Approx(5.0f));
  CHECK(model[1][1] == doctest::Approx(5.0f));
  CHECK(model[2][2] == doctest::Approx(5.0f));
}

// ===================================================================
// ROTATE TESTS
// ===================================================================

TEST_CASE("TransformComponent - Rotate 90 degrees around Y axis") {
  TransformComponent tc;
  tc.Rotate(90.0f, glm::vec3(0.0f, 1.0f, 0.0f));

  glm::mat4 expected =
      glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());

  CHECK(Mat4ApproxEquals(model, expected));
}

TEST_CASE("TransformComponent - Rotate 360 degrees returns to identity") {
  TransformComponent tc;
  tc.Rotate(360.0f, glm::vec3(0.0f, 1.0f, 0.0f));

  glm::mat4 identity(1.0f);
  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());

  CHECK(Mat4ApproxEquals(model, identity));
}

// ===================================================================
// SET POSITION TESTS
// ===================================================================

TEST_CASE("TransformComponent - SetPosition overrides previous transforms") {
  TransformComponent tc;
  tc.Translate(glm::vec3(10.0f, 20.0f, 30.0f));
  tc.Scale(glm::vec3(5.0f));

  tc.SetPosition(glm::vec3(1.0f, 2.0f, 3.0f));

  glm::vec3 pos = ExtractPosition(tc.GetModelPtr());
  CHECK(pos.x == doctest::Approx(1.0f));
  CHECK(pos.y == doctest::Approx(2.0f));
  CHECK(pos.z == doctest::Approx(3.0f));

  // Verify scale was discarded (diagonal should be 1.0, not 5.0)
  const float *ptr = tc.GetModelPtr();
  CHECK(ptr[0] == doctest::Approx(1.0f));
  CHECK(ptr[5] == doctest::Approx(1.0f));
  CHECK(ptr[10] == doctest::Approx(1.0f));
}

// ===================================================================
// RESET TESTS
// ===================================================================

TEST_CASE("TransformComponent - Reset restores identity matrix") {
  TransformComponent tc;
  tc.Translate(glm::vec3(5.0f, 5.0f, 5.0f));
  tc.Scale(glm::vec3(3.0f));
  tc.Rotate(45.0f, glm::vec3(1.0f, 0.0f, 0.0f));

  tc.Reset();

  glm::mat4 identity(1.0f);
  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());

  CHECK(Mat4ApproxEquals(model, identity));
}

// ===================================================================
// COMBINED TRANSFORM TESTS
// ===================================================================

TEST_CASE("TransformComponent - Translate then scale (order matters)") {
  TransformComponent tc;
  tc.Translate(glm::vec3(1.0f, 0.0f, 0.0f));
  tc.Scale(glm::vec3(2.0f));

  glm::mat4 expected = glm::mat4(1.0f);
  expected = glm::translate(expected, glm::vec3(1.0f, 0.0f, 0.0f));
  expected = glm::scale(expected, glm::vec3(2.0f));

  glm::mat4 model = glm::make_mat4(tc.GetModelPtr());
  CHECK(Mat4ApproxEquals(model, expected));
}

TEST_CASE("TransformComponent - IsRenderable returns false") {
  TransformComponent tc;
  CHECK(tc.IsRenderable() == false);
}
