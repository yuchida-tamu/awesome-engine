#include "doctest.h"
#include <glm/glm.hpp>
#include <string>

#include "scene/Component.h"
#include "scene/GameObject.h"
#include "scene/TransformComponent.h"
#include "rendering/RenderContext.h"

// A simple test component for verifying behavior
class MockComponent : public Component {
public:
  int updateCount = 0;
  float lastDeltaTime = 0.0f;

  void Update(float deltaTime) override {
    updateCount++;
    lastDeltaTime = deltaTime;
  }
};

// Another component type for multi-component tests
class AnotherMockComponent : public Component {
public:
  bool updated = false;

  void Update(float deltaTime) override { updated = true; }
};

// Component with constructor arguments for forwarding test
class ParameterizedComponent : public Component {
public:
  int intVal;
  std::string strVal;

  ParameterizedComponent(int i, std::string s) : intVal(i), strVal(std::move(s)) {}
};

// Renderable mock component for Draw tests
class MockRenderComponent : public Component {
public:
  int drawCount = 0;
  bool isRenderable = true;

  bool IsRenderable() const override { return isRenderable; }
  void Draw(RenderContext &renderContext, const float *modelPtr) override {
    drawCount++;
  }
};

// Component that tracks destruction
class DestructorTracker : public Component {
public:
  bool *destroyedFlag;
  DestructorTracker(bool *flag) : destroyedFlag(flag) { *destroyedFlag = false; }
  ~DestructorTracker() override { *destroyedFlag = true; }
};

// ===================================================================
// ADD / GET / HAS COMPONENT TESTS
// ===================================================================

TEST_CASE("GameObject - AddComponent returns valid pointer") {
  GameObject go;
  auto *tc = go.AddComponent<TransformComponent>();

  REQUIRE(tc != nullptr);
}

TEST_CASE("GameObject - GetComponent returns added component") {
  GameObject go;
  auto *added = go.AddComponent<TransformComponent>();
  auto *retrieved = go.GetComponent<TransformComponent>();

  CHECK(retrieved == added);
}

TEST_CASE("GameObject - GetComponent returns nullptr for missing component") {
  GameObject go;

  CHECK(go.GetComponent<TransformComponent>() == nullptr);
}

TEST_CASE("GameObject - HasComponent returns true for added component") {
  GameObject go;
  go.AddComponent<TransformComponent>();

  CHECK(go.HasComponent<TransformComponent>() == true);
}

TEST_CASE("GameObject - HasComponent returns false for missing component") {
  GameObject go;

  CHECK(go.HasComponent<TransformComponent>() == false);
}

TEST_CASE("GameObject - Multiple different component types") {
  GameObject go;
  auto *tc = go.AddComponent<TransformComponent>();
  auto *mc = go.AddComponent<MockComponent>();

  REQUIRE(tc != nullptr);
  REQUIRE(mc != nullptr);
  CHECK(go.GetComponent<TransformComponent>() == tc);
  CHECK(go.GetComponent<MockComponent>() == mc);
  CHECK(go.HasComponent<AnotherMockComponent>() == false);
}

// ===================================================================
// COMPONENT OWNERSHIP TESTS
// ===================================================================

TEST_CASE("GameObject - Component owner is set on add") {
  GameObject go;
  auto *mc = go.AddComponent<MockComponent>();

  REQUIRE(mc != nullptr);
  CHECK(mc->GetOwner() == &go);
}

// ===================================================================
// UPDATE TESTS
// ===================================================================

TEST_CASE("GameObject - Update calls Update on all components") {
  GameObject go;
  auto *mc1 = go.AddComponent<MockComponent>();
  auto *mc2 = go.AddComponent<AnotherMockComponent>();

  REQUIRE(mc1 != nullptr);
  REQUIRE(mc2 != nullptr);

  go.Update(0.016f);

  CHECK(mc1->updateCount == 1);
  CHECK(mc1->lastDeltaTime == doctest::Approx(0.016f));
  CHECK(mc2->updated == true);
}

TEST_CASE("GameObject - Update does nothing when inactive") {
  GameObject go;
  auto *mc = go.AddComponent<MockComponent>();
  REQUIRE(mc != nullptr);
  go.SetIsActive(false);

  go.Update(0.016f);

  CHECK(mc->updateCount == 0);
}

// ===================================================================
// ACTIVE STATE TESTS
// ===================================================================

TEST_CASE("GameObject - Default is active") {
  GameObject go;
  CHECK(go.IsActive() == true);
}

TEST_CASE("GameObject - SetIsActive toggles state") {
  GameObject go;

  go.SetIsActive(false);
  CHECK(go.IsActive() == false);

  go.SetIsActive(true);
  CHECK(go.IsActive() == true);
}

// ===================================================================
// ADDCOMPONENT WITH CONSTRUCTOR ARGS
// ===================================================================

TEST_CASE("GameObject - AddComponent forwards constructor arguments") {
  GameObject go;
  auto *pc = go.AddComponent<ParameterizedComponent>(42, std::string("hello"));

  REQUIRE(pc != nullptr);
  CHECK(pc->intVal == 42);
  CHECK(pc->strVal == "hello");
}

// ===================================================================
// DRAW TESTS
// ===================================================================

TEST_CASE("GameObject - Draw calls renderable components") {
  GameObject go;
  go.AddComponent<TransformComponent>();
  auto *rc = go.AddComponent<MockRenderComponent>();
  REQUIRE(rc != nullptr);

  RenderContext context{};
  go.Draw(context);

  CHECK(rc->drawCount == 1);
}

TEST_CASE("GameObject - Draw skips non-renderable components") {
  GameObject go;
  go.AddComponent<TransformComponent>();
  auto *rc = go.AddComponent<MockRenderComponent>();
  REQUIRE(rc != nullptr);
  rc->isRenderable = false;

  RenderContext context{};
  go.Draw(context);

  CHECK(rc->drawCount == 0);
}

TEST_CASE("GameObject - Draw does nothing without TransformComponent") {
  GameObject go;
  auto *rc = go.AddComponent<MockRenderComponent>();
  REQUIRE(rc != nullptr);

  RenderContext context{};
  go.Draw(context);

  CHECK(rc->drawCount == 0);
}

// ===================================================================
// COMPONENT LIFETIME TESTS
// ===================================================================

TEST_CASE("GameObject - Destructor cleans up components") {
  bool destroyed = false;
  {
    GameObject go;
    go.AddComponent<DestructorTracker>(&destroyed);
    CHECK(destroyed == false);
  } // go goes out of scope
  CHECK(destroyed == true);
}

// ===================================================================
// DUPLICATE COMPONENT BEHAVIOR
// ===================================================================

TEST_CASE("GameObject - Adding duplicate component type overwrites map entry") {
  GameObject go;
  auto *first = go.AddComponent<MockComponent>();
  auto *second = go.AddComponent<MockComponent>();

  REQUIRE(first != nullptr);
  REQUIRE(second != nullptr);
  // Map points to the second one
  CHECK(go.GetComponent<MockComponent>() == second);
  // But both exist in the vector and both get updated
  go.Update(0.016f);
  CHECK(first->updateCount == 1);
  CHECK(second->updateCount == 1);
}
