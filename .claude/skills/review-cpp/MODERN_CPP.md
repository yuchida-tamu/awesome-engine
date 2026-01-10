# Modern C++17

## Auto and type deduction

### Check for

- `[SUGGESTION]` Verbose type declarations where `auto` improves readability
- `[WARNING]` `auto` hiding important type information (e.g., `auto x = getValue()` when type matters)
- `[SUGGESTION]` Missing structured bindings for pair/tuple returns

### Patterns

```cpp
// Structured bindings (C++17)
auto [x, y, z] = getPosition();
for (const auto& [key, value] : map) { }

// Range-based for with references
for (const auto& item : container) { }  // Read-only
for (auto& item : container) { }        // Modify
```

## std::optional

### Check for

- `[SUGGESTION]` Functions returning pointer/sentinel for "maybe no value" - use `std::optional`
- `[WARNING]` Accessing `std::optional` without checking `has_value()` or using `value_or()`

```cpp
// Good
std::optional<Config> loadConfig(const std::string& path);
if (auto cfg = loadConfig(path)) {
    use(*cfg);
}

// Avoid
Config* loadConfig(const std::string& path);  // nullptr = not found
```

## std::string_view

### Check for

- `[SUGGESTION]` Function taking `const std::string&` that only reads - consider `std::string_view`
- `[WARNING]` Storing `string_view` beyond lifetime of source string
- `[SUGGESTION]` String concatenation in loops - use `std::string` with `reserve()`

```cpp
// Good - no allocation for string literals or substrings
void process(std::string_view sv);

// Be careful - sv must not outlive source
std::string_view dangerous = std::string("temp");  // Dangling!
```

## constexpr and compile-time

### Check for

- `[SUGGESTION]` Constants that could be `constexpr` instead of `const`
- `[SUGGESTION]` Simple functions that could be `constexpr`
- `[SUGGESTION]` `if constexpr` for compile-time branching instead of SFINAE

```cpp
constexpr int square(int x) { return x * x; }
constexpr int SIZE = square(10);  // Computed at compile time

template<typename T>
void process(T value) {
    if constexpr (std::is_integral_v<T>) {
        // Integer path
    } else {
        // Other path
    }
}
```

## Initialization

### Check for

- `[WARNING]` Uninitialized variables
- `[SUGGESTION]` Use `{}` initialization for safety (narrowing conversion errors)
- `[WARNING]` Most vexing parse issues - use `{}` or `=` to avoid

```cpp
int x{};      // Zero initialized
int y = 0;    // Also fine
int z;        // Uninitialized - dangerous!

std::vector<int> v{10, 20};  // Two elements: 10 and 20
std::vector<int> w(10, 20);  // Ten elements, all 20
```

## Attributes

### Check for

- `[SUGGESTION]` Missing `[[nodiscard]]` on functions where ignoring return is likely a bug
- `[SUGGESTION]` Missing `[[maybe_unused]]` on intentionally unused parameters
- `[SUGGESTION]` Missing `[[fallthrough]]` in switch statements with intentional fallthrough

```cpp
[[nodiscard]] bool save();  // Compiler warns if return ignored
void callback([[maybe_unused]] int param);  // Silences unused warning
```
