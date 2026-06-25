# slice_toolbar_bmp.ps1 -- export each tile of the legacy MFC main-toolbar
# bitmap strip (ClassBuilder/res/Toolbar.bmp) as an individual transparent PNG
# into qt/icons/toolbar_src/, named by its ToolGlyph base name. These are the
# tracing references for the SVG redraw (drop the result as qt/icons/tb_<name>.svg
# -- see qt/icons/README.md). Re-run after the strip changes.
#
#   pwsh tools/slice_toolbar_bmp.ps1

Add-Type -AssemblyName System.Drawing

$root   = Split-Path $PSScriptRoot -Parent
$src    = Join-Path $root "ClassBuilder\res\Toolbar.bmp"
$outDir = Join-Path $root "qt\icons\toolbar_src"
New-Item -ItemType Directory -Force $outDir | Out-Null

# Order == IDR_MAINFRAME TOOLBAR button order (separators excluded) == ToolGlyph.
$names = @(
    'file_new','file_open','file_save','edit_undo','edit_redo',
    'read_source','save_source','edit_cut','edit_copy','edit_paste','edit_delete',
    'file_print','app_about','zoom_in','zoom_out','zoom_full',
    'add_classdiagram','add_sequencediagram','add_group',
    'add_relation_diagramonly','add_dependency','add_note','add_lifeline','add_message',
    'add_class','add_inherit','add_relation','add_member','add_function',
    'add_constructor','add_argument','add_virtuals','add_isclass','add_type'
)

$strip = New-Object System.Drawing.Bitmap($src)
$h = $strip.Height
$n = [int]($strip.Width / $h)
for ($i = 0; $i -lt $n; $i++) {
    $rect = New-Object System.Drawing.Rectangle(($i * $h), 0, $h, $h)
    $tile = $strip.Clone($rect, $strip.PixelFormat)
    $tile.MakeTransparent([System.Drawing.Color]::FromArgb(192,192,192))
    $name = if ($i -lt $names.Count) { $names[$i] } else { "tile_$i" }
    $tile.Save((Join-Path $outDir ("{0:00}_{1}.png" -f $i, $name)),
               [System.Drawing.Imaging.ImageFormat]::Png)
    $tile.Dispose()
}
$strip.Dispose()
Write-Host "Wrote $n tiles to $outDir"
