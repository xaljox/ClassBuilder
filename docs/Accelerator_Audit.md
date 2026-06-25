# Accelerator / shortcut + menu-naming audit

Status: **PROPOSAL for review** (2026-06-24). Per the standing rule, do the whole
thing as ONE pass after the scheme is agreed — don't rebind piecemeal.

## Governing policy (JV, 2026-06-24)

1. **Same command → same shortcut everywhere.** A command that appears in several
   places (Add Class, Add Method, Delete, …) gets ONE shortcut, used at every
   location (main menu, tree context menu, CD context menu, …).
2. **Tree-building keys are globally reserved.** Keys used frequently while building
   the tree are "spent" and must not be reused for anything else anywhere — the tree
   is always available alongside the diagrams (separate docks), so its bindings are
   effectively global.
3. **CD and SD may share a key for *different* actions** — but only a key that is NOT
   a reserved tree key — because you are only ever in one diagram at a time, so a
   CD-only / SD-only key can carry a different, logical meaning in each.

Constraints: no **Ctrl+Alt** (AltGr on EU layouts — use Ctrl+Shift); a tree is a
type-ahead widget, so **plain single letters cannot be accelerators in the tree**
(they'd steal type-to-navigate) — Add-style commands therefore need a modifier.

## Current inventory (from the 2026-06-24 read of the Qt views)

**Already consistent — keep as-is:** Ctrl+N/O/W/S (File), Ctrl+Z (Undo), Ctrl+Y
(Redo), Delete (delete), Esc (cancel/clear in diagrams), Ctrl+0 / Ctrl++ / Ctrl+-
(zoom in CD+SD).

**Reserved TREE keys (globally off-limits):** Delete, Ctrl+C (copy), Ctrl+V (paste),
Ctrl+F (find), F3 (find next), Ctrl+Z, Ctrl+Y — plus plain-letter type-ahead.

**CD-only:** R / I / D / O held = drag-create Relation / Inheritance / Dependency /
diagram-Only (canvas plain-letter GESTURES, not menu accelerators); Ctrl+A = select
all; middle-drag = pan; Ctrl+Wheel = zoom.

**SD-only:** Return/Enter = open selected; Ctrl+←/→ = swap lifeline; Ctrl+↑/↓ = swap
activation; ←/→/↑/↓ = navigate shapes.

**The headline gap:** the whole **Add\*** family (Add Class / Member / Method /
Relation / Inheritance / Dependency / Extern Class / Type / Actor / Note / Diagram)
has **NO accelerators** today — it lives in "Add" submenus as Alt-mnemonics only. So
this is greenfield *assignment*, not collision-fixing.

**Real inconsistencies worth deciding:**
- Find / Find-Next exist only in the tree (Ctrl+F / F3). Diagram "find shape" is a
  different feature — propose leaving as-is (documented intentional).
- Copy/Paste exist only in the tree. CD/SD copy/paste would be a new *feature*, not
  just a binding — out of scope for this pass; note as future.
- Select-All: CD has Ctrl+A, SD and tree don't. Propose: give SD Ctrl+A too (select
  all shapes); tree select-all is rarely meaningful — leave off.
- Arrow navigation: SD has it, CD doesn't. Propose: add the same arrow navigation to
  CD for symmetry (or accept the asymmetry — flagged for JV).

## Proposed scheme

### Add\* family — `Ctrl+Shift+<letter>` (works identically in tree + CD; clears tree type-ahead; no Ctrl+Alt)

| Command            | Proposed | Notes |
|--------------------|----------|-------|
| Add **Class**      | Ctrl+Shift+C | |
| Add **Method**     | Ctrl+Shift+M | both are "Me…" so spelling doesn't decide — FREQUENCY does: Methods are added far more often than Members, so Method gets the obvious M (also folds in the "Member Function"/"Function" → "Method" rename) |
| Add **Member**     | Ctrl+Shift+B | "mem**B**er" |
| Add **Relation**   | Ctrl+Shift+R | matches the CD drag-gesture **R** |
| Add **Inheritance**| Ctrl+Shift+I | matches the CD drag-gesture **I** |
| Add **Dependency** | Ctrl+Shift+D | matches the CD drag-gesture **D** |
| Add **Relation-Only** (diagram-only) | Ctrl+Shift+O | matches the CD drag-gesture **O** — completes the R/I/D/O connection set so the Add shortcut == the gesture for all four connection types |
| Add **External Class** | Ctrl+Shift+E | "**E**xtern". X was the natural pick ("e**X**tern") but **Ctrl+Shift+X is swallowed by a global OS hotkey before CB ever sees it** — proven 2026-06-24: the identical handler on Ctrl+Shift+N fired, X never did, across QAction / QShortcut(view) / QShortcut(_tree) / raw KeyPress event-filter. So E (freed from Meta Group) takes it. |

**Extern split — DONE IN THE MODEL 2026-06-24.** De-branched in the self-hosted model:
added a virtual `ExternClasses::OnAddExternClass`, deleted the `ExternClasses`/`ExternClass`
`OnAddClass` extern overrides so `OnAddClass` is regular everywhere. The tree's
`AddExternClass` action calls `GetDataModelDoc()->GetExternClasses()->OnAddExternClass`
(reachable from any node); "Add Class" greys out on the extern group (correctly N/A there).
Menu + Ctrl+Shift+E both fire it. (Done via the pipe: `add_method` + `delete_method` rather
than `set_method_name`, which pops a dialog and over-renames call sites.)
| Add **Argument**   | Ctrl+Shift+A | Arguments are added far more often than Actors, so A lives here |
| Add **Actor**      | Ctrl+Shift+T | "ac**T**or" (A went to Argument) |
| Add **Constructor**| Ctrl+Shift+U | "constr**U**ctor" — U (mid-syllable) used so **S** stays free for IsClass, where it's the start of a syllable |
| Add **Virtual Methods** | Ctrl+Shift+V | **V**irtual |
| Add **IsClass Methods** | Ctrl+Shift+S | "IsCla**S**s" |
| Add **Group**      | Ctrl+Shift+G | |
| Add **Meta Group** | Ctrl+Shift+P | "grou**P**" — moved off E (which went to External Class). Meta Group is infrequent, so it takes the less-mnemonic free key. |
| Add **Type**       | Ctrl+Shift+Y | "t**Y**pe" |
| Add **Note**       | Ctrl+Shift+N | N is free (Ctrl+Shift+N ≠ Ctrl+N New) |
| Add Class/Sequence Diagram | (none) | infrequent — menu only |

Tree side IMPLEMENTED 2026-06-24 (C/M/B/R/I/A/T/U/V/S/G/Y + External Class **E** + Meta
Group **P** + the AddFunction→AddMethod rename). Group / Meta Group / External Class fire
via QShortcuts on the tree (no toolbar button). NOTE: Ctrl+Shift+X is unusable here — a
global OS hotkey eats it before CB; don't reuse X anywhere expecting it to fire.

CD side IMPLEMENTED 2026-06-24: context-menu items carry the shortcut hint; firing is in
`keyPressEvent`'s Ctrl+Shift block via `triggerAddShortcut(key)`, gated by selection like
the menu. CD set = C/N/I/R/O/D/B/M/U/A/V/S (R/I/D/O match the plain-key drag gestures,
which are unchanged). Plain R/I/D/O still arm drag-create; Ctrl+Shift+R/I/D/O = Add.

SD side IMPLEMENTED 2026-06-24: same pattern (menu hints + `triggerAddShortcut` in the
keyPressEvent Ctrl+Shift block). SD set = the shared class-model adds C/B/M/U/A/V/S/N at
the SAME keys + SD-specific Lifeline=L + **Message=K**; **Select-All=Ctrl+A** added (new
`selectAll`). Plain arrows still navigate, Ctrl+arrows still swap. Message=K because every
mnemonic (M/S/A) is tree-reserved and W was avoided (it sits next to the OS-grabbed
Ctrl+Shift+X — see the External Class note). The AddFunction→AddMethod rename is DONE
(2026-06-24): `AddFunction` enum + `aAddFunction` action vars → `AddMethod`/`aAddMethod`
in CD+SD, and the CD `addMemberFunction()` helper → `addMethod()`; the shared toolbar-glyph
constant `TG_ADD_FUNCTION` was left as-is (it's a bitmap-strip id, not the action name).

ACCELERATOR PASS COMPLETE across tree + CD + SD (incl. Message=K + the rename). Done.

The four **connection** commands (R/I/D/O) now use the SAME letter for the
drag-gesture and the Add accelerator — one mental model, both ways in.

Each is **enable-gated** to where it's valid (e.g. Add Member/Method only with a
class selected), exactly like the context-menu items today. Same binding on the main
**Project** menu, the tree context menu, and the CD context menu.

### Menu Alt-mnemonics — rationalise per menu so no two items in the same menu share a letter; fold in the **"Member Function" / "Function" → "Method"** rename (the original trigger). With Method now keyed Ctrl+Shift+T and labelled "Method", its mnemonic can be "Me&thod" or "&Method" depending on sibling clashes.

### Reserved / global (unchanged): the tree keys above; Ctrl+N/O/W/S; Ctrl+Z/Y; Delete; Esc; Ctrl+0/+/-.

### CD vs SD reuse: leave CD's R/I/D/O gestures and SD's arrow/Ctrl+arrow set as-is (already disjoint and policy-compliant). Spare CD letters (e.g. nothing yet) may later be reused in SD for SD-specific actions, and vice-versa, per policy #3.

## Decisions (AGREED 2026-06-24)
1. Add\* keying = **`Ctrl+Shift+<letter>`** per the table above. ✓
2. **Method = M, Member = B** (frequency breaks the Me… tie — Methods added more often). ✓
3. **Find / Copy-Paste stay tree-only.** ✓ (Diagram find/copy would be new features; may surface during use, parked.)
4. Symmetry:
   - **SD gets Select-All (Ctrl+A)** to match CD. ✓ — included in this pass.
   - **CD Up/Down within-class navigation — IMPLEMENTED + user-confirmed 2026-06-24.** `navigateClassRow`, fired from `keyPressEvent` on plain Up/Down: moves the single selection between rows of the SAME class in visual order (header → members → methods); no wrap at the ends; only active when a class row is selected. **Left/Right between classes stays DROPPED** — spatial nav in a free-form canvas is inherently surprising (e.g. Right from the rightmost class has no sensible answer); a deterministic Tab/Shift+Tab cycle would be the non-surprising route if ever wanted.

## Implementation scope for the one pass
- Add\* accelerators (`Ctrl+Shift+…`) on the Project menu + tree context menu + CD
  context menu, SAME binding each place, enable-gated to where each is valid; use a
  per-view shortcut context (WidgetWithChildrenShortcut) so the focused view's command
  fires — "same command, same key, context-appropriate selection".
- Align the four connection Adds (R/I/D/O) with their CD drag-gesture letters.
- "Member Function" / "Function" → **"Method"** rename + its Alt-mnemonic, plus a
  per-menu Alt-mnemonic dedupe.
- **SD Select-All (Ctrl+A).**
- (Deferred, separate: CD Up/Down within-class navigation.)

Once agreed, implement across QtShellWindow (menus/toolbar), MainTreeQtView,
ClassDiagramQtView, SequenceDiagramQtView in one pass. Watch
[[feedback_no_ctrl_alt_accelerators]].
