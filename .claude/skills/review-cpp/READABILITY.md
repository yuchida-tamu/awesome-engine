# Readability

## Naming conventions

### Check for

- `[NITPICK]` Inconsistent naming style within codebase
- `[SUGGESTION]` Unclear abbreviations - prefer full words
- `[NITPICK]` Single-letter variable names outside tiny scopes
- `[SUGGESTION]` Boolean names should read as questions (`isValid`, `hasData`, `canProcess`)

### Common conventions

```cpp
// Types - PascalCase
class MyClass;
struct MyStruct;
enum class MyEnum;

// Functions/methods - camelCase or snake_case (be consistent)
void processData();
void process_data();

// Variables - camelCase or snake_case
int itemCount;
int item_count;

// Member variables - m_ prefix common
int m_itemCount;

// Constants - UPPER_SNAKE_CASE or k prefix
constexpr int MAX_SIZE = 100;
constexpr int kMaxSize = 100;
```

## Const correctness

### Check for

- `[WARNING]` Method that doesn't modify state missing `const`
- `[WARNING]` Parameter that isn't modified missing `const`
- `[SUGGESTION]` Local variable that doesn't change should be `const`
- `[WARNING]` Const method returning non-const pointer/reference to member

```cpp
class Widget {
public:
    // Good - marked const since it doesn't modify state
    int getValue() const { return m_value; }

    // Good - const reference parameter
    void process(const std::string& input);

    // Bad - should be const
    int getValue() { return m_value; }  // Missing const!

private:
    int m_value;
};

// Good - const local
const int result = calculate();
```

## Function design

### Check for

- `[WARNING]` Functions longer than ~50 lines - consider splitting
- `[WARNING]` More than 3-4 parameters - consider parameter object
- `[SUGGESTION]` Output parameters - prefer return values
- `[SUGGESTION]` Boolean parameters - consider enum for clarity
- `[WARNING]` Deep nesting (>3 levels) - extract or use early returns

```cpp
// Bad - unclear boolean
render(mesh, true, false);

// Good - self-documenting
enum class Wireframe { Yes, No };
enum class DepthTest { Enabled, Disabled };
render(mesh, Wireframe::Yes, DepthTest::Disabled);

// Bad - output parameter
void getPosition(float& x, float& y, float& z);

// Good - return struct
struct Position { float x, y, z; };
Position getPosition();
```

## Comments

### Check for

- `[NITPICK]` Comments that repeat what code says
- `[SUGGESTION]` Complex algorithm without explaining the "why"
- `[NITPICK]` Commented-out code - remove or explain why kept
- `[SUGGESTION]` TODO/FIXME without context or tracking

```cpp
// Bad - states the obvious
i++; // Increment i

// Good - explains why
i++; // Skip header row in CSV

// Good - documents non-obvious behavior
// Using Welford's algorithm for numerical stability
```

## Code structure

### Check for

- `[NITPICK]` Inconsistent brace style
- `[NITPICK]` Inconsistent spacing around operators
- `[SUGGESTION]` Related code not grouped together
- `[SUGGESTION]` Public interface cluttered - consider pImpl for complex classes

### Early returns

```cpp
// Bad - deep nesting
void process(Data* data) {
    if (data != nullptr) {
        if (data->isValid()) {
            if (data->hasContent()) {
                // actual work here
            }
        }
    }
}

// Good - early returns
void process(Data* data) {
    if (data == nullptr) return;
    if (!data->isValid()) return;
    if (!data->hasContent()) return;

    // actual work here - no nesting
}
```
