# Requirement Tagging Plan — Bucket 1 (10 SW requirements)

## Scope and conventions (read first)

Goal: ensure each of 10 software requirement IDs appears as a Doxygen
`@requirement HSMCPP.<ID>` tag on the C++ entity that genuinely implements the
behavior the named GTest exercises. No code behavior changes — only comment tags.

Confirmed placement convention from reading the sources:
- `src/HsmImpl.cpp` contains **zero** inline `@requirement` tags. Every tag for an
  `Impl` method lives on that method's **declaration in `src/HsmImpl.hpp`**. So all
  `Impl` tags below go on the HsmImpl.hpp declarations, matching existing style.
- When a declaration already carries an `@requirement` line, **extend** it (append the
  new ID to the comma-separated list). When it has none, **add** a new
  `/// @requirement HSMCPP.<ID>` line immediately above the declaration.
- Line numbers below are current as of this plan; the implementer must re-anchor by the
  quoted declaration text if the file has shifted.

Verification for the whole batch (project uses `just`; see `./justfile`):
- Regenerate traceability and confirm each ID shows an "Implemented by" entry:
  rebuild requirements docs via the `just` target that runs doxygen +
  `scripts/requirements/doxygen_to_lobster.py` (e.g. `just requirements` / the
  requirements doc target), then open `build/requirements/hsmcpp_requirements.html`
  and confirm all 10 IDs are no longer listed as unlinked.
- Build must still compile (tags are comments, so no code impact):
  run the project build via `just` and confirm success.

---

## Per-requirement tagging

- [ ] 1. SWR_HSM_018 — Composite Initial Substate Validation (reject composite with no entry point).
      File: `src/HsmImpl.hpp`
      Entity: `bool getEntryPoints(const StateID_t state, const EventID_t onEvent, const VariantVector_t& transitionArgs, std::list<StateID_t>& outEntryPoints) const;`
      Current line: HsmImpl.hpp:237 (its existing tag line is HsmImpl.hpp:236).
      Action: **Extend** the existing line
      `/// @requirement HSMCPP.SWR_HSM_020, ...SWR_HSM_025` → append `, HSMCPP.SWR_HSM_018`.
      Justification: Test `ABCHsm.DISABLED_substate_error_no_entrypoint` transitions into composite
      `P1` which has substate `B` but no registered entry point; at runtime `getEntryPoints` returns
      `false` for `P1` (empty `outEntryPoints`), which is the decision point that denies activation of a
      composite lacking a designated initial substate. This is the function the test's behavior most
      directly exercises (the `HSM_ENABLE_SAFE_STRUCTURE` helpers `isTopState/isSubstate/hasParentState`
      only guard *registration* recursion, not the missing-entry-point rejection the test checks).

- [ ] 2. SWR_HSM_028 — Concurrent State Activation (multiple states simultaneously active).
      File: `src/HsmImpl.hpp`
      Entity: `HsmEventStatus doTransition(const PendingEventInfo& event);`
      Current line: HsmImpl.hpp:179 (its block tag is at HsmImpl.hpp:178: `@requirement HSMCPP.SWR_HSM_045, HSMCPP.SWR_HSM_057`).
      Action: **Extend** the existing `@requirement` line in the `/** ... */` block above `doTransition`
      → append `, HSMCPP.SWR_HSM_028`.
      Justification: `parallel_transition_01..09` / `parallel_callbacks` / `parallel_selftransition_multiple`
      exercise `doTransition`, which snapshots `mActiveStates` and iterates over every active state calling
      `handleSingleTransition`, accumulating `acceptedStates` so multiple branches stay independently active.
      This loop is the implementation of concurrent activation.

- [ ] 3. SWR_HSM_029 — Event Delivery to Concurrently Active States (evaluate event against each active state in order).
      File: `src/HsmImpl.hpp`
      Entity: `HsmEventStatus doTransition(const PendingEventInfo& event);`
      Current line: HsmImpl.hpp:179 (same block tag line HsmImpl.hpp:178 as item 2).
      Action: **Extend** the same `doTransition` block `@requirement` line → append `, HSMCPP.SWR_HSM_029`
      (so the line gains both 028 and 029).
      Justification: Test `ABCHsm.parallel_transition_08` relies on `doTransition` iterating the active-states
      snapshot and dispatching the single event to each active state independently (skipping a state whose
      child already accepted via the `isSubstateOf`/`acceptedStates` check). The per-active-state evaluation
      loop is this exact function.

- [ ] 4. SWR_HSM_030 — Idempotent State Entry on Convergence (target already active → no entry/state-changed callback).
      File: `src/HsmImpl.hpp`
      Entity: `HsmEventStatus processExternalTransition(const PendingEventInfo& event, const StateID_t fromState, const TransitionInfo& curTransition, const std::list<StateID_t>& exitedStates);`
      Current line: HsmImpl.hpp:182 (its existing tag line is HsmImpl.hpp:182: `/// @requirement HSMCPP.SWR_HSM_054, HSMCPP.SWR_HSM_056, HSMCPP.SWR_HSM_027`).
      Action: **Extend** the existing line → append `, HSMCPP.SWR_HSM_030`.
      Justification: Tests `parallel_transition_mult2one_01/_02` converge two active states onto an
      already-active target `A`. In `processExternalTransition` the guard+transition callback run, but
      `replaceActiveState(fromState, A)` → `addActiveState(A)` returns `false` because `A` is already active,
      so `onStateChanged` (and the entry-point descent) is skipped. This is where idempotent convergence is
      realized.

- [ ] 5. SWR_HSM_031 — Preservation of Active Substates on Re-Target (already-active composite keeps its subtree).
      File: `src/HsmImpl.hpp`
      Entity: `HsmEventStatus processExternalTransition(const PendingEventInfo& event, const StateID_t fromState, const TransitionInfo& curTransition, const std::list<StateID_t>& exitedStates);`
      Current line: HsmImpl.hpp:182 (same line as item 4).
      Action: **Extend** the same `processExternalTransition` line → append `, HSMCPP.SWR_HSM_031`
      (so the line gains both 030 and 031).
      Justification: Test `ABCHsm.parallel_transition_mult2one_02` re-targets already-active composite `P1`
      (whose child `C` is active). `processExternalTransition` → `replaceActiveState` leaves `P1`/`C`
      untouched (via the `isSubstateOf` guard in `replaceActiveState` and `addActiveState` returning false),
      so the active substate subtree is preserved rather than exited/re-entered. Same primary function as 030.

- [ ] 6. SWR_HSM_076 — Thread-Safe Transition Operations (concurrent external submissions are safe).
      File: `src/HsmImpl.hpp`
      Entity: `bool transitionExWithArgsArray(const EventID_t event, const bool clearQueue, const bool sync, const int timeoutMs, VariantVector_t&& args);`
      Current line: HsmImpl.hpp:94 (declaration currently has **no** `@requirement`).
      Action: **Add** a new line immediately above the declaration:
      `/// @requirement HSMCPP.SWR_HSM_076`.
      Justification: Test `ABCHsm.multithreaded_entrypoint_cancelation` submits transitions from the test
      thread while the dispatcher thread processes them. `Impl::transitionExWithArgsArray` is the single
      external entry point for all submissions; it guards `mPendingEvents` with `HSM_SYNC_EVENTS_QUEUE()`
      (`mEventsSync`) and performs the queue-clear atomically, which (with `clearPendingEvents` treating
      entry-point transitions as atomic) is what makes concurrent submission safe and keeps state-config
      consistent. The public `hsm.cpp` method of the same name is a thin forwarder, so the Impl method is
      the genuine implementer.

- [ ] 7. SWR_HSM_132 — Synchronous Transition Refused Within Execution Context (no block; cancel + report failure).
      File: `src/HsmImpl.hpp`
      Entity: `bool transitionExWithArgsArray(const EventID_t event, const bool clearQueue, const bool sync, const int timeoutMs, VariantVector_t&& args);`
      Current line: HsmImpl.hpp:94 (same declaration as item 6).
      Action: **Extend** the line added in item 6 → make it
      `/// @requirement HSMCPP.SWR_HSM_076, HSMCPP.SWR_HSM_132`
      (if item 6 is applied first, append `, HSMCPP.SWR_HSM_132`; otherwise add the combined line).
      Justification: Test `ABCHsm.transition_sync_deadlock` calls `transitionSync` from inside a state
      callback and expects `false` (not an indefinite block). The sync branch of
      `Impl::transitionExWithArgsArray` (`eventInfo.initLock()` → `eventInfo.wait(timeoutMs)` →
      `status = (DONE_OK == *transitionStatus)`) is what prevents the deadlock from hanging and reports
      failure for the refused in-context sync request. This is the function implementing the sync-request
      behavior the test exercises.

- [ ] 8. SWR_HSM_133 — Self-Transition Execution Priority (internal self-transitions first).
      File: `src/HsmImpl.hpp`
      Entity: `bool executeSelfTransitions(const PendingEventInfo& event, const std::list<TransitionInfo>& matchingTransitions);`
      Current line: HsmImpl.hpp:196 (its existing tag line is HsmImpl.hpp:195: `/// @requirement HSMCPP.SWR_HSM_043, HSMCPP.SWR_HSM_044`).
      Action: **Extend** the existing line → append `, HSMCPP.SWR_HSM_133`.
      Justification: Test `ABCHsm.parallel_selftransition_priority_01` registers both an internal
      self-transition and a regular transition on the same event and asserts the self-transition callback
      runs before the state is exited. `executeSelfTransitions` runs the internal self-transition callbacks
      first, and `handleSingleTransition` calls it before `executeExitTransition`/`processExternalTransition`,
      giving the required ordering. `executeSelfTransitions` is the function most directly responsible for the
      "self first" behavior. (`DISABLED_parallel_selftransition_priority_02` is an unfinished stub for the
      external-self case and does not change this choice.)

- [ ] 9. SWR_HSM_134 — History Entry Without Default Target (fall back to parent entry point).
      File: `src/HsmImpl.hpp`
      Entity: `void transitionToDefaultHistoryState(const StateID_t defaultTarget, const HsmTransitionCallback_t& defaultTargetTransitionCallback, const PendingEventInfo& event, const StateID_t destinationState);`
      Current line: HsmImpl.hpp:207 (its existing tag line is HsmImpl.hpp:207: `/// @requirement HSMCPP.SWR_HSM_038`).
      Action: **Extend** the existing line → append `, HSMCPP.SWR_HSM_134`.
      Justification: Test `ABCHsm.history_no_default` targets history state `H` with no recorded history and
      no default target. `transitionToDefaultHistoryState` takes the `INVALID_HSM_STATE_ID == defaultTarget`
      branch and enqueues a forced transition to the history parent only (`historyTargets.emplace_back(historyParent)`),
      which then resolves the parent's entry point to activate the child. That branch is the implementation of
      this requirement.

- [ ] 10. SWR_HSM_138 — Multiple Concurrent Timers (distinct type-safe timer IDs, independent control).
      File: `src/HsmImpl.hpp`
      Entity: `void startTimer(const TimerID_t timerID, const unsigned int intervalMs, const bool isSingleShot);`
      Current line: HsmImpl.hpp:101 (declaration currently has **no** `@requirement`).
      Action: **Add** a new line immediately above the declaration:
      `/// @requirement HSMCPP.SWR_HSM_138`.
      Justification: Test `ABCHsm.timers_start_higher_priority` registers two timers with distinct
      `TimerID_t` values and starts them independently; `Impl::startTimer` keys each operation by `timerID`
      and forwards it to the dispatcher, so starting the short timer affects only that timer while the long
      timer keeps running. It is the HSM-side entry point the test exercises for concurrent, per-ID timer
      control (stop/restart/isRunning are the sibling methods, but the test drives `startTimer`). The public
      `hsm.cpp` `startTimer` is a thin forwarder, so the Impl declaration is the genuine implementer.

---

## Notes / shared-function overlaps (expected, per task rules)

- Items 2 & 3 both land on `doTransition` (one `@requirement` line gains `SWR_HSM_028, SWR_HSM_029`).
- Items 4 & 5 both land on `processExternalTransition` (one line gains `SWR_HSM_030, SWR_HSM_031`).
- Items 6 & 7 both land on `transitionExWithArgsArray` (one new line carries `SWR_HSM_076, SWR_HSM_132`).

## Blockers

None. All 10 implementing functions were located by reading the sources and the named tests.
