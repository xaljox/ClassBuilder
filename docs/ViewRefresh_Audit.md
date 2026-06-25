# View refresh / lock / dirty-flag audit (2026-06-12)

Requested by JV: "the locking / unlock / update-view stuff has multiple tastes;
things were added to solve problem after problem — have a thorough look."
This is the inventory, the inconsistencies, and a target design with safe
migration steps. Analysis only — nothing here is implemented beyond what is
explicitly marked DONE.

## Inventory — every mechanism that refreshes a view today

| # | Mechanism | Where | What it does |
|---|-----------|-------|--------------|
| 1 | `CClassBuilderDoc::UpdateAllViews()` | ClassBuilderDoc.cpp | The full-refresh funnel: rebuilds every tree mirror (synchronously), posts a repaint to every SD/CD canvas, fires `notifyStateChanged`. Lock-aware (defers while locked). The MFC hint channel (`MOD_*`) is dead since the Qt mirrors rebuild wholesale. ~24 model call sites via the DataModelDoc forwarder. |
| 2 | `LockAllViews` / `UnLockAndUpdateAllViews` | ClassBuilderDoc.cpp | Counting lock + dirty-while-locked flag; outermost unlock fires ONE coalesced `UpdateAllViews`. 9 raw call-pairs remain (Undo/Redo/RollBack paths, open/new-document, `Qt_MainTreeDrop`). |
| 3 | `CbViewLock` (RAII) | CbViewLock.* (model class) | Ctor: wait cursor + `LockAllViews`; dtor: `UnLockAndUpdateAllViews` + arrow cursor. 25 users. Since 2026-06-12 it takes `DataModelDoc*` (was `CClassBuilderDoc*`). |
| 4 | `DataModelDoc` forwarders | DataModelDoc.cpp | **DONE 2026-06-12:** the single model→framework seam (`GetDocument()` is now private). `UpdateAllViews / LockAllViews / UnLockAndUpdateAllViews / DeferViewRefreshWhileLocked / SetModifiedFlag / NotifyStateChanged / CanUndo / CanRedo / GetTitle / GetPathName / OnSaveDocument`. |
| 5 | Per-diagram direct refresh | ClassDiagram.cpp / SequenceDiagram.cpp | `Update{Class,Sequence}DiagramViews()` — called **directly by ~16 setters**, bypassing funnel #1. Gated by `DeferViewRefreshWhileLocked()` so CbViewLock still coalesces them (the relation-slowness fix). |
| 6 | Tree-only refresh | `DataModelDoc::UpdateTreeViews()` | Fires each open `MainTreeQtView`'s Refresh callback → **synchronous** full `rebuild()` of the QTreeWidget. |
| 7 | Canvas posted repaints | Qt canvases | `RefreshCanvas` → `QWidget::update()` (async, safe from any model path) + `selectionChanged` / `editActionsChanged` emits → per-view toolbar enables. |
| 8 | `notifyStateChanged` fan-out | QtShellWindow | Shell titles + Save enable + **(2026-06-11)** by-name `refreshUndoRedoEnables` on every open view, so undo/redo buttons agree across views of one model. |
| 9 | Draw guards | DataModelDoc (BeginDraw/EndDraw), UndoBase | `_isDrawing`: undo entries created during a paint are collected and purged by EndDraw; **DONE 2026-06-12:** `SetModifiedFlag` is also gated, so a paint can no longer dirty the doc (the "opening a view shows `*`" bug). |
| 10 | Undo/Redo forced refresh | CClassBuilderDoc::Undo/Redo | Sets `m_viewsDirtyWhileLocked = true` manually so the end-of-op refresh ALWAYS happens (snapshot-restores bypass setters). |
| 11 | Belt-and-braces canvas calls | Qt canvases | Several handlers call `_pCD->UpdateClassDiagramViews(); update();` right after ops that already refresh via #1/#5. Harmless (posted, idempotent) but noise. |

## The inconsistencies ("tastes")

1. **Two refresh philosophies coexist.** Diagram-geometry setters self-refresh
   per diagram (#5); structural ops refresh everything (#1); `Gti::Add/Update`
   do #1 *plus* set the dirty flag. Which views refresh after a mutation
   depends on which class the setter happens to live in.
2. **Dirty-flag policy is scattered.** `SaveState` sets it; `Gti::Add/Update`
   set it; ~6 `OnEditContext` handlers compare `MarkLastUndo` before/after and
   set it manually; some Track-commit setters set it directly.
3. **Cost asymmetry hidden behind one name.** "Refresh" means a posted, cheap
   `update()` for canvases but a synchronous, expensive full rebuild for trees.
   The lock machinery (#2/#3) exists almost entirely to amortize the tree cost.
4. **Reentrancy hazard.** A mutation during a paint (e.g. `Gti::Add` from a
   draw-time relayout) calls `UpdateAllViews` → synchronous tree rebuild
   *inside a paintEvent*. Rare, but nothing prevents it.
5. **Raw lock pairs vs RAII.** 25 sites use `CbViewLock`; 9 use raw
   Lock/Unlock (some because they must set the manual dirty flag of #10,
   some historical).

## Target design (proposal)

One model-side API, everything else private:

- `CbViewLock` (keep, RAII, the only locking primitive).
- `DataModelDoc::RequestRefresh(scope)` with `scope ∈ {Tree, Diagram(d), All}`:
  - under a lock: accumulate scope, refresh once at outermost unlock;
  - unlocked: **post** (queued), never synchronous — uniform cost model,
    kills the in-paint hazard;
  - subsumes #1, #5, #6 and retires `DeferViewRefreshWhileLocked` as a
    separate concept.
- Dirty flag set in exactly two places: `SaveState` (undoable change) and
  Add/Delete. Everything else inherits; the manual `OnEditContext` pattern
  and direct setter calls go away.
- Rule going forward: **model mutators never call view code; they Request.**

### INCIDENT 2026-06-12 evening: the Select-Classes crash (UAF in the undo stack)

Symptom: Select Classes (place all) → Select Classes (drop one) → click the
CD canvas → access violation in `CClassBuilderDoc::CanUndo`'s inner
`dynamic_cast`. Debug heap made it deterministic; JV's debugger work pinned
it: a perfectly LIVE session `UndoNew` whose referenced object was 0xDD
(freed). Root cause chain:

1. Select Classes #1 (user op, no draw): segments created → session
   `UndoNew(segment)` entries on the real stack. Correct.
2. A later paint reroutes: `MakeNewRouting` INSIDE Draw deletes those
   session-born segments → draw-time `UndoDelete(segment)` → collected on
   the static purge list. Correct.
3. `EndDraw` purge: `~UndoDelete` FREES the segment — designed for
   born-and-died-within-one-paint churn, but these were born in a USER op,
   so their session `UndoNew` entries now dangle. The purge is the ONLY
   mechanism that deletes undo entries out of LIFO order; LIFO protects
   every other path.
4. The same `EndDraw`'s `NotifyStateChanged` → `CanUndo` walk reads the
   corpse. (Release: heap lottery — freed block often still readable, which
   is why "stable doesn't crash" misled the whole bisect. NEVER bisect a
   Release UAF by behavior; go to the debug heap first.)

Structural cause, in JV's own words from that afternoon's design review:
`UndoBase::_pDataModelDocObject` is a RAW POINTER, not a relation — the
object's destruction is invisible to the undo machinery (no backpointer).
The entry↔doc link IS a relation and cleaned up flawlessly throughout.

FIX (verified in Debug by JV, same session): `~UndoDelete` and
`~UndoSubDelete` now sweep the doc's undo AND redo stacks BEFORE freeing
the object, deleting every other entry that references it (restoring such
an entry would be the same UAF; removal is the only consistent state).
Sweep sits before `DestructorInclude` (doc relation still wired);
delete-via-iterator per RollBack's existing pattern.

FOLLOW-UPS:
- Proper fix: relation-ize `UndoBase ↔ DataModelDocObject` so object death
  voids entries automatically (design care: null currently MEANS
  UndoChangeDoc). Supervised session.
- PageHeap (`gflags /p /enable ClassBuilder.exe /full`) on STABLE to
  confirm it carries the latent bug → then refresh stable from the fixed
  build.
- SelectClassesDialog::checkItem passes a just-Delete()d shape pointer back
  into FindClassShape as the search cursor — works today (object alive on
  the undo stack), but it is a stale-cursor smell worth cleaning.
- Derived-refresh phases 1+2 were REVERTED during the hunt but are
  EXONERATED by this root cause (the crash predates them and survives their
  removal). Re-doing them is a clean decision for a fresh session — all
  design + review notes above remain valid.

## Closing note (2026-06-12, after steps 1-5): the unlocked-direct-call question

JV's last review question: the ~30 direct `UpdateAllViews()` calls that run
WITHOUT a `CbViewLock` — are they a problem? **No, by construction since
step 2.** Unlocked, `UpdateAllViews` does nothing synchronous to the views:
tree rebuilds are queued + coalesced (`scheduleRebuild`), canvas repaints are
posted idempotent `update()`s, and since mutations run on the main thread,
none of that posted work can execute until the whole operation returns —
an unlocked cascade of K calls still yields ONE rebuild per view, after the
model is consistent. Only `notifyStateChanged` (cheap) runs K times.

`CbViewLock`'s remaining value: the wait cursor, suppressing the
notifyStateChanged churn, the diagram setters' defer gate, and declaring
"this is a transaction". The original hazard it existed for — K synchronous
tree rebuilds (the 40 s rename) — is structurally impossible now even
without it. RESULTING RULE: single mutation → direct `UpdateAllViews()`
(wrapping it would add cursor flicker); multi-object op → `CbViewLock`;
a forgotten lock is cosmetic, not a performance bug.

The enforcement end-state achieved (beyond the original proposal, on JV's
closure reviews): `GetDocument`, `LockAllViews`/`UnLockAndUpdateAllViews`
(friend `CbViewLock`), and `UpdateTreeViews` (friend `CClassBuilderDoc`)
are all PRIVATE on `DataModelDoc`; `SetModified` alias deleted. The
public surface expresses only intent: `NotifyStructureChanged` (structural;
renamed from `UpdateAllViews` 2026-06-12, CB's set_method_name rewrote all
~30 stored bodies itself), `NotifyDiagramChanged` (geometry gate; renamed
from `DeferDiagramRefreshWhileLocked`), `SetModifiedFlag` (dirty funnel),
`NotifyStateChanged` (toolbar sync). `CbViewLock` gained
`busyCursor = true` ctor flag (false = single-edit variant, no cursor flip).

## NEXT DESIGN (proposal only — JV review required before any code):
## Derived refresh — delete the scattered calls entirely

JV's verdict on the rename (correct): `NotifyStructureChanged` is STILL a
redraw with a flag-setting special case — most call sites run unlocked, so
the call performs the funnel walk inline. Renaming changed nothing
fundamental. The disease is that ~30 call sites each independently decide
"views must update" — the same scattered-policy smell the dirty flag had,
and it deserves the same cure: derivation at chokepoints.

**Why the scattered calls exist (JV, historical):** Undo/Redo was added to
an already-working tool — the redraw kicks predate it. Each mutation HAD to
kick its own refresh because nothing else knew a change happened. The undo
machinery, by needing to record every mutation faithfully, then built
exactly the change-detection layer the kicks always lacked — but the kicks
were never retired. This migration completes that evolution: "you do the
one thing that must be done for undo anyway, and get consistent update
behavior via that same path." Refresh becomes the undo system's second
consumer; the undo stack is the single source of truth for "something
changed" (the dirty flag became its consumer this morning, refresh now
follows). AND THE CONVERSE (JV): a view that fails to update is no longer a
hunt for a missing refresh call — it is direct evidence of a missing
SaveState, i.e. a broken undo for that operation. The visible symptom
points at the root cause; fixing the undo fixes the refresh for free.

**Recording — pure flag-sets at the chokepoints that already own the dirty
flag, nothing else:**

| Chokepoint | Flag derived |
|---|---|
| `DataModelDocObject::SaveState` / `UndoNew` ctor / `Delete` | object is a `Gti` (tree-visible) → `structureDirty`; non-`Gti` shape → `diagramsDirty` |
| `Gti::Add` / `Gti::Remove` | `structureDirty` |
| `DataModelDoc::Undo/Redo` (replays bypass SaveState) | derived PER RESTORED ENTRY, same rule: `UndoBase::GetDataModelDocObject()` is a `Gti` → `structureDirty`; all entries non-`Gti` shapes → `diagramsDirty` only; `UndoChangeDoc` (doc-settings snapshot) → conservative full. Max over the batch wins. (JV review fix 2026-06-12: the first draft said blanket `structureDirty` — wrong, an undone note-move is geometry-only. NOTE this also improves on TODAY's behavior, which full-refreshes every undo.) |

**Classification predicates (JV, completing the design):** the object kinds
behind the rule — CD-side: `ClassDiagramShape` descendants, `NoteShapePoint`,
`ConnectionSegment`; SD-side: `SequenceDiagramShape` descendants,
`SDNoteShapePoint`, plus the SD colour-template carriers. Add Gti-style
predicates `IsCDObject()` / `IsSDObject()` (cheap virtuals — the inheritance
trees make the tests simple), used alongside `IsGti()`. **First round:** both
may simply be the inverse of `IsGti()` — one `diagramsDirty` flag, repaint
all open canvases; that functions.

**Domain boundary (JV):** the classification only ever applies to
`DataModelDocObject` descendants, because the chokepoints (SaveState /
UndoNew / Delete) are DataModelDocObject machinery — nothing else can reach
them. Objects outside that hierarchy (ViewModels,
`*ViewModelSelection`, selection state) are not saved, not undoable, and
correctly NOT part of this mechanism: their visual feedback is the canvas's
own local repaint (selectionChanged), not a document refresh. Not being a
DataModelDocObject already says everything needed. **Later precision, already enabled by the
predicates:** split into `cdDirty` / `sdDirty` so a note-point drag repaints
only CD canvases and an activation move only SD canvases — same chokepoints,
finer flags, zero new call sites.

**Derivation APPROVED by JV (2026-06-12 review)** with the decisive safety
argument: refresh correctness rides on UNDO correctness. When a shape edit
genuinely affects its origin object (e.g. a Method behind a SignalShape),
that origin must receive its own SaveState anyway for undo to work — and
that SaveState is what records `structureDirty`. A missed tree update would
therefore require a missing SaveState, i.e. an undo bug that already exists
and is caught by the most-exercised invariant in the app. Failure asymmetry
is benign: an oddball object (neither tree nor diagram) at worst costs one
spurious posted canvas repaint, never a stale tree. Finer per-type
classification rejected: complexity without gain — "the pain is in the
tree, and those are all Gti by definition."

**Acting — exactly ONE private place may repaint:**
- lock open → outermost unlock reads flags, flushes once (as today);
- no lock → ONE queued event-loop flush (lands after the running operation;
  mutations hold the main thread). CONSTRAINT discovered the hard way: the
  queued variant CANNOT live in ClassBuilderDoc.cpp (Qt quarantine — no Qt
  includes in the main target). It must route via CbPlatformCompat (a
  `Cb_PostToMainLoop`-style primitive with doc-lifetime cancellation) or a
  shell-registered hook like notifyStateChanged.
- `structureDirty` → full funnel; only `diagramsDirty` → canvases only.

**Deletions (the payoff):** all ~30 `NotifyStructureChanged()` call sites
removed (not renamed — removed); the method leaves the model's public API;
eventually the 16 setter calls to `Update*DiagramViews` too (a shape
setter's own SaveState records `diagramsDirty`). "Forgot the refresh" and
"refreshed mid-mutation" become unwritable.

**Hard cases to settle in review:**
1. ✅ RESOLVED (JV, 2026-06-12 review): the SaveState-less version-compaction
   loops behind `DataModelDialog::markModelChanged` are NOT deliberate — an
   omission. Fix: give them proper SaveState (per touched `Gti` for the
   `SetInitialVersion`/`SetVersion` loops, plus `DataModelDoc::SaveState`
   for the doc-level `_version`), making compaction undoable like any edit.
   Consequence: `markModelChanged`'s manual dirty-set becomes redundant and
   is deleted — the derived-refresh design then has ZERO exceptions; every
   mutation in the program flows through the chokepoints.
2. `ReadAllFiles` / bulk pipe edits — verify the top-level SaveState's flag
   survives the whole pass and yields exactly one flush.
3. Sweep check: any current call site with NO chokepoint on its path is
   either dead code or a latent bug TODAY — list them one by one during
   migration, don't bulk-delete blind.
4. Draw-time guards: the flags must respect `_isDrawing` exactly like the
   dirty flag does (paint-neutrality holds).

### Migration steps (each independently shippable)

1. ✅ DONE 2026-06-12 — all framework access behind `DataModelDoc` forwarders;
   `GetDocument()` private; paint guards (undo + dirty + state-broadcast: a
   draw can neither dirty the doc, leave undo entries, NOR publish transient
   stack state — `NotifyStateChanged` is `_isDrawing`-gated and `EndDraw`
   broadcasts once after an actual purge, which fixed the stale-enabled Undo
   buttons after opening a diagram view).
3. ✅ DONE 2026-06-12 (user-approved as the safe step) — every raw
   Lock/Unlock pair converted to `CbViewLock`: `DataModelDoc::Undo/Redo`
   (block-scoped so the dtor refresh still runs while `_isUndoing/_isRedoing`
   is set), `RollBack` (silent path = the designed-for null lock),
   `CClassBuilderDoc::Undo/Redo` (manual `m_viewsDirtyWhileLocked` set inside
   the RAII scope), the two new-document Init blocks, `Qt_MainTreeDrop`.
   Only the two `DataModelDoc` forwarder bodies still call the raw
   primitives — they ARE the primitive now. Smoke-verified: view-open clean,
   real edit arms undo, build green.
2. ✅ DONE 2026-06-12 (user GUI-verified same day) — the model-fired
   `RefreshTree` callback now goes through `MainTreeQtView::scheduleRebuild()`:
   one coalesced `rebuild()` queued on the event loop (`_rebuildQueued` flag;
   `_vm` re-checked when it fires, since the doc can die with the call in
   flight). Removes hazard #4 and collapses refresh bursts. Audited first:
   no external rebuild-then-read sites exist (`Qt_MainTreeSetCurrent` is gone,
   selection flows tree→model only; `findSelectFrom` reads current state, not
   post-mutation state). User-action rebuilds (filter toggles, context-menu
   deleted-copy-ops) stay synchronous by design. Pipe smoke green
   (add_class → undo → gone). GUI checks: edit updates tree promptly,
   tree drag-drop end state, undo from tree bar, doc close with edits.
4. ✅ DONE 2026-06-12 (user GUI-verified; simplified same day on JV's
   review) — diagram-only deferred refresh. `DeferViewRefreshWhileLocked`
   (one flag meaning "full refresh owed") is replaced by
   `DeferDiagramRefreshWhileLocked()` setting the WEAKER flag
   `m_diagramsDirtyWhileLocked`. At the outermost unlock: a full request
   (`m_viewsDirtyWhileLocked` — anything structural went through
   `UpdateAllViews`) wins; otherwise the unlock walks the diagrams and calls
   their `Update*DiagramViews()` — which only reaches diagrams with OPEN
   canvases, each a posted idempotent `QWidget::update()` — and skips the
   synchronous tree rebuilds entirely (those are the cost the lock exists to
   amortize), plus `notifyStateChanged()` so undo/redo enables stay fresh.
   First cut recorded per-diagram `Gti*`s in a `std::vector` for
   diagram-exact repaints; JV rejected it (unclear, dislikes std containers
   here) and the precision bought nothing — repaints already self-limit to
   open canvases — so it became one bool, which also removed the
   dangling-pointer validation walk. Net effect: a bulk geometry op under
   `CbViewLock` no longer pays tree rebuilds. Model side via pipe (forwarder
   id 40965, both funnel bodies); framework side in ClassBuilderDoc.{h,cpp}.
5. 🔶 MODEL HALF IMPLEMENTED 2026-06-12, awaiting GUI verification — the
   two-place dirty rule. Both SaveStates (`DataModelDoc::SaveState`,
   `DataModelDocObject::SaveState`) and the Add/Delete sites (`Gti::Add`,
   `Gti::Remove`, `DataModelDocObject::Delete` ×2) already set the flag and
   KEEP it; removed the redundant manual sets: `Gti::Update` (re-display of
   an already-recorded change), the 6 `OnEditContext`
   MarkLastUndo-compare-then-dirty blocks (the compare's only purpose was
   dirty; the closing `MarkLastUndo()` call STAYS — it ends the dialog's
   undo transaction), `ChildActivationShape::SetRect` (SaveState 10 lines
   up), `DataModel::ReadAllFiles` (SaveState at top),
   `DataModel::Save{All,Modified}Files` (flag was set then immediately
   cleared by the `OnSaveDocument` call right below — discovering THAT also
   exposed stale write_source docs, now fixed: write_source saves the .cbz).
   Pipe probe green: fresh load clean, CD view-open clean, add_class →
   modified:true, undo intact.

   SECOND HALF same day: object CREATION turned out to be the uncovered leg —
   `UndoNew`'s ctor (every creation's chokepoint; diagram shapes have no
   `Gti::Add`) did NOT dirty, which is why ~20 Qt canvas/dialog sites
   hand-set the flag. Fix: `UndoNew::UndoNew` now sets the flag, guarded
   like SaveState (not during undo/redo replay; draws blocked in the
   `SetModifiedFlag` funnel; the serialize ctor makes no UndoNew so loads
   stay clean). Then removed ALL redundant manual sets: 14 canvas sites
   (SequenceDiagramQtView ×11, ClassDiagramQtView ×3), 6 dialog sites
   (ConstructorCode, LifeLine, MethodCode, ProjectSettings, Signal,
   UserCode — each provably behind an explicit SaveState), 4 pipe handlers
   (optimize_placement, space_lifelines, reset_activation_offsets, trace —
   their model internals SaveState). ONE deliberate survivor:
   `DataModelDialog::markModelChanged` — its version-compaction loops use
   plain setters with no SaveState (commented in place). Probe: shape
   creation via pipe (no manual set anywhere) → modified:true, undoCount:1;
   fresh load + view-open clean. The dirty flag now flows ONLY from:
   SaveState (doc + object), UndoNew (creation), Gti::Add/Remove,
   DataModelDocObject::Delete, framework lifecycle, + the one exception.

Steps 2, 4, 5 deliberately waited for supervised sessions — they changed
observable refresh ordering. The dbg_* pipe commands (dbg_state /
dbg_list_undo / dbg_open_first_cd|sd / dbg_shell_geom) were a TEMPORARY
verification harness for that work and have since been REMOVED (verified
gone from CbCommandServer 2026-06-24).

## Open observation (2026-06-12, JV) — RESOLVED same day

The symptom was: change something in the CD, then Undo from the TREE view —
the CD didn't repaint (model restored, paint only on the next click). Exactly
the predicted datapoint: two Undo entry points, only `CClassBuilderDoc::Undo`
forced a refresh, the tree-bar buttons go through `DataModelDoc::Undo` which
didn't. Fixed by moving the forced refresh into `DataModelDoc::Undo/Redo`
(`UpdateAllViews()` inside the lock scope when anything was restored;
`RollBack` likewise on its non-silent path) and collapsing the
`CClassBuilderDoc` pair to plain forwards. User-verified both directions:
tree-undo repaints the CD note move; CD-undo updates the tree after a
class create.

## NEXT DESIGN v2 — per-view-type Notify (design AGREED by JV 2026-06-17)

> **STATUS: ROUND 1 + ROUND 2 DONE + built green (ROUND 2 confirmed in code 2026-06-17).**
> Framework `CClassBuilderDoc` has `m_treeDirty/m_cdDirty/m_sdDirty`,
> `NotifyTreeViews/CdViews/SdViews` + `setDirty` + a 3-way `FlushQueuedRefresh`
> (`NotifyStructureChanged`=tree+cd+sd kept as a compat wrapper for the unmigrated
> call sites; `NotifyGeometryChanged` was REMOVED — no callers left). 3
> `DataModelDoc` forwarders added (pipe).
>
> **ROUND 2 — DONE.** `TouchesCd()`/`TouchesSd()` are now precise `dynamic_cast`
> chains (DataModelDocObject.cpp:473-507), each conservatively OR-ing `TouchesTree()`
> (a Gti can appear on any diagram). Chokepoints (SaveState / Delete×2 / UndoNew)
> fire `if (TouchesTree) NotifyTreeViews(); if (TouchesCd) NotifyCdViews();
> if (TouchesSd) NotifySdViews();`. Undo/Redo/RollBack accumulate `tree/cd/sd` via
> `UndoBase::AccumulateTouches(tree,cd,sd)` over the batch and fire only the touched
> notifies (+ `RecalculateAllDiagrams(cd, sd)` now takes the per-type flags).
> `FlushQueuedRefresh` fans out per-type (`RepaintCdViews`/`RepaintSdViews`), so a
> CD-only edit no longer repaints SD canvases. End-to-end precision is real.
>
> **ROUND 3 (in progress):**
> - ✅ **Sort-loop CbViewLock (2026-06-17, built green, dev-launched on Matrix).**
>   All 8 sort methods — `Class`/`ClassGroup`/`MetaGroup`/`DataModel`
>   `::SortOnName`+`::SortOnPhase` — wrapped in `CbViewLock lock(GetDataModelDoc())`
>   and their trailing `NotifyStructureChanged()` deleted (each child's `SaveState()`
>   already notifies; the lock dtor fires the one coalesced flush). `Gti::SortOn*`
>   are the "wrong place" error stubs — correctly left alone.
>   Folded into new3.cbz (user-confirmed 2026-06-17). The same pass also fixed
>   MemberAndMethodGroup::CompareName/ComparePhase to the house per-swap undo
>   discipline (the two comparators missed in the 2026-05-08 sweep).
> - ✅ **Bucket (c) CONVERT done + user-verified 2026-06-17** (two open views of one
>   diagram both update on OK; note text + font-size edits propagate to both).
>   Over-broad `NotifyStructureChanged()` on canvas-only edits converted to the
>   per-view kick: `NoteShape::OnOpen` → `NotifyCdViews`, `SDNoteShape::OnOpen` →
>   `NotifySdViews`, `RelationDiagramOnlyShape::OnEditAttributes` → `NotifyCdViews`,
>   `DependencyShape::OnEditAttributes` → `NotifyCdViews` (JV confirmed CD-only).
>   A note-text / diagram-relation / dependency edit no longer rebuilds the tree.
> - 🔶 **Bucket (b) IN PROGRESS 2026-06-17 (NotifyDiagramChanged removal + first 11 deletes USER-VERIFIED: bulk/lock ops, deleted-refresh commands, two-view propagation all clean):**
>   - `NotifyDiagramChanged` REMOVED entirely. Traced the structure: `RepaintCdViews`
>     iterates the diagrams and calls `UpdateClassDiagramViews` (the leaf that does
>     `ViewModel::Refresh` → Qt callback → repaint); `NotifyCdViews` is the funnel
>     that reaches that same leaf. So a proposed `UpdateClassDiagramViews→NotifyCdViews`
>     reroute would RECURSE — rejected. Instead: both diagram funnels collapsed to just
>     the `Refresh` loop (gate dropped), framework `CClassBuilderDoc::NotifyDiagramChanged`
>     + model `DataModelDoc::NotifyDiagramChanged` deleted. Safe because the gate only
>     coalesced POSTED canvas repaints (Qt coalesces those anyway; under a lock they
>     don't paint until the op returns). Fixes the over-broad both-diagrams flag.
>     ⚠️ the two funnel bodies (ClassDiagram/SequenceDiagram::Update*DiagramViews) are
>     model methods — need read_source fold or a regen re-adds the dead gate.
>   - 11 of ~24 Qt-view `UpdateClassDiagramViews()` deletes done (colors, toggles,
>     hide/show, drop, placement, select-classes, optimize). ~13 pending (add-*,
>     editContext/editExcSpec, align, dialog handlers) + verify changeTemplateColor.
> - ⏳ **Bucket (b) DELETE (remaining)** — the big sweep: redundant `Update*DiagramViews()`
>   (31 ClassDiagramQtView + 18 SequenceDiagramQtView + 2 dialogs + ~10 model
>   setter-direct) + the remaining scattered model `NotifyStructureChanged` kicks
>   (BaseClass, Class, ClassGroup, MetaGroup, DataModel). One file at a time,
>   GUI-verified, delete only where a SaveState/UndoNew chokepoint provably sits
>   on the path (else it's a latent missing-SaveState undo bug — fix that instead).



JV's direction (paraphrased): "The `TouchesCd`/`TouchesSd`/`TouchesTree` set is a
bit crude. `TouchesCd`/`TouchesSd` aren't actually used. The structural notify is
very coarse — the intention was to split refresh per view type. The Notify should
not be `NotifyStructureChanged`/`NotifyGeometryChanged` but a notify specific to
the view type. Not all agreed; it was agreed `Touches` would be simple, but it was
already split." This section is the concrete plan to finish that split. **Nothing
here is implemented yet** — review/confirm the open decisions at the bottom first.

### Current state (verified 2026-06-16)

- **Predicates** (`DataModelDocObject`): `TouchesTree()` is real
  (`dynamic_cast<const Gti*>(this)`). `TouchesCd()` / `TouchesSd()` are **dead
  stubs** — both literally `return 1;`, no overrides, and **zero callers**
  (only `UndoBase`/`RedoBase` forward them, and nothing calls those either).
  Only `TouchesTree` is consumed.
- **Derivation harness is already in place, but binary.** Every chokepoint
  already derives tree-vs-not and fires the matching notify:
  `DataModelDocObject::SaveState` / `Delete` ×2 / `UndoNew` ctor →
  `if (TouchesTree()) NotifyStructureChanged(); else NotifyGeometryChanged();`.
  `DataModelDoc::Undo` / `Redo` / `RollBack` accumulate `structural ||=
  entry->TouchesTree()` over the batch, then fire one notify. So the wiring to
  extend is small and central.
- **The coarseness** is entirely in the two notifies' fan-out:
  - `NotifyStructureChanged` → `RefreshAllViews` = `UpdateTreeViews` + **every**
    CD canvas + **every** SD canvas + `notifyStateChanged`.
  - `NotifyGeometryChanged` → `RefreshDiagramViews` = **every** CD canvas +
    **every** SD canvas + `notifyStateChanged`.
  So a pure CD shape drag repaints all SD canvases, and an SD activation move
  repaints all CD canvases — wasted work, and the thing to fix.

### Object hierarchy (confirmed — drives the predicates)

```
DataModelDocObject
├── Shape
│   ├── ClassDiagramShape        ← CD: ClassShape, ConnectionShape, RelationShape,
│   │                                  DependencyShape, NoteShape, RelationDiagramOnlyShape…
│   └── SequenceDiagramShape     ← SD: LifeLineShape, ClassLifeLineShape, SignalShape,
│                                      Parent/ChildActivationShape, SDNoteShape…
├── ConnectionSegment            ← CD (NOT under ClassDiagramShape; + Relation*StartSegment)
├── NoteShapePoint               ← CD (NOT under a shape base)
├── SDNoteShapePoint             ← SD (NOT under a shape base)
└── Gti                          ← tree (Class, Method, Member, Group…)
```
Clean partition: no object is both CD and SD; a Gti is neither (so `TouchesTree`
catches it first and the canvas predicates never run for tree objects).

### Proposed predicates (replace the stubs — "simple", matches TouchesTree style)

```cpp
int DataModelDocObject::TouchesCd() const {
    return (dynamic_cast<const ClassDiagramShape*>(this) ||
            dynamic_cast<const ConnectionSegment*>(this)  ||
            dynamic_cast<const NoteShapePoint*>(this)) ? 1 : 0;
}
int DataModelDocObject::TouchesSd() const {
    return (dynamic_cast<const SequenceDiagramShape*>(this) ||
            dynamic_cast<const SDNoteShapePoint*>(this)) ? 1 : 0;
}
```
(One cast catches each shape base + all descendants; the two/one extra catch the
point/segment helpers that live directly under `DataModelDocObject`.) VERIFY at
implementation: the "SD colour-template carriers" the earlier note mentioned — are
they `SequenceDiagramShape` descendants? If not, add a cast.

### Proposed notify surface (view-type-specific)

- **Keep `NotifyStructureChanged`** for tree objects: a Gti can be represented on
  ANY diagram (a class box on CDs, a lifeline/actor on SDs), and the tree rebuild
  is the only expensive part — so "rebuild trees + repaint all canvases" is both
  correct and the right cost trade. (Open decision: rename to `NotifyTreeChanged`?)
- **Replace `NotifyGeometryChanged`** with **`NotifyCdChanged`** (CD canvases only)
  and **`NotifySdChanged`** (SD canvases only).
- Chokepoint derivation becomes (the only call-shape change at SaveState/Delete/
  UndoNew):
  ```cpp
  if      (TouchesTree()) NotifyStructureChanged();   // tree + all canvases
  else if (TouchesCd())   NotifyCdChanged();           // CD canvases only
  else if (TouchesSd())   NotifySdChanged();           // SD canvases only
  ```
- `Undo`/`Redo`/`RollBack`: accumulate three batch flags (`treeDirty |=
  TouchesTree()`, `cdDirty |= TouchesCd()`, `sdDirty |= TouchesSd()`) and fire the
  matching notify(s). This also makes undo of a pure CD drag stop repainting SD.

### Proposed flush split (CClassBuilderDoc)

- Replace the single `m_diagramsDirtyWhileLocked` + `m_flushStructural` bool with
  three: `m_treeDirty`, `m_cdDirty`, `m_sdDirty`.
- `NotifyCdChanged`/`NotifySdChanged`: locked → set `m_cdDirty`/`m_sdDirty`;
  unlocked → `RequestFlush` carrying that kind.
- `FlushQueuedRefresh`: `treeDirty` → `RefreshAllViews` (subsumes both canvas
  kinds). Else `{ if cdDirty → repaint CD canvases; if sdDirty → repaint SD
  canvases }` + `notifyStateChanged` once.
- `UnLockAndUpdateAllViews`: outermost unlock → `treeDirty` wins; else flush
  whichever of cd/sd are dirty. (Same coalescing as today, just finer flags.)
- `NotifyDiagramChanged` (the setter-direct coalescing twin) splits the same way
  OR is retired with the setter-direct calls in the deletion step below.

### The payoff — delete the scattered calls ("so less")

Once the chokepoints fire the right per-view notify by themselves, the explicit
calls are redundant. From the 2026-06-16 grep:
- **~46 Qt-view `_pCD->UpdateClassDiagramViews()` / `_pSD->UpdateSequenceDiagramViews()`**
  (ClassDiagramQtView ~31, SequenceDiagramQtView ~16) — after edits that already
  SaveState.
- **~25 model `NotifyStructureChanged()`** — many ARE the chokepoints (keep:
  `Gti::Add/Update/Remove`, `DataModelDocObject::Delete/SaveState`, `UndoNew`,
  `UndoDelete/UndoSubDelete`, `RedoNew`); the rest are scattered mutator kicks
  (BaseClass, Class, ClassGroup, MetaGroup, DependencyShape, NoteShape,
  SDNoteShape, RelationDiagramOnlyShape, DataModel) to review.
- **~10 model setter `Update*DiagramViews()`** (ClassDiagram, SequenceDiagram,
  ClassShape, LifeLineShape) — the "#5 setter-direct" path; a shape setter's own
  SaveState now records `cdDirty`/`sdDirty`, so these go too.

RULE (audit hard-case #3, unchanged): delete a call ONLY where a chokepoint
(SaveState / UndoNew / Delete / Gti::Add/Remove) provably sits on its path. A call
with NO chokepoint is a latent **missing-SaveState undo bug**, not a safe delete —
flag it, fix the SaveState, then the refresh follows for free. One-by-one, not a
blind sweep.

**`UpdateTreeViews` is already consolidated** — private, one call site
(`RefreshAllViews`). It is not scattered; it stays as the tree leg of the funnel.

### The "use CbViewLock when needed, so less" pattern — concrete example

`Class::SortOnName`/`SortOnPhase` (1116/1136), `ClassGroup::SortOnName`/`SortOnPhase`
(480/498), and the `MetaGroup` twins are: a loop of `child->SaveState()` (each
already fires the derived notify) followed by ONE trailing explicit
`NotifyStructureChanged()`. Two problems: the trailing call is pure redundancy, and
because each unlocked `SaveState` refreshes inline, the loop currently pays K
refreshes. **Fix = wrap the loop in `CbViewLock` (coalesce K→1) + delete the
trailing `NotifyStructureChanged()`.** That is exactly "use `CbViewLock` when
needed, so less" — fewer scattered calls AND a perf win, behaviour-neutral.

### Spot-check findings (2026-06-16) — a THIRD category: over-broad, not just redundant

Reading the shape edit handlers reveals that some scattered calls aren't redundant
duplicates of a chokepoint — they are the chokepoint's refresh done at the WRONG
granularity (full tree rebuild for a canvas-only edit):
- `NoteShape::OnEdit` (NoteShape.cpp:240) + `SDNoteShape::OnEdit` (SDNoteShape.cpp:267):
  dialog SaveState's the note, `MarkLastUndo` finalizes, then `NotifyStructureChanged()`
  — but a note is NOT a tree node, so this rebuilds the whole tree for a note-text
  edit. → should be `NotifyCdChanged` / `NotifySdChanged`.
- `RelationDiagramOnlyShape::OnEditAttributes` (RelationDiagramOnlyShape.cpp:294): a
  diagram-ONLY relation (canvas-only by definition) → `NotifyStructureChanged` is
  over-broad → `NotifyCdChanged`.
- `DependencyShape::OnEditAttributes` (DependencyShape.cpp:246): AMBIGUOUS — a
  dependency may surface in the tree; check whether the edited attributes show there
  before converting.

So the cleanup has three buckets, not two: (a) keep (the chokepoints themselves),
(b) delete (pure redundant duplicates behind a chokepoint), (c) **convert**
(Structure→Cd/Sd, an over-broad structural kick on a canvas-only edit). Bucket (c)
is only expressible AFTER the notify split exists — today there is no canvas-only-CD
notify to convert TO, which is itself an argument for doing the split first.

### Open decisions (JV — the "not all agreed" parts)

1. **Naming:** `NotifyStructureChanged` vs `NotifyTreeChanged`; `NotifyCdChanged`/
   `NotifySdChanged` vs `NotifyClassDiagramChanged`/`NotifySequenceDiagramChanged`.
2. **Gti canvas precision:** keep the structural path = tree + ALL canvases
   (simple, conservative, correct — a Gti can be on any diagram; recommended), or
   make a Gti also declare `TouchesCd`/`TouchesSd` for canvas precision (more
   complex, marginal gain since canvas repaints are cheap/posted)?
3. **Predicate form:** `dynamic_cast` (proposed, mirrors `TouchesTree`) vs virtual
   overrides per class.
4. **Sequencing:** predicates (dead → harmless) → notify split → flush split →
   delete scattered calls per-file (the only step that changes observable refresh
   ordering → supervised + GUI-tested). Each step independently buildable.

### Risk

The canvas split is benign-failure: a wrong/missing predicate = a stale canvas
(cosmetic, fixed by a click), NEVER a stale tree or a crash — the tree path stays
conservative (rebuild + all canvases) and rides on undo correctness as before.
But it IS GUI-observable, so it must be verified by clicking: drag a CD shape →
only CD canvases repaint; drag an SD activation → only SD canvases repaint;
cross-diagram structural edit (rename a class shown on both) → both update.
