# hsmcpp unit tests

GoogleTest-based unit tests for the hsmcpp library. One source file per feature
area lives in `testcases/` (states, substates, transitions, parallel, history,
timers, final states, dispatchers, …).

## Requirement traceability

Each test that verifies a software requirement links to it with the
`TEST_REQUIREMENTS` macro (from `TestsCommon.hpp`) as the **first line** of the test
body, before `TEST_DESCRIPTION`:

```cpp
#include "TestsCommon.hpp"

TEST_F(TrafficLightHsm, simple_transition) {
    TEST_REQUIREMENTS("HSMCPP.SWR_HSM_040,HSMCPP.SWR_HSM_041");  // requirement(s) verified
    TEST_DESCRIPTION("Simple transition between two states");
    // ... test body ...
}
```

Rules:
- Pass one or more comma-separated requirement UIDs (`HSMCPP.SWR_HSM_###`).
- The macro expands to zero-cost GTest `RecordProperty()` calls; it never
  changes test behavior. `lobster-gtest` reads the UIDs from the GTest XML to
  build the trace report (see `../justfile`: `just req_coverage`).
- A test that intentionally verifies no requirement (pure robustness / crash
  test) carries a short `// NOTE(...)` comment explaining why instead of a trace.
- Give every test a meaningful `TEST_DESCRIPTION`.

## Test title / scenario notation

Many test descriptions (and `TEST_DESCRIPTION` strings) use a compact ASCII
shorthand to sketch the state-machine scenario. The notation below originated in
`testcases/07_parallel.cpp` and applies across the test suite.

| Notation      | Meaning                                                                 |
|---------------|-------------------------------------------------------------------------|
| `A`, `B`, `C` | a regular state                                                         |
| `*A`          | the initial / currently active state                                    |
| `->`          | a transition                                                            |
| `-e1->`       | a transition triggered by event `e1`                                    |
| `B + C`       | a transition that activates two states at the same time (parallel)      |
| `[ ... ]`     | a parent (composite) state and its contents, e.g. `[A, B]`              |
| `#C`          | an entry point into a composite state (resolves to substate `C`)        |
| `!#C`         | an entry point whose condition evaluates to **false** (not taken)       |
| `#[C]`        | an entry point into a nested composite state                            |
| `{Cx}`        | the transition out of state `C` was blocked / cancelled                 |
| `xE`          | entry into state `E` was cancelled (e.g. its enter callback returned false) |
| `H`           | a history state (`H[x]` emphasizes the recorded/restored child)         |

Example readings:
- `*A -> B + C` — from the initial state `A`, one event activates `B` and `C`
  concurrently.
- `*A -> B + [!#C, #D]` — the event activates `B`, and enters a composite whose
  entry point `C` is skipped (false condition) while entry point `D` is taken.
- `A -> [*#B + *#C] -> D {Cx}` — a parallel configuration `B`/`C` inside a parent
  that then transitions to `D`, but that transition is cancelled.

For richer scenarios, tests also embed PlantUML (`@startuml … @enduml`) diagrams
in a comment block above the test body; those take precedence over the shorthand
when both are present.
