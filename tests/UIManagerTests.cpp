#include "core/EventBus.h"
#include "doctest.h"
#include "ui/UIElement.h"
#include "ui/UIManager.h"
#include <memory>

// A mock UIElement that tracks calls to Update and Render.
class MockUIElement : public UIElement {
public:
  int updateCount = 0;
  int renderCount = 0;
  float lastDeltaTime = 0.0f;

  void Render(Shader &shader) override { renderCount++; }
  void Update(float deltaTime) override {
    updateCount++;
    lastDeltaTime = deltaTime;
  }
};

// ===================================================================
// REGISTER / DEREGISTER TESTS
// ===================================================================

// TODO: Uncomment these tests once UIManager::Register and Deregister are
// implemented.

TEST_CASE("UIManager - Register adds element to manager") {
  // After registering an element, calling Update should forward to it.
  EventBus bus;
  UIManager manager(bus);
  auto element = std::make_unique<MockUIElement>();
  auto *rawPtr = element.get();

  manager.Register(std::move(element));
  manager.Update(0.016f);

  CHECK(rawPtr->updateCount == 1);
  CHECK(manager.Count() == 1);
}

TEST_CASE("UIManager - Deregister removes element") {
  // After deregistering, Update should no longer reach the element.
  EventBus bus;
  UIManager manager(bus);
  auto element = std::make_unique<MockUIElement>();
  auto *rawPtr = element.get();

  int id = manager.Register(std::move(element));
  manager.Deregister(id);
  manager.Update(0.016f);

  CHECK(manager.Count() == 0);
}

// TEST_CASE("UIManager - Deregistering unknown ID is safe") {
//   // Should not crash or throw when given an ID that was never registered.
//   EventBus bus;
//   UIManager manager(bus);
//   manager.Deregister(999); // no-op, should not crash
// }

// ===================================================================
// UPDATE TESTS
// ===================================================================

// TEST_CASE("UIManager - Update calls Update on all registered elements") {
//   EventBus bus;
//   UIManager manager(bus);
//   auto elem1 = std::make_unique<MockUIElement>();
//   auto elem2 = std::make_unique<MockUIElement>();
//   auto *raw1 = elem1.get();
//   auto *raw2 = elem2.get();
//
//   manager.Register(std::move(elem1));
//   manager.Register(std::move(elem2));
//   manager.Update(0.033f);
//
//   CHECK(raw1->updateCount == 1);
//   CHECK(raw2->updateCount == 1);
//   CHECK(raw1->lastDeltaTime == doctest::Approx(0.033f));
// }

// ===================================================================
// RENDER TESTS
// ===================================================================

// TEST_CASE("UIManager - Render only renders visible elements") {
//   // Invisible elements should be skipped during Render.
//   EventBus bus;
//   UIManager manager(bus);
//   auto visible = std::make_unique<MockUIElement>();
//   auto hidden = std::make_unique<MockUIElement>();
//   auto *rawVisible = visible.get();
//   auto *rawHidden = hidden.get();
//
//   hidden->SetVisible(false);
//
//   manager.Register(std::move(visible));
//   manager.Register(std::move(hidden));
//   manager.Render();
//
//   CHECK(rawVisible->renderCount == 1);
//   CHECK(rawHidden->renderCount == 0);
// }
