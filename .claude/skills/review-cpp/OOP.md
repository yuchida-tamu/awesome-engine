# OOP Principles

## SOLID principles

### Single Responsibility (S)

- `[WARNING]` Class doing multiple unrelated things
- `[SUGGESTION]` Methods that could be extracted to separate classes

### Open/Closed (O)

- `[SUGGESTION]` Code using switch/if-else on type that should use polymorphism
- `[WARNING]` Modifying existing class to add new behavior instead of extending

### Liskov Substitution (L)

- `[CRITICAL]` Derived class that breaks base class contract
- `[WARNING]` Override that throws for operations base class supports

### Interface Segregation (I)

- `[SUGGESTION]` Interface with methods not all implementers need
- `[SUGGESTION]` Classes forced to implement empty/throwing methods

### Dependency Inversion (D)

- `[SUGGESTION]` High-level module directly instantiating low-level dependencies
- `[SUGGESTION]` Consider dependency injection for testability

## Inheritance

### Check for

- `[CRITICAL]` Missing `virtual` destructor in base class with virtual methods
- `[CRITICAL]` Calling virtual functions in constructor/destructor
- `[WARNING]` Deep inheritance hierarchies (prefer composition)
- `[WARNING]` Missing `override` keyword on overridden methods
- `[SUGGESTION]` Use `final` on classes/methods not intended for further derivation

```cpp
class Base {
public:
    virtual ~Base() = default;  // Virtual destructor required!
    virtual void process() = 0;
};

class Derived : public Base {
public:
    void process() override;  // Always use override
};

class Concrete final : public Derived {  // Prevent further inheritance
    void process() override final;        // Prevent further override
};
```

## Encapsulation

### Check for

- `[WARNING]` Public member variables (except in POD/aggregate types)
- `[WARNING]` Getter returning non-const reference to private member (breaks encapsulation)
- `[SUGGESTION]` Consider making helper methods private
- `[SUGGESTION]` Use `friend` sparingly and with justification

```cpp
// Bad - exposes internal state
class Bad {
public:
    std::vector<int>& getData() { return m_data; }  // Can modify private!
private:
    std::vector<int> m_data;
};

// Good - controlled access
class Good {
public:
    const std::vector<int>& getData() const { return m_data; }
    void addData(int value) { m_data.push_back(value); }
private:
    std::vector<int> m_data;
};
```

## Design patterns

### Common patterns to recognize

- **Strategy**: Runtime-swappable algorithms via polymorphism
- **RAII**: Resource management tied to object lifetime
- **Singleton**: Single instance (use sparingly, often an anti-pattern)
- **Factory**: Object creation abstraction
- **Observer**: Event notification

### Check for

- `[WARNING]` Manual resource management that should use RAII
- `[SUGGESTION]` Switch statements on type that could be Strategy pattern
- `[WARNING]` Global mutable state that could be dependency-injected
