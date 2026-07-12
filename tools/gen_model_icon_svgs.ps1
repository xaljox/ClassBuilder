# gen_model_icon_svgs.ps1 -- generate the SVG model-icon set into src/qt/icons/.
#
# Redraws every legacy res/<name>.ico tree glyph as a crisp 16x16-viewBox SVG
# (QtModelIcons prefers <name>.svg over <name>.ico per glyph). The visual
# LANGUAGE of the .ico set is preserved -- users know it:
#   diamond colour = kind      (cyan member, magenta method, green constructor,
#                               red destructor)
#   checker fill   = variant   (colour+grey = inline, colour+white = empty)
#   yellow key     = protected,  grey padlock = private,  bare = public
#   relation arrow = colour    (grey plain, black owned, magenta cr,
#                               red cr+owned), thick+wide box = static,
#                               1/2 heads = single/multi,
#                               yellow box below = act, blue box above = pas
# ...only the rendering is modernised: flat fills, consistent outlines, shapes
# on the pixel grid so they stay sharp at 100%/150%/225% DPI.
#
# Deterministic: run it again and identical files come out. Outputs are
# committed; re-run only when changing the design. After a re-run, touch
# src/qt/resources.qrc (AUTORCC misses listed-file changes).

$ErrorActionPreference = 'Stop'
$outDir = Join-Path $PSScriptRoot '..\src\qt\icons'
$outDir = [IO.Path]::GetFullPath($outDir)
$utf8 = New-Object System.Text.UTF8Encoding($false)

# ---- palette ---------------------------------------------------------------
$P = @{
    memberFill = '#29C4D9'; memberEdge = '#0E7A8A'   # cyan
    methodFill = '#E93BDD'; methodEdge = '#8E1287'   # magenta
    ctorFill   = '#47B34F'; ctorEdge   = '#1F6E26'   # green
    dtorFill   = '#E8433C'; dtorEdge   = '#8E1B18'   # red
    blueFill   = '#3D6BE5'; blueEdge   = '#16309B'   # class/type/pas box
    yellFill   = '#FFE14D'; yellEdge   = '#937B00'   # act box / argument [N]
    keyFill    = '#F7C948'; keyEdge    = '#8A6D00'
    lockFill   = '#B9BCC0'; lockEdge   = '#55585C'
    arrowGrey  = '#8F9499'; arrowBlack = '#26282B'
    foldFill   = '#FFE3A1'; foldEdge   = '#B8912D'   # folder body
    inkEdge    = '#26282B'                            # generic dark outline
    checkerGrey = '#B9BCC0'
}

function Save-Svg([string]$name, [string]$body)
{
    $svg = "<svg xmlns=`"http://www.w3.org/2000/svg`" viewBox=`"0 0 16 16`">`n" +
           $body + "`n</svg>`n"
    [IO.File]::WriteAllText((Join-Path $outDir "$name.svg"), $svg, $script:utf8)
}

# ---- diamond family --------------------------------------------------------
# Diamond upper-right (same spot public/protected/private, so rows align);
# the visibility modifier sits lower-left.
# variant fills: plain = full colour; inline = mid tint; empty = light tint.
# Flat tints, no checker: the legacy white/colour dither read as speckle in
# the tree -- JV 2026-07-12: "egale lichte kleur ipv gespikkeld". The same
# light tint is reused by the toolbar (tb_add_virtuals) so tree and toolbar
# agree.
function DiamondBody([string]$fill, [string]$edge, [string]$variant, [string]$uid)
{
    $d = 'M10.5 1.9 L15.1 6.5 L10.5 11.1 L5.9 6.5 Z'
    return "  <path d=`"$d`" fill=`"$fill`" stroke=`"$edge`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>"
}

$keyGlyph = @"
  <circle cx=`"3.6`" cy=`"5.6`" r=`"2.3`" fill=`"$($P.keyFill)`" stroke=`"$($P.keyEdge)`" stroke-width=`"1`"/>
  <circle cx=`"3.6`" cy=`"5.6`" r=`"0.8`" fill=`"$($P.keyEdge)`"/>
  <rect x=`"2.8`" y=`"7.6`" width=`"1.6`" height=`"6.4`" rx=`"0.5`" fill=`"$($P.keyFill)`" stroke=`"$($P.keyEdge)`" stroke-width=`"0.9`"/>
  <rect x=`"4.2`" y=`"10.6`" width=`"1.7`" height=`"1.3`" fill=`"$($P.keyFill)`" stroke=`"$($P.keyEdge)`" stroke-width=`"0.9`"/>
  <rect x=`"4.2`" y=`"12.6`" width=`"1.7`" height=`"1.3`" fill=`"$($P.keyFill)`" stroke=`"$($P.keyEdge)`" stroke-width=`"0.9`"/>
"@

$lockGlyph = @"
  <path d=`"M2.6 9.2 V7.4 a2.6 2.6 0 0 1 5.2 0 V9.2`" fill=`"none`" stroke=`"$($P.lockEdge)`" stroke-width=`"1.5`"/>
  <rect x=`"1.1`" y=`"9.0`" width=`"8.2`" height=`"5.9`" rx=`"1`" fill=`"$($P.lockFill)`" stroke=`"$($P.lockEdge)`" stroke-width=`"1`"/>
  <circle cx=`"5.2`" cy=`"11.3`" r=`"1.05`" fill=`"$($P.lockEdge)`"/>
  <rect x=`"4.75`" y=`"11.8`" width=`"0.9`" height=`"1.7`" fill=`"$($P.lockEdge)`"/>
"@

$kinds = @(
    @{ key = 'member';      fill = $P.memberFill; mid = '#7FD9E7'; light = '#B8ECF4'; edge = $P.memberEdge },
    @{ key = 'method';      fill = $P.methodFill; mid = '#EE6FE2'; light = '#F29FEA'; edge = $P.methodEdge },
    @{ key = 'constructor'; fill = $P.ctorFill;   mid = '#8BCE90'; light = '#BEE5C1'; edge = $P.ctorEdge },
    @{ key = 'destructor';  fill = $P.dtorFill;   mid = '#F08A86'; light = '#F7BDBA'; edge = $P.dtorEdge }
)
$visList = @('public', 'protected', 'private')

foreach ($k in $kinds)
{
    foreach ($vis in $visList)
    {
        $mod = ''
        if ($vis -eq 'protected') { $mod = $keyGlyph }
        elseif ($vis -eq 'private') { $mod = $lockGlyph }

        # plain
        Save-Svg "${vis}_$($k.key)" ((DiamondBody $k.fill $k.edge 'plain' '') + "`n" + $mod)
        # inline (no inline_member exists; ctor/dtor/method only) -> mid tint
        if ($k.key -ne 'member') {
            Save-Svg "${vis}_inline_$($k.key)" ((DiamondBody $k.mid $k.edge 'inline' 'i') + "`n" + $mod)
        }
        # empty (methods only) -> light tint
        if ($k.key -eq 'method') {
            Save-Svg "${vis}_empty_method" ((DiamondBody $k.light $k.edge 'empty' 'e') + "`n" + $mod)
        }
    }
}

# ---- relation family -------------------------------------------------------
# act = arrow down INTO a yellow box at the bottom; pas = blue box on top,
# arrow leaving downward. single = 1 head, multi = 2; static = thick + wide.
function RelationSvg([string]$colour, [int]$heads, [bool]$static, [string]$side)
{
    $shaftW = if ($static) { 2.6 } else { 1.3 }
    $headW  = if ($static) { 4.6 } else { 3.4 }   # half-width of head triangle
    $cx = 8.0
    $b = @()

    if ($side -eq 'act') {
        $boxY = 10.6; $arrowTop = 1.2; $apexEnd = 9.9
    } else {
        # box on top, arrow beneath it
        $boxY = 1.4;  $arrowTop = 6.4; $apexEnd = 15.1
    }

    # box
    $boxW = if ($static) { 11.0 } else { 9.0 }
    $boxX = $cx - $boxW / 2.0
    $boxFill = if ($side -eq 'act') { $P.yellFill } else { $P.blueFill }
    $boxEdge = if ($side -eq 'act') { $P.yellEdge } else { $P.blueEdge }
    $b += "  <rect x=`"$boxX`" y=`"$boxY`" width=`"$boxW`" height=`"4`" fill=`"$boxFill`" stroke=`"$boxEdge`" stroke-width=`"1`"/>"

    # arrow: shaft + 1..2 heads, last apex just above the box (act) / at 15 (pas)
    $headH = if ($static) { 3.6 } else { 3.2 }
    $apex2 = $apexEnd
    $apex1 = if ($heads -eq 2) { $apexEnd - $headH } else { $apexEnd }
    $shaftEnd = $apex1 - $headH + 0.8
    $b += "  <rect x=`"$($cx - $shaftW/2)`" y=`"$arrowTop`" width=`"$shaftW`" height=`"$($shaftEnd - $arrowTop)`" fill=`"$colour`"/>"
    $b += "  <polygon points=`"$cx,$apex1 $($cx-$headW),$($apex1-$headH) $($cx+$headW),$($apex1-$headH)`" fill=`"$colour`"/>"
    if ($heads -eq 2) {
        $b += "  <polygon points=`"$cx,$apex2 $($cx-$headW),$($apex2-$headH) $($cx+$headW),$($apex2-$headH)`" fill=`"$colour`"/>"
    }
    return ($b -join "`n")
}

$relColours = @{
    ''          = $P.arrowGrey    # plain
    'owned_'    = $P.arrowBlack
    'cr_'       = $P.methodFill   # create-relation magenta
    'cr_owned_' = $P.dtorFill     # create+owned red
}
foreach ($pre in @('', 'owned_', 'cr_', 'cr_owned_'))
{
    $c = $relColours[$pre]
    Save-Svg "${pre}single_act" (RelationSvg $c 1 $false 'act')
    Save-Svg "${pre}single_pas" (RelationSvg $c 1 $false 'pas')
    Save-Svg "${pre}multi_act"  (RelationSvg $c 2 $false 'act')
    if ($pre -eq 'cr_owned_') {
        # historical name quirk: the pas icon file is cr_owned_multi.ico
        Save-Svg 'cr_owned_multi' (RelationSvg $c 2 $false 'pas')
    } else {
        Save-Svg "${pre}multi_pas" (RelationSvg $c 2 $false 'pas')
    }
}
foreach ($pre in @('', 'owned_', 'cr_', 'cr_owned_'))
{
    $c = $relColours[$pre]
    $name = if ($pre -eq 'owned_') { 'static_owned_multi' }
            elseif ($pre -eq 'cr_') { 'cr_static_multi' }
            elseif ($pre -eq 'cr_owned_') { 'cr_static_owned_multi' }
            else { 'static_multi' }
    Save-Svg "${name}_act" (RelationSvg $c 2 $true 'act')
    Save-Svg "${name}_pas" (RelationSvg $c 2 $true 'pas')
}

# ---- standalone glyphs -----------------------------------------------------

# type: the blue square
Save-Svg 'type' @"
  <rect x=`"4.5`" y=`"4.5`" width=`"7`" height=`"7`" rx=`"1`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
"@

# argument: [T][N] -- a type box and a name box
Save-Svg 'argument' @"
  <rect x=`"0.6`" y=`"4.6`" width=`"6.8`" height=`"6.8`" rx=`"1`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
  <path d=`"M2.4 6.8 H5.6 M4 6.8 V9.6`" stroke=`"#FFFFFF`" stroke-width=`"1.2`" stroke-linecap=`"round`" fill=`"none`"/>
  <rect x=`"8.6`" y=`"4.6`" width=`"6.8`" height=`"6.8`" rx=`"1`" fill=`"$($P.yellFill)`" stroke=`"$($P.yellEdge)`" stroke-width=`"1`"/>
  <path d=`"M10.6 9.6 V6.8 L13.4 9.6 V6.8`" stroke=`"$($P.inkEdge)`" stroke-width=`"1.2`" stroke-linecap=`"round`" stroke-linejoin=`"round`" fill=`"none`"/>
"@

# inherit: class box with a UML generalization arrow pointing up into it
Save-Svg 'inherit' @"
  <rect x=`"3.5`" y=`"1.4`" width=`"9`" height=`"4`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
  <polygon points=`"8,7 11.2,10.4 4.8,10.4`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"1.1`" stroke-linejoin=`"round`"/>
  <rect x=`"7.35`" y=`"10.4`" width=`"1.3`" height=`"4.2`" fill=`"$($P.inkEdge)`"/>
"@

# class: blue class SQUARE + bracket to three coloured features (yellow
# relations / magenta methods / cyan members). Square, a notch smaller than
# before, and with clear air between it and the feature boxes (JV 2026-07-12).
Save-Svg 'class' @"
  <rect x=`"0.8`" y=`"4.8`" width=`"6.4`" height=`"6.4`" rx=`"0.8`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`"/>
  <path d=`"M7.2 8 H9.8 M9.8 2.6 V13.4 M9.8 2.6 H11.4 M9.8 8 H11.4 M9.8 13.4 H11.4`" fill=`"none`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"11.6`" y=`"0.9`" width=`"3.6`" height=`"3.4`" fill=`"$($P.yellFill)`" stroke=`"$($P.yellEdge)`" stroke-width=`"0.9`"/>
  <rect x=`"11.6`" y=`"6.3`" width=`"3.6`" height=`"3.4`" fill=`"$($P.methodFill)`" stroke=`"$($P.methodEdge)`" stroke-width=`"0.9`"/>
  <rect x=`"11.6`" y=`"11.7`" width=`"3.6`" height=`"3.4`" fill=`"$($P.memberFill)`" stroke=`"$($P.memberEdge)`" stroke-width=`"0.9`"/>
"@

# externclass: dashed class square + bracket to two features (no relations)
Save-Svg 'externclass' @"
  <rect x=`"0.8`" y=`"4.8`" width=`"6.4`" height=`"6.4`" rx=`"0.8`" fill=`"$($P.blueFill)`" stroke=`"$($P.blueEdge)`" stroke-width=`"1`" stroke-dasharray=`"2.2,1.3`"/>
  <path d=`"M7.2 8 H9.8 M9.8 4.4 V11.6 M9.8 4.4 H11.4 M9.8 11.6 H11.4`" fill=`"none`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"11.6`" y=`"2.7`" width=`"3.6`" height=`"3.4`" fill=`"$($P.methodFill)`" stroke=`"$($P.methodEdge)`" stroke-width=`"0.9`"/>
  <rect x=`"11.6`" y=`"9.9`" width=`"3.6`" height=`"3.4`" fill=`"$($P.memberFill)`" stroke=`"$($P.memberEdge)`" stroke-width=`"0.9`"/>
"@

# folders: file (class files), membergroup, methodgroup, membermethodgroup.
# Accent sits at the TOP like the originals: the tab itself plus a small bar
# just below the fold line (tab-only was too subtle -- JV 2026-07-12). The
# accent is a rect clipped to the folder silhouette, so it follows the tab
# shape exactly; the outline is stroked on top.
function FolderSvg([string]$accent1, [string]$accent2)
{
    $body = 'M1.2 4.6 a1 1 0 0 1 1 -1 H5.6 l1.4 1.6 H13.8 a1 1 0 0 1 1 1 V12 a1 1 0 0 1 -1 1 H2.2 a1 1 0 0 1 -1 -1 Z'
    $b = @()
    $b += "  <path d=`"$body`" fill=`"$($P.foldFill)`"/>"
    if ($accent1) {
        # Accent along the TOP only, no vertical stacking (a bar below the tab
        # merged into one blob at tree size -- JV): the TAB itself coloured,
        # plus a thin strip continuing the tab line to the right under the top
        # edge. Drawn as explicit paths INSIDE the folder silhouette --
        # QSvgRenderer is SVG Tiny and silently IGNORES clipPath, so an
        # earlier clipped-rect version painted an unclipped full band (the
        # very blob this replaces). For the mixed group the tab (first part)
        # takes the METHOD colour and the strip (last part) the MEMBER colour
        # -- the legacy magenta-then-cyan tab, writ larger.
        # One continuous UNDERSIDE at y=6.9: the tab colour runs down to the
        # same bottom line as the strip, and the strip starts flush against
        # the tab's right edge -- tab and bar read as one ribbon with the tab
        # sticking up (JV 2026-07-12).
        $tail = if ($accent2) { $accent2 } else { $accent1 }
        $b += "  <path d=`"M1.2 6.9 V4.6 a1 1 0 0 1 1 -1 H5.6 l1.4 1.6 V6.9 Z`" fill=`"$accent1`"/>"
        $b += "  <rect x=`"7.0`" y=`"5.2`" width=`"7.3`" height=`"1.7`" fill=`"$tail`"/>"
    }
    $b += "  <path d=`"$body`" fill=`"none`" stroke=`"$($P.foldEdge)`" stroke-width=`"1`" stroke-linejoin=`"round`"/>"
    return ($b -join "`n")
}
Save-Svg 'file' (FolderSvg '' '')
Save-Svg 'membergroup' (FolderSvg $P.memberFill '')
Save-Svg 'methodgroup' (FolderSvg $P.methodFill '')
Save-Svg 'membermethodgroup' (FolderSvg $P.methodFill $P.memberFill)

# fileselected: the open-folder variant
Save-Svg 'fileselected' @"
  <path d=`"M1.2 4.6 a1 1 0 0 1 1 -1 H5.6 l1.4 1.6 H12.6 V6.8 H3.4 Z`"
        fill=`"$($P.foldFill)`" stroke=`"$($P.foldEdge)`" stroke-width=`"1`" stroke-linejoin=`"round`"/>
  <path d=`"M3.4 6.8 H14.8 L13 13 H1.4 Z`"
        fill=`"#FFF0C4`" stroke=`"$($P.foldEdge)`" stroke-width=`"1`" stroke-linejoin=`"round`"/>
"@

# classdiagram: parent box with two children
Save-Svg 'classdiagram' @"
  <rect x=`"5`" y=`"1`" width=`"6`" height=`"4`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"5`" y=`"1`" width=`"6`" height=`"1.5`" fill=`"$($P.blueFill)`"/>
  <path d=`"M8 5 V7.5 M3.5 11 V9 H12.5 V11 M8 7.5 H3.5 M8 7.5 H12.5`" fill=`"none`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"0.7`" y=`"11`" width=`"6`" height=`"4`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"0.7`" y=`"11`" width=`"6`" height=`"1.5`" fill=`"$($P.blueFill)`"/>
  <rect x=`"9.3`" y=`"11`" width=`"6`" height=`"4`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"9.3`" y=`"11`" width=`"6`" height=`"1.5`" fill=`"$($P.blueFill)`"/>
"@

# sequencediagram: two lifelines, an activation, a message arrow. The
# activation is WHITE like a real one (yellow suggested a state that doesn't
# exist -- activations are white, at most selection-coloured; JV 2026-07-12).
Save-Svg 'sequencediagram' @"
  <rect x=`"0.7`" y=`"0.7`" width=`"6`" height=`"3.6`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <rect x=`"9.3`" y=`"0.7`" width=`"6`" height=`"3.6`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <path d=`"M3.7 4.3 V15 M12.3 4.3 V15`" fill=`"none`" stroke=`"$($P.arrowGrey)`" stroke-width=`"1`" stroke-dasharray=`"1.6,1.2`"/>
  <rect x=`"2.9`" y=`"6.2`" width=`"1.6`" height=`"6`" fill=`"#FFFFFF`" stroke=`"$($P.inkEdge)`" stroke-width=`"0.9`"/>
  <path d=`"M4.5 8.2 H10.6`" stroke=`"$($P.inkEdge)`" stroke-width=`"1.1`"/>
  <polygon points=`"12.1,8.2 9.6,7 9.6,9.4`" fill=`"$($P.inkEdge)`"/>
"@

# actor: stick figure
Save-Svg 'actor' @"
  <circle cx=`"8`" cy=`"3.4`" r=`"2.1`" fill=`"none`" stroke=`"$($P.inkEdge)`" stroke-width=`"1.3`"/>
  <path d=`"M8 5.5 V10 M3.8 7.2 H12.2 M8 10 L4.6 14.4 M8 10 L11.4 14.4`"
        fill=`"none`" stroke=`"$($P.inkEdge)`" stroke-width=`"1.3`" stroke-linecap=`"round`"/>
"@

# phases: letter in a coloured disc (kept: A grey, D black, I red, T yellow,
# C green). Letters drawn as strokes -- no font dependency.
function PhaseSvg([string]$fill, [string]$letterPath, [string]$letterColour)
{
    return @"
  <circle cx=`"8`" cy=`"8`" r=`"6.6`" fill=`"$fill`" stroke=`"$($P.inkEdge)`" stroke-width=`"1`"/>
  <path d=`"$letterPath`" fill=`"none`" stroke=`"$letterColour`" stroke-width=`"1.5`" stroke-linecap=`"round`" stroke-linejoin=`"round`"/>
"@
}
Save-Svg 'analysis_phase'       (PhaseSvg '#9AA0A6' 'M5.6 11.2 L8 4.8 L10.4 11.2 M6.4 9.2 H9.6' '#FFFFFF')
Save-Svg 'design_phase'         (PhaseSvg '#3C4043' 'M6 4.8 V11.2 M6 4.8 H7.6 A3.2 3.2 0 0 1 7.6 11.2 H6' '#FFFFFF')
Save-Svg 'implementation_phase' (PhaseSvg '#E8433C' 'M6.4 4.8 H9.6 M8 4.8 V11.2 M6.4 11.2 H9.6' '#FFFFFF')
Save-Svg 'test_phase'           (PhaseSvg '#F5C400' 'M5.6 4.8 H10.4 M8 4.8 V11.2' '#3C4043')
Save-Svg 'complete_phase'       (PhaseSvg '#47B34F' 'M10.6 5.6 A3.9 3.9 0 1 0 10.6 10.4' '#FFFFFF')

Write-Output "SVG model icons generated into $outDir"
