# Code-editor ideas — possible additions

Status 2026-07-15 (evening). The model-aware editor now has: C++ syntax
colours with model types + relation iterator types, italic arguments (also
in the signature strip), current-line / brace-match / occurrence
highlighting (the yellow F2-preview across body, init pane and signature),
F2 rename (arguments and members through the model with fan-out, anything
else as a scoped text replace), model-aware completion (member access with
chain resolution, iterator deref -- scope-qualified outside the owning
class, `Class::`, iterator **loop** skeletons, `new` completes
constructors, argument types shown / names inserted, overloads distinct,
ctors/dtors/deleted filtered), hover documentation (signature + @NOTE,
overload picked by call arity), parameter hints (active argument bold,
defaults shown), who-calls-me (Ctrl/⌘-click the signature strip /
Shift+F12 / Edit menu), reformat, move-lines, model↔editor sync (renames +
undo/redo), go-to-definition (⌘-click / F12, tree-select always), editors
as dockable/tabbed **plain-QWidget** shell windows (`_pOpenWidget` seam),
and a remembered editor dock spot (the next editor re-opens docked where
the last group lived).

Remaining ideas, roughly by value-per-effort:

## Model-powered (the unfair advantage — nothing generic can do these)

1. ✅ DONE 2026-07-15. **Hover documentation from model notes.** Hover over
   a method/member → tooltip with its signature and its `@NOTE` text. The
   model's documentation becomes live API docs. *Small effort, big payoff.*
2. ✅ DONE 2026-07-15. **Who calls me.** For the method *being edited*, search every method
   body in the model (`_code`, whole-identifier) and list the
   `Class::method` callers. The model IS the index — no parsing. Trigger:
   **Ctrl/⌘-click on the signature strip** — the strip is the definition,
   and "go-to-definition while on the definition = show references" is the
   established IDE meaning of that gesture; it mirrors the identifier
   Ctrl-click, and the mouse sits high so the list drops open below.
   Keyboard: **Shift+F12** (F12 = definition, Shift+F12 = references, VS
   convention), plus a context-menu entry. The list: a popup anchored
   under the strip, Enter/double-click opens that caller's editor, Esc
   closes. No caret-identifier variant: callers of a method you *call*
   are rarely the question, and when they are, F12 onto it makes it the
   edited method — then ask there.
3. ✅ DONE 2026-07-15. **Parameter hints.** While typing inside `Name(...)`,
   show the full signature as a tooltip — argument names, types, defaults;
   active argument bold, overload picked by the argument index.
4. ✅ DONE 2026-07-16. **Completion popup enrichment.** Model icons per kind
   (method/member/argument/type/iterator — iterators carry the relation's
   FromRelation icon) and a muted right-aligned detail column (a method's
   return type, a variable's type, `class`, `iterator`/`loop`). Compact
   rows matching the who-calls-me popup. Bonus: hover on an iterator TYPE
   shows `class Owner::XIterator`; on an iterator VARIABLE the synthesized
   constructor signature (filter arg only when the relation's filter option
   is on) — the class-vs-variable parallel of `Column` / `Column cc;`.
5. ✅ DONE 2026-07-16. **Init-list completion** (constructor init pane):
   typing `_` offers exactly the members *not yet initialized* in the init
   text, inserting `_x()` with the caret inside. (A `_`-prefixed word now
   auto-triggers the popup in both panes.)
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

11. ✅ DONE 2026-07-15. **QDialog → QWidget conversion** for the editor
    windows (they are dock content now, not dialogs). Model click done:
    `Method::_pOpenWidget` is `QWidget*` (JV, with rename fan-out + version
    compact); dialogs are plain QWidgets, Esc via keyPressEvent, the
    Replace-dialog peek runs one modally via a local event loop.
12. ✅ DONE 2026-07-15. **Remember the editor dock area.** When the last
    editor tab closes, the split disappears; the place/size is remembered
    (refreshed on layout changes + at dock close) and the next editor opens
    docked there instead of floating.

## Parked / far-fetched

13. **Build/Run integration** — CB knows the files it generates; a Build
    button shelling out to cmake with output in a dock is a real (if
    scope-creepy) possibility.

Recommended next pick: the quality-of-life trio **8–10** (comment toggle,
auto-close pairs, Ctrl+wheel zoom — small, generic daily wins), then **6**
(method-not-found diagnostics) or **7** (go-to-definition for members /
arguments) as the next model-powered ones.
