# Event-Driven Input Architecture Plan

**Version**: 1.0
**Date**: 2026-03-10
**Status**: Design Phase

---

## Executive Summary

Refactor input handling from direct polling in `main.cpp` into an event-driven observer system. The goal is to decouple input detection (Input class) from input consumption (Camera, Scene, future systems) using a lightweight EventBus. The design prioritizes simplicity and uses C++17 standard library features (std::function, std::variant, std::any) without external dependencies.

---

## 1. Problem Analysis

### Current State

```
main.cpp (the orchestrator that should not be)
    |
    +-- Input::Update()              // polls raw GLFW state
    +-- Input::GetMouseOffset()      // main reads mouse delta
    +-- Input::IsKeyDown/Held()      // main reads keys
    +-- camera.UpdateFront(...)      // main pushes to camera
    +-- camera.UpdatePosition(...)   // main pushes to camera
    |
    +-- scene.Update(deltaTime)      // scene knows nothing about input
```

Problems:
- `main.cpp` contains ~20 lines of input-to-camera wiring that belongs elsewhere.
- Camera movement logic (speed, direction math) lives in main, not in Camera or a controller.
- Scene receives a raw `Camera*` but has no way to update it from input (see the NOTE comment in Scene.cpp line 28).
- Adding new input consumers (e.g., UI toggle, object selection) means adding more code to main.

### Desired State

```
main.cpp (thin orchestrator)
    |
    +-- Input::Update()           // polls raw GLFW state
    +-- EventBus::Publish(...)    // Input fires events
    |
    +-- scene.Update(deltaTime)   // scene dispatches to subscribers
         |
         +-- CameraController     // subscribed to input events, moves Camera
         +-- (future consumers)   // subscribe independently
```

---

## 2. Architecture Overview

### Component Relationship Diagram

```
+---------------------+
|     EventBus        |  (singleton or instance owned by Scene/App)
|---------------------|
| Subscribe<T>(fn)    |  returns SubscriptionID
| Unsubscribe(id)     |
| Publish<T>(event)   |  calls all handlers for type T
+---------------------+
         ^
         |  publishes
         |
+---------------------+        +-------------------------+
|    Input (static)   |        |   CameraController      |
|---------------------|        |   (new Component)        |
| Update()            |------->| subscribes to:           |
|   now also fires    |        |   KeyEvent               |
|   events into an    |        |   MouseMoveEvent         |
|   EventBus ref      |        | owns movement logic      |
+---------------------+        | updates Camera via ref   |
                               +-------------------------+
                                         |
                                         v
                               +---------------------+
                               |      Camera          |
                               | (unchanged mostly)   |
                               | UpdateFront(x,y)     |
                               | UpdatePosition(pos)  |
                               +---------------------+
```

### Event Types

```
+------------------+------------------------------------------+
| Event Struct     | Fields                                   |
+------------------+------------------------------------------+
| KeyEvent         | int key, KeyAction action (Down/Held/Up) |
| MouseMoveEvent   | float xOffset, float yOffset             |
+------------------+------------------------------------------+
```

These are plain structs, not polymorphic. The EventBus dispatches by type using `std::type_index`.

---

## 3. Detailed Component Design

### 3.1 EventBus (`src/core/EventBus.h`)

A single-header, template-based event dispatcher. No inheritance required for events.

```cpp
// src/core/EventBus.h
#pragma once

#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <algorithm>

using SubscriptionID = uint32_t;

class EventBus {
public:
    template <typename EventT>
    SubscriptionID Subscribe(std::function<void(const EventT&)> handler) {
        SubscriptionID id = m_nextId++;
        auto& subs = m_subscribers[std::type_index(typeid(EventT))];
        subs.push_back({id, [handler](const void* event) {
            handler(*static_cast<const EventT*>(event));
        }});
        return id;
    }

    void Unsubscribe(SubscriptionID id) {
        for (auto& [typeIdx, subs] : m_subscribers) {
            subs.erase(
                std::remove_if(subs.begin(), subs.end(),
                    [id](const Subscription& s) { return s.id == id; }),
                subs.end());
        }
    }

    template <typename EventT>
    void Publish(const EventT& event) {
        auto it = m_subscribers.find(std::type_index(typeid(EventT)));
        if (it != m_subscribers.end()) {
            for (auto& sub : it->second) {
                sub.callback(&event);
            }
        }
    }

private:
    struct Subscription {
        SubscriptionID id;
        std::function<void(const void*)> callback;
    };

    std::unordered_map<std::type_index, std::vector<Subscription>> m_subscribers;
    SubscriptionID m_nextId = 1;
};
```

Key decisions:
- Type-erased storage using `std::function<void(const void*)>` internally, but type-safe public API via templates.
- `SubscriptionID` for clean unsubscription without exposing iterators.
- Synchronous dispatch -- events are handled immediately on Publish. This is correct for a single-threaded game loop.
- No heap allocation per event. Events are passed by const reference.

### 3.2 Event Structs (`src/core/InputEvents.h`)

```cpp
// src/core/InputEvents.h
#pragma once

#include <glm/glm.hpp>

enum class KeyAction { Down, Held, Up };

struct KeyEvent {
    int key;
    KeyAction action;
};

struct MouseMoveEvent {
    float xOffset;
    float yOffset;
};
```

Plain data structs. No virtual methods, no inheritance. Cheap to copy.

### 3.3 Input Class Changes (`src/core/Input.h`, `src/core/Input.cpp`)

The Input class gains a reference to EventBus and publishes events during `Update()`.

Changes to Input.h:
```cpp
// Add to Input.h:
#include "core/EventBus.h"

class Input {
public:
    static void Initialize(GLFWwindow *window, EventBus& eventBus);
    static void Update();
    // Keep IsKeyDown, IsKeyHeld, GetMouseOffset for backward compat
    // ...
private:
    static EventBus* s_EventBus;  // non-owning pointer to the EventBus
    // ... existing members unchanged ...
};
```

Changes to Input.cpp `Update()`:
```cpp
void Input::Update() {
    // ... existing memcpy logic unchanged ...

    // Publish key events
    for (int i = 0; i < Config::MAX_KEYS; ++i) {
        if (s_Keys[i] && !s_KeysLastFrame[i]) {
            s_EventBus->Publish(KeyEvent{i, KeyAction::Down});
        }
        if (s_Keys[i]) {
            s_EventBus->Publish(KeyEvent{i, KeyAction::Held});
        }
        if (!s_Keys[i] && s_KeysLastFrame[i]) {
            s_EventBus->Publish(KeyEvent{i, KeyAction::Up});
        }
    }

    // ... existing mouse offset calculation ...

    // Publish mouse event (only if there was movement)
    if (s_XOffset != 0.0f || s_YOffset != 0.0f) {
        s_EventBus->Publish(MouseMoveEvent{
            s_XOffset * s_Sensitivity,
            s_YOffset * s_Sensitivity
        });
    }
}
```

IMPORTANT: The existing static polling API (IsKeyDown, IsKeyHeld, GetMouseOffset) remains intact. This is not a rewrite -- it is additive. Anything that currently polls can continue to poll. The event system runs in parallel.

### 3.4 CameraController Component (`src/scene/CameraController.h`, `src/scene/CameraController.cpp`)

A new Component that subscribes to input events and drives a Camera. This is where the movement logic from main.cpp moves to.

```cpp
// src/scene/CameraController.h
#pragma once

#include "scene/Component.h"
#include "core/EventBus.h"
#include "core/InputEvents.h"
#include "cameras/Camera.h"
#include "core/Config.h"
#include <GLFW/glfw3.h>

class CameraController : public Component {
public:
    CameraController(Camera& camera, EventBus& eventBus);
    ~CameraController();

    void Update(float deltaTime) override;

private:
    void OnKey(const KeyEvent& event);
    void OnMouseMove(const MouseMoveEvent& event);

    Camera& m_camera;
    EventBus& m_eventBus;
    SubscriptionID m_keySub;
    SubscriptionID m_mouseSub;

    // Movement state accumulated from events within a frame
    glm::vec3 m_moveDirection{0.0f};
    float m_speed = Config::DEFAULT_CAMERA_SPEED;
};
```

```cpp
// src/scene/CameraController.cpp
#include "scene/CameraController.h"

CameraController::CameraController(Camera& camera, EventBus& eventBus)
    : m_camera(camera), m_eventBus(eventBus)
{
    m_keySub = m_eventBus.Subscribe<KeyEvent>(
        [this](const KeyEvent& e) { OnKey(e); });
    m_mouseSub = m_eventBus.Subscribe<MouseMoveEvent>(
        [this](const MouseMoveEvent& e) { OnMouseMove(e); });
}

CameraController::~CameraController() {
    m_eventBus.Unsubscribe(m_keySub);
    m_eventBus.Unsubscribe(m_mouseSub);
}

void CameraController::OnKey(const KeyEvent& event) {
    if (event.action != KeyAction::Held) return;

    // Accumulate direction; actual movement applied in Update with deltaTime
    if (event.key == GLFW_KEY_W)
        m_moveDirection += m_camera.GetFront();
    if (event.key == GLFW_KEY_S)
        m_moveDirection -= m_camera.GetFront();
    if (event.key == GLFW_KEY_A)
        m_moveDirection -= m_camera.GetRight();
    if (event.key == GLFW_KEY_D)
        m_moveDirection += m_camera.GetRight();
}

void CameraController::OnMouseMove(const MouseMoveEvent& event) {
    m_camera.UpdateFront(event.xOffset, event.yOffset);
}

void CameraController::Update(float deltaTime) {
    if (glm::length(m_moveDirection) > 0.0f) {
        glm::vec3 newPos = m_camera.GetPosition()
                         + glm::normalize(m_moveDirection) * m_speed * deltaTime;
        m_camera.UpdatePosition(newPos);
        m_moveDirection = glm::vec3(0.0f);  // reset for next frame
    }
}
```

Design notes:
- Events accumulate direction intent; `Update(deltaTime)` applies movement. This keeps frame-rate-independent movement correct.
- Normalizing accumulated direction prevents diagonal movement from being faster (a bug the current main.cpp has).
- CameraController unsubscribes in its destructor -- no dangling handlers.

### 3.5 Scene Changes

Scene needs access to EventBus so it can pass it to components. Minimal change:

```cpp
// Scene.h changes:
class Scene {
public:
    Scene(EventBus& eventBus);
    // ... rest unchanged ...
private:
    EventBus& m_eventBus;
    // ... existing members ...
};
```

Scene passes `m_eventBus` when constructing components that need it (like CameraController). The `AddCamera` method changes to create a CameraController internally, or the caller creates a GameObject with a CameraController component.

### 3.6 Refactored main.cpp

```cpp
int main() {
    // ... GLFW init unchanged ...

    EventBus eventBus;
    Input::Initialize(window, eventBus);

    // ... OpenGL state setup unchanged ...

    Camera camera{};
    camera.UpdatePosition(glm::vec3(0.0f, 2.0f, 5.0f));

    Scene scene{eventBus};

    // Camera controller as a component on a GameObject
    auto cameraObj = std::make_unique<GameObject>();
    cameraObj->AddComponent<CameraController>(camera, eventBus);
    scene.AddGameObject(std::move(cameraObj));
    scene.AddCamera(&camera);

    // ... cube setup unchanged ...

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Input::Update();

        // ESC handling -- could also be an event subscriber,
        // but keeping it here is fine for a window-level concern.
        if (Input::IsKeyDown(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, true);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        scene.Update(deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ...
}
```

The main loop drops from ~40 lines of logic to ~15. All camera movement code is gone from main.

---

## 4. Frame Execution Order

Understanding the per-frame order is critical to correctness:

```
1. Input::Update()
   - Copies raw key state to current frame state
   - Calculates mouse offset
   - Publishes KeyEvent (Held/Down/Up) for all active keys
   - Publishes MouseMoveEvent if offset != 0
     --> CameraController::OnKey() accumulates direction
     --> CameraController::OnMouseMove() updates camera front

2. scene.Update(deltaTime)
   - For each GameObject:
     - gameObject->Update(deltaTime)
       --> CameraController::Update(deltaTime)
           applies accumulated movement with deltaTime
     - gameObject->Draw(context)

3. glfwSwapBuffers / glfwPollEvents
   - GLFW callbacks fire, updating raw state for next frame
```

This order means: events fire, state accumulates, then Update applies it. Clean and deterministic.

---

## 5. File Changes Summary

| File | Change Type | Description |
|------|------------|-------------|
| `src/core/EventBus.h` | NEW | Template event dispatcher (~50 lines) |
| `src/core/InputEvents.h` | NEW | KeyEvent, MouseMoveEvent structs (~15 lines) |
| `src/core/Input.h` | MODIFY | Add EventBus* member, change Initialize signature |
| `src/core/Input.cpp` | MODIFY | Publish events in Update() |
| `src/scene/CameraController.h` | NEW | Component that subscribes to input events |
| `src/scene/CameraController.cpp` | NEW | Movement logic extracted from main.cpp |
| `src/scene/Scene.h` | MODIFY | Constructor takes EventBus& |
| `src/scene/Scene.cpp` | MODIFY | Store EventBus reference |
| `src/main.cpp` | MODIFY | Create EventBus, remove camera movement code |
| `CMakeLists.txt` | MODIFY | Add new source files |

Estimated new code: ~120 lines across new files.
Estimated removed code: ~15 lines from main.cpp.
Net addition: ~105 lines.

---

## 6. Implementation Phases

### Phase 1: EventBus Foundation
- Create `EventBus.h` (header-only)
- Create `InputEvents.h` (event structs)
- Write unit tests for EventBus (subscribe, publish, unsubscribe)

### Phase 2: Input Integration
- Modify `Input` to accept and store `EventBus*`
- Add event publishing to `Input::Update()`
- Existing polling API remains -- no breaking changes
- Write tests verifying events fire correctly

### Phase 3: CameraController
- Create `CameraController` component
- Move movement logic from main.cpp
- Wire up in main.cpp
- Manual testing: WASD + mouse should behave identically to before

### Phase 4: Cleanup
- Remove dead code from main.cpp
- Consider whether `Scene::AddCamera` should be replaced by requiring a CameraController
- Update CLAUDE.md if architecture section changes

---

## 7. Alternatives Considered and Rejected

### Alternative A: Signal/Slot Library (e.g., Boost.Signals2)
Rejected: External dependency for a problem that needs ~50 lines of code. Overkill.

### Alternative B: Virtual IInputListener Interface
```cpp
class IInputListener {
    virtual void OnKeyEvent(const KeyEvent&) = 0;
    virtual void OnMouseMove(const MouseMoveEvent&) = 0;
};
```
Rejected: Forces all listeners to implement all methods. Less flexible than std::function-based subscriptions. Also requires `dynamic_cast` or explicit registration, which is more ceremony than EventBus.

### Alternative C: std::variant-Based Event Queue
A deferred queue where events are stored as `std::variant<KeyEvent, MouseMoveEvent>` and processed later.
Rejected for now: Adds complexity (queue management, when to flush). Synchronous dispatch is correct for a single-threaded game loop. If multithreading is added later, the EventBus can be extended with a queue internally without changing the public API.

### Alternative D: Make Camera subscribe directly (no CameraController)
Rejected: Violates single responsibility. Camera should be a pure data/math object (position, orientation, view matrix). Input handling is a separate concern. CameraController separates these cleanly and allows non-input-driven cameras (e.g., scripted camera paths) to use the same Camera class.

---

## 8. Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Dangling subscriber (object destroyed but still subscribed) | Medium | Crash | CameraController unsubscribes in destructor. Document this pattern. |
| Event ordering issues | Low | Subtle bugs | Single-threaded synchronous dispatch eliminates ordering ambiguity. |
| Performance of iterating all 1024 keys | Low | Negligible | 1024 iterations with branch prediction is sub-microsecond. Can optimize later with a "dirty keys" set if needed. |
| Breaking existing Input polling API | None | N/A | Polling API is kept intact. EventBus is purely additive. |
| Sensitivity applied twice (Input + Camera) | Medium | Camera moves too fast/slow | Move sensitivity multiplication into Input's event publishing so MouseMoveEvent contains already-scaled values. Remove sensitivity from Camera if present. |

---

## 9. Testing Strategy

### Unit Tests (doctest)

```cpp
// tests/EventBusTest.cpp
TEST_CASE("EventBus subscribe and publish") {
    EventBus bus;
    int received = 0;
    bus.Subscribe<KeyEvent>([&](const KeyEvent& e) {
        received = e.key;
    });
    bus.Publish(KeyEvent{42, KeyAction::Down});
    CHECK(received == 42);
}

TEST_CASE("EventBus unsubscribe stops delivery") {
    EventBus bus;
    int count = 0;
    auto id = bus.Subscribe<KeyEvent>([&](const KeyEvent&) { count++; });
    bus.Publish(KeyEvent{1, KeyAction::Down});
    CHECK(count == 1);
    bus.Unsubscribe(id);
    bus.Publish(KeyEvent{1, KeyAction::Down});
    CHECK(count == 1);  // no change
}

TEST_CASE("EventBus different event types are independent") {
    EventBus bus;
    int keyCount = 0, mouseCount = 0;
    bus.Subscribe<KeyEvent>([&](const KeyEvent&) { keyCount++; });
    bus.Subscribe<MouseMoveEvent>([&](const MouseMoveEvent&) { mouseCount++; });
    bus.Publish(KeyEvent{1, KeyAction::Down});
    CHECK(keyCount == 1);
    CHECK(mouseCount == 0);
}
```

### Integration Testing
- Manual: WASD + mouse look behavior must be identical before and after refactor.
- Verify ESC still closes the window.
- Verify no memory leaks (CameraController cleanup on Scene destruction).

---

## 10. Future Extensions (Not In Scope Now)

These are enabled by this architecture but should NOT be implemented in this phase:

- **Window resize events**: Add `WindowResizeEvent`, Scene subscribes to update projection.
- **Input action mapping**: An `InputMap` layer that translates raw keys to semantic actions ("MoveForward" instead of GLFW_KEY_W).
- **Event queue for multithreading**: Replace synchronous dispatch with a buffered queue if a separate update thread is introduced.
- **Editor mode toggle**: Subscribe to a key that toggles between game camera and free-fly editor camera.
