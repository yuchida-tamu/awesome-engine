# Memory Management

## Rule of 5

Classes managing resources (raw pointers, file handles, OpenGL objects) must implement:

1. **Destructor** - cleanup resources
2. **Copy constructor** - deep copy or delete
3. **Copy assignment** - deep copy or delete
4. **Move constructor** - transfer ownership, zero source
5. **Move assignment** - cleanup current, transfer, zero source

### Check for

- `[CRITICAL]` Raw `new` without corresponding `delete` or smart pointer wrapper
- `[CRITICAL]` Missing destructor in class with raw resource handles
- `[CRITICAL]` Incomplete Rule of 5 (some special members defined, others missing)
- `[CRITICAL]` Move constructor/assignment not zeroing source object handles
- `[WARNING]` Copy operations not deleted for non-copyable resources (OpenGL handles, file descriptors)
- `[WARNING]` Missing `noexcept` on move operations

### Patterns

**RAII wrapper for OpenGL resources:**
```cpp
class VertexBuffer {
public:
    VertexBuffer() { glGenBuffers(1, &m_id); }
    ~VertexBuffer() { if (m_id != 0) glDeleteBuffers(1, &m_id); }

    // Delete copy
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Allow move
    VertexBuffer(VertexBuffer&& other) noexcept : m_id(other.m_id) {
        other.m_id = 0;
    }
    VertexBuffer& operator=(VertexBuffer&& other) noexcept {
        if (this != &other) {
            if (m_id != 0) glDeleteBuffers(1, &m_id);
            m_id = other.m_id;
            other.m_id = 0;
        }
        return *this;
    }
private:
    unsigned int m_id = 0;
};
```

## Smart pointers

### Check for

- `[CRITICAL]` Raw `new` that should be `std::make_unique` or `std::make_shared`
- `[WARNING]` `std::shared_ptr` when `std::unique_ptr` suffices (unnecessary overhead)
- `[WARNING]` Circular references with `shared_ptr` (use `weak_ptr` to break cycles)
- `[SUGGESTION]` Raw pointer parameters for non-owning access (prefer references or `std::span`)

### Prefer

```cpp
// Good
auto ptr = std::make_unique<Foo>();
auto shared = std::make_shared<Bar>();

// Avoid
Foo* ptr = new Foo();
std::shared_ptr<Bar> shared(new Bar());
```

## Common leaks

- `[CRITICAL]` Early return before cleanup code
- `[CRITICAL]` Exception thrown between allocation and deallocation
- `[CRITICAL]` Forgetting to delete array with `delete[]`
- `[CRITICAL]` Reassigning pointer without freeing previous allocation
