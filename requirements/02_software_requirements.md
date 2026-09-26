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

The library SHALL provide an explicit initialization operation that transitions a configured state machine into an executing state. After successful initialization, the state machine SHALL be in its designated initial state.

### Event Processing Precondition

**UID**: SWR-HSM-104
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Deferring event processing until initialization completes prevents events from being processed against an incompletely established structure.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-005

Event processing SHALL begin only after initialization completes successfully.

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

The library SHALL allow the user to designate one state as the initial state during configuration. The designated initial state SHALL become active upon successful initialization of the state machine.

### Final State Support

**UID**: SWR-HSM-014
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Final states signal state machine completion and terminate event processing.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-012

The library SHALL support designating one or multiple states as a final state. When a final state becomes active, the library SHALL stop processing further events and transition the state machine to a terminated lifecycle state.

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

The library SHALL, when structure validation is enabled, detect and reject invalid hierarchical configurations. Minimum validated conditions SHALL include: circular parent-child dependencies, assignment of a child state to multiple parents, definition of a state as its own parent, composite states without a designated initial substate, and transitions whose source and target states do not share a common parent scope.

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

The library SHALL support conditional entry points where the entry point is activated based on a runtime condition callback evaluated at the time of parent state entry. The condition callback SHALL take no event argument; it evaluates only application-defined state.

### Event-Filtered Entry Points

**UID**: SWR-HSM-108
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Selecting which substate to activate based on the event that entered the parent lets one composite state route entry differently per triggering event without intermediate states.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-039

The library SHALL support entry points filtered by event, where an entry point is eligible for activation only when the event that triggered the transition into the parent state matches the entry point's configured event.

### Combined Event and Condition Entry Points

**UID**: SWR-HSM-109
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Some entry decisions depend on both the triggering event and runtime state; supporting both filters on one entry point avoids duplicating states.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-039

The library SHALL support entry points that define both an event filter and a condition callback. Such an entry point SHALL be eligible for activation only when both the event matches and the condition callback returns its expected value.

### Unconditional Entry Points

**UID**: SWR-HSM-110
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: An entry point with neither an event filter nor a condition is the default activation path and must always apply.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-039

An entry point that defines neither an event filter nor a condition callback SHALL always be eligible for activation when its parent state is entered.

### Entry Point Resolution Rules

**UID**: SWR-HSM-111
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: When multiple entry points are defined, a deterministic resolution order is required so entry behavior is predictable and reproducible.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-039

When a parent state defines multiple entry points, the library SHALL resolve activation as follows: (1) evaluate entry points that define an event filter and/or a condition, and activate all whose filters are satisfied, without activating entry points that define neither; (2) if no filtered entry point is satisfied, activate all entry points that define neither an event filter nor a condition; (3) if no entry point defines an event filter or condition, activate all entry points. Within each case, eligible entry points SHALL be activated in registration order.

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

### Concurrent State Activation

**UID**: SWR-HSM-060
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: hsmcpp has no explicit parallel-region construct; concurrent activity emerges from executing every satisfied transition for an event and from activating multiple eligible entry points. This keeps a single transition model.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-043

The library SHALL allow multiple states to be simultaneously active as a result of executing all satisfied transitions for a single event (SWR-HSM-021) and of activating multiple eligible entry points (SWR-HSM-111). The library SHALL maintain the active state of each concurrently active branch independently.

### Event Delivery to Concurrently Active States

**UID**: SWR-HSM-061
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Each concurrently active branch must get the chance to react to an event on its own terms so branches behave as logically independent parts of one state machine.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-044

The library SHALL evaluate each event against all concurrently active states in a deterministic order (registration order). Each active state SHALL evaluate the event according to its own transition definitions independently of the others. A transition executed by one active state SHALL leave the other active states free to evaluate the same event.

### Idempotent State Entry on Convergence

**UID**: SWR-HSM-125
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Concurrent branches may converge on a common target (a diamond that re-joins), and a branch may re-target a state that is already active because a sibling branch keeps its shared source active (SWR-HSM-027 restoration). In both cases the target must be entered at most once so that repeated arrivals do not re-run entry logic. The transition still executes on its own edge so its guard, source-exit, and transition callbacks are observable; only the target's activation is suppressed.

**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-076

When a transition targets a state that is already part of the active state configuration, the library SHALL execute that transition's guard condition, source-state exit callback, and transition callback per SWR-HSM-029, but SHALL NOT execute the target state's entry callback or its state-changed callback. The target state SHALL remain active and its active substate configuration SHALL be left unchanged.

### Preservation of Active Substates on Re-Target

**UID**: SWR-HSM-126
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: When the already-active target is a composite state, suppressing only the target's own entry callback is not sufficient: re-drilling into its initial substate would discard the substate progress the branch made independently (for example, an inner state that has already advanced past its initial substate). The suppression must cover the target and its entire active substate subtree so that a re-target caused by an unrelated branch never rewinds internal progress. This restatement of SWR-HSM-125 for composite targets does not apply to an external self-transition (SWR-HSM-020), which deliberately restarts the target through full exit and re-entry.

**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-076

When a transition targets an already-active composite state without explicitly targeting one of its substates, the library SHALL leave the composite state and its entire active substate configuration active and unchanged, and SHALL NOT re-enter, re-initialize, or reset any state within that subtree. This requirement does not govern a transition that explicitly targets a substate, nor an external self-transition (SWR-HSM-020), which resets the target by performing a full exit and re-entry.

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
**Rationale**: Accurate history requires that the recorded child always reflect the most recently active direct child, because a parent's active child can change while the parent remains continuously active (for example, driven by events unrelated to the parent) and not only at the moment the parent is exited. Recording only on parent exit would miss substate progress made during the parent's active lifetime. The intended implementation updates the recording whenever the parent's active direct child changes.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-045

The recorded shallow history of a parent state SHALL always reflect the direct child state that was active immediately before the most recent deactivation of that child, independently of whether the parent itself was exited. The recorded child SHALL be used for subsequent shallow history restoration.

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
**Rationale**: Accurate deep history requires that the recorded hierarchy always reflect the most recently active substate configuration, because the substate configuration can change while the parent remains continuously active and not only at the moment the parent is exited. Recording only on parent exit would miss substate progress made during the parent's active lifetime. The intended implementation updates the recording whenever any substate within the parent's subtree is activated or deactivated.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-046

The recorded deep history of a parent state SHALL always reflect the complete active substate hierarchy that was active immediately before the most recent deactivation of that hierarchy, independently of whether the parent itself was exited. The recorded hierarchy SHALL be used for subsequent deep history restoration.

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

### History State Callback

**UID**: SWR-HSM-065
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Application-defined behavior on history activation allows setup that depends on entering through a history state. The callback executes in addition to the normal entry callback and applies whether history was recorded or the default target is used.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-048

The library SHALL support an optional application-defined callback associated with a history state. The callback SHALL execute whenever the history state is activated, regardless of whether it activates a recorded target or its default target. The callback SHALL execute before the normal entry callback for the activated state.

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

The library SHALL support associating application-defined data with a transition request. The associated data SHALL be delivered to every callback executed as part of that transition — including the guard condition, exit, transition, entry, and state-changed callbacks as defined in the transition callback sequence (SWR-HSM-029) — and SHALL retain the value provided at submission time.

### State Configuration Consistency

**UID**: SWR-HSM-027
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Applications must never observe an inconsistent state configuration.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-023

When a transition is cancelled by application-defined behavior after the source state has been exited, the library SHALL restore the source state as active by re-entering it, executing its entry callback and state-changed callback as if it had been entered by a normal transition. When cancellation occurs before the source state is exited, the source state SHALL remain active with no additional callbacks. When the source state is still active because a concurrent branch shares it, restoration SHALL follow the idempotent-entry rule of SWR-HSM-125 and SHALL NOT re-enter it. Externally visible side effects performed by callbacks that executed before the cancellation SHALL remain in effect (the library does not undo them).

### Event Queue Capacity Handling

**UID**: SWR-HSM-028
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Resource-constrained applications require predictable feedback when the queue is full.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-030

The library SHALL return an error indication when an asynchronous event submission cannot be accepted because the event queue has reached its capacity limit. The default event queue capacity SHALL be 10 pending events.

### Configurable Event Queue Capacity

**UID**: SWR-HSM-106
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Different applications and dispatchers have different memory budgets and throughput needs, so the pending-event capacity must be adjustable.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-030

The library SHALL allow the event queue capacity to be configured per dispatcher.

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

For a non-hierarchical transition from source state S to target state T, the library SHALL execute applicable steps in the following order: (1) evaluate the transition guard condition, (2) source state exit callback, (3) transition callback, (4) target state entry callback, (5) target state state-changed callback. Guard evaluation SHALL complete before the source state exit callback executes; when the guard is not satisfied, no exit, transition, entry, or state-changed callback SHALL execute for that transition. Individual callback definitions and cancellation behavior are specified in SWR-HSM-030 through SWR-HSM-035.

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

The library SHALL allow a state entry callback to cancel the in-progress transition by returning a cancellation indication. When an entry callback cancels, the library SHALL stop the remaining callback sequence and restore the source state as active by re-entering it, per the restoration semantics in SWR-HSM-027. Side effects of callbacks already executed before the cancellation are not undone.

### Independent Cancellation Among Concurrent Transitions

**UID**: SWR-HSM-105
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: When a single event triggers multiple satisfied transitions (concurrent activation), cancelling one branch must not affect the others, so each branch resolves independently.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-024

When a single event triggers execution of multiple transitions, cancellation of one transition by application-defined callback behavior SHALL affect only that transition. The remaining transitions SHALL continue to be executed independently.

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

### Application-Defined Actions

**UID**: SWR-HSM-036
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Review
**Rationale**: A single unified "action" concept keeps the model coherent: callbacks, timer control, and event generation are all action types that can be attached to the same lifecycle and transition points, rather than separate mechanisms.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-025

The library SHALL treat application-defined behavior as actions. An application-defined callback SHALL be one action type; declarative timer control and declarative event generation SHALL be additional action types. The library SHALL support binding callback actions using at least: class member function pointers, free function pointers, and lambda/functor objects.

## Declarative State-Associated Actions

### Timer Control Actions

**UID**: SWR-HSM-037
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Declarative timer control eliminates boilerplate for common timeout, watchdog, and reset patterns.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support timer control actions (start, stop, restart of a specified timer) that can be attached to state entry, state exit, and transitions.

### Event Generation Actions

**UID**: SWR-HSM-040
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Declarative event generation enables transient states and cascading patterns without imperative code.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support event generation actions that generate a specified event and can be attached to state entry, state exit, and transitions.

### Multiple Actions Per Attachment Point

**UID**: SWR-HSM-107
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Real state machines frequently need several effects at one lifecycle point (e.g. stop one timer and start another on entry); a single-action limit would force artificial intermediate states.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-026

The library SHALL support attaching multiple actions to a single state entry, state exit, or transition. Multiple actions attached to the same point SHALL be executed in their defined order.

## Event Processing

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
**Rationale**: Reporting an error when a transition is requested before initialization prevents processing against an unestablished structure.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-031

The library SHALL report an error when a transition or event submission is requested on a state machine that has not been successfully initialized.

### Error on Post-Initialization Structural Modification

**UID**: SWR-HSM-112
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Structural changes after initialization would break the immutability guarantee (SWR-HSM-008); reporting an error makes the violation observable.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-031

The library SHALL report an error when a structural modification is attempted after initialization.

### Error on Invalid Identifier Parameters

**UID**: SWR-HSM-113
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Operations referencing unregistered or invalid state, event, or timer identifiers must fail observably rather than silently.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-031

The library SHALL report an error when an operation is invoked with an invalid or unregistered identifier parameter.

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

The generated code SHALL use a base class pattern where the generated class provides structure registration and declares virtual callback methods. Applications SHALL derive from the generated class to implement callback behavior. Regeneration of the base class SHALL preserve application-derived code without requiring changes to it.

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

The project SHALL provide CMake integration that automatically invokes the code generation tool when source SCXML files change, ensuring generated artifacts are regenerated during the build.

## Debugging and Observability

### Diagnostic Trace Messages

**UID**: SWR-HSM-079
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Human-readable trace messages give immediate insight into state machine activity during development and field diagnostics.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

When trace logging is enabled, the library SHALL emit human-readable trace messages describing state machine activity, including state transitions, event processing, and timer operations.

### Application-Provided Trace Output

**UID**: SWR-HSM-114
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Applications need trace output to flow to their own logging infrastructure rather than a fixed sink.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The library SHALL allow the application to provide a callback that is invoked for each trace message with the message text as a string argument. When no callback is provided, the library SHALL use a default platform-specific trace output.

### Structured Execution Log Format

**UID**: SWR-HSM-115
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A machine-readable log enables offline replay and visual analysis distinct from human-readable traces.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-062

When structured execution logging is enabled, the library SHALL record state machine execution information in a defined structured format. The log SHALL capture state transitions (source, target, event), event submissions, and timer operations (start, stop, expire), with sufficient ordering information to reconstruct the execution sequence.

### Diagnostic Tool Compatibility

**UID**: SWR-HSM-083
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: A documented, parseable format lets diagnostic tools replay and inspect execution.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-063

The structured execution log format SHALL be documented and parseable, enabling compatible diagnostic tools to replay and inspect state machine execution sequences.

### Build-Time Logging Configuration

**UID**: SWR-HSM-080
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Constrained targets must be able to exclude logging code to minimize footprint; trace and structured logging are controlled independently.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-064

The library SHALL provide independent CMake configuration options that enable or disable trace logging and structured execution logging at build time. When a logging type is disabled, its implementation SHALL be excluded from the built binary.

### Runtime Debug Enable/Disable

**UID**: SWR-HSM-081
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Runtime control allows selective debugging without rebuilding.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-061

The library SHALL provide an API to enable and disable logging at runtime without rebuilding. Changes SHALL take effect for the next logged event.

### Human-Readable Identifier Names

**UID**: SWR-HSM-084
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Readable names make log output immediately interpretable.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The library SHALL support registering human-readable string names for state and event identifiers. When logging is active, the library SHALL include registered names in log output.

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
  **ID**: SYS-HSM-073

The library SHALL support independently enabling or disabling the following capabilities via CMake configuration: trace logging, structured execution logging, structure validation, and thread safety.

## Exit Points

### Exit Point Support

**UID**: SWR-HSM-116
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Exit points let a composite state expose distinct completion paths so the parent's outgoing transition depends on how the substate region finished.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-070

The library SHALL support exit point pseudo-states within a composite state. Activation of a specific exit point by a child state SHALL select the corresponding outgoing transition from the parent state.

## SCXML Subset and Interpretation

### Supported SCXML Constructs

**UID**: SWR-HSM-117
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: The code generator must accept the specific SCXML constructs hsmcpp uses to express structure.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-054

The code generation tool SHALL parse and interpret the following SCXML constructs: scxml (initial, xmlns), state (id, initial), parallel (id), transition (event, cond, target, type), initial, final (id), onentry, onexit, and history (id, type).

### Ignored SCXML Constructs

**UID**: SWR-HSM-118
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: SCXML executable content, data model, and external communication have no hsmcpp equivalent and must be skipped without failing parsing.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-071

The code generation tool SHALL ignore SCXML executable-content, data-model, and external-communication constructs (including raise, if, elseif, else, foreach, log, datamodel, data, assign, donedata, content, param, send, cancel, invoke type/typeexpr, and finalize) without failing model processing.

### Custom SCXML Extensions

**UID**: SWR-HSM-119
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: hsmcpp assigns hsmcpp-specific meaning to selected SCXML constructs to bind callbacks and compose files.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-072

The code generation tool SHALL interpret the following constructs with hsmcpp-specific meaning: the script element (within onentry, onexit, and transition) as a named application callback; the invoke element srcexpr attribute as a named state-changed callback; and the state src attribute (and xi:include) as external-file composition.

## Timers Non-Functional

### Timer Precision Bound

**UID**: SWR-HSM-120
**Nature**: NonFunctional
**Criticality**: NonSafety
**Verification**: Analysis
**Rationale**: Timer accuracy depends on the platform dispatcher's timing facilities; the library adds no precision guarantee of its own.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-074

Timer expiration precision SHALL be bounded by the timing facilities of the platform dispatcher in use. The delivered precision SHALL be that provided by the underlying platform timing facility.

## State Machine Status

### Execution Status Query

**UID**: SWR-HSM-121
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Applications need to know whether the state machine is running or has terminated (e.g. after reaching a final state).
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-075

The library SHALL provide an operation to query the current execution status of the state machine, distinguishing at least the uninitialized, running, and terminated conditions.

## Default Logging Implementations

### Linux Default Trace Output

**UID**: SWR-HSM-122
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Each platform needs an out-of-box trace sink appropriate to its environment.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The default trace output on Linux SHALL write to the standard console output (stdout).

### Windows Default Trace Output

**UID**: SWR-HSM-123
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: Windows development builds benefit from console trace output out of the box.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The default trace output on Windows SHALL write to the standard console output (stdout).

### QNX Default Trace Output

**UID**: SWR-HSM-124
**Nature**: Functional
**Criticality**: NonSafety
**Verification**: Test
**Rationale**: QNX provides a system logging facility (slog) that is the natural default sink.
**Relations**:
- **Type**: Parent \
  **ID**: SYS-HSM-060

The default trace output on QNX SHALL write to the QNX system logging facility (slog).
