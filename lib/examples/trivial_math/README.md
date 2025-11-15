# Trivial Math Example

**Phase 0.1: Pure C++ Library Pattern**

This example demonstrates the foundational pattern for platform-agnostic C++ libraries in the halloween2 project:
- Header-only constexpr functions for compile-time evaluation
- Runtime versions for coverage testing
- GoogleTest unit tests
- 100% code coverage
- No hardware dependencies

## Purpose

Validate that the build system, test framework, and coverage reporting work correctly for pure C++ code before adding complexity.

## Project Structure

```
trivial_math/
├── include/
│   └── trivial_math.h       # Constexpr + runtime function declarations
├── src/
│   └── trivial_math.cpp     # Runtime implementations
├── test/
│   └── test_trivial_math.cpp # GoogleTest tests (12 tests)
├── CMakeLists.txt           # Build configuration
└── README.md                # This file
```

## Implementation

### trivial_math.h

Provides basic math operations in two forms:

1. **Constexpr functions**: Compile-time evaluation when possible
   - `add(a, b)`, `multiply(a, b)`, `subtract(a, b)`, `divide(a, b)`

2. **Runtime versions**: For coverage testing
   - `add_runtime(a, b)`, etc.
   - Delegates to constexpr versions but forces runtime evaluation

### Why Both Versions?

**Constexpr functions:**
- Can be evaluated at compile time
- Zero runtime cost when arguments are constants
- Modern C++ best practice

**Runtime versions:**
- Force execution at runtime
- Generate .gcda files for coverage tracking
- Ensure gcov can measure code execution

**Pattern:**
```cpp
// Constexpr version (compile-time capable)
constexpr int add(int a, int b) {
    return a + b;
}

// Runtime version (for coverage)
int add_runtime(int a, int b) {
    return add(a, b);  // Delegates to constexpr version
}
```

## Building and Testing

### Desktop Tests (Recommended)

```bash
# Configure
cmake -B build/test -DBUILD_TESTS=ON

# Build
cmake --build build/test

# Run tests
ctest --test-dir build/test --output-on-failure

# Should see: 100% tests passed, 0 tests failed out of 1
```

### Coverage Generation

```bash
# Configure with coverage enabled
cmake -B build/coverage -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON

# Build and run tests
cmake --build build/coverage
cd build/coverage && ctest && cd ../..

# Generate reports
gcovr -r . --sonarqube coverage.xml --html-details coverage-html/index.html --print-summary

# View HTML report
xdg-open coverage-html/index.html
```

### Using Pixi (Easiest)

```bash
# Run tests
pixi run test-trivial

# Generate coverage
pixi run coverage-trivial

# View coverage report
pixi run view-coverage-trivial
```

## Test Coverage

### Coverage Target: 100%

All functions exercised with multiple test cases:

**Addition tests:**
- Positive + positive
- Negative + positive
- Zero + zero
- Compile-time evaluation

**Multiplication tests:**
- Positive × positive
- Negative × positive
- Zero × non-zero
- Compile-time evaluation

**Subtraction tests:**
- Larger - smaller
- Smaller - larger (negative result)
- Zero - zero
- Compile-time evaluation

**Division tests:**
- Integer division
- Division by zero (returns 0)
- Compile-time evaluation

### Example Test

```cpp
TEST(TrivialMathTest, AdditionWorks) {
    // Runtime version (for coverage)
    EXPECT_EQ(trivial::add_runtime(2, 3), 5);
    EXPECT_EQ(trivial::add_runtime(-1, 1), 0);
    EXPECT_EQ(trivial::add_runtime(0, 0), 0);
}

TEST(TrivialMathTest, ConstexprAdditionWorks) {
    // Constexpr version (compile-time when possible)
    constexpr int result = trivial::add(2, 3);
    EXPECT_EQ(result, 5);

    // Also works at runtime
    EXPECT_EQ(trivial::add(2, 3), 5);
}
```

## Coverage Results

Expected output from `gcovr`:

```
------------------------------------------------------------------------------
File                                       Lines    Exec  Cover
------------------------------------------------------------------------------
lib/examples/trivial_math/src/trivial_math.cpp
                                              16      16   100%
------------------------------------------------------------------------------
TOTAL                                         16      16   100%
------------------------------------------------------------------------------
```

## Design Rationale

### Why Constexpr?

1. **Performance**: Computations done at compile time when possible
2. **Modern C++**: Demonstrates C++17+ features
3. **Safety**: Compile-time evaluation catches errors earlier

### Why Separate Runtime Versions?

1. **Coverage testing**: gcov needs runtime execution
2. **Verification**: Ensures constexpr logic is correct
3. **Pattern demonstration**: Shows how to make logic testable

### Why No Hardware?

1. **Foundation first**: Validate toolchain before adding complexity
2. **Fast iteration**: Tests run instantly
3. **CI/CD friendly**: No hardware needed for testing
4. **Platform agnostic**: Same code works everywhere

## Common Issues

### Issue: Coverage shows 0%

**Cause**: Tests didn't run or coverage data not generated

**Solution**:
```bash
# Verify tests ran
ctest --test-dir build/coverage --verbose

# Check for .gcda files
find build/coverage -name "*.gcda"

# Regenerate coverage
cd build/coverage && ctest && gcovr -r ../.. --print-summary
```

### Issue: gcov can't find source files

**Cause**: Running gcovr from wrong directory

**Solution**:
```bash
# gcovr must be run from project root
cd /path/to/sonarcloud-cpp-minimal
gcovr -r . --print-summary
```

### Issue: SonarCloud shows 0% coverage

**Cause**: Coverage report not in correct format or location

**Solution**:
```bash
# Verify coverage.xml exists in project root
ls -l coverage.xml

# Check format is SonarQube XML
head -n 5 coverage.xml  # Should contain <coverage version="1">

# Verify sonar-project.properties
grep coverageReportPaths sonar-project.properties
```

## Extending This Pattern

This pattern scales to complex libraries:

### State Machine Example
```cpp
namespace state_machine {

enum class State { IDLE, RUNNING, PAUSED };

constexpr State transition(State current, bool start, bool pause) {
    if (current == State::IDLE && start) return State::RUNNING;
    if (current == State::RUNNING && pause) return State::PAUSED;
    return current;
}

// Runtime version for coverage
State transition_runtime(State current, bool start, bool pause);

} // namespace state_machine
```

### Kinematics Example
```cpp
namespace kinematics {

struct Position { float x, y, z; };

constexpr Position lerp(Position a, Position b, float t) {
    return {
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    };
}

// Runtime version for coverage
Position lerp_runtime(Position a, Position b, float t);

} // namespace kinematics
```

## Success Metrics

- ✅ 12 comprehensive test cases (all passing)
- ✅ 100% line coverage (16/16 lines)
- ✅ 100% function coverage (8/8 functions)
- ✅ Both constexpr and runtime versions tested
- ✅ Division by zero edge case handled
- ✅ CMake configuration works
- ✅ GoogleTest integration works
- ✅ Coverage generation works
- ✅ Pattern documented and reusable

## Next Steps

Phase 0.2 (blink_led) builds on this pattern by adding:
- Hardware abstraction via interfaces
- Dependency injection via templates
- Mock implementations for testing
- Arduino .ino wrapper

See `/home/griswald/personal/sonarcloud-cpp-minimal/projects/examples/blink_led/` for the next pattern.

## References

- **GoogleTest**: https://github.com/google/googletest
- **gcovr**: https://gcovr.com/
- **CMake**: https://cmake.org/cmake/help/latest/
- **Modern C++ constexpr**: https://en.cppreference.com/w/cpp/language/constexpr

---

**Created:** 2025-11-14
**Pattern:** Pure C++ Library with Constexpr + Runtime Versions
**Coverage:** 100% (12 tests)
**Status:** ✅ Complete
