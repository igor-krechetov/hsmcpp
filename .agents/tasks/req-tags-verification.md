# Requirement Tagging Verification — Bucket 1 (10 SW requirements)

Iteration: first iteration (no `req-tags-review.json` present when this ran).

All changes are Doxygen `@requirement` comment tags only. No runtime logic,
signatures, or behavior were changed. Generated files under
`build/requirements/` were produced by the regeneration recipe, not edited by
hand. No git commit was created.

## 1. BEFORE / AFTER linked count

- Metric: distinct `SWR_HSM_*` IDs present in `build/requirements/code.lobster`
  (= "SW with Code link"), measured with
  `grep -oE 'SWR_HSM_[0-9]+' build/requirements/code.lobster | sort -u | wc -l`.
- BEFORE: **87** of 138.
- AFTER:  **97** of 138.
- Delta: **+10**, exactly the 10 target IDs. Rendered doc:
  `Wrote build/requirements/hsmcpp_requirements.html (77 SYS, 138 SW, 16 terms; with code/test tracing)`.

## 2. Regeneration path used

Full toolchain was available (`just`, `doxygen`, `trlc`, `python3`,
`lobster-report`, `lobster-trlc` all on PATH).

- Ran `just req_build full` from the repo root. It executed
  `doxygen config/lobster/Doxyfile`,
  `python3 scripts/requirements/doxygen_to_lobster.py --all build/doxygen/xml build/requirements/code.lobster`,
  the test-tag scan, and `render_html.py`. Completed exit 0 (only pre-existing
  `@threadsafe` / `@rstref` unknown-command doxygen warnings, unrelated to this change).
- Re-ran `just req_build full` a second time AFTER clang-format reflowed the
  `getEntryPoints` tag line, to confirm the wrapped `///` comment is still parsed
  correctly. Result unchanged: 97 distinct IDs, SWR_HSM_018 and SWR_HSM_025 both present.
- Also ran `just req_coverage` (depends on `req_build full` + `test_trace`): built,
  ran the `_trace` test suite (`hsmUnitTestsSTD_trace` Passed, 1/1), merged GTest XML
  (269 unique tagged tests), produced `report.lobster`, and rendered
  `build/requirements/hsmcpp_coverage.html`. Completed exit 0.

## 3. The 10 IDs — tagged entity and source location

All tags live on declarations in `src/HsmImpl.hpp` (per repo convention:
`HsmImpl.cpp` carries no inline `@requirement` tags; tags live on the `.hpp`
declaration). Line numbers are post-clang-format.

| # | Requirement | Tagged entity | Tag line (HsmImpl.hpp) | Action |
|---|-------------|---------------|------------------------|--------|
| 1 | SWR_HSM_018 | `Impl::getEntryPoints` | 242–243 (wrapped `///` line) | extended existing SWR_HSM_020..025 line |
| 2 | SWR_HSM_028 | `Impl::doTransition` | 179 (`/** */` block) | extended existing SWR_HSM_045, _057 line |
| 3 | SWR_HSM_029 | `Impl::doTransition` | 179 (`/** */` block) | extended same line (gains 028 + 029) |
| 4 | SWR_HSM_030 | `Impl::processExternalTransition` | 183 | extended existing SWR_HSM_054, _056, _027 line |
| 5 | SWR_HSM_031 | `Impl::processExternalTransition` | 183 | extended same line (gains 030 + 031) |
| 6 | SWR_HSM_076 | `Impl::transitionExWithArgsArray` | 94 | added new `/// @requirement` line |
| 7 | SWR_HSM_132 | `Impl::transitionExWithArgsArray` | 94 | same new line (carries 076 + 132) |
| 8 | SWR_HSM_133 | `Impl::executeSelfTransitions` | 197 | extended existing SWR_HSM_043, _044 line |
| 9 | SWR_HSM_134 | `Impl::transitionToDefaultHistoryState` | 210 | extended existing SWR_HSM_038 line |
| 10| SWR_HSM_138 | `Impl::startTimer` | 102 | added new `/// @requirement` line |

Note: SWR_HSM_076 and SWR_HSM_132 also resolve onto `Impl::transitionSimple`
via the pre-existing `/// @requirement_wrapper transitionExWithArgsArray`
directive on that method — this is the repo's existing wrapper mechanism, not a
new tag, which is why those two IDs show a count of 2 in code.lobster.

## 4. Grep proof — all 10 IDs present in code.lobster

Command: `grep -c "SWR_HSM_<id>\b" build/requirements/code.lobster`

```
SWR_HSM_018: 1
SWR_HSM_028: 1
SWR_HSM_029: 1
SWR_HSM_030: 1
SWR_HSM_031: 1
SWR_HSM_076: 2
SWR_HSM_132: 2
SWR_HSM_133: 1
SWR_HSM_134: 1
SWR_HSM_138: 1
```

Entities resolved (from code.lobster `refs` → `location`):

```
src/HsmImpl.hpp:416  Impl::transitionExWithArgsArray      -> SWR_HSM_076, SWR_HSM_132
src/HsmImpl.hpp:499  Impl::startTimer                     -> SWR_HSM_138
src/HsmImpl.hpp:632  Impl::transitionSimple (wrapper)     -> SWR_HSM_076, SWR_HSM_132
src/HsmImpl.hpp:1062 Impl::doTransition                   -> SWR_HSM_028, SWR_HSM_029
src/HsmImpl.hpp:1126 Impl::processExternalTransition      -> SWR_HSM_030, SWR_HSM_031
src/HsmImpl.hpp:1261 Impl::executeSelfTransitions         -> SWR_HSM_133
src/HsmImpl.hpp:1420 Impl::transitionToDefaultHistoryState-> SWR_HSM_134
src/HsmImpl.hpp:1578 Impl::getEntryPoints                 -> SWR_HSM_018
```

All 10 IDs are present and tied to a real implementing function entity.

## 5. req_coverage

Available and run (see section 2). `hsmcpp_coverage.html` rendered successfully.

## 6. clang-format

`clang-format` (unversioned) is not on PATH and the repo has no `format.sh`; the
repo's formatting script is `scripts/local/apply_clang_format.sh`. Ran the
equivalent with `clang-format-14` over `src/`, `examples/`, `tests/` (the same
file globs the script uses). This reflowed the long `getEntryPoints`
`@requirement` line onto a continuation `///` line; re-running `just req_build full`
confirmed doxygen still parses both IDs on that wrapped comment (SWR_HSM_018 and
SWR_HSM_025 both still linked, count still 97). clang-format also normalized
pre-existing formatting drift across other files in the working tree; those are
not part of this change's logic. No commit was created.
