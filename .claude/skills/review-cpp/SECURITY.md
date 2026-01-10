# Security and Undefined Behavior

## Undefined behavior

### Critical issues

- `[CRITICAL]` Dereferencing null pointer
- `[CRITICAL]` Out-of-bounds array/vector access
- `[CRITICAL]` Use after free / use after move
- `[CRITICAL]` Signed integer overflow
- `[CRITICAL]` Uninitialized variable read
- `[CRITICAL]` Data race (concurrent access without synchronization)
- `[CRITICAL]` Infinite recursion / stack overflow
- `[CRITICAL]` Modifying const object through cast
- `[CRITICAL]` Accessing inactive union member

### Check for

```cpp
// Null dereference
Foo* ptr = getPtr();
ptr->method();  // UB if ptr is null - check first!

// Out of bounds
vec[i];      // UB if i >= vec.size()
vec.at(i);   // Throws std::out_of_range - safer

// Signed overflow
int x = INT_MAX;
x + 1;  // UB! Use unsigned or check before
```

## Buffer overflows

### Check for

- `[CRITICAL]` Raw array with unchecked index
- `[CRITICAL]` `strcpy`, `sprintf`, `gets` - use safe alternatives
- `[WARNING]` Manual memory size calculations (use `sizeof`, `std::size`)
- `[SUGGESTION]` Prefer `std::array`, `std::vector`, `std::string` over raw arrays

```cpp
// Dangerous
char buffer[100];
strcpy(buffer, userInput);  // Buffer overflow if input > 99 chars

// Safe alternatives
std::string buffer = userInput;  // Automatic sizing
strncpy(buffer, input, sizeof(buffer) - 1);  // Bounded copy
```

## Integer issues

### Check for

- `[WARNING]` Signed/unsigned comparison (implicit conversion issues)
- `[WARNING]` Integer truncation in assignments
- `[CRITICAL]` Division by zero possibility
- `[WARNING]` Size type mismatches (`int` vs `size_t`)

```cpp
// Dangerous comparison
int i = -1;
if (i < vec.size()) { }  // -1 converts to huge unsigned!

// Safe
if (i >= 0 && static_cast<size_t>(i) < vec.size()) { }
```

## Input validation

### Check for

- `[CRITICAL]` File paths from user input without sanitization
- `[WARNING]` Unchecked array indices from external input
- `[WARNING]` Missing bounds checking on string parsing
- `[SUGGESTION]` Validate all external data at system boundaries

## Exception safety

### Levels

- **No-throw**: Operation cannot fail (destructors, move operations)
- **Strong**: If exception thrown, state unchanged (rollback)
- **Basic**: No leaks, object in valid state (but may be modified)
- **None**: No guarantees (avoid!)

### Check for

- `[CRITICAL]` Destructor that can throw
- `[WARNING]` Move constructor/assignment that can throw (breaks `noexcept` promise)
- `[WARNING]` Partial state modification before potentially throwing operation
- `[SUGGESTION]` Mark non-throwing functions `noexcept`

```cpp
// Strong guarantee pattern
void update(const Data& newData) {
    Data temp = newData;    // Copy that might throw
    std::swap(m_data, temp); // swap is noexcept - commit point
}
```

## Thread safety

### Check for

- `[CRITICAL]` Shared mutable state without mutex/atomic
- `[WARNING]` Lock ordering inconsistency (deadlock risk)
- `[SUGGESTION]` Use `std::lock_guard` or `std::scoped_lock` over manual lock/unlock
- `[WARNING]` Returning reference to data protected by internal lock

```cpp
// Bad - lock released before use
const Data& getData() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_data;  // Reference returned after lock released!
}

// Good - return copy
Data getData() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_data;  // Returns copy while locked
}
```
