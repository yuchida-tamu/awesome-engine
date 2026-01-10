# Performance

## Copies and moves

### Check for

- `[WARNING]` Passing large objects by value instead of const reference
- `[WARNING]` Missing `std::move` when transferring ownership
- `[WARNING]` Using `std::move` on const objects (does nothing, silently copies)
- `[CRITICAL]` Using moved-from object (undefined state)
- `[SUGGESTION]` Return by value for move-enabled types (RVO/NRVO applies)

```cpp
// Bad - unnecessary copy
void process(std::vector<int> data);

// Good - const reference for read-only
void process(const std::vector<int>& data);

// Good - move when transferring ownership
void setData(std::vector<int> data) {  // Sink parameter
    m_data = std::move(data);
}

// Caller:
std::vector<int> v = createData();
obj.setData(std::move(v));  // Transfer ownership
// v is now in unspecified state - don't use!
```

## Container efficiency

### Check for

- `[SUGGESTION]` `reserve()` before known-size insertions to avoid reallocations
- `[WARNING]` `push_back` of temporary that could use `emplace_back`
- `[SUGGESTION]` Range-based algorithms instead of manual loops
- `[WARNING]` Linear search in large container (consider `unordered_map/set`)
- `[SUGGESTION]` `shrink_to_fit()` after removing many elements if memory matters

```cpp
// Good - reserve capacity upfront
std::vector<Widget> widgets;
widgets.reserve(1000);

// Good - construct in place
widgets.emplace_back(arg1, arg2);  // Constructs directly

// Avoid - creates temporary then moves
widgets.push_back(Widget(arg1, arg2));
```

## Loop optimization

### Check for

- `[WARNING]` Computing constant value inside loop
- `[WARNING]` Calling `size()` or `length()` every iteration (cache it for non-vector)
- `[SUGGESTION]` `++i` vs `i++` for iterators (prefix avoids temporary)
- `[WARNING]` String concatenation in loop (use `+=` and `reserve`)

```cpp
// Bad
for (size_t i = 0; i < str.size(); ++i) { }  // size() called each iteration

// Good
const size_t len = str.size();
for (size_t i = 0; i < len; ++i) { }

// Note: std::vector::size() is already O(1) and typically optimized
```

## Memory layout

### Check for

- `[SUGGESTION]` Struct member ordering (group by size to reduce padding)
- `[NITPICK]` Consider cache locality for frequently accessed data
- `[SUGGESTION]` Array of structs vs struct of arrays for hot paths

```cpp
// Bad - 24 bytes with padding
struct Bad {
    char a;      // 1 byte + 7 padding
    double b;    // 8 bytes
    char c;      // 1 byte + 7 padding
};

// Good - 16 bytes
struct Good {
    double b;    // 8 bytes
    char a;      // 1 byte
    char c;      // 1 byte + 6 padding
};
```

## Avoid premature optimization

- Don't sacrifice readability for micro-optimizations without profiling data
- Focus on algorithmic complexity first (O(n) vs O(n^2))
- Profile before optimizing
