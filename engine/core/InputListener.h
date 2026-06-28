#pragma once

#include "core/EventBus.h"

// Abstract interface for any class that listens to EventBus events.
//
// Classes that inherit from InputListener follow a subscribe-in-constructor,
// unsubscribe-in-destructor pattern:
//   1. In your constructor, call m_eventBus.Subscribe<SomeEvent>(...) and
//      store the returned SubscriptionID.
//   2. In your destructor, call m_eventBus.Unsubscribe(id) for each
//      subscription to avoid dangling callbacks.
//
// CameraController already uses this pattern. UIManager will too.
class InputListener {
public:
  InputListener(EventBus &eventBus) : m_eventBus(eventBus) {}
  virtual ~InputListener() = default;

protected:
  EventBus &m_eventBus;
};
