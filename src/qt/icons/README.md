# ClassBuilder icons — the single place to work from

ClassBuilder has **two** icon sets. Both load a redrawn `.svg` from *this folder*
in preference to the legacy raster, so the whole set can be modernised **one
glyph at a time** — add a single `.svg`, rebuild, and only that icon goes vector.

| Set | Used for | Legacy source | SVG override (drop here) | Code |
|-----|----------|---------------|--------------------------|------|
| **Model / tree icons** | tree node rows + the tree's Add buttons reuse a few | `ClassBuilder/res/<name>.ico` (one file per glyph) | `<name>.svg` (same base name) | `qt/QtModelIcons.cpp` |
| **Toolbar glyphs** | the main / diagram / tree **toolbars** (Add Class, Add Note, Add Lifeline, Delete, zoom, …) | `ClassBuilder/res/Toolbar.bmp` — a 34-tile 16×16 **strip** | `tb_<name>.svg` | `qt/QtToolBarIcons.cpp` |

## Toolbar glyphs — the "new" toolbar buttons

The toolbar icons were never individual files: they live as tiles inside the one
bitmap strip `res/Toolbar.bmp`. To give you something to trace, the strip is
sliced into individual transparent PNGs in **`toolbar_src/`**, named by glyph:

```
toolbar_src/00_file_new.png ... 21_add_note.png 22_add_lifeline.png
24_add_class.png 30_add_argument.png 31_add_virtuals.png 32_add_isclass.png ...
```

(Re-generate with `pwsh tools/slice_toolbar_bmp.ps1` if the strip changes.)

To replace one, draw `tb_<name>.svg` **in this folder** (e.g. `tb_add_note.svg`,
`tb_add_lifeline.svg`, `tb_edit_delete.svg`). `QtToolBarIcons.cpp` lists every
`<name>` and the order; `Qt_ToolBarIcon(TG_*)` then prefers your SVG over the
strip tile. The mask colour in the strip is `RGB(192,192,192)` (made transparent
on load) — your SVG just needs a transparent background.

## Model / tree glyphs

Drop `<name>.svg` here using the **same base name** as `res/<name>.ico` (e.g.
`public_member.svg`, `class.svg`). `QtModelIcons.cpp` lists all 67 base names.
The `.ico` files are already individual, so they need no slicing — open them in
`ClassBuilder/res/` directly as references.

## Requirements

SVG rendering needs Qt's **Svg module** (`CB_HAVE_SVG`). The static Qt at
`C:/Qt-static` is currently qtbase-only — rebuild it with the `qtsvg` module
(`Projects/qt-static/build-static-qt.bat`) to enable the overrides. Until then
CMake reports "no Svg module" and the legacy `.ico` / strip tiles are used. No
code change is needed when qtsvg arrives — just reconfigure. The SVGs in this
folder are bundled by the `qt/icons/*.svg` glob in `CMakeLists.txt`.

## Drawing tips

- Design on a pixel grid; keep shapes axis-aligned so the 16 px render is crisp.
- Many model icons are systematic variants (public/protected/private = one glyph
  in three colours; relation icons = single/multi × act/pas × owned/static/cr).
  Draw the few base glyphs, then compose — not 60+ unique drawings.
- The toolbar "Add X" glyphs are typically the node glyph + a small green `+`
  badge; keep that family consistent.
- Save as Optimized / Plain SVG.
