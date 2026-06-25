# Plan — per-project line-ending setting (CRLF / LF) for code generation

**Goal.** Each CB project chooses the line ending its generated `.cpp/.h` use — a
**fixed** stored choice, **not** auto-detection. The choice is platform-guessed
only when the project is *first* created; thereafter it is exactly what's stored.
This project (ClassBuilder itself) stays **CRLF**; the feature is for *new*
projects, and any existing/older `.cbz` is **flipped to CRLF on load**.

**Decisions (JV, 2026-06-25):**
- A **bool**, a **fixed** choice (no "auto" mode — auto always guesses).
- Surfaced as a **two-option radio** in the DataModel dialog's **Code Generation**
  group.
- **Reuse the dead `__notUsed_mfcSerialize` serialize slot** — no new field, no
  appended bytes, nothing misaligns in old files.
- **Older `.cbz` flip to CRLF on open**; the **platform guess** is used **only**
  when the setting is first established (new project).

---

## 1. The model field — recycle the dead slot

Rename the retired member `__notUsed_mfcSerialize` → **`_crlf`** (bool, `true` =
CRLF, `false` = LF) **in place**, keeping its **serialize position unchanged**
(store ~[DataModel.cpp:3453](../src/model/DataModel.cpp), read ~`:3506`). Because
the byte already exists in every `.cbz` ever written, reading/writing it
misaligns nothing — this is the whole reason for reusing the slot instead of
adding a field.

- **Constructor default = `true` (CRLF).**
- This is a self-host edit: renaming the member in CB regenerates
  `DataModel.{h,cpp}` (member + `GetCrlf`/`SetCrlf` + the serialize line). Do not
  reorder the serialize entry.

## 2. Serialization — flip older files to CRLF

`DataModel`'s serial version is **already 3** (bumped earlier; **no new bump** —
past bumps were touchy). The slot is read on every load (so nothing misaligns),
but it holds the **stale `_mfcSerialize` value**. Override by version:

- **Read:**
  ```cpp
  archive >> _crlf;
  if (_objectVersion < 3) _crlf = true;   // pre-v3: discard stale value, force CRLF
  ```
- **Store:** `archive << _crlf;` (saves stamp v3, keep the real choice).

Older files — the static `cbd`→`cbz` conversions, **v1/v2 at most** — load as CRLF
automatically.

**Caveat — files already at v3** (the limited set currently being worked on,
*including ClassBuilder's own `ClassBuilder.CBZ`*) carry the stale byte as `_crlf`,
because nothing distinguishes "v3 before this feature" from "v3 after." Worst case
you just **flip that one value**: open the project, set the CRLF radio, save.
**Do this for `ClassBuilder.CBZ` before the first regen** — if its `_crlf` reads
back `false`, a regen would emit CB's own source as LF. (See the
[serialize-field trap](../../.claude/projects/c--Users-jimmy-Projects-ClassBuilder/memory/project_classbuilder_serialize_field_add_breaks_old_cbz.md).)

## 3. The initial guess — first-set only, then fixed

The platform guess applies **only when a brand-new project is created** — never
re-evaluated at regen (that would be the "auto" behavior we're rejecting):

```cpp
// new project only:
#ifdef _WIN32
    _crlf = true;    // CRLF
#else
    _crlf = false;   // LF
#endif
```

Older projects do **not** get the platform guess — they're flipped to CRLF (§2).
After first-set, `_crlf` is whatever's stored or whatever the user picks in the
radio. (Optional later: a global app pref "default for new projects = CRLF / LF"
to override the platform guess once.)

## 4. Emit hook — normalize at file write (the one real lever)

`NL` is `#define NL "\015\012"` ([ClassBuilderInclude.h:458](../src/model/ClassBuilderInclude.h)),
used in **adjacent string-literal concatenation** (`"...)" NL`), so it cannot
become a runtime variable without rewriting hundreds of sites. **Leave `NL`
alone.** Codegen keeps building with internal CRLF; convert the whole buffer to
the project's EOL **just before writing each file**, via one helper:

```cpp
void Cb_WriteGeneratedFile(const char* path, CbString content, bool crlf);
// if (!crlf) content.Replace("\r\n", "\n"); then ofstream(binary) << content;
```

Route the generated-source writes through it (all `ofstream(... ios::binary)`):
- [Class.cpp:1797](../src/model/Class.cpp) — per-class `.cpp`
- [Class.cpp:2017](../src/model/Class.cpp) — per-class `.h`
- [DataModel.cpp:2335](../src/model/DataModel.cpp) — the master `.h`
- (grep `ofstream` across `src/model` for any other generated-output writers)

Each site can reach the `DataModel` (`GetDataModel()` / `GetClass()->GetDataModel()`)
→ read `GetCrlf()`. These are `//@CODE` bodies → edits round-trip into the model.

## 5. Read hook — keep the round-trip EOL-stable

The bison parser reads sources back with `fopen(..., "r")`
([Read.y:449/504/576](../src/model/Read.y)). **Text mode differs by platform** —
Windows `"r"` strips `\r`; macOS/Linux `"r"` keeps it. Don't let that leak into
the model.

Requirement: **normalize incoming EOL to the model's internal canonical (CRLF) on
read**, independent of platform and of the project's emit choice. Net effect: the
`.cbz` always stores method bodies as CRLF; only emitted files differ. So flipping
a project's setting (or opening an LF project) does **not** make every body look
"changed." Read binary + normalize explicitly rather than relying on `fopen("r")`.

## 6. UI — DataModel dialog, Code Generation group

`groupCodeGen` ([DataModelDialog.ui:207](../src/qt/DataModelDialog.ui)) is a grid of
checkboxes. Add a **two-radio** choice (auto-exclusive in the same parent):
- `radioCrlf` — "Windows (CRLF)"
- `radioLf`   — "Unix (LF)"

Bind in `DataModelDialog.cpp` like the existing checkboxes:
- **load:** `radioCrlf->setChecked(model->GetCrlf()); radioLf->setChecked(!model->GetCrlf());`
- **save:** `model->SetCrlf(radioCrlf->isChecked());`

## 7. Composes with `.gitattributes`

CB controls what's *written*; git controls what's *stored/checked-out*. New LF
project → set its CB choice to LF **and** give that repo `.gitattributes` with
`eol=lf`; a CRLF project (like this one) → both CRLF. Aligning them is what
prevents the flip-flop.

## 8. Test plan

1. Old (pre-v2) `.cbz` loads → `_crlf` forced true → CRLF output unchanged.
2. New project on Windows → CRLF radio; on Mac → LF radio (once CB runs there).
3. Flip a v2 project CRLF↔LF → regenerate → files switch EOL; method-body bytes in
   the `.cbz` are **identical** both ways.
4. Round-trip: regenerate LF, read sources back, save → no spurious body changes.
5. Self-host: build CB itself (CRLF) → output unchanged.

## 9. Order of work

1. Rename `__notUsed_mfcSerialize` → `_crlf` in CB (self-host), default `true`,
   keep serialize slot position; add the `if (_objectVersion < 3) _crlf = true;`
   read override (version is already 3 — no new bump). Flip `ClassBuilder.CBZ` to
   CRLF + save before regenerating.
2. `Cb_WriteGeneratedFile` helper; route the write sites through it.
3. Normalize the read path to canonical CRLF.
4. Dialog radios + binding.
5. New-project platform guess (first-set only; + optional global pref).
6. Regenerate, self-host-verify, test per §8.
