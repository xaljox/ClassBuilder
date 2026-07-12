# gen_toolbar_icon_svgs.ps1 -- generate the tb_<name>.svg toolbar glyphs into
# src/qt/icons/ (QtToolBarIcons prefers tb_<name>.svg over the legacy
# res/Toolbar.bmp strip tile, per glyph).
#
# Same design language as the model icons (tools/gen_model_icon_svgs.ps1):
# identical palette, and every Add-button reuses the matching model glyph
# (diamond, class square, folder, [T][N], diagram, ...) shrunk to the lower
# right, with the shared yellow "add" star at the upper left -- the same
# composition as the legacy tiles. File/edit/zoom glyphs are drawn as clean
# outline icons in one dark ink.
#
# tb_edit_undo.svg / tb_edit_redo.svg are hand-made already (green, per the
# disabled-greying note in QtToolBarIcons.cpp) and are NOT regenerated here.
#
# Deterministic; outputs are committed. Touch src/qt/resources.qrc after a
# re-run (AUTORCC misses listed-file content changes).

$ErrorActionPreference = 'Stop'
$outDir = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\src\qt\icons'))
$utf8 = New-Object System.Text.UTF8Encoding($false)

$P = @{
    memberFill = '#29C4D9'; memberEdge = '#0E7A8A'
    methodFill = '#E93BDD'; methodEdge = '#8E1287'
    ctorFill   = '#47B34F'; ctorEdge   = '#1F6E26'
    dtorFill   = '#E8433C'; dtorEdge   = '#8E1B18'
    blueFill   = '#3D6BE5'; blueEdge   = '#16309B'
    yellFill   = '#FFE14D'; yellEdge   = '#937B00'
    foldFill   = '#FFE3A1'; foldEdge   = '#B8912D'
    ink        = '#3C4043'
    inkSoft    = '#5F6368'
    starFill   = '#F5C400'; starEdge   = '#8A6D00'
    paper      = '#FFFFFF'; paperEdge  = '#5F6368'
}

function Save-Tb([string]$name, [string]$body)
{
    $svg = "<svg xmlns=`"http://www.w3.org/2000/svg`" viewBox=`"0 0 16 16`">`n" +
           $body + "`n</svg>`n"
    [IO.File]::WriteAllText((Join-Path $outDir "tb_$name.svg"), $svg, $script:utf8)
}

# The shared "add" star, upper-left.
$star = @"
  <path d=`"M3.1 0.4 L3.9 2.3 L5.8 3.1 L3.9 3.9 L3.1 5.8 L2.3 3.9 L0.4 3.1 L2.3 2.3 Z`"
        fill=`"$($P.starFill)`" stroke=`"$($P.starEdge)`" stroke-width=`"0.8`" stroke-linejoin=`"round`"/>
"@

# Wrap an object glyph: scale 0.72, anchored lower-right, star on top-left.
function AddGlyph([string]$objectSvg)
{
    return "$star  <g transform=`"translate(4.5 4.5) scale(0.72)`">`n$objectSvg`n  </g>"
}

# ---- shared mini-glyphs (match gen_model_icon_svgs.ps1 geometry) -----------
function Diamond([string]$fill, [string]$edge)
{
    return "    <path d=`"M8 1.4 L14.6 8 L8 14.6 L1.4 8 Z`" fill=`"$fill`" stroke=`"$edge`" stroke-width=`"1.4`" stroke-linejoin=`"round`"/>"
}
$classGlyph = @"
    <rect x=`"0.8`" y=`"4.8`" width=`"6.4`" height=`"6.4`" rx=`"0.8`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
    <path d=`"M7.2 8 H9.8 M9.8 2.6 V13.4 M9.8 2.6 H11.4 M9.8 8 H11.4 M9.8 13.4 H11.4`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <rect x=`"11.6`" y=`"0.9`" width=`"3.6`" height=`"3.4`" fill=`"$($P.yellFill)`" stroke=`"$($P.yellEdge)`" stroke-width=`"0.9`"/>
    <rect x=`"11.6`" y=`"6.3`" width=`"3.6`" height=`"3.4`" fill=`"$($P.methodFill)`" stroke=`"$($P.methodEdge)`" stroke-width=`"0.9`"/>
    <rect x=`"11.6`" y=`"11.7`" width=`"3.6`" height=`"3.4`" fill=`"$($P.memberFill)`" stroke=`"$($P.memberEdge)`" stroke-width=`"0.9`"/>
"@
$inheritGlyph = @"
    <rect x=`"3.5`" y=`"1.4`" width=`"9`" height=`"4`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
    <polygon points=`"8,7 11.2,10.4 4.8,10.4`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
    <rect x=`"7.35`" y=`"10.4`" width=`"1.3`" height=`"4.2`" fill=`"$($P.ink)`"/>
"@
$folderGlyph = @"
    <path d=`"M1.2 4.6 a1 1 0 0 1 1 -1 H5.6 l1.4 1.6 H13.8 a1 1 0 0 1 1 1 V12 a1 1 0 0 1 -1 1 H2.2 a1 1 0 0 1 -1 -1 Z`"
          fill=`"$($P.foldFill)`" stroke=`"$($P.foldEdge)`" stroke-width=`"1`" stroke-linejoin=`"round`"/>
"@
$classdiagramGlyph = @"
    <rect x=`"5`" y=`"1`" width=`"6`" height=`"4`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <rect x=`"5`" y=`"1`" width=`"6`" height=`"1.5`" fill=`"$($P.blueFill)`"/>
    <path d=`"M8 5 V7.5 M3.5 11 V9 H12.5 V11 M8 7.5 H3.5 M8 7.5 H12.5`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <rect x=`"0.7`" y=`"11`" width=`"6`" height=`"4`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <rect x=`"0.7`" y=`"11`" width=`"6`" height=`"1.5`" fill=`"$($P.blueFill)`"/>
    <rect x=`"9.3`" y=`"11`" width=`"6`" height=`"4`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <rect x=`"9.3`" y=`"11`" width=`"6`" height=`"1.5`" fill=`"$($P.blueFill)`"/>
"@
$sequencediagramGlyph = @"
    <rect x=`"0.7`" y=`"0.7`" width=`"6`" height=`"3.6`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <rect x=`"9.3`" y=`"0.7`" width=`"6`" height=`"3.6`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <path d=`"M3.7 4.3 V15 M12.3 4.3 V15`" fill=`"none`" stroke=`"$($P.inkSoft)`" stroke-width=`"1`" stroke-dasharray=`"1.6,1.2`"/>
    <rect x=`"2.9`" y=`"6.2`" width=`"1.6`" height=`"6`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"0.9`"/>
    <path d=`"M4.5 8.2 H10.6`" stroke=`"$($P.ink)`" stroke-width=`"1.1`"/>
    <polygon points=`"12.1,8.2 9.6,7 9.6,9.4`" fill=`"$($P.ink)`"/>
"@
$argumentGlyph = @"
    <rect x=`"0.6`" y=`"4.6`" width=`"6.8`" height=`"6.8`" rx=`"1`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
    <path d=`"M2.4 6.8 H5.6 M4 6.8 V9.6`" stroke=`"#FFFFFF`" stroke-width=`"1.2`" stroke-linecap=`"round`" fill=`"none`"/>
    <rect x=`"8.6`" y=`"4.6`" width=`"6.8`" height=`"6.8`" rx=`"1`" fill=`"$($P.yellFill)`" stroke=`"$($P.yellEdge)`" stroke-width=`"1`"/>
    <path d=`"M10.6 9.6 V6.8 L13.4 9.6 V6.8`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-linecap=`"round`" stroke-linejoin=`"round`" fill=`"none`"/>
"@
$typeGlyph = "    <rect x=`"4.5`" y=`"4.5`" width=`"7`" height=`"7`" rx=`"1`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>"

# ---- Add buttons ------------------------------------------------------------
Save-Tb 'add_member'      (AddGlyph (Diamond $P.memberFill $P.memberEdge))
Save-Tb 'add_function'    (AddGlyph (Diamond $P.methodFill $P.methodEdge))
Save-Tb 'add_constructor' (AddGlyph (Diamond $P.ctorFill   $P.ctorEdge))
Save-Tb 'add_class'           (AddGlyph $classGlyph)
Save-Tb 'add_inherit'         (AddGlyph $inheritGlyph)
Save-Tb 'add_group'           (AddGlyph $folderGlyph)
Save-Tb 'add_classdiagram'    (AddGlyph $classdiagramGlyph)
Save-Tb 'add_sequencediagram' (AddGlyph $sequencediagramGlyph)
Save-Tb 'add_argument'        (AddGlyph $argumentGlyph)
Save-Tb 'add_type'            (AddGlyph $typeGlyph)

# add_virtuals / add_isclass: FOUR method diamonds in a 2x2 -- four reads as
# "multiple methods" where two could pass for a pair (JV 2026-07-12).
# Virtuals take the LIGHT method tint -- the same colour the tree paints
# declared-only (empty) methods, so tree and toolbar agree; IsClass methods
# take the full method magenta.
$methodLight = '#F29FEA'
# The 2x2 itself is rotated 45 degrees like the diamonds (JV): the four nest
# top/right/bottom/left of centre, each set slightly apart so they read as
# four separate diamonds (centre offset > radius = the gap).
function FourDiamonds([string]$fill)
{
    $b = @()
    $r = 3.2; $d = 4.7
    foreach ($c in @(@(8, (8 - $d)), @((8 + $d), 8), @(8, (8 + $d)), @((8 - $d), 8)))
    {
        $x = $c[0]; $y = $c[1]
        $b += "    <path d=`"M$x $($y-$r) L$($x+$r) $y L$x $($y+$r) L$($x-$r) $y Z`" fill=`"$fill`" stroke=`"$($P.methodEdge)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>"
    }
    return ($b -join "`n")
}
# Larger wrapper than the stock AddGlyph: the rotated composite leaves its
# upper-left corner empty, so it can sit closer to the add star and take
# more of the tile (JV 2026-07-12).
Save-Tb 'add_virtuals' ($star + "  <g transform=`"translate(3.0 3.0) scale(0.79)`">`n" + (FourDiamonds $methodLight) + "`n  </g>")
Save-Tb 'add_isclass'  ($star + "  <g transform=`"translate(3.0 3.0) scale(0.79)`">`n" + (FourDiamonds $P.methodFill) + "`n  </g>")

# add_relation: like the inheritance glyph, but with the aggregation diamond
# directly under the class box (the marker starts at the top -- JV 2026-07-12)
$relationGlyph = @"
    <rect x=`"3.5`" y=`"1.4`" width=`"9`" height=`"4`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
    <path d=`"M8 6.2 L10.7 9 L8 11.8 L5.3 9 Z`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
    <rect x=`"7.35`" y=`"11.8`" width=`"1.3`" height=`"3.2`" fill=`"$($P.ink)`"/>
"@
Save-Tb 'add_relation' (AddGlyph $relationGlyph)

# add_relation_diagramonly: no class box -- the diamond itself at the top,
# relation line hanging down
$relDiagGlyph = @"
    <path d=`"M8 1.2 L10.7 4 L8 6.8 L5.3 4 Z`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
    <rect x=`"7.35`" y=`"6.8`" width=`"1.3`" height=`"8`" fill=`"$($P.ink)`"/>
"@
Save-Tb 'add_relation_diagramonly' (AddGlyph $relDiagGlyph)

# add_dependency: dashed line ENDING in the open arrowhead -- a dependency's
# marker sits at the endpoint (unlike relation/inherit, whose diamond/triangle
# sits at the top under the owning class -- JV 2026-07-12).
$dependencyGlyph = @"
    <path d=`"M2 2 V10.5 H10.5`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.3`" stroke-dasharray=`"2.6,1.8`"/>
    <path d=`"M10.4 8.4 L14.4 10.5 L10.4 12.6`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-linejoin=`"round`" stroke-linecap=`"round`"/>
"@
Save-Tb 'add_dependency' (AddGlyph $dependencyGlyph)

# add_note: a note page with folded corner
$noteGlyph = @"
    <path d=`"M1.5 2.5 H14.5 V10.5 L11.5 13.5 H1.5 Z`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
    <path d=`"M11.5 13.5 V10.5 H14.5`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
    <path d=`"M3.6 5.5 H12.4 M3.6 8 H12.4 M3.6 10.5 H9`" stroke=`"$($P.inkSoft)`" stroke-width=`"1`"/>
"@
Save-Tb 'add_note' (AddGlyph $noteGlyph)

# add_lifeline: head box + dashed lifeline
$lifelineGlyph = @"
    <rect x=`"3.5`" y=`"1`" width=`"9`" height=`"4.6`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1.1`"/>
    <path d=`"M8 5.6 V15`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-dasharray=`"2.2,1.6`"/>
"@
Save-Tb 'add_lifeline' (AddGlyph $lifelineGlyph)

# add_message: activation bar + message arrow (activation WHITE like a real
# one -- yellow suggested a state that doesn't exist, JV 2026-07-12)
$messageGlyph = @"
    <rect x=`"1.6`" y=`"2`" width=`"2.4`" height=`"12`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1`"/>
    <path d=`"M4.6 8 H11.6`" stroke=`"$($P.ink)`" stroke-width=`"1.3`"/>
    <polygon points=`"14.6,8 11,6.2 11,9.8`" fill=`"$($P.ink)`"/>
"@
Save-Tb 'add_message' (AddGlyph $messageGlyph)

# ---- file / edit / view glyphs ----------------------------------------------
$docBody = @"
  <path d=`"M3.5 1.2 H10 L12.8 4 V14.8 H3.5 Z`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-linejoin=`"round`"/>
  <path d=`"M10 1.2 V4 H12.8`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-linejoin=`"round`"/>
"@
Save-Tb 'file_new' ($star + $docBody.Replace('M3.5 1.2 H10', 'M4.5 2.2 H10.4').Replace('L12.8 4 V14.8 H3.5 Z', 'L13.4 5 V14.8 H4.5 Z').Replace('M10 1.2 V4 H12.8', 'M10.4 2.2 V5 H13.4'))

Save-Tb 'file_open' @"
  <path d=`"M1.5 3.2 a1 1 0 0 1 1 -1 H5.4 l1.3 1.5 H12 a1 1 0 0 1 1 1 V6.4 H1.5 Z`"
        fill=`"$($P.foldFill)`" stroke=`"$($P.foldEdge)`" stroke-width=`"1`" stroke-linejoin=`"round`"/>
  <path d=`"M2.6 6.4 H15 L12.8 13.2 H1.5 Z`"
        fill=`"#FFF0C4`" stroke=`"$($P.foldEdge)`" stroke-width=`"1`" stroke-linejoin=`"round`"/>
"@

Save-Tb 'file_save' @"
  <path d=`"M2 3 a1 1 0 0 1 1 -1 H12 L14 4 V13 a1 1 0 0 1 -1 1 H3 a1 1 0 0 1 -1 -1 Z`"
        fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
  <rect x=`"4.4`" y=`"2.4`" width=`"6.4`" height=`"3.8`" fill=`"#FFFFFF`" stroke=`"$($P.blueEdge)`" stroke-width=`"0.9`"/>
  <rect x=`"8.4`" y=`"3`" width=`"1.6`" height=`"2.6`" fill=`"$($P.blueEdge)`"/>
  <rect x=`"4`" y=`"8.6`" width=`"8`" height=`"5.4`" rx=`"0.6`" fill=`"#FFFFFF`" stroke=`"$($P.blueEdge)`" stroke-width=`"0.9`"/>
"@

Save-Tb 'edit_cut' @"
  <circle cx=`"3.4`" cy=`"12.6`" r=`"2`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.3`"/>
  <circle cx=`"9.8`" cy=`"13.2`" r=`"2`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.3`"/>
  <path d=`"M4.8 11.2 L12.6 1.6 M8.6 11.5 L3.4 1.6`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.3`" stroke-linecap=`"round`"/>
"@

Save-Tb 'edit_copy' @"
  <rect x=`"1.6`" y=`"1.6`" width=`"8.4`" height=`"10.4`" rx=`"0.8`" fill=`"$($P.paper)`" stroke=`"$($P.inkSoft)`" stroke-width=`"1.1`"/>
  <rect x=`"5.6`" y=`"4`" width=`"8.4`" height=`"10.4`" rx=`"0.8`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.2`"/>
  <path d=`"M7.4 7 H12.2 M7.4 9.2 H12.2 M7.4 11.4 H10.6`" stroke=`"$($P.inkSoft)`" stroke-width=`"1`"/>
"@

Save-Tb 'edit_paste' @"
  <rect x=`"2`" y=`"2.4`" width=`"9`" height=`"12`" rx=`"1`" fill=`"$($P.foldFill)`" stroke=`"$($P.foldEdge)`" stroke-width=`"1.1`"/>
  <rect x=`"4.4`" y=`"1.2`" width=`"4.2`" height=`"2.6`" rx=`"0.8`" fill=`"#C9CDD2`" stroke=`"$($P.inkSoft)`" stroke-width=`"1`"/>
  <rect x=`"6.6`" y=`"6`" width=`"8`" height=`"9`" rx=`"0.8`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.2`"/>
  <path d=`"M8.4 8.6 H12.8 M8.4 10.8 H12.8 M8.4 13 H11.2`" stroke=`"$($P.inkSoft)`" stroke-width=`"1`"/>
"@

Save-Tb 'edit_delete' @"
  <path d=`"M3 3 L13 13 M13 3 L3 13`" stroke=`"$($P.dtorFill)`" stroke-width=`"2.6`" stroke-linecap=`"round`"/>
"@

Save-Tb 'file_print' @"
  <rect x=`"4`" y=`"1.4`" width=`"8`" height=`"3.6`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.1`"/>
  <rect x=`"1.8`" y=`"5`" width=`"12.4`" height=`"6`" rx=`"1`" fill=`"#C9CDD2`" stroke=`"$($P.ink)`" stroke-width=`"1.1`"/>
  <circle cx=`"12.4`" cy=`"7`" r=`"0.9`" fill=`"$($P.ctorFill)`"/>
  <rect x=`"4`" y=`"9.6`" width=`"8`" height=`"5`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.1`"/>
  <path d=`"M5.6 11.6 H10.4 M5.6 13.2 H9`" stroke=`"$($P.inkSoft)`" stroke-width=`"1`"/>
"@

Save-Tb 'app_about' @"
  <path d=`"M4.2 5.2 a3.8 3.4 0 1 1 5.4 3.1 c-1 .5 -1.6 1 -1.6 2.2 V11`"
        fill=`"none`" stroke=`"$($P.starFill)`" stroke-width=`"2.4`" stroke-linecap=`"round`"/>
  <circle cx=`"8`" cy=`"14`" r=`"1.4`" fill=`"$($P.starFill)`"/>
  <path d=`"M4.2 5.2 a3.8 3.4 0 1 1 5.4 3.1 c-1 .5 -1.6 1 -1.6 2.2 V11`"
        fill=`"none`" stroke=`"$($P.starEdge)`" stroke-width=`"0.6`" stroke-linecap=`"round`" opacity=`"0.35`"/>
"@

function Magnifier([string]$inner)
{
    return @"
  <circle cx=`"6.8`" cy=`"6.8`" r=`"4.8`" fill=`"#FFFFFF`" stroke=`"$($P.ink)`" stroke-width=`"1.5`"/>
  <path d=`"M10.4 10.4 L14.6 14.6`" stroke=`"$($P.ink)`" stroke-width=`"2.2`" stroke-linecap=`"round`"/>
$inner
"@
}
Save-Tb 'zoom_in'  (Magnifier "  <path d=`"M4.6 6.8 H9 M6.8 4.6 V9`" stroke=`"$($P.ink)`" stroke-width=`"1.4`" stroke-linecap=`"round`"/>")
Save-Tb 'zoom_out' (Magnifier "  <path d=`"M4.6 6.8 H9`" stroke=`"$($P.ink)`" stroke-width=`"1.4`" stroke-linecap=`"round`"/>")
Save-Tb 'zoom_full' (Magnifier @"
  <rect x=`"4.7`" y=`"4.9`" width=`"4.2`" height=`"3.8`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.2`"/>
"@)

# read_source / save_source: a C++ document with an arrow out of / into it.
function SourceDoc([string]$arrow)
{
    return @"
  <path d=`"M2.5 1.2 H9 L11.8 4 V14.8 H2.5 Z`" fill=`"$($P.paper)`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-linejoin=`"round`"/>
  <path d=`"M9 1.2 V4 H11.8`" fill=`"none`" stroke=`"$($P.ink)`" stroke-width=`"1.2`" stroke-linejoin=`"round`"/>
  <text x=`"4`" y=`"9.4`" font-family=`"Arial`" font-size=`"5.2`" font-weight=`"bold`" fill=`"$($P.blueFill)`">C++</text>
$arrow
"@
}
# Direction arrows sized to READ at toolbar size (a small arrow shrank to a
# green dot -- JV 2026-07-12): a chunky filled arrow over the document's
# lower half, with a thin white halo to lift it off the doc edge. The
# document IS the C++ source, so READ points OUT of it (source -> model) and
# WRITE points INTO it (model -> source) -- JV. Arrow in the same blue as
# the C++ label keeps the glyph calm and two-tone.
Save-Tb 'read_source' (SourceDoc @"
  <path d=`"M15.6 11.4 L11.6 7.8 V9.9 H7.4 V12.9 H11.6 V15 Z`"
        fill=`"$($P.blueFill)`" stroke=`"#FFFFFF`" stroke-width=`"0.9`" stroke-linejoin=`"round`"/>
"@)
Save-Tb 'save_source' (SourceDoc @"
  <path d=`"M7.2 11.4 L11.2 7.8 V9.9 H15.4 V12.9 H11.2 V15 Z`"
        fill=`"$($P.blueFill)`" stroke=`"#FFFFFF`" stroke-width=`"0.9`" stroke-linejoin=`"round`"/>
"@)

Write-Output "toolbar SVGs generated into $outDir"
