# System Requirements — Agent Steering Guide

## Purpose

This document defines how AI agents SHALL work with hsmcpp System Requirements.

The System Requirements describe the **hsmcpp product/system as experienced by its users and integrators**. hsmcpp is treated as a system consisting of the runtime library and associated tools, including code generation, debugging, visualization, and integration capabilities.

System Requirements SHALL describe **what the hsmcpp product must provide**, not how the software implements it.

The intended long-term goal is to maintain requirements artifacts that support integration of hsmcpp into safety-critical automotive software and provide a suitable basis for traceability toward Software Requirements, Software Architecture, implementation, and verification.

---

## 1. System Requirement Abstraction Level

A System Requirement SHALL specify one or more of:

* a product capability;
* externally observable behavior;
* a supported execution environment;
* a product-level integration capability;
* a product-level constraint;
* a product-level quality attribute.

A System Requirement SHOULD be understandable without knowledge of the hsmcpp source code or internal architecture.

A reader should be able to understand the requirement without knowing:

* C++ class names;
* function names;
* internal data structures;
* internal interfaces;
* synchronization primitives;
* event dispatcher implementations;
* specific implementation algorithms.

---

## 2. What Belongs in System Requirements

System Requirements MAY describe:

### Product capabilities

Examples:

* hierarchical state-machine modeling;
* state-machine execution;
* event-driven transitions;
* guarded transitions;
* parallel states;
* history states;
* timers;
* state actions;
* debugging;
* visualization;
* SCXML-based code generation.

### Observable behavioral semantics

Examples:

* event ordering;
* transition priority;
* state entry/exit semantics;
* guard evaluation;
* history restoration;
* timer behavior;
* cancellation behavior;
* concurrency semantics.

### Product integration

Examples:

* minimal integration requirements;
* support for application-specific execution environments;
* ability to operate without requiring a particular external framework;
* support for custom execution environments.

### Supported environments

Supported operating systems, RTOSes, architectures, or development environments MAY be System Requirements when they define the supported product scope.

For example:

> The hsmcpp system SHALL support execution on QNX.

The requirement SHALL NOT unnecessarily specify the implementation mechanism used to achieve this support.

### Product-level constraints

Examples:

* minimum supported C++ standard;
* resource constraints;
* timing constraints;
* memory constraints;
* restrictions on dynamic allocation;
* required portability characteristics.

---

## 3. What Does Not Belong in System Requirements

Implementation details SHALL normally be specified at Software Requirement level.

The following SHOULD NOT appear in System Requirements unless there is a compelling externally visible reason:

### API details

Avoid:

* `initialize()`
* `transition()`
* `transitionSync()`
* `emitEvent()`
* `startTimer()`
* `stopTimer()`

Instead describe the capability or behavior provided by those APIs.

### C++ implementation details

Avoid:

* `StateID_t`
* `EventID_t`
* `TimerID_t`
* `VariantVector_t`
* specific class names;
* specific callback signatures;
* specific C++ inheritance relationships.

### Internal architecture

Avoid:

* `IHsmEventDispatcher`;
* mutexes;
* condition variables;
* queues implemented in a particular way;
* worker threads;
* specific synchronization mechanisms;
* internal memory-management mechanisms.

For example, instead of:

> The library SHALL define an abstract interface `IHsmEventDispatcher`.

use a product-level requirement such as:

> The hsmcpp system SHALL support integration with application-specific execution environments.

The software architecture may then implement this capability using `IHsmEventDispatcher`.

### Tool implementation details

Avoid specifying:

* exact generated C++ class structures;
* internal generator classes;
* implementation-specific file layouts;
* CMake implementation details.

System Requirements should specify what the tool produces or supports.

---

## 4. System vs Software Requirements

Use the following distinction:

### System Requirement

Answers:

> What capability or behavior does the hsmcpp product need to provide?

Example:

> The hsmcpp system SHALL support hierarchical state-machine execution.

### Software Requirement

Answers:

> What must the hsmcpp software do to implement that system capability?

Example:

> The runtime SHALL represent parent-child state relationships using the internal state hierarchy representation.

A System Requirement may therefore have multiple Software Requirements allocated to it.

The traceability direction SHOULD generally be:

System Requirement
→ Software Requirement
→ Software Architecture / Design
→ Implementation
→ Verification

---

## 5. Requirement Independence

Each requirement SHOULD express one clear obligation.

Avoid combining unrelated obligations.

For example, avoid:

> The system SHALL support Linux with Qt, multithreading, timers, and concurrency-safe transitions.

Prefer separate requirements for:

* Linux support;
* Qt integration;
* multithreading;
* concurrency behavior;
* timer behavior.

This makes requirements independently traceable and verifiable.

---

## 6. Implementation-Neutral Wording

Prefer externally observable terminology.

### Prefer

> The hsmcpp system SHALL process queued events in FIFO order.

### Avoid

> The dispatcher SHALL pop events from a FIFO queue.

The first specifies behavior.

The second specifies implementation.

---

## 7. API Names

API names MAY appear in Software Requirements when necessary to define the software interface.

They SHOULD NOT appear in System Requirements unless the API itself is intentionally part of the externally required product interface.

For example:

**System level:**

> The hsmcpp system SHALL provide synchronous state-machine interaction.

**Software level:**

> The runtime SHALL provide a `transitionSync()` API implementing synchronous state-machine interaction.

---

## 8. Platform Requirements

Platform support is a product-level concern.

If hsmcpp is required to support a platform, that support SHOULD be explicitly captured at System level.

Example:

> The hsmcpp system SHALL support execution on QNX.

The detailed implementation of QNX support belongs at Software level.

Do not use System Requirements to prescribe a particular dispatcher, thread implementation, or OS API unless that implementation is itself an explicit product requirement.

---

## 9. Nature

System Requirements SHALL have a `Nature` field.

Allowed values:

* `Functional`
* `NonFunctional`

Use `Functional` when the requirement specifies a capability or externally observable behavior.

Use `NonFunctional` when the requirement constrains a capability through a quality, performance, resource, timing, portability, or other engineering constraint.

Do not classify requirements based merely on whether they sound important.

Examples:

* hierarchical state-machine execution → Functional;
* SCXML code generation → Functional;
* QNX support → Functional;
* FIFO event processing → Functional;
* maximum memory usage → NonFunctional;
* maximum transition latency → NonFunctional;
* no dynamic allocation in a defined execution context → NonFunctional.

The distinction SHALL be applied consistently across System and Software Requirements.

---

## 10. Safety Classification

Generic hsmcpp System Requirements SHOULD NOT receive an ASIL classification.

System Requirements describe the reusable hsmcpp product. Their eventual safety relevance depends on the safety function and application in which hsmcpp is integrated.

Do not infer an ASIL from the importance of a requirement.

Safety relevance MAY be captured at Software Requirement level when appropriate, but this does not itself assign an ASIL to hsmcpp.

The reusable product baseline should remain applicable to QM and different ASIL contexts.

---

## 11. Verification

Every System Requirement SHALL be written so that its satisfaction can ultimately be verified.

Avoid vague statements such as:

> The system SHALL be easy to use.

Prefer measurable or objectively verifiable statements such as:

> The hsmcpp system SHALL support execution without requiring application integration with a specific external event-processing framework.

Verification methods should be selected based on the actual nature of the requirement.

---

## 12. Rationale

The rationale should explain **why the product needs the capability**, not why a particular implementation was selected.

Avoid:

> An abstract dispatcher interface allows the implementation to use different threading mechanisms.

Prefer:

> Applications may use hsmcpp in different execution environments; therefore the product must not require integration with a specific execution framework.

---

## 13. Agent Decision Rule

When modifying or creating a System Requirement, apply this test:

> **Could a product user or system integrator verify the requirement without knowing how hsmcpp is implemented?**

If yes, it is potentially suitable for System level.

If no, determine whether it should become a Software Requirement instead.

A second test is:

> **Does the requirement describe what hsmcpp provides, or how hsmcpp provides it?**

* What → System Requirement.
* How → Software Requirement.

When uncertain, prefer the higher-level, implementation-neutral formulation and preserve the implementation detail for the Software Requirement layer.

---

## 14. Required Quality

System Requirements SHALL be:

* implementation-independent where practical;
* atomic;
* unambiguous;
* externally observable;
* objectively verifiable;
* traceable;
* consistent with the hsmcpp product scope;
* free from unnecessary API and source-code terminology.

The System Requirements document SHOULD remain substantially smaller and more abstract than the Software Requirements document.

The purpose of System Requirements is to establish the **product contract and scope**, not to reproduce the software architecture.
