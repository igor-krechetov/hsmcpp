# hsmcpp System Requirements

**Grammar**: system.gra.md
**Prefix**: SYS-HSM-

## Product Scope and Constraints

### C++ Standard Compatibility

**UID**: SYS-HSM-001
**Nature**: NonFunctional
**Rationale**: hsmcpp targets embedded and desktop environments; C++11 is the minimum standard widely available on embedded toolchains.

The hsmcpp system SHALL compile and function correctly with C++11.

### Type-Safe Identifiers

**UID**: SYS-HSM-002
**Nature**: Functional
**Rationale**: Type-safe identifiers prevent accidental misuse of raw values and enable compile-time verification of state machine structure by users.

The hsmcpp system SHALL provide type-safe state, event, and timer identifiers that allow users to define domain-specific enumeration types.

### Configurable Build Options

**UID**: SYS-HSM-003
**Nature**: Functional
**Rationale**: Selective compilation minimizes binary size and dependencies on resource-constrained platforms, enabling hsmcpp to be deployed in environments with varying capability requirements.

The hsmcpp system SHALL provide build configuration options to selectively include or exclude platform integrations, thread-safety mechanisms, structure validation, and diagnostic support.

### Independent Event Processing

**UID**: SYS-HSM-004
**Nature**: Functional
**Rationale**: Applications use hsmcpp in different execution environments; the product must not require integration with a specific external execution framework.

The hsmcpp system SHALL provide event processing without requiring applications to integrate a specific external event-processing framework or runtime.

### Application-Specific Execution Environment Integration

**UID**: SYS-HSM-005
**Nature**: Functional
**Rationale**: Applications with existing event loops or proprietary frameworks need the ability to integrate hsmcpp into their execution environment.

The hsmcpp system SHALL support integration with application-specific execution environments.

## State-Machine Execution

### Explicit Initialization

**UID**: SYS-HSM-006
**Nature**: Functional
**Rationale**: Explicit initialization separates structure configuration from runtime execution, allowing the full state machine topology to be established and validated before any transitions occur.

The hsmcpp system SHALL provide an explicit mechanism to transition a configured state machine into execution.

### Structure Before Execution

**UID**: SYS-HSM-007
**Nature**: Functional
**Rationale**: Requiring structure to be established before execution enables safe concurrent event processing without topology changes during operation.

The hsmcpp system SHALL require the state-machine structure to be fully established before execution begins.

### Single Execution Context for Application Behavior

**UID**: SYS-HSM-008
**Nature**: Functional
**Rationale**: Executing all application-defined behavior within a single execution context per state machine provides a predictable execution model and eliminates data races within the state machine's own processing.

The hsmcpp system SHALL execute application-defined state and transition behavior for a given state machine within a single execution context.

### Multiple State Machines Per Execution Context

**UID**: SYS-HSM-009
**Nature**: Functional
**Rationale**: Sharing an execution context reduces resource consumption on constrained platforms and simplifies integration with application event loops.

The hsmcpp system SHALL support multiple state machine instances sharing a single execution context.

## State Modeling

### State Definition

**UID**: SYS-HSM-010
**Nature**: Functional
**Rationale**: States are the fundamental building blocks of a hierarchical state machine.

The hsmcpp system SHALL support definition of uniquely identified states with optional application-defined behavior associated with state lifecycle events.

### Initial State

**UID**: SYS-HSM-011
**Nature**: Functional
**Rationale**: Every well-formed state machine requires a defined starting state to ensure deterministic behavior upon initialization.

The hsmcpp system SHALL allow the user to designate one state as the initial state, which becomes active upon initialization.

### Final State

**UID**: SYS-HSM-012
**Nature**: Functional
**Rationale**: Final states enable composite state completion semantics, allowing parent states to react when a substate machine reaches its terminal condition.

The hsmcpp system SHALL support final states. When a final state is entered, the system SHALL automatically generate a specified event to signal completion.

### Active State Information

**UID**: SYS-HSM-013
**Nature**: Functional
**Rationale**: Applications need to inspect the current state machine configuration for conditional logic outside the state machine.

The hsmcpp system SHALL provide the ability to determine which states are currently active and whether a specific state is active.

## Transitions

### Event-Triggered Transitions

**UID**: SYS-HSM-014
**Nature**: Functional
**Rationale**: Event-driven transitions are the core mechanism for state machine progression.

The hsmcpp system SHALL support transitions between states triggered by uniquely identified events.

### Guarded Transitions

**UID**: SYS-HSM-015
**Nature**: Functional
**Rationale**: Guards allow dynamic routing of transitions based on runtime conditions without proliferating states.

The hsmcpp system SHALL support guarded transitions whose execution depends on an application-defined condition.

### Self-Transitions

**UID**: SYS-HSM-016
**Nature**: Functional
**Rationale**: Internal self-transitions allow in-state processing without triggering exit/entry side effects, while external self-transitions reset state context.

The hsmcpp system SHALL support self-transitions in two modes: internal (no state exit/entry) and external (full exit and re-entry of the state).

### Transition Priority

**UID**: SYS-HSM-017
**Nature**: Functional
**Rationale**: Deterministic priority resolution ensures predictable behavior when multiple guarded transitions compete for the same event.

When multiple transitions match the same event from the same state, the hsmcpp system SHALL evaluate them in a deterministic order and execute the first transition whose guard condition is satisfied.

### Synchronous Interaction

**UID**: SYS-HSM-018
**Nature**: Functional
**Rationale**: Synchronous interaction allows callers to wait for completion of a requested state-machine operation before continuing.

The hsmcpp system SHALL provide a mechanism for a caller to request a state-machine transition synchronously and receive its completion or failure result.

### Pending Event Discard

**UID**: SYS-HSM-019
**Nature**: Functional
**Rationale**: Discarding pending events enables cancel-all-pending-work semantics for high-priority state changes such as error recovery or shutdown.

The hsmcpp system SHALL provide the ability to discard all pending events.

### Transition Possibility Check

**UID**: SYS-HSM-020
**Nature**: Functional
**Rationale**: Dry-run checks allow application logic to determine reachability without side effects.

The hsmcpp system SHALL provide the ability to determine whether a specified event can trigger a successful transition from the current state without changing state machine state.

### Failed Transition Notification

**UID**: SYS-HSM-021
**Nature**: Functional
**Rationale**: Applications need to detect when a requested transition does not complete so that they can perform logging, error handling, or alternative processing.

The hsmcpp system SHALL provide notification to the application when a transition request does not complete successfully.

### Transition-Associated Data

**UID**: SYS-HSM-022
**Nature**: Functional
**Rationale**: Passing context data through transitions eliminates the need for external shared state between transition triggers and application-defined behavior.

The hsmcpp system SHALL allow arbitrary application data to be associated with a transition request and delivered to all application-defined behavior triggered by that transition.

### State Configuration Consistency

**UID**: SYS-HSM-074
**Nature**: Functional
**Rationale**: Applications must not encounter a partially transitioned or otherwise invalid active state configuration when a transition cannot be completed.

The hsmcpp system SHALL maintain a valid active state configuration after every transition request, including transition requests that do not complete successfully.

### Event Queue Capacity Handling

**UID**: SYS-HSM-076
**Nature**: Functional
**Rationale**: Resource-constrained applications require predictable behavior when the state machine cannot accept additional pending events.

The hsmcpp system SHALL report asynchronous event submission failure when the event cannot be accepted for processing.

## State and Transition Actions

### State Entry Behavior

**UID**: SYS-HSM-023
**Nature**: Functional
**Rationale**: Entry behavior enables initialization logic and validation that can reject transitions based on runtime conditions evaluated at the target state.

The hsmcpp system SHALL support execution of application-defined actions when a state is entered. Entry behavior SHALL be able to prevent the transition from completing.

### State Exit Behavior

**UID**: SYS-HSM-024
**Nature**: Functional
**Rationale**: Exit behavior enables cleanup logic and allows states to refuse departure if preconditions for leaving are not met.

The hsmcpp system SHALL support execution of application-defined actions when a state is exited. Exit behavior SHALL be able to prevent the transition from completing.

### State-Changed Behavior

**UID**: SYS-HSM-025
**Nature**: Functional
**Rationale**: Post-activation behavior provides a safe point to execute logic that depends on the state being fully entered, without the ability to cancel.

The hsmcpp system SHALL support execution of application-defined actions after a state has become fully active.

### Transition Behavior

**UID**: SYS-HSM-026
**Nature**: Functional
**Rationale**: Transition-associated behavior allows side effects to be associated with the act of transitioning rather than with either endpoint state.

The hsmcpp system SHALL support execution of application-defined actions associated with a state transition.

### Guard Evaluation

**UID**: SYS-HSM-027
**Nature**: Functional
**Rationale**: Runtime guard conditions enable dynamic transition selection based on application state.

The hsmcpp system SHALL evaluate an application-defined condition before executing a guarded transition. The transition SHALL proceed only when the condition result matches the configured expected value.

### Automatic Timer Control

**UID**: SYS-HSM-028
**Nature**: Functional
**Rationale**: Declarative timer control on state lifecycle eliminates boilerplate code for common timeout and watchdog patterns.

The hsmcpp system SHALL support automatic timer control (start, stop, restart) associated with state entry and state exit events.

### Automatic Event Generation

**UID**: SYS-HSM-029
**Nature**: Functional
**Rationale**: Automatic event generation on state lifecycle enables transient states and cascading state machine patterns without explicit application code.

The hsmcpp system SHALL support automatic event generation associated with state entry and state exit events.

### Action Conditioned on Successful Transition

**UID**: SYS-HSM-030
**Nature**: Functional
**Rationale**: Automatic actions represent committed side effects that should only occur when the state machine actually reaches or leaves the configured state.

The hsmcpp system SHALL execute automatic state-associated actions only when the corresponding transition completes successfully.

### Multiple Behavior Binding Styles

**UID**: SYS-HSM-031
**Nature**: Functional
**Rationale**: hsmcpp users employ different coding paradigms; the product must accommodate object-oriented, procedural, and functional approaches to defining state-machine behavior.

The hsmcpp system SHALL support multiple styles for binding application-defined behavior to state lifecycle and transition operations.

## Event Processing

### Asynchronous Event Processing

**UID**: SYS-HSM-032
**Nature**: Functional
**Rationale**: Asynchronous processing decouples event producers from state machine execution, enabling responsive systems that do not block callers.

The hsmcpp system SHALL support asynchronous event processing where the caller submitting an event returns immediately and the event is processed independently.

### FIFO Event Ordering

**UID**: SYS-HSM-033
**Nature**: Functional
**Rationale**: FIFO ordering preserves causal relationships between events in a deterministic manner.

The hsmcpp system SHALL process queued events in FIFO order unless pending events are explicitly discarded or a higher-priority internal processing rule applies.

### Re-Entrancy Prevention

**UID**: SYS-HSM-034
**Nature**: Functional
**Rationale**: Nested event processing during an active transition can produce inconsistent state machine configurations.

The hsmcpp system SHALL prevent re-entrant event processing, ensuring that events generated during an active processing cycle are deferred for later processing.

### Internal Work Priority

**UID**: SYS-HSM-035
**Nature**: Functional
**Rationale**: Prioritizing internally generated work ensures that state-machine processing completes required internal operations before new external events alter the state-machine state.

The hsmcpp system SHALL process internally generated events before externally submitted events that are pending for the same execution context.

### Invalid Operation Handling

**UID**: SYS-HSM-083
**Nature**: Functional
**Rationale**: Applications require predictable behavior when API operations cannot be completed because of invalid state-machine configuration, lifecycle state, or operation parameters.

The hsmcpp system SHALL provide defined error handling for operations that cannot be completed due to invalid parameters, invalid state-machine configuration, or the current state-machine lifecycle state.

## Concurrency

### Concurrent Event Submission

**UID**: SYS-HSM-036
**Nature**: Functional
**Rationale**: Multi-threaded applications frequently produce events from worker threads or network callbacks that must be safely delivered to the state machine.

The hsmcpp system SHALL support receiving events from execution contexts other than the context performing state-machine processing.

### Concurrent Timer Operations

**UID**: SYS-HSM-037
**Nature**: Functional
**Rationale**: Timers may be controlled from different execution contexts than the one running the event-processing loop.

The hsmcpp system SHALL support concurrency-safe timer control operations.

### Concurrent Transition Requests

**UID**: SYS-HSM-038
**Nature**: Functional
**Rationale**: The transition interface is the primary entry point for external contexts to interact with the state machine and must remain safe for concurrent use.

The hsmcpp system SHALL support concurrency-safe transition requests from multiple execution contexts.

### Non-Allocating Transition Request

**UID**: SYS-HSM-039
**Nature**: Functional
**Rationale**: Embedded systems may require state-machine interaction from contexts in which dynamic memory allocation is prohibited (interrupts, signals, etc.).

The hsmcpp system SHALL provide a transition-request mechanism that does not perform dynamic memory allocation.

### Configurable Thread Safety

**UID**: SYS-HSM-040
**Nature**: Functional
**Rationale**: Single-threaded or bare-metal environments benefit from eliminating synchronization overhead when thread safety is not needed.

The hsmcpp system SHALL provide a compile-time option to enable or disable thread safety mechanisms.

### Non-Polling Synchronous Wait

**UID**: SYS-HSM-041
**Nature**: NonFunctional
**Rationale**: Continuous polling wastes CPU resources, which is critical on battery-powered and real-time systems.

The hsmcpp system SHALL implement synchronous transition waiting without continuously polling the transition completion condition.

## Hierarchical States

### Hierarchical State Relationships

**UID**: SYS-HSM-042
**Nature**: Functional
**Rationale**: Hierarchical nesting enables complex behavior to be decomposed into manageable layers with shared transitions at the parent level.

The hsmcpp system SHALL support hierarchical parent-child relationships between states, forming a nested state structure.

### Hierarchical Structure Validation

**UID**: SYS-HSM-043
**Nature**: Functional
**Rationale**: Structural validation catches invalid hierarchical configurations during configuration rather than producing undefined behavior during execution.

The hsmcpp system SHALL reject invalid hierarchical configurations, including circular parent-child relationships and assigning a state to more than one parent.

### Entry Points

**UID**: SYS-HSM-044
**Nature**: Functional
**Rationale**: Entry points enable directed activation into specific substates based on the triggering context.

The hsmcpp system SHALL support entry points that determine which child state becomes active when a parent state is entered.

### Conditional Entry Points

**UID**: SYS-HSM-045
**Nature**: Functional
**Rationale**: Conditional entry points enable dynamic routing into substates based on runtime conditions without requiring intermediate transition states.

The hsmcpp system SHALL support conditional entry points where the entry substate is selected based on an application-defined condition.

### Hierarchical Transition Resolution

**UID**: SYS-HSM-046
**Nature**: Functional
**Rationale**: Hierarchical event propagation is the defining feature of statecharts, allowing parent states to handle events not handled by their children.

When no transition is defined for an event in the active substate, the hsmcpp system SHALL propagate the event to ancestor states until a matching transition is found or the topmost state is reached.

### Hierarchical State Lifecycle

**UID**: SYS-HSM-078
**Nature**: Functional
**Rationale**: Applications rely on predictable lifecycle behavior when hierarchical state configurations are entered or exited.

The hsmcpp system SHALL execute state lifecycle behavior according to the hierarchical relationships of the active state configuration when entering or exiting hierarchical states.

## Parallel States

### Parallel Region Support

**UID**: SYS-HSM-047
**Nature**: Functional
**Rationale**: Parallel states model independent concurrent concerns within a single state machine without requiring separate state machine instances.

The hsmcpp system SHALL support parallel (orthogonal) regions where multiple states are simultaneously active within a parent state.

### Independent Event Processing in Parallel Regions

**UID**: SYS-HSM-048
**Nature**: Functional
**Rationale**: Independent processing ensures that parallel regions behave as logically separate state machines sharing a parent context.

The hsmcpp system SHALL deliver events to all active parallel regions, allowing each region to independently process or ignore each event.

## History States

### Shallow History

**UID**: SYS-HSM-049
**Nature**: Functional
**Rationale**: Shallow history enables resume-where-you-left-off behavior at one level of nesting.

The hsmcpp system SHALL support shallow history states that remember and restore the last active direct child state of the parent when re-entering via the history state.

### Deep History

**UID**: SYS-HSM-050
**Nature**: Functional
**Rationale**: Deep history restores the full nested state configuration, enabling seamless resumption of complex multi-level workflows.

The hsmcpp system SHALL support deep history states that remember and restore the entire substate hierarchy that was active when the parent was last exited.

### History Default Target

**UID**: SYS-HSM-051
**Nature**: Functional
**Rationale**: A default target provides deterministic behavior on first entry when no prior history exists.

The hsmcpp system SHALL allow a default target state to be specified for a history state. When no history has been recorded, the system SHALL transition to the default target.

### History Restoration Behavior

**UID**: SYS-HSM-052
**Nature**: Functional
**Rationale**: Applications may need to perform initialization or resource acquisition that differs depending on whether a state is being entered fresh or restored from history.

The hsmcpp system SHALL support execution of application-defined behavior when a state is entered via history restoration.

## Timers

### Timer-to-Event Binding

**UID**: SYS-HSM-053
**Nature**: Functional
**Rationale**: Timer-to-event binding integrates timeout behavior directly into the state machine model, enabling time-based transitions.

The hsmcpp system SHALL support timers that generate a specified event upon expiration.

### Single-Shot Timers

**UID**: SYS-HSM-054
**Nature**: Functional
**Rationale**: Single-shot timers model deadlines, timeouts, and delayed transitions.

The hsmcpp system SHALL support single-shot timers that fire once after the specified interval and then stop automatically.

### Repeating Timers

**UID**: SYS-HSM-055
**Nature**: Functional
**Rationale**: Repeating timers model periodic polling, heartbeats, and watchdog refresh patterns.

The hsmcpp system SHALL support repeating timers that fire periodically at the specified interval until explicitly stopped.

### Timer Lifecycle Control

**UID**: SYS-HSM-056
**Nature**: Functional
**Rationale**: Full timer lifecycle control is necessary for implementing timeout extension, cancellation, and reset patterns.

The hsmcpp system SHALL provide the ability to start, stop, and restart timers.

### Timer Status Query

**UID**: SYS-HSM-057
**Nature**: Functional
**Rationale**: Timer status information allows conditional logic to avoid redundant operations.

The hsmcpp system SHALL provide the ability to query whether a specific timer is currently running.

## Code Generation

### SCXML Input Format

**UID**: SYS-HSM-058
**Nature**: Functional
**Rationale**: SCXML is a W3C standard that enables interoperability with visual editors and other statechart tools. Part of the SCXML standard (specifically scripting) is not required for hsmcpp.

The hsmcpp system SHALL accept SCXML models conforming to the hsmcpp-supported SCXML model subset as an input format for defining state-machine structure.

### Executable State-Machine Generation

**UID**: SYS-HSM-059
**Nature**: Functional
**Rationale**: Generated source code eliminates manual state-machine structure registration and maintains consistency between the model and implementation.

The hsmcpp system SHALL provide automated generation of source code implementing a state machine defined by a supported SCXML model.

### Model Validation

**UID**: SYS-HSM-081
**Nature**: Functional
**Rationale**: Invalid state-machine models must be detected before they are used for execution or code generation.

The hsmcpp system SHALL provide validation of state-machine models and SHALL report structural or configuration errors that prevent valid state-machine execution or generation.

### Diagram Generation

**UID**: SYS-HSM-060
**Nature**: Functional
**Rationale**: Generated diagrams provide always-up-to-date visual documentation of the state machine design.

The hsmcpp system SHALL provide generation of state diagram visualizations from SCXML models.

### Model Composition

**UID**: SYS-HSM-061
**Nature**: Functional
**Rationale**: Composition enables modular state machine definitions where large models can be split across multiple files for maintainability.

The hsmcpp system SHALL support composition of state machine models from multiple source files.

### Automatic Regeneration

**UID**: SYS-HSM-062
**Nature**: Functional
**Rationale**: Automatic regeneration ensures generated artifacts stay in sync with source models without manual intervention during the build process.

The hsmcpp system SHALL support automatic regeneration of generated artifacts when their source models change.
TODO: make a SW REQ for SYS-HSM-059.

## Debugging and Observability

### Diagnostic Logging

**UID**: SYS-HSM-063
**Nature**: Functional
**Rationale**: Structured logs enable post-mortem analysis of state machine behavior using dedicated tooling.

The hsmcpp system SHALL support recording a structured diagnostic log capturing state transitions, events, and timer operations.

### Runtime Diagnostic Control

**UID**: SYS-HSM-064
**Nature**: Functional
**Rationale**: Runtime control allows selective debugging of specific scenarios without the overhead of always-on logging.

The hsmcpp system SHALL provide the ability to enable and disable diagnostic logging at runtime without rebuilding.

### Configurable Log Destination

**UID**: SYS-HSM-065
**Nature**: Functional
**Rationale**: Configurable destinations enable logging to appropriate locations in different deployment environments.

The hsmcpp system SHALL allow the diagnostic log destination to be configured.

### Diagnostic Log Replay and Analysis

**UID**: SYS-HSM-066
**Nature**: Functional
**Rationale**: Visual replay of state machine execution significantly reduces debugging time compared to reading raw logs.

The hsmcpp system SHALL produce diagnostic logs in a format that supports replay and analysis of state-machine execution by compatible diagnostic tools.

### Human-Readable State and Event Information

**UID**: SYS-HSM-067
**Nature**: Functional
**Rationale**: Readable names in diagnostic output make logs immediately interpretable without cross-referencing identifier definitions.

The hsmcpp system SHALL support resolution of state and event identifiers to human-readable names in diagnostic output.

## Platform Support

### Linux Platform Support

**UID**: SYS-HSM-068
**Nature**: Functional
**Rationale**: Linux is the primary development and deployment platform for most hsmcpp use cases.

The hsmcpp system SHALL support execution on Linux.

### QNX Platform Support

**UID**: SYS-HSM-069
**Nature**: Functional
**Rationale**: QNX is a common RTOS in automotive and industrial systems where hierarchical state machines are heavily used.

The hsmcpp system SHALL support execution on QNX.

### Windows Platform Support

**UID**: SYS-HSM-070
**Nature**: Functional
**Rationale**: Windows support enables development and testing on developer workstations and Windows-based embedded systems.

The hsmcpp system SHALL support execution on Windows.

### FreeRTOS Platform Support

**UID**: SYS-HSM-071
**Nature**: Functional
**Rationale**: FreeRTOS is a widely deployed RTOS in embedded systems.

The hsmcpp system SHALL support execution on FreeRTOS (V10.3.1 and later).

### Arduino Platform Support

**UID**: SYS-HSM-072
**Nature**: Functional
**Rationale**: Arduino environments are single-threaded cooperative systems commonly used for embedded state-machine applications.

The hsmcpp system SHALL support execution on Arduino.
