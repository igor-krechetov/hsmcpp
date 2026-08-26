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

The hsmcpp system SHALL provide type-safe state, event, and timer identifiers that allow users to define domain-specific identifier sets distinguishable at compile time.

### Independent Event Processing

**UID**: SYS-HSM-003
**Nature**: Functional
**Rationale**: Applications use hsmcpp in different execution environments; the product must not require integration with a specific external execution framework.

The hsmcpp system SHALL provide event processing without requiring applications to integrate a specific external event-processing framework or runtime.

### Application-Specific Execution Environment Integration

**UID**: SYS-HSM-004
**Nature**: Functional
**Rationale**: Applications with existing event loops or proprietary frameworks need the ability to integrate hsmcpp into their execution environment.

The hsmcpp system SHALL support integration with application-specific execution environments.

## State-Machine Execution

### Explicit Initialization

**UID**: SYS-HSM-005
**Nature**: Functional
**Rationale**: Explicit initialization separates structure configuration from runtime execution, allowing the full state machine topology to be established and validated before any transitions occur.

The hsmcpp system SHALL provide an explicit mechanism to transition a configured state machine into execution.

### Structure Before Execution

**UID**: SYS-HSM-006
**Nature**: Functional
**Rationale**: Requiring structure to be established before execution enables safe concurrent event processing without topology changes during operation.

The hsmcpp system SHALL require the state-machine structure to be fully established before execution begins.

### Hierarchical Structure Validation

**UID**: SYS-HSM-038
**Nature**: Functional
**Rationale**: Structural validation catches invalid hierarchical configurations during configuration rather than producing undefined behavior during execution.

The hsmcpp system SHALL support validation of hierarchical state-machine configurations to detect and report invalid configurations.

### Sequential Application Callback Execution

**UID**: SYS-HSM-007
**Nature**: Functional
**Rationale**: Executing all application-defined callbacks sequentially within a single execution context per state machine provides a predictable execution model for application code.

The hsmcpp system SHALL execute application-defined state and transition callbacks for a given state machine within a single execution context.

### Shared Execution Context

**UID**: SYS-HSM-008
**Nature**: Functional
**Rationale**: Sharing an execution context reduces resource consumption on constrained platforms and simplifies integration with application event loops.

The hsmcpp system SHALL support multiple state machine instances sharing a single execution context.

### State Machine Release and Re-initialization

**UID**: SYS-HSM-009
**Nature**: Functional
**Rationale**: Applications may need to reset a state machine to its initial configuration in response to error recovery, mode changes, or reconfiguration without creating a new instance.

The hsmcpp system SHALL support releasing a state machine from execution and returning it to a state that allows re-initialization.

### State Machine Execution Status

**UID**: SYS-HSM-xxx
**Nature**: Functional
**Rationale**: Applications may need to know if statemachine is running or have finished it's execution.

The hsmcpp system SHALL provide an explicit mechanism to query state of the state machine.

## State Modeling

### State Definition

**UID**: SYS-HSM-010
**Nature**: Functional
**Rationale**: States are the fundamental building blocks of a hierarchical state machine.

The hsmcpp system SHALL support definition of uniquely identified states with optional application-defined callbacks associated with state lifecycle events.

### Initial State

**UID**: SYS-HSM-011
**Nature**: Functional
**Rationale**: Every well-formed state machine requires a defined starting state to ensure deterministic behavior upon initialization.

The hsmcpp system SHALL allow the user to designate one state as the initial state, which becomes active upon initialization.

### Final State

**UID**: SYS-HSM-012
**Nature**: Functional
**Rationale**: Final states allow to define exit conditions to allow gracefull termination of applications.

The hsmcpp system SHALL support final states. When a final state is entered, the system SHALL stop processing further events.

### Active State Information

**UID**: SYS-HSM-013
**Nature**: Functional
**Rationale**: Applications need to inspect the current state machine configuration for conditional logic outside the state machine.

The hsmcpp system SHALL provide the ability to determine which states are currently active and whether a specific state is active.

## Hierarchical States

### Hierarchical State Relationships

**UID**: SYS-HSM-037
**Nature**: Functional
**Rationale**: Hierarchical nesting enables complex behavior to be decomposed into manageable layers with shared transitions at the parent level.

The hsmcpp system SHALL support hierarchical parent-child relationships between states, forming a nested state structure.

### Entry Points

**UID**: SYS-HSM-039
**Nature**: Functional
**Rationale**: Entry points enable directed activation into specific substates based on the triggering context.

The hsmcpp system SHALL support entry points that determine which child state becomes active when a parent state is entered.

### Exit Points

**UID**: SYS-HSM-070
**Nature**: Functional
**Rationale**: Exit points enable a composite state to expose multiple distinct exit paths, allowing different transitions from the parent based on how the substate region completed.

The hsmcpp system SHALL support exit points that determine which outgoing transition from a parent state is taken based on the exit path selected by a child state.

### Conditional Entry Points

**UID**: SYS-HSM-040
**Nature**: Functional
**Rationale**: Conditional entry points enable dynamic routing into substates based on runtime conditions without requiring intermediate transition states.

The hsmcpp system SHALL support conditional entry points where the entry substate is selected based on an application-defined condition.

### Hierarchical Transition Resolution

**UID**: SYS-HSM-041
**Nature**: Functional
**Rationale**: Hierarchical event propagation is the defining feature of statecharts, allowing parent states to handle events not handled by their children.

The hsmcpp system SHALL support transition resolution across hierarchical state relationships.

### Hierarchical State Lifecycle

**UID**: SYS-HSM-042
**Nature**: Functional
**Rationale**: Applications rely on predictable lifecycle callback execution when hierarchical state configurations are entered or exited.

The hsmcpp system SHALL execute state lifecycle callbacks according to the hierarchical relationships of the active state configuration when entering or exiting hierarchical states.

## Parallel States

### Parallel Region Support

**UID**: SYS-HSM-043
**Nature**: Functional
**Rationale**: Parallel states model independent concurrent concerns within a single state machine without requiring separate state machine instances.

The hsmcpp system SHALL support parallel (orthogonal) regions where multiple states are simultaneously active within a parent state.

### Independent Event Processing in Parallel Regions

**UID**: SYS-HSM-044
**Nature**: Functional
**Rationale**: Independent processing ensures that parallel regions behave as logically separate state machines sharing a parent context.

The hsmcpp system SHALL deliver events to all active parallel regions, allowing each region to independently process or ignore each event.

## History States

### Shallow History

**UID**: SYS-HSM-045
**Nature**: Functional
**Rationale**: Shallow history enables resume-where-you-left-off behavior at one level of nesting.

The hsmcpp system SHALL support shallow history states that remember and restore the last active direct child state of the parent when re-entering via the history state.

### Deep History

**UID**: SYS-HSM-046
**Nature**: Functional
**Rationale**: Deep history restores the full nested state configuration, enabling seamless resumption of complex multi-level workflows.

The hsmcpp system SHALL support deep history states that remember and restore the entire substate hierarchy that was active when the parent was last exited.

### History Default Target

**UID**: SYS-HSM-047
**Nature**: Functional
**Rationale**: A default target provides deterministic behavior on first entry when no prior history exists.

The hsmcpp system SHALL allow a default target state to be specified for a history state. When no history has been recorded, the system SHALL transition to the default target.

### History Restoration Behavior

**UID**: SYS-HSM-048
**Nature**: Functional
**Rationale**: Applications may need to perform initialization or resource acquisition that differs depending on whether a state is being entered fresh or restored from history.

The hsmcpp system SHALL support execution of application-defined callbacks when a state is entered via history restoration.

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

The hsmcpp system SHALL support guarded transitions whose execution depends on runtime condition evaluation.

### Self-Transitions

**UID**: SYS-HSM-016
**Nature**: Functional
**Rationale**: Internal self-transitions allow in-state processing without triggering exit/entry side effects, while external self-transitions reset state context.

The hsmcpp system SHALL support self-transitions in two modes: internal (no state exit/entry) and external (full exit and re-entry of the state).

### Transition Priority

**UID**: SYS-HSM-017
**Nature**: Functional
**Rationale**: Deterministic priority resolution ensures predictable behavior when multiple guarded transitions compete for the same event.

When multiple transitions match the same event from the same state, the hsmcpp system SHALL evaluate them in a deterministic order and execute all transitions whose guard condition is satisfied.

### Transition-Associated Data

**UID**: SYS-HSM-022
**Nature**: Functional
**Rationale**: Passing context data through transitions eliminates the need for external shared state between transition triggers and application-defined callbacks.

The hsmcpp system SHALL allow arbitrary application data to be associated with a transition request and delivered to all application-defined callbacks triggered by that transition.

### State Configuration Consistency

**UID**: SYS-HSM-023
**Nature**: Functional
**Rationale**: Applications must not encounter a partially transitioned or otherwise invalid active state configuration when a transition cannot be completed.

The hsmcpp system SHALL maintain a valid active state configuration after every transition request, including transition requests that do not complete successfully.

### Transition Cancellation

**UID**: SYS-HSM-024
**Nature**: Functional
**Rationale**: Applications may need to prevent a transition from completing based on runtime conditions evaluated during the transition, enabling validation and precondition enforcement.

The hsmcpp system SHALL support cancellation of an in-progress transition by application-defined callbacks.

## State and Transition Actions

### State and Transition Callbacks

**UID**: SYS-HSM-025
**Nature**: Functional
**Rationale**: Applications require the ability to associate custom logic with state lifecycle events and transitions to implement initialization, cleanup, validation, and side-effect behavior.

The hsmcpp system SHALL support execution of application-defined callbacks associated with state lifecycle events and state transitions.

### Declarative State-Associated Actions

**UID**: SYS-HSM-026
**Nature**: Functional
**Rationale**: Declarative actions on state lifecycle eliminate boilerplate code for common timeout, watchdog, and cascading state-machine patterns.

The hsmcpp system SHALL support declarative actions associated with state lifecycle events and transitions. Supported actions include timer control, event generation.

## Event Processing

### Asynchronous Event Processing

**UID**: SYS-HSM-027
**Nature**: Functional
**Rationale**: Asynchronous processing decouples event producers from state machine execution, enabling responsive systems that do not block callers.

The hsmcpp system SHALL support asynchronous event processing where the caller submitting an event returns immediately and the event is processed independently.

### Synchronous Event Processing

**UID**: SYS-HSM-018
**Nature**: Functional
**Rationale**: Synchronous interaction allows callers to wait for completion of a requested state-machine operation before continuing.

The hsmcpp system SHALL support synchronous event processing where the caller submitting an event blocks until the transition completes or fails and receives the result.

### FIFO Event Ordering

**UID**: SYS-HSM-028
**Nature**: Functional
**Rationale**: FIFO ordering preserves causal relationships between events in a deterministic manner.

The hsmcpp system SHALL process queued events in FIFO order unless pending events are explicitly discarded or a higher-priority internal processing rule applies.

### Re-Entrancy Prevention

**UID**: SYS-HSM-029
**Nature**: Functional
**Rationale**: Nested event processing during an active transition can produce inconsistent state machine configurations.

The hsmcpp system SHALL prevent re-entrant event processing, ensuring that events generated during an active processing cycle are deferred for later processing.

### Event Queue Capacity Handling

**UID**: SYS-HSM-030
**Nature**: Functional
**Rationale**: Resource-constrained applications require predictable behavior when the state machine cannot accept additional pending events.

The hsmcpp system SHALL report asynchronous event submission failure when the event cannot be accepted for processing.

### Invalid Operation Handling

**UID**: SYS-HSM-031
**Nature**: Functional
**Rationale**: Applications require predictable behavior when API operations cannot be completed because of invalid state-machine configuration, lifecycle state, or operation parameters.

The hsmcpp system SHALL provide defined error handling for operations that cannot be completed due to invalid parameters, invalid state-machine configuration, or the current state-machine lifecycle state.

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

## Concurrency

### Concurrent Transition Requests

**UID**: SYS-HSM-032
**Nature**: Functional
**Rationale**: Multi-threaded applications frequently produce events from worker threads or network callbacks that must be safely delivered to the state machine.

The hsmcpp system SHALL support receiving events from execution contexts other than the context performing state-machine processing.

### Concurrent Timer Operations

**UID**: SYS-HSM-033
**Nature**: Functional
**Rationale**: Timers may be controlled from different execution contexts than the one running the event-processing loop.

The hsmcpp system SHALL support concurrency-safe timer control operations.

### Restricted-Context Transition Requests

**UID**: SYS-HSM-035
**Nature**: Functional
**Rationale**: Embedded systems may require state-machine interaction from execution contexts where dynamic memory allocation is not permitted, such as interrupt service routines or signal handlers.

The hsmcpp system SHALL support transition requests from execution contexts where dynamic memory allocation is not permitted.

### Configurable Thread Safety

**UID**: SYS-HSM-036
**Nature**: NonFunctional
**Rationale**: Single-threaded or bare-metal environments benefit from eliminating synchronization overhead when concurrent access is not required.

The hsmcpp system SHALL support operation without thread-safety overhead in single-threaded or cooperative execution environments.

## Timers

### Timer-to-Event Binding

**UID**: SYS-HSM-049
**Nature**: Functional
**Rationale**: Timer-to-event binding integrates timeout behavior directly into the state machine model, enabling time-based transitions.

The hsmcpp system SHALL support timers that generate a specified event upon expiration.

### Single-Shot Timers

**UID**: SYS-HSM-050
**Nature**: Functional
**Rationale**: Single-shot timers model deadlines, timeouts, and delayed transitions.

The hsmcpp system SHALL support single-shot timers that fire once after the specified interval and then stop automatically.

### Repeating Timers

**UID**: SYS-HSM-051
**Nature**: Functional
**Rationale**: Repeating timers model periodic polling, heartbeats, and watchdog refresh patterns.

The hsmcpp system SHALL support repeating timers that fire periodically at the specified interval until explicitly stopped.

### Timer Lifecycle Control

**UID**: SYS-HSM-052
**Nature**: Functional
**Rationale**: Full timer lifecycle control is necessary for implementing timeout extension, cancellation, and reset patterns.

The hsmcpp system SHALL provide the ability to start, stop, and restart timers.

### Timer Status Query

**UID**: SYS-HSM-053
**Nature**: Functional
**Rationale**: Timer status information allows conditional logic to avoid redundant operations.

The hsmcpp system SHALL provide the ability to query whether a specific timer is currently running.

## Code Generation

### SCXML Input Format

**UID**: SYS-HSM-054
**Nature**: Functional
**Rationale**: SCXML is a W3C standard that enables interoperability with visual editors and other statechart tools. Part of the SCXML standard (specifically scripting) is not required for hsmcpp.

The hsmcpp system SHALL accept SCXML models conforming to the hsmcpp-supported SCXML model subset as an input format for defining state-machine structure.

### Executable State-Machine Generation

**UID**: SYS-HSM-055
**Nature**: Functional
**Rationale**: Generated source code eliminates manual state-machine structure registration and maintains consistency between the model and implementation.

The hsmcpp system SHALL provide automated generation of source code implementing a state machine defined by a supported SCXML model.

### Model Validation

**UID**: SYS-HSM-056
**Nature**: Functional
**Rationale**: Invalid state-machine models must be detected before they are used for execution or code generation.

The hsmcpp system SHALL provide validation of state-machine models and SHALL report structural or configuration errors that prevent valid state-machine execution or generation.

### Diagram Generation

**UID**: SYS-HSM-057
**Nature**: Functional
**Rationale**: Generated diagrams provide always-up-to-date visual documentation of the state machine design.

The hsmcpp system SHALL provide generation of state diagram visualizations from SCXML models.

### Model Composition

**UID**: SYS-HSM-058
**Nature**: Functional
**Rationale**: Composition enables modular state machine definitions where large models can be split across multiple files for maintainability.

The hsmcpp system SHALL support composition of state machine models from multiple source files.

### Automatic Regeneration

**UID**: SYS-HSM-059
**Nature**: Functional
**Rationale**: Automatic regeneration ensures generated artifacts stay in sync with source models without manual intervention during the build process.

The hsmcpp system SHALL support automatic regeneration of generated artifacts when their source models change.

## Debugging and Observability

### Diagnostic Logging

**UID**: SYS-HSM-060
**Nature**: Functional
**Rationale**: Structured logs enable post-mortem analysis of state machine behavior using dedicated tooling.

The hsmcpp system SHALL support recording a structured diagnostic log capturing state transitions, events, and timer operations.

### Runtime Diagnostic Control

**UID**: SYS-HSM-061
**Nature**: Functional
**Rationale**: Runtime control allows selective debugging of specific scenarios without the overhead of always-on logging.

The hsmcpp system SHALL provide the ability to enable and disable diagnostic logging at runtime without rebuilding.

### Diagnostic Log Replay and Analysis

**UID**: SYS-HSM-063
**Nature**: Functional
**Rationale**: Visual replay of state machine execution significantly reduces debugging time compared to reading raw logs.

The hsmcpp system SHALL produce diagnostic logs in a format that supports replay and analysis of state-machine execution by compatible diagnostic tools.

## Platform Support

### Linux Platform Support

**UID**: SYS-HSM-065
**Nature**: Functional
**Rationale**: Linux is the primary development and deployment platform for most hsmcpp use cases.

The hsmcpp system SHALL support execution on Linux.

### QNX Platform Support

**UID**: SYS-HSM-066
**Nature**: Functional
**Rationale**: QNX is a common RTOS in automotive and industrial systems where hierarchical state machines are heavily used.

The hsmcpp system SHALL support execution on QNX.

### Windows Platform Support

**UID**: SYS-HSM-067
**Nature**: Functional
**Rationale**: Windows support enables development and testing on developer workstations and Windows-based embedded systems.

The hsmcpp system SHALL support execution on Windows.

### FreeRTOS Platform Support

**UID**: SYS-HSM-068
**Nature**: Functional
**Rationale**: FreeRTOS is a widely deployed RTOS in embedded systems.

The hsmcpp system SHALL support execution on FreeRTOS (V10.3.1 and later).

### Arduino Platform Support

**UID**: SYS-HSM-069
**Nature**: Functional
**Rationale**: Arduino environments are single-threaded cooperative systems commonly used for embedded state-machine applications.

The hsmcpp system SHALL support execution on Arduino.
