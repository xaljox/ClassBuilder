# Plan: iterator `IsFirst()` / `IsLast()` from `int` to `bool`

Status: **analysis done, not yet executed** (JV 2026-07-17: "not a job for now").

## Analysis (facts, verified 2026-07-17)

**Definitions.** `int IsLast()` / `int IsFirst()` exist in exactly two runtime
headers, as macro-generated inline methods of the relation iterators:

- [include/CB_IteratorMulti.h](../include/CB_IteratorMulti.h) — 4 iterator
  variants (lines ~163, ~359, ~496, ~712)
- [include/CB_IteratorStaticMulti.h](../include/CB_IteratorStaticMulti.h) — 2
  variants (lines ~77, ~246)

12 method definitions total (6× `IsLast`, 6× `IsFirst`). These are the ONLY
`int`s in those two headers; every other predicate in the runtime headers
(`IsCritical...`) already returns `bool`, so `bool` is established style there.

**Bodies are already boolean.** Every definition returns a pointer comparison
(`GetLast() == _ref`), which is a `bool` in C++ — the `int` in the signature
is the only legacy part. Zero behavior change.

**Call sites.** 31 in the repo (src/model + src/qt), found via
`grep -rn '\.Is\(Last\|First\)()' src/`. **All** appear in boolean context
(`if (...)`, `!...`); **none** binds the result to an `int` variable, none
takes a member-function pointer to them. So: no call-site edits needed, the
model's stored method bodies (ClassBuilder.CBZ) need no changes.

**Codegen is untouched.** The code generator emits `CB_*` macro *invocations*;
it never emits the iterator method signatures themselves (they expand from the
headers at compile time). Regenerating sources after the change must produce a
zero diff. The model has no modeled representation of the iterator API either
(that is why the editor carries the hardcoded `kIteratorOwnMethods` table).

**Downstream projects** (colleague projects using CB-generated code) receive
the change when they next update their copy of the runtime headers. Source
compatible: `bool` converts implicitly to `int`, `IsLast() == 1` still holds
(`true` promotes to 1), varargs (`printf("%d", ...)`) promote fine. Only a
member-function-pointer of type `int (Iter::*)()` would break — none found
here; flag it in the release note for downstream.

## Phases

1. **Header edit** — 12 signatures `int Is...` → `bool Is...` in the two
   headers above. Mechanical; the bodies stay as they are.
2. **Editor knowledge** — in
   [src/qt/ModelCompletionProvider.cpp](../src/qt/ModelCompletionProvider.cpp),
   `kIteratorOwnMethods`: the `IsFirst` / `IsLast` entries' `type` from
   `"int "` to `"bool "` (hover + completion detail then show the new
   signature).
3. **Rebuild + self-host verify** — full rebuild (the headers sit under the
   PCH, so effectively everything recompiles, ~minutes). Then open
   `src/model/ClassBuilder.CBZ` in CB, regenerate sources, and confirm a
   **zero git diff** (proves codegen never carried the signature). Smoke-test
   an iterator loop (e.g. `Matrix.CBZ`, `iX.IsLast()` hover shows `bool`).
4. **Cross-platform recompile** — plain rebuild on Windows/macOS/Linux; the
   change is header-only, tracked in git, either side can execute it.
5. *(Optional, separate decision)* — sweep the remaining `CB_*` headers for
   other int-as-bool signatures: none found today (`IsCritical` is already
   `bool`), so this phase is expected empty. The model's own `BOOL` usage
   stays out of scope per the CLAUDE.md convention (no mass rewrite).

## Risk assessment

**Low.** No ABI concern (all inline, single exe), no wire-format concern (not
serialized), no model/codegen concern (zero-diff check in phase 3 proves it),
no call-site rewrites. The only real cost is the full recompile and the
self-host verification round.
