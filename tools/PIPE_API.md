# ClassBuilder Command API — Reference

> **This file is written to be handed to a script or an AI agent as-is.** It is
> the complete contract: transport below, then every command with its
> parameters, return value and traps.

ClassBuilder hosts a **JSON-over-TCP** server on **loopback only**
(`127.0.0.1`, port **51777**) — identical on Windows, macOS and Linux. It starts
with the application; there is no flag to enable it. One JSON request per line,
one JSON reply per line.

```sh
echo '{"cmd":"ping"}' | nc 127.0.0.1 51777
{"ok":true,"result":{"build":"Aug 24 2026 14:15:42","pong":true}}
```

The port is **single-instance**: the first ClassBuilder to start binds 51777 and
a second one silently fails to bind, leaving it undriveable. Set **`CB_CMD_PORT`**
before launch to give an instance its own port (e.g. `CB_CMD_PORT=51778`), so a
dev build can be driven alongside the stable one.

Requests are handled **on the GUI thread, one line at a time**: a command that
opens a modal dialog blocks the connection until it is dismissed. Those commands
are flagged individually below — prefer the non-modal variants when scripting.

> **Historical note.** This was a Win32 named pipe (`\\.\pipe\ClassBuilder`,
> `CB_PIPE_NAME`) until the Qt port. That transport, and the environment variable
> that configured it, **no longer exist** — hence the rename from "Pipe API". The
> JSON protocol and every command below are unchanged.

- Request:  `{"cmd":"<name>","params":{...}}`
- Reply OK: `{"ok":true,"result":<value>}`
- Reply err:`{"ok":false,"error":"<msg>"}`

Commands operate on the **server-targeted document**. By default that target is implicit — whichever model the GUI reports as active. `select_document {title|path}` (see *Document selection* below) sets a **sticky** server-side target decoupled from the GUI's active window, so a script can drive one model while you keep editing another. The target stays until the next `select_document`; if its document is closed, doc-targeting commands report `no active document` until a new one is selected.

Source: [src/model/CbCommandServer.cpp](../src/model/CbCommandServer.cpp) (command implementations), [src/qt/QtCommandServer.cpp](../src/qt/QtCommandServer.cpp) (transport)

---

## Lifecycle

### `ping`
Liveness check.

| param | type | required | notes |
|-------|------|----------|-------|
| `echo` | any  | no | echoed back |

**Returns:** `{"pong":true,"build":"<compile timestamp>","echo":<echo if given>}`

`build` is the binary's compile timestamp — use it to confirm *which* ClassBuilder you are driving before issuing edits.

### `new_model`
Creates a new untitled document via the first registered doc template (mirrors File / New).

**Params:** none.
**Returns:** `{"title":"...","path":"..."}`
**Note:** opens the New-Model wizard dialog modally — **blocks the pipe** until the user clicks OK or Cancel. Prefer `new_model_basic` / `new_model_serialize` for scripted flows.

### `new_model_basic` `{name, h_file?}`
Creates a new model with **serialize OFF**, bypassing the wizard. Only the default groups (`ExternClasses`, `OtherTypes`, `Actors`) and scalar types are populated.

| param | type | default | notes |
|-------|------|---------|-------|
| `name` | string | required | DataModel name and doc title |
| `h_file` | string | `name + ".h"` | master header file |

**Returns:** `{"title":"...","path":"..."}`
**Invariant:** once created without serialize, serialize cannot be enabled later (matches the GUI rule). Use `new_model_serialize` if serialize is wanted.

### `new_model_serialize` `{name, document_class, h_file?, undo_redo?}`
Creates a new model with **serialize ON**, bypassing the wizard. `InitSerialize` runs: creates `CbObject` (extern), `<document_class>`, `<document_class>Object`, the 1-to-many relation, `_membersOnly`, `SerializeMembersOnly`. With `undo_redo:true`, `InitUndoRedo` also runs (UndoBase/RedoBase scaffolding).

| param | type | default | notes |
|-------|------|---------|-------|
| `name` | string | required | DataModel name and doc title |
| `document_class` | string | required | name `Xxx` for the document class; `XxxObject` is the polymorphic root |
| `h_file` | string | `name + ".h"` | master header file |
| `undo_redo` | bool | `false` | enable undo/redo at creation; can be enabled later but never disabled |

**Returns:** `{"title":"...","path":"..."}`
**Invariant:** once created with serialize ON it cannot be turned off; once `undo_redo` is enabled it cannot be disabled.

---

## Read

### `active_doc`
Info on the currently active MDI doc.

**Returns:** `{"title":"...","path":"...","modified":bool}` or `null` if none open.

### `list_classes`
Names of all `Class` objects in the active model.

**Returns:** `["Foo","Bar",...]`

### `find_class` `{name}`
Typed `Class` lookup (excludes ExternClass). Lightweight existence probe.
**Returns:** `BaseClassRecord` or `null`. Use `get_class` for full detail.

### Member-name convention

Member names are stored **without** the prefix. The Class's `member_prefix` (default `"_"`, often `"m_"`) is added uniformly at codegen time. This matches the GUI: you type `id` in the dialog and the tool emits `_id` (or `m_id`). Same rule applies to the pipe — pass `name:"id"`, not `name:"_id"`. `set_class_member_prefix` overrides the Class-level prefix; `get_member` exposes both the bare `name` and the rendered `prefixed_name`.

### `get_class` `{name}`
Rich record for a `Class`: serialize flag, files, note, inheritances, members, methods, member_prefix.

**Returns:** or `null` if not found:
```json
{
  "name": "Foo",
  "kind": "Class",
  "serialize": true,
  "h_file": "Foo.h",
  "cpp_file": "Foo.cpp",
  "dll_export": false,
  "note": "...",
  "inherits": [{"name":"FooDocObject","virtual":true}, ...],
  "members": [MemberRecord, ...],
  "methods": [MethodRecord, ...]
}
```

### `list_members` `{class}`
Granular alternative to `get_class` for clients that only want the member list.
**Returns:** `[MemberRecord, ...]` or `null` if class not found.

### `list_inherits` `{class}`
Outgoing inheritance edges (the bases this class inherits from).
**Returns:** `[{"name": "...", "virtual": bool}, ...]` or `null` if class not found.

### `find_extern_class` `{name}`
`BaseClass` lookup narrowed to ExternClass / Class.
**Returns:** `BaseClassRecord` or `null`.

### `find_method` `{class, name}`
First method matching `name` on the named class.
**Returns:** `MethodRecord` or `null`.

### `find_method_by_id` `{class, id}`
`id` matches the `//@CODE_NNNN` tag the generator embeds.
**Returns:** `MethodRecord` or `null`.

### `find_member` `{class, name}`
**Returns:** `MemberRecord` or `null`.

### `list_class_methods` `{class}`
All methods on the named class. Use this (not `find_method`) when overloads exist.
**Returns:** `[MethodRecord, ...]`

### `list_methods_named` `{name}`
Every class that has a method matching `name`. Returns `[{class, method}, ...]`. The bulk-Serialize finder.

### `find_methods_using_type` `{type}`
Every method whose argument list contains the named type. Returns `[{class, method, arg_name, arg_type}, ...]`. Useful for migration audits — e.g. `find_methods_using_type CArchive` returns every site that needs a CbArchive twin during Phase B of the undo port.

Walk: `BaseClass` (which IS-A `Type`) → iterate `Variable` children → filter `IsArgument()` → cast to `Argument` → `GetMethod()` → `GetBaseClass()`.

---

## Mutate

### `add_class` `{name, serialize?, h_file?}`
Creates a new `Class` on the active model. Mirrors the GUI's `DataModel::OnAddClass` path.

| param | type | default | notes |
|-------|------|---------|-------|
| `name` | string | required | must be unique across BaseClasses in the model |
| `serialize` | bool | model's serialize flag | when `true` the class auto-inherits the model's `DocumentObject` (which transitively inherits `CbObject`) |
| `h_file` | string | empty | header filename; CB derives a default at codegen if blank |

**Returns:** `{name, kind:"Class", serialize, h_file}`.
**Errors:** duplicate name; `serialize:true` on a non-serialize model (no `DocumentObject` to inherit from).

### `delete_class` `{name}`
Removes a `Class`. Honors the GUI invariants:
- cannot delete the document or document-object class
- cannot delete a class that has its own outgoing inheritances (matches `Class::OnDelete`'s `GetInheritCount` guard — remove inherits first via `remove_inherit` once that lands)

Skips the GUI's confirmation dialog.

**Returns:** `{name}` (the deleted class's name).

### `set_class_serialize` `{name, value}`
Toggles the class's serialize flag. Calls `Class::SetSerialize`, which restructures inheritance to honor the model invariants:
- **off → on:** keeps the first existing base that leads (transitively) to the model's `DocumentObject`, deletes the rest. Adds `Inherit(class, DocumentObject)` if no such base exists.
- **on → off:** removes the docObject inherit and the auto-generated serialize methods (Serialize, SerializeRelations, etc.).

Rejects `value:true` when the model itself is not serialize-on (no `DocumentObject` to inherit from).
**Returns:** `{name, serialize}`.

### `set_class_h_file` `{name, value}`
**Returns:** `{name, h_file}`.

### `set_class_note` `{name, value}`
**Returns:** `{name, note}`.

### `set_class_dll_export` `{name, value}`
**Returns:** `{name, dll_export}`.

### `set_class_member_prefix` `{name, value}`
Class-level member prefix override (e.g. `"_"`, `"m_"`). Member names are bare in storage; codegen emits `prefix + bare`. Overrides the DataModel default for this class.
**Returns:** `{name, member_prefix}`.

### `add_inherit` `{class, base, virtual?}`
Adds an outgoing inheritance edge from `class` to `base`.

**Restricted to non-serialize classes.** For serialize-on classes, inheritance is auto-managed by `set_class_serialize` (toggle off → modify → on if you need to swap a serialize class's base).

| param | type | default | notes |
|-------|------|---------|-------|
| `class` | string | required | must be an existing `Class` (not extern-only) |
| `base` | string | required | any `ExternClass` or `Class` |
| `virtual` | bool | `false` | sets the C++ `virtual` modifier on the inheritance |

**Returns:** `{class, base, virtual}`.
**Errors:** class is serialize-on; class == base; would create a cycle (`base->IsBaseClass(class)`); class already inherits from this base.

### `remove_inherit` `{class, base}`
Removes the inheritance edge from `class` to `base`.

**Restricted to non-serialize classes** (same rationale as `add_inherit`).

**Returns:** `{class, base}`.
**Errors:** class is serialize-on; no such edge exists.

### `get_member` `{class, name}`
Rich record for a member: type, access, flags, initialization, note, getter/setter access. The lightweight `find_member` returns only `{name, type}`.

**Returns:** or `null` if not found:
```json
{
  "name": "count",            // bare — the prefix is class-level (see Member-name convention)
  "prefixed_name": "_count",  // = class.member_prefix + name (codegen form)
  "type": "int",
  "access": "private",
  "static": false,
  "serialize": true,
  "initialization": "0",
  "note": "...",
  "getter": "public",         // or "none"
  "setter": "none"
}
```

### `add_member` `{class, name, type, access?, static?, serialize?, initialization?, note?, getter?, setter?}`
Creates a new member.

| param | type | default | notes |
|-------|------|---------|-------|
| `class` | string | required | |
| `name` | string | required | must be unique on the class |
| `type` | string | required | accepts modifiers (`Foo*`, `Foo&`, `Foo[]`, `Foo[N]`); bare type must already exist |
| `access` | `"public"` \| `"protected"` \| `"private"` | `"private"` | |
| `static` | bool | `false` | |
| `serialize` | bool | `true` | include in generated `Serialize` body |
| `initialization` | string | empty | C++ initializer expression |
| `note` | string | empty | |
| `getter` / `setter` | `"none"` \| `"public"` \| `"protected"` \| `"private"` | `"none"` | when set, creates a `GetMemberMethod` / `SetMemberMethod` with that access; static flag mirrors the member |

**Returns:** the rich `get_member` record.

### `delete_member` `{class, name}`
Removes the member; cascades to its getter/setter if any (owned-active relation). Skips the GUI's confirmation dialog.
**Returns:** `{class, name}`.

### `set_member_name` `{class, name, value}`
Renames the member; rejects collisions with other members on the same class.
**Returns:** the rich record.

### `set_member_type` `{class, name, value}`
Changes the member's type. `value` accepts modifiers like `add_member`. Re-parents the member onto the new `Type` (matches the dialog's `Type::MoveVariableLast` path). Existing getter/setter are **not** auto-regenerated — call `set_member_getter` / `set_member_setter` explicitly if signatures need to refresh.

### `set_member_access` `{class, name, value}` — `"public"` / `"protected"` / `"private"`
### `set_member_static` `{class, name, value}` — bool. Keeps getter/setter static-flag in sync (mirrors dialog).
### `set_member_serialize` `{class, name, value}` — bool
### `set_member_initialization` `{class, name, value}` — string (use `""` to clear)
### `set_member_note` `{class, name, value}` — string

### `set_member_getter` `{class, name, access}`
### `set_member_setter` `{class, name, access}`
Manage the member's `GetMemberMethod` / `SetMemberMethod`.

`access`: `"none"` deletes the existing method (if any); `"public"` / `"protected"` / `"private"` creates the method if absent and sets its access. Mirrors `CMemberDialog::Update`'s create/delete/update flow.

**Returns:** the rich `get_member` record.

---

### `add_method` `{class, name, return_type?, args?, access?, virtual?, static?, const?, pure?, body?}`
Creates a new method on the class. Goes through the same path as a GUI add — undo, view refresh, `NotifyAddMethod` all run.

| param | type | default | notes |
|-------|------|---------|-------|
| `class` | string | required | |
| `name` | string | required | |
| `return_type` | string | `"void"` | type modifiers (`*`, `&`, `[]`, `[N]`) parsed from string |
| `args` | array | `[]` | each `{name, type, default?}` |
| `access` | `"public"`\|`"protected"`\|`"private"` | `"public"` | |
| `virtual`, `static`, `const`, `pure` | bool | `false` | |
| `body` | string | empty | code for the `//@CODE_NNNN` user region |

**Returns:** the created `MethodRecord`.

### `set_method_body` `{class, name | id, body}`
Replaces the user-region body of an existing method. Prefer `id` when overloads exist.
**Returns:** `MethodRecord`. If the method is a constructor or destructor and the new body lacks `ConstructorInclude(...)` / `DestructorInclude(...)`, the result also includes a `warning` field — the operation still succeeds, but relation wiring / cascade-delete will be missing in the generated code.

### `delete_method` `{class, name | id}`
Removes the method (NotifyRemoveMethod, full GUI-equivalent path). Prefer `id` for overloads.
**Returns:** `{id, name, class}` (captured before deletion).

### `add_constructor` `{class, init?, explicit?, args?, access?, body?}`
Creates a `Constructor` (Method subclass with `_init` initializer list and `_explicit` flag). Mirrors `BaseClass::OnAddConstructor`: name is auto-set to the class name, return-type is empty (constructor convention), and `Constructor::CreateArguments` runs to seed any default arguments. Caller-supplied `args` are appended after that.

| param | type | default | notes |
|-------|------|---------|-------|
| `class` | string | required | |
| `init` | string | empty | initializer list, e.g. `"_count(0), _flag(false)"` |
| `explicit` | bool | `false` | C++ `explicit` keyword |
| `args` | array | `[]` | extra args appended after `CreateArguments` |
| `access` | `"public"`/`"protected"`/`"private"` | `"public"` | overrides the default |
| `body` | string | empty | constructor body (`@CODE_NNNN` region) |

**Returns:** the `MethodRecord` for the new constructor.

### `add_argument` `{class, method, name, type, default?}`
Appends one argument to an existing method.
**Returns:** the parent `MethodRecord` (with the new arg in `args`).

### Method attribute setters
All accept either `id` (preferred for overloads) or `name` to identify the method on the class.

- `set_method_access` `{class, id|name, value}` — `"public"` / `"protected"` / `"private"`
- `set_method_virtual` `{class, id|name, value:bool}`
- `set_method_static` `{class, id|name, value:bool}`
- `set_method_const` `{class, id|name, value:bool}`
- `set_method_pure` `{class, id|name, value:bool}`
- `set_method_dll_export` `{class, id|name, value:bool}`
- `set_method_name` `{class, id, value}` — rename; prefer `id` since name lookup is ambiguous with overloads
- `set_method_return_type` `{class, id|name, value}` — accepts modifiers; re-parents onto the new `Type`
- `set_method_note` `{class, id|name, value}` — the `@NOTE_NNNN` comment above the method, distinct from the `@CODE_NNNN` body managed by `set_method_body`

**Returns:** the updated `MethodRecord`.

### Argument-level commands
Method is identified by `method` (name) or `method_id`. Argument is identified by `arg` (name) or `arg_index` (0-based).

- `delete_argument` `{class, method|method_id, arg|arg_index}` — removes one argument
- `set_argument_name` `{class, method|method_id, arg|arg_index, value}`
- `set_argument_type` `{class, method|method_id, arg|arg_index, value}` — accepts modifiers
- `set_argument_default` `{class, method|method_id, arg|arg_index, value}`
- `move_argument` `{class, method|method_id, arg|arg_index, position, target?|target_index?}` — `position` is `"first"` / `"last"` / `"before"` / `"after"`; `target` (or `target_index`) required for `before`/`after`

`delete_argument` returns `{method_id, arg}`; the setters return the parent `MethodRecord`.

---

## Extern class

### `add_extern_class` `{name, suppress_forward?}`
Creates an `ExternClass`. Rejects duplicates against any existing `BaseClass` (Class or ExternClass).
**Returns:** `{name, kind:"ExternClass", suppress_forward}`.

### `delete_extern_class` `{name}`
Removes an `ExternClass`. Rejects if any class still inherits from it (incoming inherit count > 0). Use `delete_class` for a `Class` (the typed lookup rejects mismatches).
**Returns:** `{name}`.

### `list_extern_classes`
Names of every pure `ExternClass` (excludes `Class`, which is reachable via `list_classes`).
**Returns:** `[string, ...]`.

### `set_extern_class_suppress_forward` `{name, value}`
Toggles the `_suppressForwardDeclaration` flag on the named class (works for `Class` or `ExternClass`).
**Returns:** `{name, suppress_forward}`.

---

## Type system

### `add_type` `{name}`
Creates a new `OtherType`. Rejects duplicates against any existing `Type` (OtherType, ExternClass, or Class — all live in the same `FindType` namespace).
**Returns:** `{name, kind:"OtherType"}`.

### `list_types`
Every `Type` in the model, tagged with `kind` (`"OtherType"` / `"ExternClass"` / `"Class"`).
**Returns:** `[{name, kind}, ...]`.

### `delete_type` `{name}`
Removes an `OtherType`. Rejects if:
- any `Variable` references it (mirrors `OtherType::OnDelete`)
- it's a built-in name (`""`, `"void"`, `"int"`)
- the type is actually a `Class` or `ExternClass` (use `delete_class` / `delete_extern_class`)

**Returns:** `{name}`.

---

## Class commands (continued)

### `set_class_name` `{name, value}`
Renames a `Class`. Rejects collisions with any existing `BaseClass`.
**Returns:** `{name}`.

---

## Relations

A relation is a directed edge between two classes (the **from-class** and the **to-class**) with a name on each side. Lookup key for read/mutate commands: `{class, from_name, to_name?}` — `class` is the from-side; `to_name` is optional and only needed to disambiguate when the same `from_name` is reused with multiple to-classes.

**Kinds (`kind` field, mutually exclusive):**
- `"single"` — a single-pointer relation. To-side is at most one object.
- `"multi"` — a per-instance container of objects. Supports filter iterators and tree implementations.
- `"static_multi"` — a single shared container across all instances. Forbidden when either side is a template or a serialize class (matches `IDC_STATICMULTI` gate in `RelationDialog`).

**Aggregation (`owned`):** when `true`, the relation owns the target objects (cascade-deletes them). Default `true`.

**Critical (`critical`):** when `true`, the container is wrapped in CB_CRITICAL_* macros (locking on access).

**Filter (`filter`):** when `true`, the multi relation generates filter-iterator variants that take a predicate and only iterate matching elements. Not applicable to `static_multi`.

**Implementations (multi only — not single, not static_multi):**
- `"list"` — plain doubly-linked list (default).
- `"value_tree"` — bit-pattern indexed tree on an integer-typed member. Cheaper than AVL (no rotations, no compare beyond bit shift) but unordered. Use when the indexed member is a unique ID and you only need fast find. Set `unique:true` to promote to `UniqueValueTree` (the model knows duplicates are impossible).
- `"avl_tree"` — balanced BST keyed on a member supporting `<`, `<=`, `>`, `>=`, `==`. Works for integer or string-like members; gives ordered iteration.

For `value_tree` and `avl_tree`, `member` is required and must exist on the to-class.

### `list_relations` `{class}`
All outgoing relations of the class. **Returns:** `[RelationRecord, ...]`.

### `get_relation` `{class, from_name, to_name?}`
**Returns:** `RelationRecord` or sets `error: "no such relation on the class"`.

### `add_relation` `{from_class, to_class, from_name?, to_name?, kind?, owned?, critical?, filter?, implementation?, member?, unique?, note?}`

| param | default | notes |
|-------|---------|-------|
| `from_class`, `to_class` | required | both must be Classes (not extern) |
| `from_name` | `from_class.GetBaseName()` | unique on the from-class with `to_name` |
| `to_name` | `to_class.GetBaseName()` | |
| `kind` | `"multi"` | `"single"` / `"multi"` / `"static_multi"` |
| `owned` | `true` | aggregation |
| `critical` | `false` | |
| `filter` | `false` | rejected on `static_multi` |
| `implementation` | `"list"` | only valid for `kind:"multi"` |
| `member` | required for trees | must exist on to-class |
| `unique` | `false` | only valid for `value_tree` |
| `note` | empty | |

**Returns:** the `RelationRecord`.

### `delete_relation` `{class, from_name, to_name?}`
**Returns:** `{from_class, to_class, from_name, to_name}`.

### Relation setters
All take `{class, from_name, to_name?, value}`:

- `set_relation_from_name`, `set_relation_to_name` — rename endpoint
- `set_relation_note`
- `set_relation_owned` — bool
- `set_relation_critical` — bool
- `set_relation_filter` — bool; rejects on `static_multi`

`kind` change post-creation is **not** exposed: it cascades into find-method deletion, macro-method regeneration, and constructor-include arg updates that the dialog handles. Delete + recreate instead.

### `set_relation_implementation` `{class, from_name, to_name?, implementation, member?, unique?}`
Changes the relation's implementation. Deletes the existing `RelationMember` and creates a new one. Restricted to multi (non-static) relations. Same `member` / `unique` rules as `add_relation`.
**Returns:** the `RelationRecord`.

## Class diagrams

A `ClassDiagram` is a named drawing surface owned by the DataModel. It shows selected classes (as `ClassShape`s) and the `Inherit` / `Relation` edges between them. Visibility flags on the diagram control which member/method categories render on each shape. UML notation is set ON unconditionally (the legacy non-UML style isn't exposed).

### `add_class_diagram` `{name, classes?, auto_place?, public_members?, public_methods?, protected_members?, protected_methods?, private_members?, private_methods?, get_set_methods?}`

| param | default | notes |
|-------|---------|-------|
| `name` | required | unique within the model |
| `classes` | `[]` | list of class names to add as initial shapes |
| `auto_place` | `false` | run `Grid::Place` after adding the classes |
| `public_members` / `public_methods` | `true` | visibility flags |
| `protected_*` / `private_*` / `get_set_methods` | `false` | |

**Returns:** the diagram record `{name, classes:[], <flags...>}`.

### `list_class_diagrams`
Names of every diagram on the model.

### `get_class_diagram` `{name}`
Full record incl. the classes currently on it.

### `delete_class_diagram` `{name}`

### `add_class_to_diagram` `{diagram, class}`
Adds one class as a new `ClassShape`. Position is the first non-overlapping grid slot; run `auto_place_diagram` afterward to clean up layout. Rejects duplicates.

### `remove_class_from_diagram` `{diagram, class}`

### `auto_place_diagram` `{name}`
Runs `Grid::Place` on the diagram. Equivalent to **Class Diagram → Optimize Placement** in the GUI.

### Showing members / methods on class shapes

Diagram-level visibility flags (`public_members` / `public_methods` / etc.) only gate **what's allowed to render**. Each `ClassShape` still needs explicit `MemberShape` / `MethodShape` children for the items to actually appear — exactly what the GUI's "Select members and methods" dialog adds.

All three commands below run `Grid::Place` automatically after adding shapes (since adding features grows the class shape and may overlap neighbours). Pass `auto_place:false` to skip — useful when batching multiple show calls before a single placement at the end.

#### `show_class_members` `{diagram, class, members:[name,...], auto_place?}`
Adds a `MemberShape` for each named member on the class's shape. Members already shown are skipped. Errors out on the first unknown member name.
**Returns:** `{diagram, class, added:[name,...]}`.

#### `show_class_methods` `{diagram, class, methods:[name,...], auto_place?}`
Same, for methods. Names are matched by `FindMethodWithName` so overloads share a name match (the first one wins).

#### `show_class_features` `{diagram, auto_place?}`
Bulk: walks every class shape on the diagram and adds shapes for every feature matching the diagram's visibility flags. Filtering rules:

- **members:** include if `access` matches an enabled diagram flag (`public_members` / `protected_members` / `private_members`)
- **methods:** include if (a) non-macro, (b) not a constructor or destructor, (c) `access` matches an enabled flag, (d) if it's a `GetMemberMethod` / `SetMemberMethod` then `get_set_methods` must be on

Idempotent — already-shown shapes are skipped, so safe to call repeatedly.
**Returns:** `{diagram, member_shapes_added, method_shapes_added}`.

---

## Groups (folder structure)

CB's tree topology (sibling vs. nested):

```
(doc root, invisible)
├── <model-name>                         — the model's top node, named at model
│                                          creation (e.g. "Matrix", "ClassBuilder").
│                                          Backed by the `DataModel` class — the
│                                          name displayed is the model name, not
│                                          the literal string "DataModel".
│   ├── classes / class-diagrams / sequence-diagrams at top level
│   └── ClassGroup, ClassGroup, …        — direct under the model node
│       └── (classes + diagrams within)
├── MetaGroup #1                         — SIBLING of the model node in the tree
│   └── ClassGroup, ClassGroup, …
│       └── (classes + diagrams within)
├── MetaGroup #2
└── (Actors, ExternClasses, … other doc-level nodes)
```

So **classes and diagrams can live at one of three levels**:

1. **Top** — directly under the model node (no parent param).
2. **Inside a MetaGroup** — pass `parent_meta_group_id`.
3. **Inside a ClassGroup** — pass `parent_class_group_id`. The ClassGroup itself sits under either the model node or a MetaGroup.

(In the model layer, MetaGroups are owned by the DataModel via a passive relation; the GUI presents them as siblings of the model node. Both ClassGroup constructors exist — `ClassGroup(DataModel*)` and `ClassGroup(MetaGroup*)` — so the pipe routes accordingly.)

Groups are addressed by **id** (names are not guaranteed unique across the model — two ClassGroups in different MetaGroups can share a name).

### `add_meta_group` `{name}`
**Returns:** `{id, name, kind:"meta"}`.

### `add_class_group` `{name, parent_meta_group_id?}`
Creates a ClassGroup at DataModel root or inside the given MetaGroup.
**Returns:** `{id, name, kind:"class", parent_meta_group_id?}`.

### `list_groups`
Walks DataModel-direct ClassGroups, plus every MetaGroup and its nested ClassGroups. Result is a flat array of `{id, kind, name, parent_meta_group_id?}` entries.

### `delete_group` `{id}`
Works for MetaGroup, ClassGroup, **or MemberAndMethodGroup** (see below). CB will refuse a non-empty group.

### In-class `MemberAndMethodGroup`

At class level, a member or method either sits **directly under its class** or **inside a MemberAndMethodGroup** (an in-class folder). The group is a non-owning index — the class still owns the member/method; the group just tags it.

#### `add_member_and_method_group` `{class, name}`
**Returns:** `{id, name, class}`.

#### `set_member_group` `{class, member, group_id?}`
Moves the named member into the group (`group_id` ≠ 0) or removes it from its current group, putting it back directly under the class (`group_id` omitted or 0).

#### `set_method_group` `{class, method | method_id, group_id?}`
Same as `set_member_group` but for methods. Use `method_id` for overloaded names.

Add/move/remove flow:
```powershell
$g = Send 'add_member_and_method_group' @{ class = 'Foo'; name = 'Layout' }
Send 'set_method_group' @{ class = 'Foo'; method = 'OptimizePlacement'; group_id = $g.id }
Send 'set_method_group' @{ class = 'Foo'; method = 'SpaceLifeLines';   group_id = $g.id }
```

### Using a group as a diagram parent

Both `add_class_diagram` and `add_sequence_diagram` (and `add_call_trace`) accept any of these parent params, in order of precedence:

| param | parent | example |
|-------|--------|---------|
| `parent_class` | a named `BaseClass` | nesting an SD under a class |
| `parent_class_group_id` | a `ClassGroup` id | nesting in a folder |
| `parent_meta_group_id` | a `MetaGroup` id | nesting in a meta-folder |
| *(none)* | `DataModel` root | top-level |

So to put an OptimizePlacement walkthrough in the "SequenceDiagram classes" folder:

```powershell
$groups = Send 'list_groups'
$folder = $groups | Where-Object { $_.name -eq 'SequenceDiagram classes' -and $_.kind -eq 'class' }
Send 'add_call_trace' @{
    name                  = 'OptimizePlacement walkthrough'
    start_class           = 'SequenceDiagram'
    start_method          = 'OptimizePlacement'
    parent_class_group_id = $folder.id
}
```

---

## Actors and sequence diagrams

An `Actor` is a model-level entity (under the `Actors` tree node) representing an external user / system. Actors are placed on sequence diagrams as `ActorLifeLineShape`s, alongside `ClassLifeLineShape`s (one per class). A `SequenceDiagram` owns its lifelines and an implicit `RootActivationShape` (the "outside the system" trigger). Activations are `ChildActivationShape`s arranged in a tree rooted at the root activation; nested activations are wired via `SignalShape`s. Each activation may bind to a `Method` (passive relation) so a viewer can navigate to the implementation.

Lifelines, activations and signals are addressed by the global object **id** (the same UINT returned in `MethodRecord.id`, used by `@CODE_NNNN` markers). Diagrams and actors are addressed by **name**.

### Actor commands

#### `add_actor` `{name, note?}`
Creates an Actor on the model.
**Returns:** `{name, id}`.

#### `list_actors`
**Returns:** array of actor names.

#### `delete_actor` `{name}`
Fails if the actor still has lifelines on any diagram.

#### `set_actor_note` `{name, value}`

### Sequence-diagram lifecycle

#### `add_sequence_diagram` `{name, parent_class?, scale?, numbering?, arguments?, argument_names?, scope?, caption?, note?}`

| param | default | notes |
|-------|---------|-------|
| `name` | required | unique within the model |
| `parent_class` | DataModel root | name of a BaseClass to nest the diagram under |
| `scale` | 80 | percent |
| `numbering` | `"none"` | one of `"none" / "1" / "1.1.1" / "a" / "a.a.a" / "A" / "A.A.A"` |
| `arguments` | `false` | show signal arguments |
| `argument_names` | `false` | include arg names alongside types |
| `scope` | `false` | show method scope qualifiers |
| `caption` / `note` | empty | RTF-style caption / note |

**Returns:** the full SequenceDiagramRecord (see below).

#### `list_sequence_diagrams`
**Returns:** array of names.

#### `get_sequence_diagram` `{name}`
**Returns:** SequenceDiagramRecord:
```json
{
  "name":"...", "scale":80, "numbering":"none",
  "arguments":false, "argument_names":false, "scope":false,
  "caption":"", "note":"",
  "lifelines":   [LifeLineRecord, ...],
  "activations": [ActivationRecord, ...]
}
```

#### `delete_sequence_diagram` `{name}`
Fails if any view is still open on it.

### Lifelines

#### `add_actor_lifeline` `{diagram, actor, x?}`
Adds an `ActorLifeLineShape` on the named diagram for the named actor. `x` defaults to the right edge of the current last lifeline + 20.
**Returns:** LifeLineRecord `{id, name, kind:"actor", actor, x}`.

#### `add_class_lifeline` `{diagram, class, x?}`
Adds a `ClassLifeLineShape` for the named class.
**Returns:** LifeLineRecord `{id, name, kind:"class", class, x}`.

#### `list_lifelines` `{diagram}`
**Returns:** array of LifeLineRecords in left-to-right list order.

#### `delete_lifeline` `{diagram, id}`
Deletes a lifeline (and every activation hosted on it, plus signals).

### Activations

#### `add_root_child_activation` `{diagram, lifeline, method?, class?, method_id?, creation?, destruction?}`
Adds a top-level activation directly under the diagram's root. Use this for the activation that *kicks off* the sequence (no incoming signal). Bind it to a method by either `{class, method}` or `{method_id}` — both optional.
**Returns:** ActivationRecord.

#### `add_child_activation` `{diagram, sender_activation, lifeline, method?, class?, method_id?, creation?, destruction?, signal_name?, signal_label?, signal_async?, signal_enable_return?, signal_return?}`
Adds a nested activation invoked by a signal from `sender_activation`. The signal shape is created automatically; its attributes can be overridden via the `signal_*` fields, or set later via the `set_signal_*` commands.
**Returns:** ActivationRecord with `signal_id`.

#### `set_activation_method` `{diagram, activation, method?, class?, method_id?}`
Re-binds (or clears) the activation's method binding. Pass no method fields to clear.
**Returns:** ActivationRecord.

#### `add_signal` `{diagram, sender_activation, receiver_activation, signal_name?, signal_label?, signal_async?, signal_enable_return?, signal_return?}`
Wires two **existing** activations together — the pipe equivalent of the GUI Ctrl+click "Add Message" connect-flow. Re-parents the receiver into the sender's subtree (preserving the receiver's own descendants) and creates a `SignalShape` between them. If the receiver was already nested under another activation, the existing signal is reused (just moved to the new sender) rather than a duplicate being created.

Use this when you want to lay down activations in arbitrary order — typically place a few `add_root_child_activation` (loose under root), then `add_signal` to wire them up after. The `add_child_activation` shorthand combines "create + connect" but requires the sender to exist first; `add_signal` decouples the two.

**Refuses:**
- Same sender + receiver (`"sender and receiver are the same activation"`).
- Sender is already the receiver's direct parent (`"receiver is already a direct child of sender"`).
- Sender is in the receiver's subtree (`"would create a cycle"`).

**Returns:** the receiver's updated ActivationRecord (now including `signal_id` and `sender_activation_id`).

#### `delete_activation` `{diagram, activation}`
Cascades to children and their signals.

### Signal setters

All addressed by `{diagram, signal, value}`. The signal id comes from the `signal_id` field on its receiver `ActivationRecord`.

- `set_signal_name` — the displayed method-call text
- `set_signal_label` — the optional `[…]` guard / label
- `set_signal_return` — the return-value text
- `set_signal_note` — note text
- `set_signal_async` — bool (async/sync)
- `set_signal_enable_return` — bool (draw the return arrow)
- `set_signal_scope` — bool (show class scope on call)
- `set_signal_arguments` — bool (show argument types)
- `set_signal_argument_names` — bool (show argument names alongside types)

### Layout

Same three actions exposed in the GUI right-click menu:

- `optimize_placement` `{diagram}` — barycenter reorder, activation-offset reset, horizontal packing, leftmost snap.
- `space_lifelines` `{diagram}` — horizontal-only spacing, preserving the current lifeline order.
- `reset_activation_offsets` `{diagram}` — clear manual vertical tweaks on every activation.

All three are undoable as a single step.

### Diagram view commands (GUI-facing) — ADDED 2026-07-02

#### `open_diagram` `{name}`
Opens the GUI view of a class- **or** sequence-diagram, exactly like double-clicking it in the tree (dockable view in the shell). `name` (alias `diagram`) is tried as a ClassDiagram first, then a SequenceDiagram. Each call opens a **new** view (sub-window semantics), so call once per diagram.
**Returns:** `{name, kind:"class_diagram"|"sequence_diagram"}`.

#### `export_diagram_svg` `{diagram, path, tight?, margin?}`
Renders the named diagram to a standalone `.svg` (vector, selection-free). Reuses the diagram's open view when present; opens one first when none exists. Works for class- and sequence-diagrams. Built for scripted / AI documentation pipelines: construct a diagram via the pipe, export it, embed the `.svg`.

By default (`tight` omitted or `false`) the export is at page extent (page size from the diagram, A4 default if unset). Pass `tight: true` to instead crop to the actual shapes' bounding rect, inflated by `margin` model-units of padding on every side (default `50`, i.e. 5 grid squares at the 10-unit snap) — use this when embedding a diagram into a document, so a small diagram doesn't export as mostly page whitespace. The view's **Export SVG** toolbar button always exports tight (a full-page export is rarely useful once you can crop); the pipe defaults to page extent for backward compatibility but should normally be called with `tight: true` too.
**Returns:** `{diagram, kind, path, tight}`.
**Errors:** unknown diagram name; `SVG export failed` when the Qt Svg module is unavailable or the file can't be written.

### Moving existing objects between parents

Pipe equivalent of GUI drag-and-drop: reparent things that already exist. Diagrams reparent via Gti's tree (the same call CB's own `Drop` handlers use); classes reparent by changing their `ClassGroup` membership (their DataModel ownership is unchanged); ClassGroups reparent between the model root and a MetaGroup.

#### `move_class_diagram` `{name, parent_class?, parent_class_group_id?, parent_meta_group_id?}`
Reparents an existing ClassDiagram. Parent params follow the same precedence as `add_class_diagram`; omit all three for the model root.

#### `move_sequence_diagram` `{name, parent_class?, parent_class_group_id?, parent_meta_group_id?}`
Same for a SequenceDiagram.

#### `move_class` `{class, parent_class_group_id?}`
Sets the named class's ClassGroup membership. Pass no `parent_class_group_id` (or 0) to remove it from its current group, putting it back at the model root level. Class ownership (always DataModel) is unchanged.

#### `move_class_group` `{id, parent_meta_group_id?}`
Moves a ClassGroup between the model root and a MetaGroup. Pass no `parent_meta_group_id` to move it to the root.

### Call-trace bootstrap

#### `add_call_trace` `{name, start_class, start_method | start_method_id, actor?, actor_id?, max_depth?, skip_methods?, skip_pattern?, parent_class?, parent_class_group_id?, parent_meta_group_id?, scale?, numbering?, arguments?, argument_names?, scope?, caption?, note?}`

Bootstraps a SequenceDiagram from a starting method by **text-scanning its body** for `Identifier(` call patterns, looking each name up in the model, and recursing depth-first. The root activation is bound to the start method; child activations are bound to whichever methods their call names unambiguously match. After the tree is built, `optimize_placement` is run for layout.

| param | default | notes |
|-------|---------|-------|
| `name` | required | diagram name, unique within the model |
| `start_class` + `start_method` (or `start_method_id`) | required | entry-point method |
| `actor` (name) or `actor_id` | none | if given, place an `ActorLifeLineShape` as the leftmost lifeline with a loose root activation, and hang the start activation below it via an auto-signal — i.e. "the actor calls `start_method`". This also makes the actor the layout *anchor* (first child of root) so `optimize_placement` keeps it leftmost. |
| `max_depth` | `3` | recursion cap (root + max_depth−1 levels of descendants) |
| `skip_methods` | `[]` | array of exact method-name strings to ignore — e.g. trivial getters/setters that add vertical noise without structural value. |
| `skip_pattern` | none | single ECMAScript regex; method names that match are skipped. Convenient when you want to prune "all the offset getters/setters" with one rule. Skipped names show up as `user_skipped` in `trace_stats` (separate from `ambiguous_skipped`). |
| `parent_class` / `parent_class_group_id` / `parent_meta_group_id` / `scale` / `numbering` / `arguments` / `argument_names` / `scope` / `caption` / `note` | inherited from `add_sequence_diagram` | same semantics |

**Returns:** the full SequenceDiagramRecord plus a `trace_stats` block:
```json
{ "name":"OptimizePlacement", "lifelines":[...], "activations":[...],
  "trace_stats": {
    "activations_created":    14,
    "ambiguous_skipped":       8,
    "unknown_class_skipped":   0,
    "max_depth":               3
  }
}
```

**How matching works:**
- Strings and comments are skipped in the body scan.
- A short keyword list filters C++ control-flow and casts (`if`, `dynamic_cast`, `sizeof`, …) and common MFC/CB types (`CRect`, `BOOL`, `RGB`, …) that look like calls but aren't.
- For each unique identifier found, the scan asks the model "is there exactly one class with a method by that name?"
  - **0 matches** → call ignored (stdlib, template, macro, or unknown name).
  - **1 match** → bind activation to that method, place on a lifeline for its class.
  - **>1 matches** → skipped (ambiguous); counted in `ambiguous_skipped` so you know to inspect.
- A method already on the current recursion path is skipped (cycle break); the same method may still appear under a different parent branch.
- **Loop marking:** the scanner tracks a brace stack and notes when a call sits inside the body of a `for` / `while` / `do` block. The created signal's *clause* gets set to `"*"` (the CB convention for "called in a loop"), so the diagram shows the loop semantics without needing the iterator class on a lifeline. If a name appears both inside and outside a loop, the `"*"` clause wins (conservative). Note: only braced loop bodies are detected — single-statement loops without `{}` are missed.

**Limitations:**
- No type analysis. Iterator-deref / pointer-deref calls (`iAct->Compare(...)`) only resolve when the method name happens to be unique across the model. Overloaded names — especially common ones like `Compare`, `Reset`, `Update` — produce ambiguous misses.
- Macro-generated methods (`GetFirstXxx`, `AddXxxLast`, `Sort##NameTo`, …) match by name too and can cause ambiguity.
- Treat the output as a **starting sketch**: prune false matches with `delete_activation`, add missing ones with `add_child_activation`.

**Example:** generate the OptimizePlacement walkthrough mentioned earlier, kicked off by an actor, placed in the "SequenceDiagram classes" folder, and with trivial offset getters/setters pruned:
```powershell
$groups = (Send 'list_groups').result
$folder = $groups | Where-Object { $_.name -eq 'SequenceDiagram classes' -and $_.kind -eq 'class' }
Send 'add_actor' @{ name = 'Some User' }
Send 'add_call_trace' @{
    name                  = 'OptimizePlacement walkthrough'
    start_class           = 'SequenceDiagram'
    start_method          = 'OptimizePlacement'
    actor                 = 'Some User'
    parent_class_group_id = $folder.id
    max_depth             = 4
    numbering             = '1.1.1'
    skip_pattern          = '^(Get|Set).*(Offset|Color)$'
}
```

**Layout note:** at the moment `add_call_trace` runs there's no view DC available, so `optimize_placement`'s "snap leftmost to canonical x" step uses pre-text-measurement bounding rects and the diagram can end up shifted right of where it'll eventually settle. Clicking **Optimize Placement** once in the GUI after the diagram opens corrects it (the view's DC is in scope by then). This is the same no-view-DC limitation noted in the memory entries for `Grid::Place` and pipe-driven layout.

### Records

**LifeLineRecord**
```json
{ "id":42, "name":"User",  "kind":"actor", "actor":"User",  "x":120 }
{ "id":43, "name":"Order", "kind":"class", "class":"Order", "x":340 }
```

**ActivationRecord**
```json
{
  "id":50,
  "lifeline_id":43,
  "method":"Place", "method_id":1234, "method_class":"Order",
  "signal_id":51,                    // omitted on root-child activations
  "sender_activation_id":40,
  "creation":false, "destruction":false,
  "sequence_number":1, "sequence_subnumber":0
}
```

---

## Find methods (relation children)

A `FindMethod` is a generated lookup that lives on the from-side of a multi relation. It iterates the relation and returns the first element whose value(s) match the given argument(s). Each argument carries a `path` that becomes the comparison expression in the codegen; `InitCode` chooses the impl-specific fast path (avl/value-tree) when an argument's `MemberArgument` matches the relation's tree-key, otherwise it generates an iterate-loop.

In the GUI a tree of reachable members + navigated relations is offered for selection; the pipe API has the script supply the same data explicitly: which Member or navigated Class each argument represents, plus the path string that the body should compare against.

### `add_find_method` `{class, from_name, to_name?, args, name?, access?, reverse?, next?}`

Creates a `FindMethod` on the named multi relation.

| param | default | notes |
|-------|---------|-------|
| `class` | required | the from-side class |
| `from_name`, `to_name?` | required / optional | relation lookup key |
| `args` | required | non-empty array — see below |
| `name` | `"Find" + ToName"` (or `"FindReverse" + ToName"` if reverse) | override the auto-generated name |
| `access` | `"public"` | |
| `reverse` | `false` | iterate `--i` instead of `++i` |
| `next` | `false` | adds a `startAfter<Type>` (or `startBefore<Type>` if reverse) defaulted to `0` for continuing iteration |

Each `args[i]` is `{path, member?, nav_class?, arg_name?}`. **Exactly one** of `member` or `nav_class` must be present:
- `member: {class, name}` — creates a `MemberArgument` bound to that Member; type and modifiers auto-derived. Use this for "compare a member of the iterated object" or "compare a member reachable via navigation that ultimately ends at a Member".
- `nav_class: "<Class>"` — creates a plain `Argument` typed `<Class>*`. Use this when the path navigates to an object identity (e.g. `"->GetParent()"`) rather than a member value. Default arg name is `p<Class>`; override with `arg_name`.

`path` is the access expression as the codegen will emit it — must start with `->` (e.g. `"->_id"`, `"->GetName()"`, `"->GetParent()->_id"`).

**Returns:** the created `MethodRecord`.

### `delete_find_method` `{class, from_name, to_name?, name|id}`
Removes a `FindMethod` from the relation. Prefer `id` (overloaded find methods can share names). **Returns:** `{id, name}`.

### `RelationRecord`
```json
{
  "from_class": "Foo",
  "to_class":   "Bar",
  "from_name":  "Foo",
  "to_name":    "Bar",
  "kind":       "multi",
  "owned":      true,
  "critical":   false,
  "filter":     false,
  "implementation": "list",   // or "value_tree" / "unique_value_tree" / "avl_tree"
  "member":     null,         // member name when implementation is a tree, else null
  "note":       ""
}
```

---

## Source generation / read-back

The headless equivalents of **File ▸ Save Source** and **File ▸ Read Source** —
so a pipe-driven workflow can edit the model (`set_method_body`,
`set_argument_type`, `add_method`, …) and then flush to disk and/or import disk
hand-edits without touching the GUI. Both `_chdir` to the project directory
first (codegen uses paths relative to it), exactly like the menu handlers.

### `write_source` `{modified_only?}`
Regenerates `.h`/`.cpp` from the model (`DataModel::SaveModifiedFiles` /
`SaveAllFiles`).

| param | type | default | notes |
|-------|------|---------|-------|
| `modified_only` | bool | `true` | `true` = only classes whose model changed since the last save (the dialog's "Save modifications"); `false` = rewrite every file |

**Returns:** `{modified_only, warnings, errors, warning_messages?, error_messages?}`.
**Note:** `set_method_body` and the other mutators update the *model* only; call
`write_source` afterward to get the change onto disk for a build.
**Note:** `Save{Modified,All}Files` end with `OnSaveDocument(GetPathName())` —
`write_source` therefore **also saves the `.cbz`** and clears the modified flag
(same as the GUI's Save Source). A `save_cbz:true` param still exists but is a
redundant second save. Still checkpoint with `save_cb` *before* `write_source`:
a failed write rolls the model back to the last-saved `.cbz`.

### `read_source` `{all?}`
Imports on-disk hand-edits back into the model — `//@CODE` bodies and
`@START_USER` regions (`DataModel::ReadAllFiles`). Marks one undo step.

| param | type | default | notes |
|-------|------|---------|-------|
| `all` | bool | `false` | `false` = only files modified on disk since the last save; `true` = re-read every file unconditionally |

**Returns:** `{all, warnings, errors, warning_messages?, error_messages?}`.

---

## CBZ round-trip (test only — see warning)

> **WARNING:** `load_cb` / `save_cb` are parity-test conveniences, not production-quality. `load_cb` corrupts views holding pointers into the swapped-out model — typically crashes the process within seconds (`ucrtbase` __fastfail). Use only against an inactive doc you can throw away. A clean rewrite is deferred to the Qt port.

### `save_cb` `{path}`
Writes the active doc as `.cbz` (CbArchive + Zstd). Does not change the doc's modified flag or pathname.
**Returns:** `{path, bytes_logical, bytes_on_disk}`.

### `load_cb` `{path}`
Replaces the active doc's contents from `path` via the CbArchive code path. **Crashes the GUI shortly after** — see warning above.
**Returns:** `{title, path, bytes_logical, objects_loaded}`.

---

## Record shapes

### `MethodRecord`
```json
{
  "id": 1234,
  "name": "Foo",
  "return_type": "int",
  "static": false,
  "virtual": false,
  "const": false,
  "pure": false,
  "dll_export": false,
  "args": [ { "name": "x", "type": "int", "default": "" } ]
}
```

### `MemberRecord`
```json
{ "name": "_count", "type": "int" }
```

### `BaseClassRecord`
```json
{ "name": "Foo", "kind": "Class" }
```
`kind` is `"Class"`, `"ExternClass"`, or `"BaseClass"`.

### `Argument` (inside `MethodRecord.args`)
```json
{ "name": "x", "type": "int*", "default": "" }
```
`type` includes the modifier suffixes (`*`, `**`, `&`, `[]`, `[N]`) just like the GUI shows them.

---

## Type-modifier syntax

`return_type` and argument `type` strings accept modifiers parsed by `ParseTypeString`:
- `Foo*`, `Foo**` — pointer / pointer-to-pointer
- `Foo&` — reference
- `Foo[]`, `Foo[N]` — array / sized array

The bare type name (without modifiers) must already exist in the model's Type list — `add_method` does **not** auto-create types.

---

## Document selection (server-side target, decoupled from GUI focus) — IMPLEMENTED 2026-06-24

The pipe targets ONE document. By default it follows the GUI's active model; `select_document` overrides that with a sticky server-side target so a script can drive one model while you edit another.

- `list_documents` → `{documents:[{title, path, modified, gui_active, selected}, ...], count}` — `gui_active` = what the GUI has up front; `selected` = what the pipe is currently targeting (with no explicit selection, that's the gui_active doc).
- `select_document {title | path}` → `{title, path, selected:true}` — sets the sticky server-side target; **exact** title/path match; error if no open doc matches. Does **not** change the GUI's active window.
- `current_document` → `{title, path, modified, explicit}` of the targeted doc, or `null` if none / it was closed. `explicit:false` means it's following GUI focus (no `select_document` in effect).
- `close_document {title | path, save?}` → `{closed:true, title, path}` — headless close, **no save prompt**. `save:true` writes the `.cbz` first, but only if the doc already has a path (an untitled doc closes without saving — avoids a blocking Save-As dialog). If the closed doc was the selected target, the target clears (commands follow GUI focus again until re-selected).

The target is **sticky** until the next `select_document`. If the targeted doc is closed, doc-targeting commands report `no active document` until a new one is selected (the target is honoured strictly — no silent fall-back to GUI focus once chosen).

## Gaps (not implemented yet — high-priority candidates)

Roughly ordered by leverage for tool-driven workflows. See [project_classbuilder_pipe_api_expansion_idea.md](../../.claude/...) memory for prioritisation rationale.

**Class / structural:** all core in. Future: groups (`add_group`, `set_class_group`).

**Member-level:** all in.

**Method-level:** all core in. Future: `set_method_calling_convention`, `set_method_signature` (atomic args swap).

**Type system:** all in — `add_type`, `delete_type`, `list_types`, and **`set_type_declaration {name, declaration}`** (sets an OtherType's typedef/enum/struct body; added 2026-06-24, closes the add_type→GUI round-trip).

**Diagrams:** *(still open)*
- `create_class_diagram(name, classes[])` with auto-layout (mirror `SelectClassesDialog` placement)
- `add_class_to_diagram(diagram, class)` / `remove_class_from_diagram(...)`

**Phase / lifecycle:** *(still open)*
- `set_phase(class|member|method, phase)` — bulk phase ops

**Diagnostic:** IMPLEMENTED 2026-06-24 —
- `list_commands` → `{commands:[...], count}` (every registered command name, sorted)
- `ping` now also returns `build` (the server's `__DATE__ " " __TIME__` stamp — detects a stale binary still owning the pipe)

---

## Transport — localhost TCP socket

The server listens on a **TCP socket at `127.0.0.1:51777`** (loopback only — never exposed off-box). Override the port with the `CB_CMD_PORT` environment variable (a DEV build typically uses a different port, e.g. `51778`, so it can run beside a stable instance). TCP replaced the old Windows named pipe so the connection method and these docs are **identical on Windows, macOS, and Linux** — connect from any language/tool that speaks TCP, one newline-terminated JSON request per line, one reply per line.

### Connecting from PowerShell (example)

```powershell
$port   = if ($env:CB_CMD_PORT) { [int]$env:CB_CMD_PORT } else { 51777 }
$client = [System.Net.Sockets.TcpClient]::new('127.0.0.1', $port)
$stream = $client.GetStream()
$rd = [System.IO.StreamReader]::new($stream)
$wr = [System.IO.StreamWriter]::new($stream); $wr.AutoFlush = $true

function Send($cmd) { $wr.WriteLine($cmd); $rd.ReadLine() }

Send '{"cmd":"list_classes"}'
Send '{"cmd":"add_method","params":{"class":"Foo","name":"Bar","return_type":"int","args":[{"name":"x","type":"int"}]}}'

$client.Close()
```

For scripts, dot-source the shared helper instead of re-inlining the connect: `. tools/CbCmd.ps1` then `Cb-Connect; Cb-Send '{"cmd":"ping"}'`. (The older one-off `tools/*.ps1` migration scripts still open the legacy named pipe inline and predate this switch — convert them to `CbCmd.ps1` if they need to run again.)
