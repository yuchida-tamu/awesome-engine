# Project-Specific Patterns

This file documents conventions specific to the awesome-engine project. Violations of these patterns should be flagged during review.

## C++ Standard

This project uses **C++17**. Leverage C++17 features where appropriate.

## Resource-Managing Classes (Rule of 5)

Classes managing OpenGL resources (VAO, VBO, textures, shaders) must follow Rule of 5:

### Pattern

```cpp
class ResourceClass {
public:
    ResourceClass();
    ~ResourceClass();

    // Delete copy (OpenGL handles can't be copied)
    ResourceClass(const ResourceClass&) = delete;
    ResourceClass& operator=(const ResourceClass&) = delete;

    // Allow move with noexcept
    ResourceClass(ResourceClass&&) noexcept;
    ResourceClass& operator=(ResourceClass&&) noexcept;

private:
    unsigned int m_handle = 0;
};
```

### Check for

- `[CRITICAL]` Resource class missing any of the 5 special members
- `[CRITICAL]` Move operations not zeroing source handles
- `[WARNING]` Move operations missing `noexcept`
- `[WARNING]` Copy operations not explicitly deleted for resource classes
- `[CRITICAL]` Copying `Shader` or similar resource classes by value

### Example classes in project

- `Shader` - manages OpenGL shader program
- `Skybox` - manages cubemap texture and VAO
- Any class with `glGen*` in constructor

## Include Order

GLAD must be included before GLFW. Use a blank line to prevent formatters from reordering:

```cpp
#include <glad/glad.h>

#include <GLFW/glfw3.h>
```

### Check for

- `[CRITICAL]` GLFW included before GLAD
- `[WARNING]` Missing blank line separator between GLAD and GLFW

## Pass-by-Reference for Resource Classes

Resource-managing classes must be passed by reference, never by value.

```cpp
// Good
void render(const Shader& shader);
void setup(Shader& shader);

// Bad - would attempt to copy (won't compile with deleted copy)
void render(Shader shader);
```

### Check for

- `[CRITICAL]` Attempting to pass resource class by value

## Strategy Pattern (PostProcessing)

Post-processing effects use the Strategy pattern. New effects should:

1. Inherit from `PostProcessEffectStrategy`
2. Implement the required interface methods
3. Be swappable at runtime

### Check for

- `[SUGGESTION]` New post-processing code not using Strategy pattern
- `[WARNING]` Strategy implementations with hardcoded dependencies

## Source Organization

Follow the existing directory structure:

| Directory | Purpose |
|-----------|---------|
| `core/` | Config, Input, TextureLoader |
| `cameras/` | Camera implementations |
| `meshes/` | Mesh base class and implementations |
| `rendering/` | Shader, PostProcessing, Skybox |
| `scene/` | Scene management, Gizmos |

### Check for

- `[SUGGESTION]` New files placed in wrong directory
- `[NITPICK]` Inconsistent file naming (use PascalCase for classes)

## Testing

- Tests use doctest framework
- Test files go in `tests/`
- Use `UNIT_TEST` macro to expose private members for testing

### Check for

- `[SUGGESTION]` New functionality without corresponding tests
- `[WARNING]` Tests accessing private members without `UNIT_TEST` macro

## Member Variable Naming

Use `m_` prefix for member variables:

```cpp
class MyClass {
private:
    int m_value;
    std::string m_name;
};
```

### Check for

- `[NITPICK]` Member variables without `m_` prefix
