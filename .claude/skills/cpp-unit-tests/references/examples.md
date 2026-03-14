# Test Examples Reference

Concrete examples from this project demonstrating each testing pattern.

## Table of Contents
1. [Complete test file template](#complete-test-file-template)
2. [Component test with mocks](#component-test-with-mocks)
3. [Static state test with reset](#static-state-test-with-reset)
4. [EventBus subscription tests](#eventbus-subscription-tests)
5. [Matrix and transform tests](#matrix-and-transform-tests)
6. [SUBCASE grouping](#subcase-grouping)
7. [Draw and rendering tests](#draw-and-rendering-tests)

---

## Complete test file template

Minimal template for a new test file:

```cpp
#include "doctest.h"
#include "TestHelpers.h"

#include "path/to/ClassUnderTest.h"

// ===================================================================
// CONSTRUCTOR TESTS
// ===================================================================

TEST_CASE("ClassName - Default constructor initializes correctly") {
  ClassName obj;
  CHECK(obj.GetValue() == expectedDefault);
}

// ===================================================================
// CORE BEHAVIOR TESTS
// ===================================================================

TEST_CASE("ClassName - MethodName does expected thing") {
  ClassName obj;
  obj.MethodName(input);
  CHECK(obj.GetResult() == expected);
}

// ===================================================================
// EDGE CASES
// ===================================================================

TEST_CASE("ClassName - Handles zero input") {
  ClassName obj;
  obj.MethodName(0);
  CHECK(obj.GetResult() == expectedForZero);
}
```

---

## Component test with mocks

From `GameObjectTests.cpp` — testing GameObject with mock components:

```cpp
class MockComponent : public Component {
public:
  int updateCount = 0;
  float lastDeltaTime = 0.0f;

  void Update(float deltaTime) override {
    updateCount++;
    lastDeltaTime = deltaTime;
  }
};

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
```

### Forwarding constructor arguments

```cpp
class ParameterizedComponent : public Component {
public:
  int intVal;
  std::string strVal;
  ParameterizedComponent(int i, std::string s) : intVal(i), strVal(std::move(s)) {}
};

TEST_CASE("GameObject - AddComponent forwards constructor arguments") {
  GameObject go;
  auto *pc = go.AddComponent<ParameterizedComponent>(42, std::string("hello"));
  REQUIRE(pc != nullptr);
  CHECK(pc->intVal == 42);
  CHECK(pc->strVal == "hello");
}
```

### Lifetime tracking

```cpp
class DestructorTracker : public Component {
public:
  bool *destroyedFlag;
  DestructorTracker(bool *flag) : destroyedFlag(flag) { *flag = false; }
  ~DestructorTracker() override { *flag = true; }
};

TEST_CASE("GameObject - Destructor cleans up components") {
  bool destroyed = false;
  {
    GameObject go;
    go.AddComponent<DestructorTracker>(&destroyed);
    CHECK(destroyed == false);
  }
  CHECK(destroyed == true);
}
```

---

## Static state test with reset

From `InputTests.cpp` — managing global state between tests:

```cpp
static EventBus s_TestEventBus;

void ResetInputState() {
    memset(Input::s_Keys, 0, Config::MAX_KEYS * sizeof(bool));
    memset(Input::s_KeysLastFrame, 0, Config::MAX_KEYS * sizeof(bool));
    memset(Input::s_KeysRaw, 0, Config::MAX_KEYS * sizeof(bool));
    Input::s_FirstMouse = true;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;
    Input::s_XOffset = 0.0f;
    Input::s_YOffset = 0.0f;
    Input::s_EventBus = &s_TestEventBus;
}

void SetupMouseState() {
    Input::s_FirstMouse = false;
    Input::s_LastX = 400.0;
    Input::s_LastY = 300.0;
    Input::s_CurrentX = 400.0;
    Input::s_CurrentY = 300.0;
}

TEST_CASE("Input - GLFW_REPEAT action is ignored") {
    ResetInputState();
    Input::KeyCallBack(nullptr, GLFW_KEY_W, 0, GLFW_REPEAT, 0);
    Input::Update();
    CHECK(Input::IsKeyHeld(GLFW_KEY_W) == false);
}
```

---

## EventBus subscription tests

From `EventBusTests.cpp`:

```cpp
struct TestEvent { int value; };

TEST_CASE("EventBus - Unsubscribe during publish does not crash") {
  EventBus bus;
  SubscriptionID id = 0;
  int count = 0;

  id = bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) {
        bus.Unsubscribe(id);  // re-entrant unsubscribe
        count++;
      }));

  bus.Publish(TestEvent{1});
  CHECK(count == 1);

  bus.Publish(TestEvent{2});
  CHECK(count == 1);  // handler was removed
}
```

---

## Matrix and transform tests

From `TransformComponentTests.cpp`:

```cpp
TEST_CASE("TransformComponent - SetPosition overrides previous transforms") {
  TransformComponent tc;
  tc.Translate(glm::vec3(10.0f, 20.0f, 30.0f));
  tc.Scale(glm::vec3(5.0f));

  tc.SetPosition(glm::vec3(1.0f, 2.0f, 3.0f));

  glm::vec3 pos = ExtractPosition(tc.GetModelPtr());
  CHECK(pos.x == doctest::Approx(1.0f));
  CHECK(pos.y == doctest::Approx(2.0f));
  CHECK(pos.z == doctest::Approx(3.0f));

  // Verify scale was discarded
  const float *ptr = tc.GetModelPtr();
  CHECK(ptr[0] == doctest::Approx(1.0f));   // not 5.0
  CHECK(ptr[5] == doctest::Approx(1.0f));
  CHECK(ptr[10] == doctest::Approx(1.0f));
}
```

### Comparing against glm reference

```cpp
TEST_CASE("GetCameraView - Uses correct look-at parameters") {
  Camera camera{};
  camera.UpdatePosition(glm::vec3(0.0f, 0.0f, 5.0f));

  glm::mat4 view = camera.GetCameraView();

  glm::vec3 pos = camera.GetPosition();
  glm::vec3 front = camera.GetFront();
  glm::vec3 up = camera.GetUp();
  glm::mat4 expected = glm::lookAt(pos, pos + front, up);

  CHECK(Mat4ApproxEquals(view, expected));
}
```

---

## SUBCASE grouping

From `CameraControllerTests.cpp` — 4 directional tests sharing one setup:

```cpp
TEST_CASE("CameraController - WASD movement directions") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  SUBCASE("W key moves camera forward") {
    glm::vec3 posBefore = camera.GetPosition();
    glm::vec3 front = camera.GetFront();
    bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
    controller.Update(1.0f);
    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(front)) ==
          doctest::Approx(1.0f).epsilon(0.01));
  }

  SUBCASE("S key moves camera backward") {
    // ... same pattern, check dot product == -1.0
  }

  SUBCASE("A key moves camera left") {
    // ... check against right vector, dot == -1.0
  }

  SUBCASE("D key moves camera right") {
    // ... check against right vector, dot == 1.0
  }
}
```

---

## Draw and rendering tests

From `GameObjectTests.cpp`:

```cpp
class MockRenderComponent : public Component {
public:
  int drawCount = 0;
  bool isRenderable = true;
  bool IsRenderable() const override { return isRenderable; }
  void Draw(RenderContext &ctx, const float *modelPtr) override { drawCount++; }
};

TEST_CASE("GameObject - Draw calls renderable components") {
  GameObject go;
  go.AddComponent<TransformComponent>();  // required for Draw to work
  auto *rc = go.AddComponent<MockRenderComponent>();
  REQUIRE(rc != nullptr);

  RenderContext context{};
  go.Draw(context);
  CHECK(rc->drawCount == 1);
}

TEST_CASE("GameObject - Draw does nothing without TransformComponent") {
  GameObject go;
  auto *rc = go.AddComponent<MockRenderComponent>();
  REQUIRE(rc != nullptr);

  RenderContext context{};
  go.Draw(context);
  CHECK(rc->drawCount == 0);  // no transform → no draw
}
```

---

## Scene-level tests

From `SceneTests.cpp` — testing the inactive-object bug fix:

```cpp
TEST_CASE("Scene - Update skips inactive objects but continues to next") {
  EventBus bus;
  Scene scene(bus);
  Camera camera;
  scene.AddCamera(&camera);

  auto go1 = std::make_unique<GameObject>();
  auto *mc1 = go1->AddComponent<SceneMockComponent>();
  go1->SetIsActive(false);
  scene.AddGameObject(std::move(go1));

  auto go2 = std::make_unique<GameObject>();
  auto *mc2 = go2->AddComponent<SceneMockComponent>();
  scene.AddGameObject(std::move(go2));

  scene.Update(0.016f);

  CHECK(mc1->updateCount == 0);  // inactive, skipped
  CHECK(mc2->updateCount == 1);  // active, still reached
}
```
