# hsmcpp Software Requirements

**Grammar**: software.gra.md
**Prefix**: SWR-HSM-

## Product Scope and Constraints

### C++ Standard Compatibility

**UID**: SWR-HSM-001
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: C++11 is the minimum standard widely available on embedded toolchains; forward compatibility ensures adoption on newer platforms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-001

The library SHALL compile and function correctly with C++11 and supported later C++ standard revisions.

### Type-Safe State Identification

**UID**: SWR-HSM-002
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Type-safe identifiers prevent accidental misuse of raw numeric values and enable compile-time checking.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL provide a type-safe state identifier type that the compiler can distinguish from unrelated integer or enumeration types. The library SHALL allow users to define domain-specific state identifier sets.

### Type-Safe Event Identification

**UID**: SWR-HSM-003
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL provide a type-safe event identifier type that the compiler can distinguish from unrelated integer or enumeration types. The library SHALL allow users to define domain-specific event identifier sets.

### Type-Safe Timer Identification

**UID**: SWR-HSM-004
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-002

The library SHALL provide a type-safe timer identifier type that the compiler can distinguish from unrelated integer or enumeration types. The library SHALL allow users to define domain-specific timer identifier sets.

### Built-In Event Processing

**UID**: SWR-HSM-005
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Decoupling the state machine from any particular execution environment allows the same logic to run on different platforms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL provide at least one event-processing implementation sufficient to execute a state machine without requiring integration with an external event-processing framework.

### Application-Provided Event-Processing Integration

**UID**: SWR-HSM-006
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Custom integration enables use with proprietary event loops, RTOSes, or frameworks not covered by built-in support.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL provide a software interface allowing an application-provided event-processing implementation. Adding a custom event-processing implementation SHALL be possible without modifying library source code.

## State-Machine Execution

### Initialization

**UID**: SWR-HSM-007
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Explicit initialization separates structure configuration from runtime execution.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-005

The library SHALL provide an explicit initialization operation that transitions a configured state machine into an executing state. After successful initialization, the state machine SHALL be in its designated initial state and ready to process events. Event processing SHALL begin only after initialization completes successfully.

### Structure Immutability After Initialization

**UID**: SWR-HSM-008
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Immutable structure after initialization enables safe concurrent event processing without locks on topology data.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-006

The library SHALL require that the complete state machine structure (states, transitions, and actions) is fully configured before initialization. The library SHALL reject structural modifications attempted after initialization with a defined error indication.

### Callback Serialization Per Instance

**UID**: SWR-HSM-009
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Serialized callback execution provides a predictable execution model for each state machine instance.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-007

The library SHALL serialize execution of application-defined callbacks for a given state machine instance, ensuring at most one callback executes at any time.

### Multiple Instances Per Event-Processing Context

**UID**: SWR-HSM-010
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Sharing an event-processing context reduces resource consumption on constrained platforms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-008

The library SHALL support multiple state machine instances sharing a single event-processing context. Each state machine instance SHALL process events independently while sharing the underlying event-processing infrastructure.

### Release and Re-initialization

**UID**: SWR-HSM-011
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Release allows applications to reset a state machine without destroying the instance.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-009

The library SHALL provide a release operation that terminates execution of a state machine and returns it to a state in which re-initialization is possible. After release, event processing SHALL resume only after successful re-initialization.

## State Modeling

### State Registration

**UID**: SWR-HSM-012
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: States are the fundamental building blocks of a hierarchical state machine.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-010

The library SHALL support registration of uniquely identified states. Each state SHALL be identifiable by its type-safe state identifier.

### Initial State Designation

**UID**: SWR-HSM-013
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Every well-formed state machine requires a defined starting state.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-011

The library SHALL allow the user to designate one state as the initial state during configuration. The designated initial state SHALL become active upon successful initialization.

### Final State Support

**UID**: SWR-HSM-014
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Final states signal state machine completion and terminate event processing.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-012

The library SHALL support designating a state as a final state. When a final state becomes active, the library SHALL stop processing further events and transition the state machine to a terminated lifecycle state.

### Active State Query

**UID**: SWR-HSM-015
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Applications need to inspect the current state machine configuration for conditional logic.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-013

The library SHALL provide the ability to query which states are currently active. The library SHALL provide the ability to query whether a specified state identifier is currently active.

## Transitions

### Event-Triggered Transition Registration

**UID**: SWR-HSM-016
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Event-driven transitions are the core mechanism for state machine progression.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-014

The library SHALL support registration of transitions between states triggered by type-safe event identifiers. Each transition SHALL specify a source state, a target state, and a triggering event.

### Guard Condition Registration

**UID**: SWR-HSM-017
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Guards allow dynamic routing of transitions based on runtime conditions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-015

The library SHALL support associating a guard condition callback with a transition. The guard callback SHALL return a boolean result. The transition SHALL execute only when the guard result matches the configured expected value.

### Otherwise Transitions

**UID**: SWR-HSM-018
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Otherwise transitions provide deterministic fallback routing when no guarded transition is satisfied.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-015

The library SHALL support registration of otherwise transitions that execute when no guarded transition for the same event from the same source state is satisfied. Otherwise transitions SHALL be evaluated after all guarded transitions have been evaluated and rejected.

### Internal Self-Transitions

**UID**: SWR-HSM-019
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Internal self-transitions allow in-state processing without triggering exit/entry side effects.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-016

The library SHALL support internal self-transitions that do not trigger state exit or entry callbacks.

### External Self-Transitions

**UID**: SWR-HSM-020
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: External self-transitions reset state context by performing full exit and re-entry.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-016

The library SHALL support external self-transitions that perform full exit and re-entry of the state, including execution of exit and entry callbacks.

### Deterministic Transition Priority

**UID**: SWR-HSM-021
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Deterministic priority resolution ensures predictable behavior with multiple guarded transitions. Registration order is the intentional API-level priority mechanism.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-017

When multiple transitions from the same state match the same event, the library SHALL evaluate guard conditions in registration order and SHALL execute the first transition whose guard condition is satisfied. Registration order SHALL be the defined priority mechanism.

### Synchronous Transition with Timeout

**UID**: SWR-HSM-022
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Synchronous transitions simplify sequencing logic where the caller must know the outcome.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-018

The library SHALL provide a synchronous transition operation that blocks the caller until the transition completes or fails. If the transition does not complete within a configurable timeout period, the operation SHALL return a timeout indication. A timeout indication SHALL indicate that the transition outcome is not available to the caller at the time the operation returns.

### Pending Event Discard on Transition

**UID**: SWR-HSM-023
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Discarding pending events enables cancel-all-pending-work semantics for high-priority state changes.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-019

The library SHALL provide the ability to discard all pending events (both externally submitted and internally generated) before processing a newly requested transition.

### Transition Possibility Query

**UID**: SWR-HSM-024
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Dry-run checks allow application logic to determine reachability without side effects.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-020

The library SHALL provide an operation to determine whether a specified event can cause a successful transition from the current active state configuration without changing state machine state. The operation SHALL evaluate guard conditions only and SHALL be side-effect-free with respect to callbacks, active state configuration, and conditional entry points.

### Failed Transition Notification

**UID**: SWR-HSM-025
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Failed transition notifications enable logging, error handling, and defensive programming.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-021

The library SHALL invoke an application-defined callback when a transition request does not result in a successful state change. The callback SHALL distinguish between transition rejection (no transition matched the event from the current state after guard evaluation) and transition cancellation (a matching transition was selected but cancelled by application-defined callback behavior).

### Transition Data Passing

**UID**: SWR-HSM-026
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Passing context data through transitions eliminates the need for external shared state.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-022

The library SHALL support associating application-defined data with a transition request. The associated data SHALL remain available to all callbacks executed as part of that transition and SHALL retain the value provided at submission time.

### State Configuration Consistency

**UID**: SWR-HSM-027
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Applications must never observe an inconsistent state configuration.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-023

When a transition is cancelled by application-defined behavior, the library SHALL restore the active state configuration to the configuration that existed before the transition attempt. The library does NOT undo externally visible side effects performed by callbacks that executed before the cancellation.

### Event Queue Capacity Handling

**UID**: SWR-HSM-028
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Resource-constrained applications require predictable feedback when the queue is full.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-030

The library SHALL return an error indication when an asynchronous event submission cannot be accepted because the event queue has reached its capacity limit.

## Transition Execution Model

### Transition Callback Sequence

**UID**: SWR-HSM-029
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A defined callback execution sequence is essential for deterministic behavior, testability, and safety traceability.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

For a non-hierarchical transition from source state S to target state T, the library SHALL execute applicable callbacks in the following order: (1) source state exit callback, (2) transition callback, (3) target state entry callback, (4) target state state-changed callback. Individual callback definitions and cancellation behavior are specified in SWR-HSM-030 through SWR-HSM-035. Guard evaluation SHALL precede callback execution.

### Exit Callback Cancellation

**UID**: SWR-HSM-030
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Exit cancellation enables states to refuse departure if preconditions are not met.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-024

The library SHALL allow a state exit callback to cancel the in-progress transition by returning a cancellation indication. When an exit callback cancels, no further callbacks in the transition sequence SHALL execute, the source state SHALL remain active, and no compensating callbacks SHALL be invoked for the cancellation.

### Entry Callback Cancellation

**UID**: SWR-HSM-031
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Entry cancellation enables validation logic that rejects transitions at the target state.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-024

The library SHALL allow a state entry callback to cancel the in-progress transition by returning a cancellation indication. When an entry callback cancels, the library SHALL stop the callback sequence and restore the source state as the active state. No compensating entry or exit callbacks SHALL be invoked during restoration. Side effects of callbacks already executed before the cancellation are not undone.

### State Entry Callback

**UID**: SWR-HSM-032
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Entry callbacks enable initialization logic tied to becoming active.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

The library SHALL execute an application-defined entry callback when a state is entered as part of a transition. The entry callback SHALL receive any data associated with the triggering transition.

### State Exit Callback

**UID**: SWR-HSM-033
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Exit callbacks enable cleanup logic and resource release.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

The library SHALL execute an application-defined exit callback when a state is exited as part of a transition.

### State-Changed Callback

**UID**: SWR-HSM-034
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Post-activation callback provides a safe point for operations that depend on the state being fully entered.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

The library SHALL execute an application-defined state-changed callback after a state has become fully active. The state-changed callback SHALL receive any data associated with the triggering transition.

### Transition Callback

**UID**: SWR-HSM-035
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Transition callbacks enable side effects associated with the act of transitioning.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

The library SHALL support an application-defined callback associated with a transition. The transition callback SHALL receive any data associated with the triggering transition. The transition callback is non-cancellable.

### Multiple Behavior Binding Styles

**UID**: SWR-HSM-036
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Review
**Rationale**: Supporting multiple binding styles accommodates different programming paradigms. These are intentional API guarantees.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

The library SHALL support associating application-defined behavior with state lifecycle and transition operations using at least: class member function pointers, free function pointers, and lambda/functor objects.

## Declarative State-Associated Actions

### Automatic Timer Start on State Lifecycle

**UID**: SWR-HSM-037
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Declarative timer start eliminates boilerplate for common timeout patterns.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support configuration to automatically start a specified timer when a state is entered or exited.

### Automatic Timer Stop on State Lifecycle

**UID**: SWR-HSM-038
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Automatic timer cancellation prevents stale timeouts from firing after state transitions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support configuration to automatically stop a specified timer when a state is entered or exited.

### Automatic Timer Restart on State Lifecycle

**UID**: SWR-HSM-039
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Timer restart enables watchdog-style patterns where re-entering a state resets a deadline.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support configuration to automatically restart a specified timer with its original parameters when a state is entered or exited.

### Automatic Event Generation on State Lifecycle

**UID**: SWR-HSM-040
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Automatic event generation enables transient states and cascading patterns.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support configuration to automatically generate a specified event when a state is entered or exited.

### Declarative Action Execution Condition

**UID**: SWR-HSM-041
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Declarative actions are committed side effects that should only occur on successful transitions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL execute a declarative state-associated action only after its associated state lifecycle operation has completed successfully. Declarative actions SHALL be skipped for cancelled state lifecycle operations.

## Event Processing

### Asynchronous Event Submission

**UID**: SWR-HSM-042
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Asynchronous processing decouples event producers from state machine execution.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-027

The library SHALL provide an asynchronous event submission operation where the caller returns immediately without waiting for the event to be processed. The submitted event SHALL be queued for later processing by the event-processing context.

### FIFO Event Processing Order

**UID**: SWR-HSM-043
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: FIFO ordering preserves causal relationships between events.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-028

The library SHALL process externally submitted events in first-in-first-out order within the external event queue. The library SHALL maintain FIFO ordering unless pending events are explicitly discarded by the application.

### Internal Event Priority

**UID**: SWR-HSM-044
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Prioritizing internally generated work ensures state machine housekeeping (completion events, automatic actions) finishes before new external events alter the configuration. This is an intentional software-level semantic.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-028

The library SHALL process internally generated events before the next externally submitted event. Internally generated events include events produced by declarative actions, timer expirations processed during a transition, and events generated as a result of entering a final state. All pending internally generated events, including those generated recursively, SHALL be processed before the next externally submitted event. Internally generated events SHALL be processed in generation order.

### Deferred Event Processing

**UID**: SWR-HSM-045
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Preventing nested processing during an active transition avoids inconsistent state configurations.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-029

When application-defined behavior triggered by an event submits additional events, the library SHALL defer those events for later processing rather than executing them immediately within the current processing cycle.

### Invalid Operation Error Conditions

**UID**: SWR-HSM-046
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Defined error conditions prevent undefined behavior from invalid API usage and provide a testable contract.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-031

The library SHALL report an error for the following conditions: transition requests on an uninitialized state machine, structural modifications after initialization, operations with invalid identifier parameters, and event submission when the event queue is full. Additional implementation-specific error conditions MAY be reported.

### Error Reporting Mechanism

**UID**: SWR-HSM-047
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Review
**Rationale**: A consistent error-reporting mechanism enables uniform error handling across all library operations independently of specific error conditions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-031

The library SHALL provide a uniform mechanism for reporting operation failures to the caller. All operations capable of failing SHALL use this common mechanism to communicate failure to the caller.

## Concurrency and Thread Safety

### Thread-Safe Timer Operations

**UID**: SWR-HSM-049
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Timers may be started or stopped from different threads than the event-processing loop.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-033

The library SHALL guarantee that timer control operations (start, stop, restart, and status query) preserve timer state consistency when invoked concurrently from multiple threads. Each operation SHALL produce a valid timer state upon completion.

### Thread-Safe Transition Operations

**UID**: SWR-HSM-050
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: The transition and event submission interfaces are the primary entry points for external execution contexts.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-032

The library SHALL guarantee that externally submitted transition requests and event submissions operate correctly when invoked concurrently from multiple execution contexts. The library SHALL preserve data integrity and state configuration consistency throughout concurrent access.

### Restricted-Context Transition Mechanism

**UID**: SWR-HSM-051
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Restricted execution contexts (interrupt service routines, signal handlers) cannot safely use dynamic memory allocation.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-035

The library SHALL provide a transition mechanism suitable for execution from restricted contexts in which dynamic memory allocation is not permitted. The mechanism SHALL use only pre-allocated resources at invocation time.

### Compile-Time Thread Safety Configuration

**UID**: SWR-HSM-052
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Single-threaded environments benefit from eliminating synchronization overhead.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-036

The library SHALL support a compile-time configuration option to enable or disable thread-safety mechanisms. When thread safety is disabled, the library SHALL omit synchronization operations from event submission, timer operations, and transition operations. When disabled, the application is responsible for ensuring single-threaded access.

### Single-Threaded Structural Configuration

**UID**: SWR-HSM-053
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Design
**Rationale**: Structural configuration occurs during setup before event processing begins.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-036

Structural configuration operations (registration of states, transitions, and actions) SHALL be performed from a single execution context. This constraint applies only during the configuration phase preceding initialization and does not restrict runtime operation.

## Hierarchical States

### Parent-Child State Registration

**UID**: SWR-HSM-054
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Hierarchical nesting enables complex behavior to be decomposed into manageable layers.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-037

The library SHALL support registration of hierarchical parent-child relationships between states. A child state SHALL be active only when its parent state is active.

### Hierarchical Structure Validation

**UID**: SWR-HSM-055
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Structural validation catches design errors at configuration time.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-038

The library SHALL, when structure validation is enabled, detect and reject invalid hierarchical configurations. Minimum validated conditions SHALL include: circular parent-child dependencies, assignment of a child state to multiple parents, definition of a state as its own parent, and composite states without a designated initial substate.

### Entry Point Registration

**UID**: SWR-HSM-056
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Entry points enable directed activation into specific substates.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-039

The library SHALL support registration of entry point substates. When an entry point is defined for a parent state, the library SHALL activate the designated child state upon entry to the parent.

### Conditional Entry Point Registration

**UID**: SWR-HSM-057
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Conditional entry points enable dynamic routing into substates based on runtime conditions.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-040

The library SHALL support conditional entry points where the active child state is selected based on a runtime condition callback evaluated at the time of parent state entry. The condition callback SHALL receive the triggering event identifier as input.

### Hierarchical Event Propagation

**UID**: SWR-HSM-058
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Hierarchical event propagation is the defining feature of statecharts.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-041

When no applicable transition for an event can be executed in the active substate, the library SHALL evaluate applicable transitions in successive ancestor states until a transition can be executed or the topmost state is reached.

### Hierarchical Callback Order

**UID**: SWR-HSM-059
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Predictable lifecycle ordering is essential for initialization/cleanup logic in nested states.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-042

When entering a hierarchical state configuration, the library SHALL execute entry callbacks in top-down order (ancestor before descendant). When exiting, the library SHALL execute exit callbacks in bottom-up order (descendant before ancestor).

## Parallel States

### Parallel Region Registration

**UID**: SWR-HSM-060
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Parallel states model independent concurrent concerns within a single state machine.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-043

The library SHALL support registration of parallel (orthogonal) regions within a parent state. Each parallel region SHALL maintain its own active state independently of other regions. All regions SHALL become active when the parent state is entered.

### Parallel Region Event Delivery

**UID**: SWR-HSM-061
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Independent processing ensures parallel regions behave as logically separate state machines.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-044

The library SHALL deliver each event to all active parallel regions in a deterministic order (registration order). Each region SHALL evaluate the event according to its own transition definitions independently of other regions. A transition executed by one region SHALL not prevent other regions from evaluating the same event.

## History States

### Shallow History Registration and Restoration

**UID**: SWR-HSM-062
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Shallow history enables resume-where-you-left-off behavior at one level of nesting.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-045

The library SHALL support registration of shallow history states. When re-entering a parent state via a shallow history state, the library SHALL restore the last active direct child state of that parent.

### Shallow History Recording

**UID**: SWR-HSM-093
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Accurate history requires recording the active child state at the moment the parent is exited.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-045

The library SHALL record the active direct child state when the parent state is exited. The recorded state SHALL be used for subsequent shallow history restoration.

### Shallow History Clearing

**UID**: SWR-HSM-094
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Release and re-initialization represent lifecycle boundaries where accumulated history must be discarded.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-045

The library SHALL clear all recorded shallow history upon state machine release or re-initialization.

### Deep History Registration and Restoration

**UID**: SWR-HSM-063
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Deep history restores the full nested state configuration for seamless resumption.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-046

The library SHALL support registration of deep history states. When re-entering a parent state via a deep history state, the library SHALL restore the entire substate hierarchy that was active when the parent was last exited.

### Deep History Recording

**UID**: SWR-HSM-095
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Accurate deep history requires recording the complete active substate hierarchy at the moment the parent is exited.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-046

The library SHALL record the complete active substate hierarchy when the parent state is exited. The recorded hierarchy SHALL be used for subsequent deep history restoration.

### Deep History Clearing

**UID**: SWR-HSM-096
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Release and re-initialization represent lifecycle boundaries where accumulated history must be discarded.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-046

The library SHALL clear all recorded deep history upon state machine release or re-initialization.

### History Default Target

**UID**: SWR-HSM-064
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A default target provides deterministic behavior on first entry when no prior history exists.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-047

The library SHALL allow a default target state to be specified for a history state. When a transition targets a history state and no history has been recorded, the library SHALL activate the default target state instead.

### History Restoration Callback

**UID**: SWR-HSM-065
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Application-defined behavior on history restoration allows setup that depends on which historical state is restored. The restoration callback executes in addition to the normal entry callback.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-048

The library SHALL support an optional application-defined callback associated with restoration through a history state. The restoration callback SHALL execute when a history state activates its recorded target. The restoration callback SHALL execute before the normal entry callback for the restored state.

## Timer Management

### Timer Registration and Event Association

**UID**: SWR-HSM-066
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Timers provide a time-based event source enabling timeout patterns within the state machine.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-049

The library SHALL support registration of timers identified by type-safe timer identifiers. Each timer SHALL be associated with a specified event identifier. When a timer expires, the library SHALL deliver the associated event to the state machine for processing.

### Single-Shot Timer Mode

**UID**: SWR-HSM-067
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Single-shot timers enable one-time deadline and timeout patterns.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-050

The library SHALL support single-shot timers that fire once after the specified interval elapses and then stop automatically without further intervention.

### Repeating Timer Mode

**UID**: SWR-HSM-068
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Repeating timers enable periodic patterns such as heartbeats, polling, and watchdog refreshes.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-051

The library SHALL support repeating timers that fire periodically at the specified interval until explicitly stopped. Each firing SHALL deliver the associated event to the state machine.

### Timer Start Operation

**UID**: SWR-HSM-069
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Starting a timer initiates the countdown for event delivery.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-052

The library SHALL provide an operation to start a timer with a specified interval and mode (single-shot or repeating).

### Timer Stop Operation

**UID**: SWR-HSM-070
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Stopping a timer cancels pending event delivery.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-052

The library SHALL provide an operation to stop a currently running timer. Stopping a timer that is not running SHALL have no adverse effect. After a timer stop operation completes, the timer SHALL generate no further expiration events. Any expiration event already queued but not yet processed at the time of stop SHALL be discarded.

### Timer Restart Operation

**UID**: SWR-HSM-071
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Restart enables watchdog-reset patterns without separate stop/start sequences.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-052

The library SHALL provide an operation to restart a timer with its original parameters, resetting the countdown from zero. Restart SHALL be equivalent to stop followed by start: any previously queued expiration event SHALL be discarded before the new countdown begins.

### Timer Running Status Query

**UID**: SWR-HSM-072
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Querying timer status enables conditional logic that depends on whether a timeout is pending.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-053

The library SHALL provide an operation to determine whether a specific timer is currently running. The query SHALL return a definitive running or not-running indication.

## Code Generation

### SCXML Input Parsing

**UID**: SWR-HSM-073
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: SCXML is a W3C standard enabling interoperability with visual editors and other statechart tools.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-054

The code generation tool SHALL parse W3C SCXML format files as input for defining state machine structure. The tool SHALL support the hsmcpp-relevant subset of SCXML elements including states, transitions, parallel regions, and history states.

### SCXML Composition Support

**UID**: SWR-HSM-074
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Composition support enables modular SCXML definitions where large state machines can be split across files.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-058

The code generation tool SHALL support assembling a state machine definition from multiple SCXML source files.

### Generated State-Machine Content

**UID**: SWR-HSM-075
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Review
**Rationale**: Generated code eliminates manual configuration errors and ensures the implementation matches the model.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-055

The code generation tool SHALL produce C++ source and header files that implement the state machine defined by the input SCXML model. The generated artifacts SHALL include all state, event, and timer identifier definitions and the complete structural configuration (state registration, transition registration, timer bindings).

### Application Extension Mechanism

**UID**: SWR-HSM-097
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Review
**Rationale**: The base class pattern is the public API contract enabling regeneration without overwriting application logic.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-055

The generated code SHALL use a base class pattern where the generated class provides structure registration and declares virtual callback methods. Applications SHALL derive from the generated class to implement callback behavior. Regeneration of the base class SHALL not require modification of application-derived code.

### SCXML Model Validation

**UID**: SWR-HSM-076
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Invalid models must be detected before code generation to prevent generating incorrect state machines.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-056

The code generation tool SHALL validate the input SCXML model before generating code. The tool SHALL report errors for structural and semantic violations that would prevent valid state machine execution. Minimum validated conditions SHALL include: undefined state references in transitions, circular hierarchies, and missing initial states in composite states.

### PlantUML Diagram Generation

**UID**: SWR-HSM-077
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Generated diagrams provide always-up-to-date visual documentation.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-057

The code generation tool SHALL produce PlantUML state diagram files from SCXML input. The generated diagrams SHALL represent states, transitions, guards, hierarchical relationships, and parallel regions.

### Automatic Regeneration on Model Change

**UID**: SWR-HSM-078
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Automatic regeneration ensures generated code stays in sync with models without manual intervention.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-059

The project SHALL provide build-system integration that automatically invokes the code generation tool when source SCXML files change, ensuring generated artifacts are regenerated during the build process.

## Debugging and Observability

### Structured Debug Log Format

**UID**: SWR-HSM-079
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A structured format enables post-mortem analysis and tool-based replay.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

When debug support is enabled, the library SHALL record state machine execution information in a defined structured log format. The log SHALL capture: state transitions (source, target, event), event submissions, and timer operations (start, stop, expire). Log entries SHALL include sufficient ordering information to reconstruct the execution sequence.

### Compile-Time Debug Exclusion

**UID**: SWR-HSM-080
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Embedded deployments may need to completely eliminate debug code to minimize binary size.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The library SHALL support a compile-time option that excludes debug logging implementation from the built binary when disabled.

### Runtime Debug Enable/Disable

**UID**: SWR-HSM-081
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Runtime control allows selective debugging without rebuilding.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-061

The library SHALL provide an API to enable and disable debug logging at runtime without rebuilding. Changes SHALL take effect for the next logged event.

### Configurable Debug Log Destination

**UID**: SWR-HSM-082
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Different deployment environments require different log destinations.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The library SHALL support configuring the debug log output destination. The library SHALL allow applications to provide a custom log output implementation.

### Diagnostic Tool Compatibility

**UID**: SWR-HSM-083
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Visual replay significantly reduces debugging time compared to reading raw logs.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-063

The debug log format SHALL be documented and parseable, enabling compatible diagnostic tools to replay and inspect state machine execution sequences.

### Human-Readable Identifier Names

**UID**: SWR-HSM-084
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Readable names make debug output immediately interpretable.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The library SHALL support registering human-readable string names for state and event identifiers. When debug logging is active, the library SHALL include registered names in log output.

## Platform Support

### Linux Platform Support

**UID**: SWR-HSM-085
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Linux is the primary development and deployment platform.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-065

The library SHALL compile and execute correctly on Linux.

### Linux STD Event-Processing Dispatcher

**UID**: SWR-HSM-098
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A C++ standard library-based dispatcher provides out-of-box event processing on Linux without external dependencies.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL provide an event-processing dispatcher for Linux based on the C++ standard library (std::thread, std::condition_variable).

### QNX Platform Support

**UID**: SWR-HSM-086
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: QNX is a common RTOS in automotive and industrial systems.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-066

The library SHALL compile and execute correctly on QNX.

### QNX STD Event-Processing Dispatcher

**UID**: SWR-HSM-099
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: QNX supports POSIX and C++ standard threading; a std-based dispatcher enables immediate use.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL provide an event-processing dispatcher for QNX based on the C++ standard library.

### Windows Platform Support

**UID**: SWR-HSM-087
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Windows support enables development and testing on developer workstations.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-067

The library SHALL compile and execute correctly on Windows.

### Windows STD Event-Processing Dispatcher

**UID**: SWR-HSM-100
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A C++ standard library-based dispatcher provides out-of-box event processing on Windows.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL provide an event-processing dispatcher for Windows based on the C++ standard library.

### FreeRTOS Platform Support

**UID**: SWR-HSM-088
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: FreeRTOS is widely deployed in embedded systems.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-068

The library SHALL compile and execute correctly on FreeRTOS (V10.3.1 and later).

### FreeRTOS Event-Processing Dispatcher

**UID**: SWR-HSM-101
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: FreeRTOS requires a dedicated dispatcher using native task and queue primitives.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL provide an event-processing dispatcher for FreeRTOS based on FreeRTOS tasks and queues.

### Arduino Platform Support

**UID**: SWR-HSM-089
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Arduino environments are single-threaded cooperative systems.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-069

The library SHALL compile and execute correctly on Arduino.

### Arduino Event-Processing Dispatcher

**UID**: SWR-HSM-102
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Arduino lacks threading; the dispatcher must operate cooperatively within the Arduino loop() function.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-003

The library SHALL provide an event-processing dispatcher for Arduino that operates cooperatively within a single-threaded execution model.

### Arduino Execution Constraints

**UID**: SWR-HSM-103
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Arduino's constrained environment requires restricted-context compatibility and single-threaded operation guarantees.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-036

The Arduino configuration SHALL support the restricted-context transition mechanism. The Arduino configuration SHALL operate without thread-safety overhead.

### GLib Event-Processing Integration

**UID**: SWR-HSM-090
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: GLib-based applications benefit from native integration with the GLib main loop.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL provide an event-processing implementation that integrates with the GLib main event loop.

### Qt Event-Processing Integration

**UID**: SWR-HSM-091
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Qt-based applications benefit from native integration with the Qt event system.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-004

The library SHALL provide an event-processing implementation that integrates with the Qt event loop.

### Optional Capability Configuration

**UID**: SWR-HSM-092
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Selective capability inclusion minimizes binary size and dependencies on resource-constrained platforms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-036

The library SHALL support independently enabling or disabling the following capabilities via build configuration: debug logging, structure validation, and thread safety.
