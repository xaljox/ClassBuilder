# convert_color_regions.ps1 -- COLORREF->CbColorRef and RGB()/GetRValue->Cb_*
# inside the round-trippable regions of the generated sources (@CODE bodies,
# @INIT ctor lists, @START_USER blocks, @NOTE comments). Companion to the
# CbColorRef model rename: the rename retypes member/arg/return COLORREF, but
# the RGB()/GetRValue MACROS inside bodies + member-init values need this pass.
# Argument DEFAULTS (= RGB(0,0,0) in generated .h signatures) are model data,
# NOT round-trippable -- handled separately via the pipe set_argument_default.
#
#   powershell tools/convert_color_regions.ps1           # dry run
#   powershell tools/convert_color_regions.ps1 -Apply    # then: read_source + write_source

param([switch]$Apply)
$ErrorActionPreference = 'Stop'
$srcDir = Join-Path $PSScriptRoot '..\ClassBuilder'

function Convert-Line([string]$line) {
    # \bRGB\( (not Cb_RGB( -- the underscore blocks the word boundary), and the
    # GetXValue macros. Skip double-quoted string literals (emitter output).
    $segments = [regex]::Split($line, '("(?:[^"\\]|\\.)*")')
    for ($i = 0; $i -lt $segments.Count; $i++) {
        if ($i % 2 -eq 0) {
            $segments[$i] = $segments[$i] `
                -creplace '\bCOLORREF\b', 'CbColorRef' `
                -creplace '\bRGB\(', 'Cb_RGB(' `
                -creplace '\bGetRValue\(', 'Cb_GetR(' `
                -creplace '\bGetGValue\(', 'Cb_GetG(' `
                -creplace '\bGetBValue\(', 'Cb_GetB('
        }
    }
    return ($segments -join '')
}

$filesChanged = 0; $linesChanged = 0
foreach ($file in Get-ChildItem (Join-Path $srcDir '*.cpp'), (Join-Path $srcDir '*.h')) {
    $text = [IO.File]::ReadAllText($file.FullName)
    if ($text -cnotmatch '\{//@CODE_|//@START_USER|//@INIT_|/\*@NOTE_') { continue }

    $lines = $text -split "(\r?\n)"
    $inBody = $false; $inNote = $false; $fileHits = 0
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        $marker = [regex]::Replace($line, '"(?:[^"\\]|\\.)*"', '""')
        if ($marker -cmatch '\{//@CODE_\d|//@START_USER|//@INIT_\d') { $inBody = $true;  continue }
        if ($marker -cmatch '\}//@CODE_\d|//@END_USER')   { $inBody = $false; continue }
        if ($marker -cmatch '^/\*@NOTE_\d') { $inNote = $true;  continue }
        if ($inNote -and $marker -cmatch '\*/') { $inNote = $false; continue }
        if (-not ($inBody -or $inNote)) { continue }

        $new = Convert-Line $line
        if ($new -cne $line) {
            $fileHits++; $linesChanged++
            if (-not $Apply) { Write-Host ("{0,-26} {1}" -f $file.Name, $line.Trim()) }
            $lines[$i] = $new
        }
    }
    if ($fileHits -gt 0) {
        $filesChanged++
        if ($Apply) {
            [IO.File]::WriteAllText($file.FullName, ($lines -join ''))
            Write-Host ("{0,-26} {1} lines" -f $file.Name, $fileHits)
        }
    }
}
$verb = if ($Apply) { 'Converted' } else { 'WOULD convert' }
Write-Host "`n$verb $linesChanged lines in $filesChanged files."
if (-not $Apply) { Write-Host 'Dry run -- re-run with -Apply, then read_source + write_source.' }
