# Architect Log: Event-Driven Input System Design

**Date**: 2026-03-10
**Task**: Design event/observer system for input handling

## Files Analyzed

- `src/main.cpp` - Main loop with inline input-to-camera wiring
- `src/core/Input.h` / `Input.cpp` - Static GLFW callback wrapper with polling API
- `src/cameras/Camera.h` / `Camera.cpp` - Pure math/state camera (position, orientation, view matrix)
- `src/scene/Scene.h` / `Scene.cpp` - Scene graph with raw Camera* and GameObject vector
- `src/scene/Component.h` - Base class for ECS-like components
- `src/scene/GameObject.h` - Entity with type-indexed component map
- `src/core/Config.h` - Constants
- `src/rendering/RenderContext.h` - Per-frame projection/view matrices

## Key Observations

1. Input class is purely static with GLFW callbacks. It stores raw/current/last-frame key state for robust polling. Mouse offset is calculated per frame in Update().

2. Camera is a clean data object -- position, front, up vectors plus pitch/yaw. It has no knowledge of input. Good separation already.

3. main.cpp contains the glue: reads Input state, computes camera speed * deltaTime, updates camera position. This is ~15 lines that belong in a component.

4. Scene receives Camera* but has a NOTE comment (line 28 in Scene.cpp) acknowledging the problem: "I have to update Input somehow, so that it updates camera positions."

5. GameObject has a working component system with type_index lookup. CameraController fits naturally as a Component.

6. The existing bug: diagonal movement (W+D simultaneously) is sqrt(2) faster because directions are added without normalization. The CameraController design fixes this by accumulating then normalizing.

## Decisions Made

| Decision | Rationale |
|----------|-----------|
| Template-based EventBus over virtual interface | Avoids forcing listeners to implement unused methods. std::function is flexible enough. |
| Synchronous dispatch, not queued | Single-threaded engine. Queuing adds complexity with no benefit. |
| Keep polling API alongside events | Non-breaking. Some uses (ESC check in main) are cleaner as polls. |
| CameraController as Component, not Camera modification | Camera stays pure math. Controller is the input-aware layer. Enables scripted cameras. |
| EventBus as instance, not singleton | Testable. Can be passed by reference. Avoids global state. |
| Sensitivity applied at publish time in Input | Single source of truth. Subscribers get final values. |

## Assumptions

- No multithreading will be introduced in the near term
- The engine will remain single-window
- Event count per frame is small (< 20 events) so performance of std::function dispatch is not a concern
- Component destruction order is deterministic (unique_ptr vector cleared in order)
