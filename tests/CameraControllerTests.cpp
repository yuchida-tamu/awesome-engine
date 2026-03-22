#include "doctest.h"
#include "TestHelpers.h"

#include "cameras/Camera.h"
#include "core/Config.h"
#include "core/EventBus.h"
#include "core/Input.h"
#include "core/InputEvents.h"
#include "scene/CameraController.h"
#include <GLFW/glfw3.h>
#include <cstring>

// ===================================================================
// CONSTRUCTION / DESTRUCTION TESTS
// ===================================================================

TEST_CASE("CameraController - Subscribes to events on construction") {
  Camera camera;
  EventBus bus;

  glm::vec3 posBefore = camera.GetPosition();

  {
    CameraController controller(camera, bus);

    // Publish a held key event — controller should accumulate movement
    bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
    controller.Update(1.0f);

    // Camera should have moved
    CHECK_FALSE(Vec3ApproxEquals(camera.GetPosition(), posBefore));
  }
}

TEST_CASE("CameraController - Unsubscribes on destruction") {
  Camera camera;
  EventBus bus;

  // Add a control subscriber that stays alive to verify the bus still works
  int controlCount = 0;
  bus.Subscribe<KeyEvent>(
      std::function<void(const KeyEvent &)>([&](const KeyEvent &) { controlCount++; }));

  {
    CameraController controller(camera, bus);
  } // controller destroyed here

  glm::vec3 posAfter = camera.GetPosition();

  // Publish events after destruction — camera should not move
  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});

  // Control subscriber still receives events (bus is working)
  CHECK(controlCount == 1);
  // But the controller's handler was removed — camera did not move
  CHECK(Vec3ApproxEquals(camera.GetPosition(), posAfter));
}

// ===================================================================
// KEY EVENT TESTS
// ===================================================================

TEST_CASE("CameraController - Only responds to Held key action") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 posStart = camera.GetPosition();

  // Down and Up should be ignored
  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Down});
  controller.Update(1.0f);
  CHECK(Vec3ApproxEquals(camera.GetPosition(), posStart));

  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Up});
  controller.Update(1.0f);
  CHECK(Vec3ApproxEquals(camera.GetPosition(), posStart));
}

TEST_CASE("CameraController - WASD movement directions") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  SUBCASE("W key moves camera forward") {
    glm::vec3 posBefore = camera.GetPosition();
    glm::vec3 front = camera.GetFront();
    glm::vec3 frontXZ = glm::vec3(front.x, 0.0f, front.z);

    bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
    controller.Update(1.0f);

    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(frontXZ)) ==
          doctest::Approx(1.0f).epsilon(0.01));
  }

  SUBCASE("S key moves camera backward") {
    glm::vec3 posBefore = camera.GetPosition();
    glm::vec3 front = camera.GetFront();
    glm::vec3 frontXZ = glm::vec3(front.x, 0.0f, front.z);

    bus.Publish(KeyEvent{GLFW_KEY_S, KeyAction::Held});
    controller.Update(1.0f);

    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(frontXZ)) ==
          doctest::Approx(-1.0f).epsilon(0.01));
  }

  SUBCASE("A key moves camera left") {
    glm::vec3 posBefore = camera.GetPosition();
    glm::vec3 right = camera.GetRight();

    bus.Publish(KeyEvent{GLFW_KEY_A, KeyAction::Held});
    controller.Update(1.0f);

    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(right)) ==
          doctest::Approx(-1.0f).epsilon(0.01));
  }

  SUBCASE("D key moves camera right") {
    glm::vec3 posBefore = camera.GetPosition();
    glm::vec3 right = camera.GetRight();

    bus.Publish(KeyEvent{GLFW_KEY_D, KeyAction::Held});
    controller.Update(1.0f);

    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(right)) ==
          doctest::Approx(1.0f).epsilon(0.01));
  }
}

TEST_CASE("CameraController - No movement without key events") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 posBefore = camera.GetPosition();
  controller.Update(1.0f);

  CHECK(Vec3ApproxEquals(camera.GetPosition(), posBefore));
}

TEST_CASE("CameraController - Movement scales with deltaTime") {
  Camera camera1;
  EventBus bus1;
  CameraController controller1(camera1, bus1);

  Camera camera2;
  EventBus bus2;
  CameraController controller2(camera2, bus2);

  glm::vec3 start1 = camera1.GetPosition();
  bus1.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  controller1.Update(0.5f);
  float dist1 = glm::length(camera1.GetPosition() - start1);

  glm::vec3 start2 = camera2.GetPosition();
  bus2.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  controller2.Update(1.0f);
  float dist2 = glm::length(camera2.GetPosition() - start2);

  CHECK(dist2 == doctest::Approx(dist1 * 2.0f).epsilon(0.01));
}

// ===================================================================
// MOUSE EVENT TESTS
// ===================================================================

TEST_CASE("CameraController - Mouse move updates camera front direction") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 frontBefore = camera.GetFront();

  // Must hold right mouse button and skip first drag frame
  bus.Publish(MouseClickEvent{0.0f, 0.0f, GLFW_MOUSE_BUTTON_LEFT,
                              KeyAction::Down});
  bus.Publish(MouseMoveEvent{0.0f, 0.0f}); // first frame skipped

  bus.Publish(MouseMoveEvent{10.0f, 0.0f});

  glm::vec3 frontAfter = camera.GetFront();
  CHECK_FALSE(Vec3ApproxEquals(frontBefore, frontAfter));
}

TEST_CASE("CameraController - Mouse move without left button held does not "
          "change camera") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 frontBefore = camera.GetFront();

  bus.Publish(MouseMoveEvent{10.0f, 5.0f});

  CHECK(Vec3ApproxEquals(camera.GetFront(), frontBefore));
}

TEST_CASE("CameraController - Mouse move with left button held updates "
          "camera") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 frontBefore = camera.GetFront();

  bus.Publish(MouseClickEvent{0.0f, 0.0f, GLFW_MOUSE_BUTTON_LEFT,
                              KeyAction::Down});
  bus.Publish(MouseMoveEvent{0.0f, 0.0f}); // skip first drag frame
  bus.Publish(MouseMoveEvent{10.0f, 0.0f}); // actual movement

  CHECK_FALSE(Vec3ApproxEquals(camera.GetFront(), frontBefore));
}

TEST_CASE("CameraController - First mouse move after drag start is skipped") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 frontBefore = camera.GetFront();

  bus.Publish(MouseClickEvent{0.0f, 0.0f, GLFW_MOUSE_BUTTON_LEFT,
                              KeyAction::Down});
  bus.Publish(MouseMoveEvent{100.0f, 100.0f}); // large offset, but skipped

  CHECK(Vec3ApproxEquals(camera.GetFront(), frontBefore));
}

TEST_CASE("CameraController - Right button does not enable camera look") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 frontBefore = camera.GetFront();

  bus.Publish(MouseClickEvent{0.0f, 0.0f, GLFW_MOUSE_BUTTON_RIGHT,
                              KeyAction::Down});
  bus.Publish(MouseMoveEvent{0.0f, 0.0f});
  bus.Publish(MouseMoveEvent{10.0f, 5.0f});

  CHECK(Vec3ApproxEquals(camera.GetFront(), frontBefore));
}

TEST_CASE("CameraController - Camera stops updating after left button "
          "released") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  // Start dragging
  bus.Publish(MouseClickEvent{0.0f, 0.0f, GLFW_MOUSE_BUTTON_LEFT,
                              KeyAction::Down});
  bus.Publish(MouseMoveEvent{0.0f, 0.0f}); // skip first
  bus.Publish(MouseMoveEvent{10.0f, 0.0f}); // actual move

  // Release
  bus.Publish(MouseClickEvent{0.0f, 0.0f, GLFW_MOUSE_BUTTON_LEFT,
                              KeyAction::Up});

  glm::vec3 frontAfterRelease = camera.GetFront();

  // Move mouse after release — should not change
  bus.Publish(MouseMoveEvent{20.0f, 20.0f});
  CHECK(Vec3ApproxEquals(camera.GetFront(), frontAfterRelease));
}

TEST_CASE("CameraController - WASD still works without mouse button held") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 posBefore = camera.GetPosition();

  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  controller.Update(1.0f);

  CHECK_FALSE(Vec3ApproxEquals(camera.GetPosition(), posBefore));
}

// ===================================================================
// INTEGRATION TESTS (Full Input → EventBus → CameraController pipeline)
// ===================================================================

// Helper to reset Input state for integration tests
static void ResetInputForIntegration(EventBus &bus) {
  memset(Input::s_Keys, 0, Config::MAX_KEYS * sizeof(bool));
  memset(Input::s_KeysLastFrame, 0, Config::MAX_KEYS * sizeof(bool));
  memset(Input::s_KeysRaw, 0, Config::MAX_KEYS * sizeof(bool));
  memset(Input::s_MouseButtons, 0, Config::MAX_MOUSE_BUTTONS * sizeof(bool));
  memset(Input::s_MouseButtonsLastFrame, 0,
         Config::MAX_MOUSE_BUTTONS * sizeof(bool));
  memset(Input::s_MouseButtonsRaw, 0,
         Config::MAX_MOUSE_BUTTONS * sizeof(bool));
  Input::s_FirstMouse = false;
  Input::s_LastX = 400.0;
  Input::s_LastY = 300.0;
  Input::s_CurrentX = 400.0;
  Input::s_CurrentY = 300.0;
  Input::s_XOffset = 0.0f;
  Input::s_YOffset = 0.0f;
  Input::s_EventBus = &bus;
}

TEST_CASE("Integration - Left-drag through Input pipeline updates camera") {
  EventBus bus;
  ResetInputForIntegration(bus);

  Camera camera;
  CameraController controller(camera, bus);

  glm::vec3 frontBefore = camera.GetFront();

  // Simulate: glfwPollEvents fires right-click + cursor move
  Input::MouseButtonCallBack(nullptr, GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
  Input::MouseCallBack(nullptr, 450.0, 300.0);

  // Frame 1: Input::Update detects Down, publishes MouseClickEvent + MouseMoveEvent
  Input::Update();

  // Camera should NOT have moved (firstDragFrame skipped the MouseMoveEvent)
  CHECK(Vec3ApproxEquals(camera.GetFront(), frontBefore));

  // Simulate: continued dragging
  Input::MouseCallBack(nullptr, 500.0, 300.0);

  // Frame 2: Input::Update publishes Held + MouseMoveEvent
  Input::Update();

  // Camera SHOULD have moved now
  CHECK_FALSE(Vec3ApproxEquals(camera.GetFront(), frontBefore));
}

// ===================================================================
// COMBINED INPUT TESTS
// ===================================================================

TEST_CASE("CameraController - Diagonal movement with W+D") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  glm::vec3 posBefore = camera.GetPosition();
  glm::vec3 front = camera.GetFront();
  glm::vec3 right = camera.GetRight();

  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  bus.Publish(KeyEvent{GLFW_KEY_D, KeyAction::Held});
  controller.Update(1.0f);

  glm::vec3 movement = camera.GetPosition() - posBefore;

  // Movement should have components in both front and right directions
  float frontComponent = glm::dot(movement, glm::normalize(front));
  float rightComponent = glm::dot(movement, glm::normalize(right));

  CHECK(frontComponent > 0.0f);
  CHECK(rightComponent > 0.0f);
}

TEST_CASE("CameraController - Movement resets after Update") {
  Camera camera;
  EventBus bus;
  CameraController controller(camera, bus);

  bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  controller.Update(1.0f);

  glm::vec3 posAfterFirstUpdate = camera.GetPosition();

  // No new key events — should not move
  controller.Update(1.0f);
  CHECK(Vec3ApproxEquals(camera.GetPosition(), posAfterFirstUpdate));
}
