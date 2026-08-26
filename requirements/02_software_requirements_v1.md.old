# hsmcpp Software Requirements

**Grammar**: software.gra.md
**Prefix**: SWR-HSM-

## Core State Machine Engine

### C++ Standard Compatibility

**UID**: SWR-HSM-001
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: C++11 is the minimum standard widely available on embedded toolchains while still providing the language features needed by the library; forward compatibility ensures adoption on newer platforms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL compile and function correctly with C++11 and any later C++ standard revision. No application-visible behavior SHALL differ based on the C++ standard version used for compilation.

### Type-Safe State, Event, and Timer Identification

**UID**: SWR-HSM-002
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Inspection
**Rationale**: Type-safe identifiers prevent accidental misuse of raw numeric values and enable compile-time checking of state machine structure.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL provide type-safe, uniquely identifiable designators for states, events, and timers. The library SHALL allow users to define domain-specific identifier sets that the compiler can distinguish from unrelated integer or enumeration types.

### Independent Event Processing

**UID**: SWR-HSM-003
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Inspection
**Rationale**: Decoupling the state machine from any particular execution environment allows the same state machine logic to run on real-time operating systems, desktop platforms, and bare-metal environments without modification.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL provide built-in event-processing capabilities sufficient to execute a state machine without requiring integration with an external event-processing framework. The library SHALL supply at least one ready-to-use event-processing mechanism as part of its distribution.

### Single-Threaded Callback Execution Model

**UID**: SWR-HSM-004
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Analysis
**Rationale**: A single-threaded execution model for application-defined behavior eliminates data races within user-provided logic and removes the need for internal synchronization in application callbacks.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL execute all application-defined state and transition behavior on a single execution context. The library SHALL guarantee that no two application-defined behaviors execute concurrently, ensuring that application code does not require internal synchronization.

### HSM Initialization

**UID**: SWR-HSM-005
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Explicit initialization separates structure configuration from runtime execution, allowing the full state machine topology to be validated before any transitions occur.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL support explicit initialization that establishes the defined initial runtime state of the state machine. After successful initialization, the state machine SHALL be in its designated initial state and ready to process events. The library SHALL not process events before initialization completes successfully.

### Structure Immutability After Initialization

**UID**: SWR-HSM-006
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Analysis
**Rationale**: Immutable structure after initialization enables safe concurrent event processing without locks on the topology data and prevents accidental corruption of the state machine graph during operation.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL require that the complete state machine structure (states, transitions, and actions) is fully configured before initialization. The library SHALL either reject structural modifications attempted after initialization or leave the resulting behavior explicitly unspecified.

### Multiple HSM Instances Per Event-Processing Context

**UID**: SWR-HSM-007
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Sharing an event-processing context reduces resource consumption on constrained platforms and simplifies integration with application event loops that manage multiple state machines.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL support multiple state machine instances sharing a single event-processing context. Each state machine instance SHALL process events independently while sharing the underlying event-processing infrastructure.

## State Management

### State Definition

**UID**: SWR-HSM-008
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: States are the fundamental building blocks of a hierarchical state machine; associating behavior with state lifecycle events allows applications to react to state changes.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL support definition of uniquely identified states. Each state may have application-defined behavior associated with entry, exit, and state-changed lifecycle events.

### Initial State Configuration

**UID**: SWR-HSM-009
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Every well-formed state machine requires a defined starting state to ensure deterministic behavior upon initialization.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL allow the user to designate one state as the initial state. The designated initial state SHALL become active upon successful initialization of the state machine.

### Final State Support

**UID**: SWR-HSM-010
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Final states enable composite state completion semantics, allowing parent states to react when a substate machine reaches its terminal condition.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL support designating a state as a final state. When a final state becomes active, the library SHALL automatically generate a specified event. If no event is specified, the library SHALL re-use the triggering event that caused the transition into the final state.

### Active State Query

**UID**: SWR-HSM-011
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Applications need to inspect the current state machine configuration for conditional logic outside the state machine.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL provide the ability to determine which states are currently active. The library SHALL provide the ability to determine whether a specified state is active.

### Last Active State Query

**UID**: SWR-HSM-012
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: In state machines with parallel regions, the most recently activated state provides a deterministic single-state answer for simple queries.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL provide the ability to determine the most recently activated state.

## State Transitions

### Event-Triggered Transitions

**UID**: SWR-HSM-013
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Event-driven transitions are the core mechanism for state machine progression; uniquely identified events ensure unambiguous routing.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support transitions between states triggered by uniquely identified events.

### Conditional Transitions (Guards)

**UID**: SWR-HSM-014
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Guards allow dynamic routing of transitions based on runtime conditions without proliferating states.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support guard conditions on transitions. A guarded transition SHALL only execute when its associated runtime condition is satisfied.

### Self-Transitions

**UID**: SWR-HSM-015
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Internal self-transitions allow in-state processing without triggering exit and entry side effects, while external self-transitions reset state context.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support self-transitions in two modes: internal self-transitions that do not trigger state exit or entry behavior, and external self-transitions that perform full exit and re-entry of the state.

### Transition Priority

**UID**: SWR-HSM-016
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Deterministic priority resolution ensures predictable behavior when multiple conditional transitions compete for the same event.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

When multiple transitions from the same state match the same event, the library SHALL resolve competing transitions deterministically and SHALL execute the first transition whose guard condition is satisfied.

### Synchronous Transition Execution

**UID**: SWR-HSM-017
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Synchronous transitions simplify sequencing logic in scenarios where the caller must know the outcome before proceeding.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support the ability to request a transition synchronously and wait for its completion. If the transition does not complete within a defined timeout period, the wait SHALL terminate with a timeout indication.

### Pending Event Discard

**UID**: SWR-HSM-018
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Discarding pending events enables cancel-all-pending-work semantics for high-priority state changes such as error recovery or shutdown.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support the ability to discard all pending events before processing a newly requested event.

### Transition Possibility Check

**UID**: SWR-HSM-019
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Dry-run checks allow application logic to determine reachability without side effects, enabling conditional UI or decision logic.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support the ability to determine whether a specified event can cause a successful transition from the current state without changing the state machine state.

### Failed Transition Notification

**UID**: SWR-HSM-020
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Failed transition notifications enable logging, error handling, and defensive programming in response to unhandled or unprocessable events.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL notify application-defined behavior when an event cannot be processed due to missing transitions, failed guards, or cancellation by application-defined behavior.

### Transition-Associated Data

**UID**: SWR-HSM-021
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Passing context data through transitions eliminates the need for external shared state between transition triggers and handlers.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL support associating arbitrary application data with a transition request. The library SHALL make that data available to all application-defined behavior triggered by that transition.

## State and Transition Actions

### State Entry Behavior

**UID**: SWR-HSM-022
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Executing application-defined behavior on state entry enables initialization logic and resource acquisition tied to becoming active; the ability to cancel prevents entering an invalid configuration.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL execute application-defined entry behavior before a state becomes active. The entry behavior SHALL receive any data associated with the triggering transition. The entry behavior SHALL be able to prevent the state from becoming active, thereby cancelling the transition.

### State Exit Behavior

**UID**: SWR-HSM-023
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Executing application-defined behavior on state exit enables cleanup logic and resource release; the ability to cancel prevents leaving a state when preconditions for departure are not met.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL execute application-defined exit behavior before a state becomes inactive. The exit behavior SHALL be able to prevent the transition, keeping the state active.

### State-Changed Behavior

**UID**: SWR-HSM-024
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Executing behavior after a state has fully activated provides a safe point for operations that depend on the state machine having committed to the new configuration.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL execute application-defined state-changed behavior after a state has become fully active. The state-changed behavior SHALL receive any data associated with the triggering transition.

### Transition Behavior

**UID**: SWR-HSM-025
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Behavior executed during a transition enables side effects that logically belong to the act of transitioning rather than to entering or exiting a particular state.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL support application-defined behavior associated with the execution of a transition. The transition behavior SHALL execute after the source state has been exited and before the target state is entered. The transition behavior SHALL receive any data associated with the triggering transition.

### Guard Evaluation

**UID**: SWR-HSM-026
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Runtime guard conditions enable dynamic transition selection based on application state, allowing a single event to route to different targets depending on evaluated conditions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL evaluate a runtime condition before executing a guarded transition. The transition SHALL proceed only when the condition result matches the configured expected value.

### Automatic Timer Start

**UID**: SWR-HSM-027
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Declarative timer start on state lifecycle eliminates boilerplate code for common timeout patterns and ensures timers are reliably started when states change.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL support automatically starting a specified timer when a state is entered or exited, based on configuration.

### Automatic Timer Stop

**UID**: SWR-HSM-028
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Automatic timer cancellation prevents stale timeouts from firing after state transitions, reducing error-prone manual timer management.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL support automatically stopping a specified timer when a state is entered or exited, based on configuration.

### Automatic Timer Restart

**UID**: SWR-HSM-029
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Timer restart enables watchdog-style patterns where re-entering a state resets a deadline without requiring explicit stop-then-start sequences.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL support automatically restarting a specified timer with its original parameters when a state is entered or exited, based on configuration.

### Automatic Event Generation

**UID**: SWR-HSM-030
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Automatic event generation on state lifecycle enables transient states and cascading state machine patterns without requiring external event sources.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL support automatically generating a specified event when a state is entered or exited, based on configuration.

### Action Trigger Configuration

**UID**: SWR-HSM-031
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Inspection
**Rationale**: Configurable triggers provide flexibility to associate automatic actions with either the entry or exit phase of a state lifecycle.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL allow automatic state-associated actions to be configured to occur either when entering or when leaving a state.

### Action Conditioned on Successful Transition

**UID**: SWR-HSM-032
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Automatic actions represent committed side effects that should only occur when the state machine actually reaches or leaves the configured state; executing them on cancelled transitions would produce inconsistent behavior.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL execute automatic state-associated actions only when the corresponding state transition completes successfully. The library SHALL NOT execute automatic actions when a transition is cancelled by entry or exit behavior.

### Behavior Binding

**UID**: SWR-HSM-033
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Supporting multiple binding styles accommodates different programming paradigms and enables flexible integration with application architectures.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL support associating application-defined behavior with state lifecycle and transition operations using multiple binding styles.

## Event Processing

### Asynchronous Event Processing

**UID**: SWR-HSM-034
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Asynchronous processing decouples event producers from state machine execution, enabling responsive systems where callers are not blocked by event handling.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-005

The library SHALL support asynchronous event processing where the caller submitting an event returns immediately without executing the state machine processing. The library SHALL process the submitted event independently of the submitting caller.

### Event Processing Order (FIFO)

**UID**: SWR-HSM-035
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: FIFO ordering preserves causal relationships between events and ensures deterministic processing when multiple events are queued.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-005

The library SHALL process asynchronously submitted events in first-in-first-out order. The library SHALL maintain FIFO ordering unless pending events are explicitly discarded by the application.

### Re-Entrancy Prevention

**UID**: SWR-HSM-036
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Nested event processing during an active transition can violate sequential event-processing guarantees and produce inconsistent state machine configurations.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-005

The library SHALL prevent nested event processing during an active event-processing cycle. When application-defined behavior triggered by an event submits additional events, the library SHALL defer those events for later processing rather than executing them immediately within the current cycle.

### Internal Work Priority

**UID**: SWR-HSM-037
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Prioritizing internally generated work ensures that state machine housekeeping (such as automatic event generation or completion events) finishes before externally submitted events alter the configuration.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-005

The library SHALL process internally generated state machine work with higher priority than externally submitted events. The library SHALL execute pending internal work immediately after the current event processing completes, before processing the next externally submitted event.

## Concurrency and Thread Safety

### Thread-Safe Event Submission

**UID**: SWR-HSM-038
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Multi-threaded applications frequently produce events from worker threads or network callbacks that must be safely delivered to the state machine without data corruption or undefined behavior.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL guarantee that event submission operates correctly when performed concurrently from multiple threads. Concurrent event submissions SHALL NOT cause data corruption, lost events, or undefined behavior.

### Thread-Safe Timer Operations

**UID**: SWR-HSM-039
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Timers may be started or stopped from different threads than the one running the event-processing loop; safe concurrent access prevents race conditions and resource corruption.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL guarantee that timer control operations (start, stop, restart, and status query) operate correctly when invoked concurrently from multiple threads. Concurrent timer operations SHALL NOT cause data corruption or undefined behavior.

### Thread-Safe Transition Operations

**UID**: SWR-HSM-040
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: The transition interface is the primary entry point for external threads to interact with the state machine and must remain safe for concurrent use to prevent state corruption.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL guarantee that externally accessible transition operations operate correctly when invoked concurrently from multiple threads. Concurrent transition requests SHALL NOT cause state corruption or undefined behavior.

### ISR-Safe Transitions

**UID**: SWR-HSM-041
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Certain concurrency contexts (interrupt service routines, signal handlers) cannot safely use dynamic memory allocation; a pre-allocated mechanism enables safe state machine interaction from such contexts.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL provide a transition mechanism suitable for execution from concurrency contexts in which dynamic memory allocation is prohibited. The mechanism SHALL allow interrupt service routines and signal handlers to request transitions without allocating memory at invocation time.

### Configurable Thread Safety

**UID**: SWR-HSM-042
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Single-threaded or bare-metal environments benefit from eliminating synchronization overhead when thread safety is not required; configurability allows the library to serve both use cases.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL support enabling or disabling thread-safety mechanisms according to deployment requirements. When thread safety is disabled, the library SHALL omit synchronization overhead from event submission, timer operations, and transition operations.

### Non-Thread-Safe Structural Configuration

**UID**: SWR-HSM-043
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Review
**Rationale**: Structural configuration occurs during setup before the state machine begins processing events; restricting configuration to a single thread simplifies initialization and avoids unnecessary synchronization overhead.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

Structural configuration of states, transitions, and actions SHALL be completed from a single thread before initialization. The library is not required to guarantee correct behavior when structural configuration is performed concurrently from multiple threads.

### CPU-Efficient Synchronous Wait

**UID**: SWR-HSM-044
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Analysis
**Rationale**: Efficient waiting preserves CPU resources, which is critical on battery-powered and real-time systems where busy-wait loops waste energy and reduce scheduling availability.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL implement synchronous transition waiting in a CPU-efficient manner. The wait mechanism SHALL NOT consume processor resources while the transition has not yet completed.

## Hierarchical and Parallel States

### Hierarchical State Relationships

**UID**: SWR-HSM-045
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Hierarchical nesting enables complex behavior to be decomposed into manageable layers with shared transitions at the parent level.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support hierarchical parent-child relationships between states, forming a nested state structure. A child state SHALL be active only when its parent state is active.

### Hierarchical Structure Validation

**UID**: SWR-HSM-046
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Structural validation catches design errors at setup time rather than producing unexpected behavior at runtime.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

When structure validation is enabled, the library SHALL detect and reject invalid hierarchical configurations. The library SHALL reject circular dependencies, assignment of a child state to multiple parents, and definition of a state as its own parent.

### Entry Points

**UID**: SWR-HSM-047
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Entry points enable directed activation into specific substates based on the triggering context.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support entry point substates that determine which child state becomes active when the parent state is entered. When an entry point is defined for a parent state, the library SHALL activate the designated child state upon entry to the parent.

### Conditional Entry Points

**UID**: SWR-HSM-048
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Conditional entry points enable dynamic routing into substates based on runtime conditions without requiring intermediate transition states.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support conditional entry points where the active child state is selected based on a runtime condition evaluated at the time of parent state entry. When the triggering event is relevant, the library SHALL make it available to the condition evaluation.

### Hierarchical Transition Resolution

**UID**: SWR-HSM-049
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Hierarchical event propagation is the defining feature of statecharts, allowing parent states to handle events not handled by their children.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

When no transition is defined for an event in the active substate, the library SHALL propagate the event to parent states. The library SHALL attempt to find a matching transition at each successive ancestor level until a match is found or the topmost state is reached.

### Parallel Region Support

**UID**: SWR-HSM-050
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Parallel states model independent concurrent concerns within a single state machine without requiring separate state machine instances.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support parallel (orthogonal) regions where multiple states are simultaneously active within a parent state. Each parallel region SHALL maintain its own active state independently of other regions.

### Independent Event Processing in Parallel Regions

**UID**: SWR-HSM-051
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Independent processing ensures that parallel regions behave as logically separate state machines sharing a parent context.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL deliver each event to all active parallel regions. Each region SHALL independently process or ignore the event based on its own transition definitions.

### Shallow History

**UID**: SWR-HSM-052
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Shallow history enables resume-where-you-left-off behavior at one level of nesting without restoring deeper substates.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support shallow history states. When re-entering a parent state via a shallow history state, the library SHALL restore the last active direct child state of that parent. The library SHALL record the active direct child state when the parent state is exited.

### Deep History

**UID**: SWR-HSM-053
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Deep history restores the full nested state configuration, enabling seamless resumption of complex multi-level workflows.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support deep history states. When re-entering a parent state via a deep history state, the library SHALL restore the entire substate hierarchy that was active when the parent was last exited. The library SHALL record the full substate hierarchy when the parent state is exited.

### History Default Target

**UID**: SWR-HSM-054
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: A default target provides deterministic behavior on first entry when no prior history exists, preventing undefined state configurations.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL allow a default target state to be specified for a history state. When a transition targets a history state and no history has been recorded, the library SHALL activate the default target state instead.

### History Restoration Behavior

**UID**: SWR-HSM-055
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Application-defined behavior on history restoration allows applications to perform setup that depends on which historical state is being restored.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL support optional application-defined behavior associated with restoration through a history state. The library SHALL execute the restoration behavior when a history state activates its recorded target.

## Timer Management

### Timer Definition and Event Association

**UID**: SWR-HSM-056
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Timers provide a time-based event source that enables timeout patterns, periodic polling, and deadline enforcement within the state machine without requiring external scheduling infrastructure.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-008

The library SHALL support uniquely identified timers, each associated with a specified event. When a timer expires, the library SHALL deliver the associated event to the state machine for processing.

### Single-Shot Timers

**UID**: SWR-HSM-057
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Single-shot timers enable one-time deadline and timeout patterns where an event fires exactly once after a specified delay, preventing unintended repeated firings.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-008

The library SHALL support single-shot timers that fire once after the specified interval elapses and then stop automatically without further intervention.

### Repeating Timers

**UID**: SWR-HSM-058
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Repeating timers enable periodic patterns such as heartbeats, polling, and watchdog refreshes where the same event must fire at regular intervals until explicitly cancelled.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-008

The library SHALL support repeating timers that fire periodically at the specified interval until explicitly stopped. Each firing SHALL deliver the associated event to the state machine.

### Timer Lifecycle Control

**UID**: SWR-HSM-059
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Full lifecycle control (start, stop, restart) allows applications to manage timer behavior dynamically in response to state changes, user actions, or external conditions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-008

The library SHALL support starting a timer with a specified interval and mode (single-shot or repeating), stopping a currently running timer, and restarting a timer with its original parameters. Stopping a timer that is not running SHALL have no adverse effect.

### Timer Running Status Query

**UID**: SWR-HSM-060
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Querying timer status enables conditional logic that depends on whether a timeout is pending, supporting patterns such as conditional timer restart and status reporting.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-008

The library SHALL provide the ability to determine whether a specific timer is currently running. The query SHALL return a definitive running or not-running indication for the specified timer.

## Tooling, Platforms, and Build

### SCXML Input Format

**UID**: SWR-HSM-061
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: SCXML is a W3C standard that enables interoperability with visual editors and other statechart tools, providing a vendor-neutral input format for state machine definitions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The code generation tool SHALL accept W3C SCXML format as input for defining state machine structure.

### C++ Code Generation

**UID**: SWR-HSM-062
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Generated code eliminates manual configuration errors and ensures the implementation matches the model exactly.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The code generation tool SHALL produce C++ source artifacts that implement the state machine defined by the input SCXML model. The generated artifacts SHALL include all state, event, and timer definitions and the complete structural configuration.

### PlantUML Diagram Generation

**UID**: SWR-HSM-063
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Generated diagrams provide always-up-to-date visual documentation of the state machine design without manual diagram maintenance.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The code generation tool SHALL produce PlantUML state diagram files from SCXML input.

### SCXML Include Support

**UID**: SWR-HSM-064
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Include support enables modular SCXML definitions where large state machines can be split across multiple files for maintainability.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The code generation tool SHALL support SCXML file composition via include elements and state source attributes, allowing a state machine definition to be assembled from multiple files.

### Automatic Build Integration

**UID**: SWR-HSM-065
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Build integration ensures generated code stays in sync with models without manual regeneration steps, reducing the risk of stale artifacts.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The project SHALL support automatic regeneration of generated artifacts when their source state-machine definitions change.

### Debug Logging

**UID**: SWR-HSM-066
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Structured logs enable post-mortem analysis of state machine behavior using dedicated tooling.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

When debug support is enabled, the library SHALL record state machine execution information including state transitions, events, and timer operations in a defined structured log format.

### Runtime Debug Enable/Disable

**UID**: SWR-HSM-067
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Runtime control allows selective debugging of specific scenarios without the overhead of always-on logging and without requiring a rebuild or redeployment.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support enabling and disabling debug logging at runtime without rebuilding or redeploying.

### Configurable Log Destination

**UID**: SWR-HSM-068
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Configurable destinations enable logging to appropriate locations in different deployment environments (files, consoles, network endpoints).
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support configuring the debug log destination.

### Visual Debugger Compatibility

**UID**: SWR-HSM-069
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Visual replay of state machine execution significantly reduces debugging time compared to reading raw logs.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The debug log format SHALL be compatible with the visual analysis tool for replaying and inspecting state machine execution.

### Human-Readable Names in Debug Output

**UID**: SWR-HSM-070
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Inspection
**Rationale**: Readable names in debug output make logs immediately interpretable without cross-referencing identifier definitions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support resolving state and event identifiers to human-readable names in debug output.

### Linux Platform Support

**UID**: SWR-HSM-071
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Linux is the primary development and deployment platform for most use cases of the library.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support execution on Linux, including full multi-threading and concurrency-safe transitions.

### QNX Platform Support

**UID**: SWR-HSM-072
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: QNX is a common RTOS in automotive and industrial systems where hierarchical state machines are heavily used.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support execution on QNX, including multi-threading support.

### Windows Platform Support

**UID**: SWR-HSM-073
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Windows support enables development and testing on developer workstations and Windows-based embedded systems.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support execution on Windows, including multi-threading support.

### FreeRTOS Platform Support

**UID**: SWR-HSM-074
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: FreeRTOS is the most widely deployed RTOS in embedded systems and requires native task-based integration.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support execution on FreeRTOS (V10.3.1 and later), including multi-threading and concurrency-safe transitions.

### Arduino Platform Support

**UID**: SWR-HSM-075
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Arduino environments are single-threaded cooperative systems that rely on polling loops rather than preemptive scheduling.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support execution on Arduino. The Arduino configuration SHALL support concurrency-safe transitions but SHALL NOT require multi-threading.

### Custom Event-Processing Integration

**UID**: SWR-HSM-076
**Nature**: Functional
**Criticality**: NonSafety
**Security**: No
**Verification**: Test
**Rationale**: Custom integration enables use with proprietary event loops, RTOSes, or frameworks not covered by built-in platform support.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support integration with application-defined event-processing mechanisms.

### Optional Capabilities

**UID**: SWR-HSM-077
**Nature**: NonFunctional
**Criticality**: NonSafety
**Security**: No
**Verification**: Inspection
**Rationale**: Selective capability inclusion minimizes binary size and dependencies on resource-constrained platforms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL support independently enabling or disabling the following capabilities according to deployment requirements: debug logging, structure validation, thread safety, and platform-specific integrations.
