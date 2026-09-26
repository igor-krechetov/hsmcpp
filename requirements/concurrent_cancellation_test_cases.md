# Test Cases: Concurrent Transitions, Cancellation, Idempotent Entry, and History

Working notes to be converted into a formal test specification later. Covers the
corner cases explored while drafting SYS-HSM-076, SWR-HSM-125, SWR-HSM-126, the
SWR-HSM-027 cross-reference, and the SWR-HSM-093/095 always-current history
recording revision.

## Notation

- `*X`  = state X is active AND fired its entry + state-changed callbacks this cycle (freshly entered).
- `X`   = state X is active but was NOT freshly entered this cycle (no entry/state-changed fired).
- `X{...}` = composite state X with its active substate configuration in braces.
- `o-> Y` = initial substate of a composite is Y.
- A branch "cancels" = its target entry callback returns a cancellation indication (SWR-HSM-031),
  after the source has already been exited.
- Active states are tracked per-state; the "machine" is just the collection of active states
  (there is no separate machine-level state-changed callback).

## Requirements under test

- SYS-HSM-017 / SWR-HSM-021 - execute ALL transitions whose guard is satisfied (emergent concurrency).
- SYS-HSM-024 / SWR-HSM-030 / SWR-HSM-031 - transition cancellation via exit/entry callbacks.
- SWR-HSM-027 - restore source on cancellation after exit; re-enter fires onEnter + state-changed;
  restoring an already-active shared source is a no-op (per SWR-HSM-125).
- SWR-HSM-105 - cancelling one concurrent branch does not affect sibling branches.
- SYS-HSM-076 / SWR-HSM-125 - idempotent entry: a transition targeting an already-active state
  runs its guard/exit/transition callbacks but NOT the target's entry/state-changed callbacks.
- SWR-HSM-126 - re-target of an already-active COMPOSITE state preserves the entire active
  substate subtree (no re-init to initial substate). Explicit reset only via external
  self-transition (SWR-HSM-020).
- SWR-HSM-093 / SWR-HSM-095 - always-current history recording (reflects last-active child/subtree
  regardless of whether the parent was formally exited).

---

## TC-1: Basic concurrent fan-out, all branches succeed

Model:
```
A -e1-> B
A -e1-> C
```
- Initial: `*A`
- Fire e1.
- Expected: `*B, *C` (A exited; B and C each entered once). A not active.
- Verifies: SYS-HSM-017 execute-all-satisfied.

## TC-2: Concurrent fan-out, one branch cancels (shared source)

Model:
```
A -e1-> B
A -e1-> C
```
- Initial: `*A`
- Fire e1. A->B succeeds; A->C entry callback cancels.
- Branch independence (SWR-HSM-105): C branch resolves alone.
- A is a SHARED source. A->B succeeded and left A; A->C cancels and would restore A.
- Expected (current ratified behavior, SWR-HSM-105 + SWR-HSM-027): `*A, *B`, C not active.
  A is restored (re-entered) by the cancelled C branch even though the B branch left it.
- Edge/known wart: the failed sibling re-activates the shared source. See TC-3 for the
  multi-event consequence.
- Verifies: SWR-HSM-105, SWR-HSM-027, SWR-HSM-025 (failure notification distinguishes
  cancellation from rejection).

## TC-3: Repeated e1 with C cancelling twice then accepting (shared-source stickiness)

Model:
```
A -e1-> B
A -e1-> C
```
- Initial: `*A`
- e1 #1: A->B succeeds (`*B`); A->C cancels -> restore A (`*A`). Config: `*A, *B`.
- e1 #2: A still active (shared source). Both branches fire again.
  - A->B targets B, which is ALREADY ACTIVE -> SWR-HSM-125: transition callback runs,
    but B entry/state-changed SUPPRESSED. B NOT re-entered. (Without SWR-HSM-125 this
    would spuriously re-enter B every retry - the bug the rule fixes.)
  - A->C cancels -> restore A. Config: `*A, B` (B active, not freshly entered).
- e1 #3: A->C ACCEPTS -> `*C`. A->B again targets active B -> suppressed.
  Both branches now leave A (B branch) / complete (C branch); A restored by neither.
- Expected final: `*C, B` with A gone.
- Verifies: SWR-HSM-125 prevents B re-entry across retries; A only persists while C keeps failing.

## TC-4: Diamond convergence - target activated once

Model:
```
A -e1-> B -e3-> C
A -e2-> D -e4-> C
```
- Precondition: A fanned out so `*B, *D` both active.
- e3: `B -e3-> C`. D ignores e3. B exits, C enters. Config: `*C, D`.
- e4: `D -e4-> C`. C ignores e4. D exits, C already active ->
  SWR-HSM-125: D->C transition callback runs; C entry/state-changed SUPPRESSED.
- Expected final: `*C` (entered exactly once), D gone.
- Verifies: SYS-HSM-076 / SWR-HSM-125 join semantics; transition callback on the
  second edge still observable, target entry not duplicated.

## TC-5: Composite re-target preserves internal progress

Model:
```
A -e1-> B { o-> B1 -e2-> B2 }
A -e1-> C
```
- e1 #1: A->B enters B, drills to B1 (`*B{*B1}`); A->C cancels -> restore A.
  Config: `*A, *B{*B1}`.
- e2: `B1 -e2-> B2`. Config: `*A, *B{*B2}` (B advanced internally; B not re-entered).
- e1 #2: A->C cancels again; A->B targets already-active B.
  - Expected (SWR-HSM-126, option #1): B and its subtree left UNCHANGED -> `B{B2}` preserved.
    NO rewind to B1. B.onEnter/B1.onEnter NOT re-run.
  - Rejected alternative (option #2): re-initialize B to B1 - would discard e2 progress.
    Only legitimate via an explicit external self-transition (SWR-HSM-020).
- Expected final config after e1 #2: `*A, B{B2}` (A restored by the cancelled C branch;
  B unchanged; C not active).
- Verifies: SWR-HSM-126 whole-subtree preservation; carve-out for SWR-HSM-020.

## TC-6: Explicit reset via external self-transition (contrast to TC-5)

Model:
```
B { o-> B1 -e2-> B2 }
B -e5-> B   (external self-transition, SWR-HSM-020)
```
- Precondition: `*B{*B2}`.
- e5: external self-transition performs FULL exit + re-entry of B.
- Expected: B exited (B2, then B exit callbacks), B re-entered, drilled to initial B1.
  Config: `*B{*B1}`.
- Verifies: SWR-HSM-020 reset path is NOT suppressed by SWR-HSM-125/126 (external
  self-transition explicitly restarts the target).

## TC-7: Internal self-transition does not exit/enter (control)

Model:
```
B -e6-> B   (internal self-transition, SWR-HSM-019/SYS-HSM-016 internal mode)
```
- Precondition: `*B{*B2}`.
- e6: internal self-transition; transition callback runs, no exit/entry.
- Expected: `B{B2}` unchanged, no entry/exit callbacks, transition callback fired once.
- Verifies: internal-mode self-transition semantics; distinct from TC-6.

## TC-8: History restore when parent never left (idempotency composes with history)

Model:
```
A -e1-> H(P)         ; H(P) = history state of composite P, resolves to last-active child
A -e1-> C
P { o-> P1 -e2-> P2 }
```
- e1 #1: A->H(P) enters P, resolves history -> initial P1 (no prior record). A->C cancels -> restore A.
  Config: `*A, *P{*P1}`.
- e2: `P1 -e2-> P2`. Config: `*A, *P{*P2}` (parent P NOT exited).
- e1 #2: A->C cancels again; A->H(P) fires. H is an actual state that auto-transitions onward.
  - H tries to restore P's active configuration. Since P was never left, P is already active.
  - Resolved target P is ALREADY ACTIVE -> SWR-HSM-125/126: P{P2} preserved, no rewind.
- Expected final: `*A, P{P2}` (A restored by cancelled C branch; P unchanged; C not active).
- Verifies: history restoration composes with idempotent entry; the resolved concrete
  state (P), not the transient routing through H, is what the already-active test applies to.

## TC-9: Always-current history recording (the recording-trigger fix)

Model:
```
P { o-> P1 -e2-> P2 -e7-> P3 }
Q                                  ; some other top-level state
P -e8-> Q                          ; leaves P
Q -e9-> H(P)                       ; deep or shallow history re-entry of P
```
- Sequence: enter P (`*P{*P1}`) -> e2 -> `P{P2}` -> e7 -> `P{P3}`.
- e8: leave P for Q. Under OLD record-on-exit this snapshots P3 at exit (coincidentally correct here).
- KEY VARIANT proving the fix: interleave a period where P's child changes while P stays
  active across unrelated events, then leave P. Under always-current recording (SWR-HSM-093/095)
  the recorded child must be the LAST active child (P3), regardless of when/whether the parent
  was exited between changes.
- Adversarial variant: if the parent is re-entered via H WHILE still active (TC-8 style),
  the recorded value must never be a stale entry-time default (P1); it must reflect P3.
- e9: Q -e9-> H(P). Restore.
  - Shallow: restore last active DIRECT child of P.
  - Deep: restore full active substate hierarchy of P.
- Expected: restoration lands on the truly last-active configuration (P3 / deep subtree),
  not the initial substate and not a stale exit-time snapshot.
- Verifies: SWR-HSM-093 (shallow) and SWR-HSM-095 (deep) always-current recording -
  recorded value reflects the child/subtree active immediately before its most recent
  deactivation, independent of parent exit timing.

## TC-10: History clearing on release/re-init (regression guard)

- After recording history, release or re-initialize the state machine.
- Expected: recorded shallow (SWR-HSM-094) and deep (SWR-HSM-096) history cleared;
  next entry via history resolves to initial substate, not a pre-release value.
- Verifies: always-current recording did not accidentally bypass the clearing lifecycle.

---

## Open verification points (decide during test-spec authoring)

1. Ordering of restoration re-entry vs sibling entry in the SAME event fan-out
   (inter-branch determinism). SWR-HSM-029 fixes intra-branch order; inter-branch
   interleaving of a restoration onEnter against a sibling onEnter is not yet pinned.
2. Transition callback firing on a suppressed-entry edge (TC-4/TC-5): confirmed SHOULD fire
   per SWR-HSM-125; add explicit assertion that its side effects are observed exactly once.
3. Shared-source outcome choice: current spec = TC-2/TC-3 "A restored by failed sibling".
   If the alternative "successful sibling consumes the shared exit, A gone" is ever adopted,
   TC-2/TC-3 expected configs change - keep this test file in sync with that decision.
