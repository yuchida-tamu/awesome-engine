---
name: review-cpp
description: Reviews C++ code for memory management, modern C++17 idioms, OOP principles, performance, security, and readability. Use when reviewing C++ files, checking code quality, critiquing implementations, or when the user asks about issues in .cpp or .h files.
---

# C++ Code Review

Reviews C++ code with severity-based feedback and educational explanations. Targets C++17 standard.

## Severity levels

| Level | Use for |
|-------|---------|
| `[CRITICAL]` | Memory leaks, undefined behavior, security vulnerabilities, resource management bugs |
| `[WARNING]` | Performance issues, missing move semantics, poor practices that could cause bugs |
| `[SUGGESTION]` | Modern C++ improvements, better idioms, cleaner alternatives |
| `[NITPICK]` | Style preferences, minor readability improvements |

## Review workflow

Copy this checklist and track progress:

```
Review Progress:
- [ ] Step 1: Read the target file
- [ ] Step 2: Read PROJECT_PATTERNS.md for project conventions
- [ ] Step 3: Analyze memory management (see MEMORY.md)
- [ ] Step 4: Check modern C++17 usage (see MODERN_CPP.md)
- [ ] Step 5: Evaluate OOP principles (see OOP.md)
- [ ] Step 6: Assess performance (see PERFORMANCE.md)
- [ ] Step 7: Check security concerns (see SECURITY.md)
- [ ] Step 8: Review readability (see READABILITY.md)
- [ ] Step 9: Output findings sorted by severity
```

## Output format

Use this structure for review output:

```markdown
## Code Review: <filename>

### Summary
X critical, Y warnings, Z suggestions, W nitpicks

### Findings

[CRITICAL] Line <N>: <issue title>

<problematic code snippet>

**Issue**: <concise description of what's wrong>

**Fix**:
<corrected code snippet>

**Learn**: <educational explanation - why this matters, the underlying principle>

---

[WARNING] Line <N>: <issue title>
...
```

## Guidelines

- Always read PROJECT_PATTERNS.md first to understand project-specific conventions
- Provide concrete code fixes, not just descriptions
- Educational explanations should teach the "why" not just the "what"
- Group related issues together
- Prioritize findings by severity
- Be specific about line numbers
- If no issues found in a category, skip it (don't say "no issues found")
