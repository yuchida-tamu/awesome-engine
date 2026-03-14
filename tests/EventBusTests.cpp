#include "doctest.h"
#include "core/EventBus.h"

// Simple test event types
struct TestEvent {
  int value;
};

struct AnotherEvent {
  float x;
  float y;
};

// ===================================================================
// SUBSCRIBE AND PUBLISH TESTS
// ===================================================================

TEST_CASE("EventBus - Subscribe and publish delivers event to handler") {
  EventBus bus;
  int received = -1;

  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &e) { received = e.value; }));

  bus.Publish(TestEvent{42});
  CHECK(received == 42);
}

TEST_CASE("EventBus - Multiple subscribers receive the same event") {
  EventBus bus;
  int countA = 0;
  int countB = 0;

  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { countA++; }));
  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { countB++; }));

  bus.Publish(TestEvent{1});
  CHECK(countA == 1);
  CHECK(countB == 1);

  bus.Publish(TestEvent{2});
  CHECK(countA == 2);
  CHECK(countB == 2);
}

TEST_CASE("EventBus - Different event types are independent") {
  EventBus bus;
  int testCount = 0;
  int anotherCount = 0;

  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { testCount++; }));
  bus.Subscribe<AnotherEvent>(
      std::function<void(const AnotherEvent &)>([&](const AnotherEvent &) { anotherCount++; }));

  bus.Publish(TestEvent{1});
  CHECK(testCount == 1);
  CHECK(anotherCount == 0);

  bus.Publish(AnotherEvent{1.0f, 2.0f});
  CHECK(testCount == 1);
  CHECK(anotherCount == 1);
}

TEST_CASE("EventBus - Event data is passed correctly") {
  EventBus bus;
  float rx = 0.0f, ry = 0.0f;

  bus.Subscribe<AnotherEvent>(
      std::function<void(const AnotherEvent &)>([&](const AnotherEvent &e) {
        rx = e.x;
        ry = e.y;
      }));

  bus.Publish(AnotherEvent{3.14f, 2.72f});
  CHECK(rx == doctest::Approx(3.14f));
  CHECK(ry == doctest::Approx(2.72f));
}

// ===================================================================
// UNSUBSCRIBE TESTS
// ===================================================================

TEST_CASE("EventBus - Unsubscribe stops delivery") {
  EventBus bus;
  int count = 0;

  SubscriptionID id = bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { count++; }));

  bus.Publish(TestEvent{1});
  CHECK(count == 1);

  bus.Unsubscribe(id);

  bus.Publish(TestEvent{2});
  CHECK(count == 1); // Should not increment
}

TEST_CASE("EventBus - Unsubscribe only removes the targeted subscriber") {
  EventBus bus;
  int countA = 0;
  int countB = 0;

  SubscriptionID idA = bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { countA++; }));
  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { countB++; }));

  bus.Unsubscribe(idA);

  bus.Publish(TestEvent{1});
  CHECK(countA == 0);
  CHECK(countB == 1);
}

TEST_CASE("EventBus - Unsubscribe with invalid ID does nothing") {
  EventBus bus;
  int count = 0;

  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) { count++; }));

  bus.Unsubscribe(9999); // Invalid ID

  bus.Publish(TestEvent{1});
  CHECK(count == 1); // Subscriber should still be active
}

// ===================================================================
// EDGE CASES
// ===================================================================

TEST_CASE("EventBus - Publish with no subscribers does not crash") {
  EventBus bus;
  bus.Publish(TestEvent{42}); // Should not crash
}

TEST_CASE("EventBus - Subscribe returns unique IDs") {
  EventBus bus;

  SubscriptionID id1 = bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([](const TestEvent &) {}));
  SubscriptionID id2 = bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([](const TestEvent &) {}));
  SubscriptionID id3 = bus.Subscribe<AnotherEvent>(
      std::function<void(const AnotherEvent &)>([](const AnotherEvent &) {}));

  CHECK(id1 != id2);
  CHECK(id2 != id3);
  CHECK(id1 != id3);
}

TEST_CASE("EventBus - Multiple publishes deliver each time") {
  EventBus bus;
  int total = 0;

  bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &e) { total += e.value; }));

  bus.Publish(TestEvent{10});
  bus.Publish(TestEvent{20});
  bus.Publish(TestEvent{30});

  CHECK(total == 60);
}

TEST_CASE("EventBus - Unsubscribe during publish does not crash") {
  EventBus bus;
  SubscriptionID id = 0;
  int count = 0;

  id = bus.Subscribe<TestEvent>(
      std::function<void(const TestEvent &)>([&](const TestEvent &) {
        bus.Unsubscribe(id);
        count++;
      }));

  // Should not crash due to iterator invalidation
  bus.Publish(TestEvent{1});
  CHECK(count == 1);

  // Handler was removed, so this should not increment
  bus.Publish(TestEvent{2});
  CHECK(count == 1);
}
