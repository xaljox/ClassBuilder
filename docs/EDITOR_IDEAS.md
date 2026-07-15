# Code-editor ideas — possible additions

Status 2026-07-15. The model-aware editor now has: C++ syntax colours with
model types + relation iterator types, italic arguments (also in the
signature strip), current-line / brace-match / occurrence highlighting (the
yellow F2-preview across body, init pane and signature), F2 rename
(arguments and members through the model with fan-out, anything else as a
scoped text replace), model-aware completion (member access with chain
resolution, iterator deref, `Class::`, iterator **loop** skeletons, argument
types shown / names inserted, overloads distinct), reformat, move-lines,
model↔editor sync (renames + undo/redo), go-to-definition (⌘-click / F12,
tree-select always), and editors as dockable/tabbed shell windows.

Remaining ideas, roughly by value-per-effort:

## Model-powered (the unfair advantage — nothing generic can do these)

1. **Hover documentation from model notes.** Hover over a method/member →
   tooltip with its signature and its `@NOTE` text. The model's
   documentation becomes live API docs. *Small effort, big payoff.*
2. **Who calls me.** For the method *being edited*, search every method
   body in the model (`_code`, whole-identifier) and list the
   `Class::method` callers; double-click opens that editor. The model IS
   the index — no parsing. Fits a dockable results pane. No
   caret-identifier variant: callers of a method you *call* are rarely the
   question, and when they are, F12 onto it makes it the edited method —
   then ask there.
3. **Parameter hints.** While typing inside `Name(...)`, show the full
   signature (`GetInterfaceCpp`) as a tooltip — argument names, types,
   defaults.
4. **Completion popup enrichment.** Model icons per kind
   (method/member/argument/type/iterator — `Qt_ModelIcon` exists) and a
   return-type column.
5. **Init-list completion** (constructor init pane): typing `_` offers
   exactly the members *not yet initialized* in the init text, inserting
   `_x()` with the caret inside.
6. **Method-not-found diagnostics.** `var->Method()` whose receiver class
   resolves but lacks the method → red squiggle before save/regenerate.
   Only for resolvable receivers, so no false alarms.
7. **Go-to-definition for members/arguments.** ⌘-click on `_member` or an
   argument → select it in the tree (methods and classes already work).

## Editor quality-of-life (generic but real daily wins)

8. **Comment toggle** — Ctrl+/ comments/uncomments the selection with `//`.
9. **Auto-close pairs** — `(` inserts `()` with type-over; same for quotes.
10. **Ctrl+wheel font zoom** in the editor.

## Architecture / shell

11. **QDialog → QWidget conversion** for the editor windows (they are dock
    content now, not dialogs). Needs a model click: `Method::_pOpenDialog`
    type `QDialog*` → `QWidget*` (+ accessor types). Removes the residual
    dialog semantics (result codes, Esc/default-button behaviour).
12. **Remember the editor dock area.** When the last editor tab closes, the
    split disappears; remember its place/size so the next editor opens
    docked there instead of floating.

## Parked / far-fetched

13. **Build/Run integration** — CB knows the files it generates; a Build
    button shelling out to cmake with output in a dock is a real (if
    scope-creepy) possibility.

Recommended next pick: **1 + 3** (hover docs + parameter hints) — both are
small, reuse the existing resolver, and make the model's knowledge visible
exactly where you type. Then **2** (who calls me) as the next big one.
