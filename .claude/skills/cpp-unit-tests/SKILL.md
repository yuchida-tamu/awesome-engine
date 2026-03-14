---
name: cpp-unit-tests
description: Write and maintain C++ unit tests using the doctest framework for this OpenGL engine project. Use this skill whenever the user asks to write tests, add test coverage, create a test file, test a class, or verify behavior for any C++ code in this project. Also trigger when the user mentions doctest, TEST_CASE, CHECK/REQUIRE, test helpers, or asks "how do I test X". Even if the user just says "add tests for Scene" or "test the new component", this skill applies.
---

# C++ Unit Test Skill

Guide for writing unit tests in this project. Tests use the **doctest** framework, live in `tests/`, and are built via CMake.

## Before Writing Tests

1. **Read the class under test** — understand public API, private state (accessible via `UNIT_TEST` macro), and dependencies.
2. **Read `tests/TestHelpers.h`** — use shared comparison helpers instead of writing local ones.
3. **Check `CMakeLists.txt`** — know which source files are already linked into `run-tests`.

## Test File Conventions

### File naming and placement
- One test file per class: `tests/<ClassName>Tests.cpp`
- New test files must be added to `CMakeLists.txt` under the `run-tests` target.
- If the class under test has a `.cpp` file not yet in `run-tests`, add it too.

### Include structure
Only `tests/InputTests.cpp` defines `DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN`. All other test files simply include `"doctest.h"`.

```cpp
#include "doctest.h"
#include "TestHelpers.h"  // shared helpers — always include this
// other includes as needed
```

**GLAD/GLFW order**: If the test needs OpenGL headers, include `<glad/glad.h>` before `<GLFW/glfw3.h>` with a blank line separator to prevent formatter reordering.

### Test naming
Use the pattern `"ClassName - Description of behavior"`:
```cpp
TEST_CASE("TransformComponent - Translate moves position") { ... }
```

### Test organization
Group tests with section comment headers:
```cpp
// ===================================================================
// CONSTRUCTOR TESTS
// ===================================================================
```

Common sections: Constructor, Method-specific, Edge Cases, Integration.

## Writing Assertions

### CHECK vs REQUIRE
- Use `CHECK` for most assertions — test continues on failure, giving more diagnostic info.
- Use `REQUIRE` before dereferencing a pointer — prevents crashes on null:
  ```cpp
  auto *tc = go.AddComponent<TransformComponent>();
  REQUIRE(tc != nullptr);  // crash-guard
  CHECK(tc->GetModelPtr()[0] == doctest::Approx(1.0f));
  ```

### Floating-point comparison
Two options, both acceptable:
```cpp
// Option 1: doctest::Approx (preferred for single values)
CHECK(value == doctest::Approx(1.0f));

// Option 2: shared helpers (preferred for vec3/mat4)
CHECK(Vec3ApproxEquals(actual, expected));
CHECK(Mat4ApproxEquals(actualMat, expectedMat));
CHECK(FloatApproxEquals(a, b));
```

### Shared helpers from TestHelpers.h
| Helper | Purpose |
|--------|---------|
| `FloatApproxEquals(a, b, epsilon)` | Compare floats within tolerance |
| `Vec3ApproxEquals(a, b, epsilon)` | Compare `glm::vec3` values |
| `Mat4ApproxEquals(a, b, epsilon)` | Compare `glm::mat4` matrices |
| `ExtractPosition(modelPtr)` | Extract position `vec3` from a model matrix `float*` |

Default epsilon is `0.0001f`. Do not redefine these in test files.

## Test Patterns

### SUBCASE for variations
When testing the same setup with different inputs, use `SUBCASE` to avoid duplicated boilerplate:
```cpp
TEST_CASE("CameraController - WASD movement directions") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  SUBCASE("W key moves camera forward") {
    // ...
  }
  SUBCASE("S key moves camera backward") {
    // ...
  }
}
```

### Testing private methods via UNIT_TEST macro
Functions should not be made public just for testing. If a private method needs direct test coverage, use the `UNIT_TEST` macro to conditionally expose it only in test builds:

**In the header** — add a guarded access block before the `private:` section that contains the method:
```cpp
class MyClass {
public:
  void PublicMethod();

#ifdef UNIT_TEST
public:
#else
private:
#endif
  std::vector<Item *> GetFilteredItems();

private:
  std::vector<Item> m_items;
};
```

The method stays private in production builds. Tests can call it directly because `UNIT_TEST` is defined for the test target.

### Mock components
Extend `Component` for test doubles. Keep them minimal:
```cpp
class MockComponent : public Component {
public:
  int updateCount = 0;
  void Update(float deltaTime) override { updateCount++; }
};
```

For renderable mocks, override `IsRenderable()` and `Draw()`:
```cpp
class MockRenderComponent : public Component {
public:
  int drawCount = 0;
  bool IsRenderable() const override { return true; }
  void Draw(RenderContext &ctx, const float *modelPtr) override { drawCount++; }
};
```

For lifetime tracking:
```cpp
class DestructorTracker : public Component {
public:
  bool *destroyedFlag;
  DestructorTracker(bool *flag) : destroyedFlag(flag) { *flag = false; }
  ~DestructorTracker() override { *flag = true; }
};
```

### Static state reset
Classes with static state (like `Input`) need explicit reset between tests:
```cpp
void ResetInputState() {
    memset(Input::s_Keys, 0, Config::MAX_KEYS * sizeof(bool));
    // ... reset all static members
    Input::s_EventBus = &s_TestEventBus;
}
```
Call this at the start of every test case that touches static state.

### Testing event-driven code
Use `EventBus` to test subscription/publish behavior:
```cpp
TEST_CASE("Controller - Unsubscribes on destruction") {
  Camera camera;
  EventBus bus;

  // Control subscriber to verify bus still works after destruction
  int controlCount = 0;
  bus.Subscribe<KeyEvent>(
      std::function<void(const KeyEvent &)>([&](const KeyEvent &) { controlCount++; }));

  {
    CameraController controller(camera, bus);
  } // destroyed

  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  CHECK(controlCount == 1);        // bus works
  CHECK(Vec3ApproxEquals(camera.GetPosition(), glm::vec3(0.0f)));  // handler removed
}
```

## What to Test

For each class, cover:

1. **Construction** — default values, parameterized constructors
2. **Core behavior** — each public method with typical inputs
3. **Edge cases** — zero, negative, boundary values, invalid input
4. **State transitions** — active/inactive, before/after operations
5. **Ownership & lifetime** — component ownership, cleanup on destruction
6. **Integration** — combined operations that exercise multiple methods

### Common pitfalls to avoid
- **False-positive tests**: Don't just check `determinant != 0` — compare against the expected `glm::lookAt` result.
- **Missing bounds checks**: If the production code indexes arrays, test with out-of-range values.
- **Untested `continue` vs `return`**: When a loop skips items, verify subsequent items are still processed.
- **Forgetting `SetPosition` resets scale**: Verify the full matrix, not just the position column.

## CMakeLists.txt Updates

When adding a new test file, update the `run-tests` target:

```cmake
add_executable(run-tests
    tests/InputTests.cpp
    tests/CameraTests.cpp
    # ... existing test files ...
    tests/NewClassTests.cpp        # ← add test file
    src/path/to/NewClass.cpp       # ← add source if not already present
    # ... existing source files ...
)
```

The test target includes `${CMAKE_SOURCE_DIR}/tests` in its include directories, so `#include "TestHelpers.h"` works directly.

## Build & Run

```bash
make test                # build and run all tests
make test-verbose        # with all assertions + timing
./bin/run-tests -s       # show all assertions
./bin/run-tests --list-test-cases  # list all tests
```

## Reference

For full test examples showing all these patterns in practice, read `references/examples.md`.
