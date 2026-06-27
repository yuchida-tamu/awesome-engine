#include "doctest.h"
#include "TestHelpers.h"

#include "core/Layer.h"
#include "core/LayerStack.h"

#include <memory>
#include <vector>

// LayerStack owns an ordered set of layers (as unique_ptr). PushLayer adds to
// the "layer" region; PushOverlay adds after all layers (overlays always
// render/update last, and -- later -- receive events first when iterated in
// reverse). The stack attaches on push, and on destruction it detaches each
// layer (the unique_ptr frees the memory automatically).

namespace {

class MockLayer : public Layer {
public:
  int id;
  int *attachCount;
  int *detachCount;
  bool *destroyed;

  MockLayer(int id_, int *attach, int *detach, bool *destroyedFlag = nullptr)
      : id(id_), attachCount(attach), detachCount(detach),
        destroyed(destroyedFlag) {}
  ~MockLayer() override {
    if (destroyed)
      *destroyed = true;
  }
  void OnAttach() override {
    if (attachCount)
      ++*attachCount;
  }
  void OnDetach() override {
    if (detachCount)
      ++*detachCount;
  }
};

} // namespace

TEST_CASE("LayerStack - attaches on push and keeps overlays after layers") {
  int attach = 0, detach = 0;
  LayerStack stack;

  // A layer pushed AFTER an overlay must still sort before it.
  stack.PushLayer(std::make_unique<MockLayer>(1, &attach, &detach));
  stack.PushOverlay(std::make_unique<MockLayer>(9, &attach, &detach));
  stack.PushLayer(std::make_unique<MockLayer>(2, &attach, &detach));

  CHECK(attach == 3); // every push attaches exactly once
  CHECK(detach == 0);

  std::vector<int> order;
  for (auto &layer : stack) // auto& -- a unique_ptr can't be copied
    order.push_back(static_cast<MockLayer *>(layer.get())->id);

  CHECK(order == std::vector<int>{1, 2, 9}); // layers in push order, overlay last
}

TEST_CASE("LayerStack - destruction detaches and frees its layers") {
  int attach = 0, detach = 0;
  bool d1 = false, d2 = false;
  {
    LayerStack stack;
    stack.PushLayer(std::make_unique<MockLayer>(1, &attach, &detach, &d1));
    stack.PushOverlay(std::make_unique<MockLayer>(2, &attach, &detach, &d2));
  } // stack goes out of scope here

  CHECK(detach == 2); // each layer detached on teardown
  CHECK(d1);          // ...and freed (unique_ptr auto-deletes)
  CHECK(d2);
}
