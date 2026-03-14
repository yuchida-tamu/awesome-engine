#include "doctest.h"
#include "TestHelpers.h"

#include "cameras/Camera.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "scene/CameraController.h"
#include <GLFW/glfw3.h>

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

    bus.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
    controller.Update(1.0f);

    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(front)) ==
          doctest::Approx(1.0f).epsilon(0.01));
  }

  SUBCASE("S key moves camera backward") {
    glm::vec3 posBefore = camera.GetPosition();
    glm::vec3 front = camera.GetFront();

    bus.Publish(KeyEvent{GLFW_KEY_S, KeyAction::Held});
    controller.Update(1.0f);

    glm::vec3 movement = camera.GetPosition() - posBefore;
    CHECK(glm::dot(glm::normalize(movement), glm::normalize(front)) ==
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

  bus1.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  controller1.Update(0.5f);
  float dist1 = glm::length(camera1.GetPosition() - glm::vec3(0.0f));

  bus2.Publish(KeyEvent{GLFW_KEY_W, KeyAction::Held});
  controller2.Update(1.0f);
  float dist2 = glm::length(camera2.GetPosition() - glm::vec3(0.0f));

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

  bus.Publish(MouseMoveEvent{10.0f, 0.0f});

  glm::vec3 frontAfter = camera.GetFront();
  CHECK_FALSE(Vec3ApproxEquals(frontBefore, frontAfter));
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
