#include "doctest.h"
#include <memory>

#include "cameras/Camera.h"
#include "core/EventBus.h"
#include "scene/Component.h"
#include "scene/GameObject.h"
#include "scene/Scene.h"
#include "scene/TransformComponent.h"

// Mock component to track Update calls
class SceneMockComponent : public Component {
public:
  int updateCount = 0;
  void Update(float deltaTime) override { updateCount++; }
};

// ===================================================================
// CONSTRUCTOR TESTS
// ===================================================================

TEST_CASE("Scene - Constructor creates valid scene") {
  EventBus bus;
  Scene scene(bus);
  // Should not crash — scene is valid
}

// ===================================================================
// ADD TESTS
// ===================================================================

TEST_CASE("Scene - AddGameObject accepts unique_ptr") {
  EventBus bus;
  Scene scene(bus);

  auto go = std::make_unique<GameObject>();
  scene.AddGameObject(std::move(go));
  // Should not crash
}

TEST_CASE("Scene - AddCamera accepts camera pointer") {
  EventBus bus;
  Scene scene(bus);
  Camera camera;

  scene.AddCamera(&camera);
  // Should not crash
}

// ===================================================================
// UPDATE TESTS
// ===================================================================

TEST_CASE("Scene - Update with no cameras returns early") {
  EventBus bus;
  Scene scene(bus);

  auto go = std::make_unique<GameObject>();
  auto *mc = go->AddComponent<SceneMockComponent>();
  scene.AddGameObject(std::move(go));

  scene.Update(0.016f);

  // No cameras means early return — components not updated
  CHECK(mc->updateCount == 0);
}

TEST_CASE("Scene - Update with camera calls Update and Draw on game objects") {
  EventBus bus;
  Scene scene(bus);
  Camera camera;
  scene.AddCamera(&camera);

  auto go = std::make_unique<GameObject>();
  auto *mc = go->AddComponent<SceneMockComponent>();
  go->AddComponent<TransformComponent>();
  scene.AddGameObject(std::move(go));

  scene.Update(0.016f);

  CHECK(mc->updateCount == 1);
}

TEST_CASE("Scene - Update skips inactive objects but continues to next") {
  EventBus bus;
  Scene scene(bus);
  Camera camera;
  scene.AddCamera(&camera);

  // First object: inactive
  auto go1 = std::make_unique<GameObject>();
  auto *mc1 = go1->AddComponent<SceneMockComponent>();
  go1->SetIsActive(false);
  scene.AddGameObject(std::move(go1));

  // Second object: active — should still be processed
  auto go2 = std::make_unique<GameObject>();
  auto *mc2 = go2->AddComponent<SceneMockComponent>();
  scene.AddGameObject(std::move(go2));

  scene.Update(0.016f);

  // Inactive object's component is not updated (GameObject::Update checks active)
  CHECK(mc1->updateCount == 0);
  // Active object after inactive one must still be reached (validates bug fix)
  CHECK(mc2->updateCount == 1);
}
