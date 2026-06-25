# convert_body_BOOL_to_bool.ps1 -- rewrite BOOL/TRUE/FALSE to bool/true/false
# INSIDE the round-trippable regions of the generated sources, editing the
# files directly on disk: {//@CODE_NNNN method bodies, //@INIT_NNNN ctor
# init lists, and //@START_USER.. //@END_USER blocks (.h and .cpp).
# Companion to convert_BOOL_to_bool.ps1 (which did the model-level signatures
# via the pipe); these regions are round-trip content, so the workflow is:
# -Apply, then in CB accept/run Read Source and Save.
#
# SAFE BY DESIGN:
#   * Dry-run by default: lists every line it WOULD change. -Apply to write.
#   * Only text inside @CODE / USER markers is touched; generated code outside
#     them is left alone (regenerated from the model anyway).
#   * String literals are SKIPPED: several CB bodies emit source text; a
#     "BOOL"/"TRUE"/"FALSE" inside a string is OUTPUT, not code, and must stay.
#   * Whole-word, case-sensitive matches only. CRLF and encoding preserved
#     (in-place text edit, line ends untouched).
#   * Flags (does not change) lines where a converted variable is compared to
#     -1 (the GetMessage-style BOOL tri-state idiom) -- review those by hand.
#
#   powershell tools/convert_body_BOOL_to_bool.ps1            # dry run
#   powershell tools/convert_body_BOOL_to_bool.ps1 -Apply     # do it

param([switch]$Apply)

$ErrorActionPreference = 'Stop'
$srcDir = Join-Path $PSScriptRoot '..\ClassBuilder'

# Replace whole-word BOOL/TRUE/FALSE outside double-quoted string literals.
function Convert-Line([string]$line) {
    # Split into string-literal and non-literal segments. Handles \" escapes.
    $segments = [regex]::Split($line, '("(?:[^"\\]|\\.)*")')
    for ($i = 0; $i -lt $segments.Count; $i++) {
        if ($i % 2 -eq 0) {   # even index = outside string literals
            $segments[$i] = $segments[$i] -creplace '\bBOOL\b', 'bool' `
                                          -creplace '\bTRUE\b', 'true' `
                                          -creplace '\bFALSE\b', 'false'
        }
    }
    return ($segments -join '')
}

$filesChanged = 0; $linesChanged = 0; $flagged = @()

foreach ($file in Get-ChildItem (Join-Path $srcDir '*.cpp'), (Join-Path $srcDir '*.h')) {
    $text = [IO.File]::ReadAllText($file.FullName)
    if ($text -cnotmatch '\{//@CODE_|//@START_USER|//@INIT_') { continue }

    $lines = $text -split "(\r\n)"          # keep separators as tokens
    $inBody = $false
    $fileHits = 0
    $inNote = $false
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        # Marker tests run on the line with string literals BLANKED: codegen
        # emitter bodies contain marker text INSIDE strings (str += "//@START_
        # USER1") which must not flip the region state.
        $marker = [regex]::Replace($line, '"(?:[^"\\]|\\.)*"', '""')
        # //@INIT_ marks the ctor signature line; the init list follows until
        # the {//@CODE_ line (which itself also opens an editable body).
        if ($marker -cmatch '\{//@CODE_\d|//@START_USER|//@INIT_\d') { $inBody = $true;  continue }
        if ($marker -cmatch '\}//@CODE_\d|//@END_USER')   { $inBody = $false; continue }
        # /*@NOTE_NNNN ... */ note comments round-trip into the model too.
        if ($marker -cmatch '^/\*@NOTE_\d') { $inNote = $true;  continue }
        if ($inNote -and $marker -cmatch '\*/') { $inNote = $false; continue }
        if (-not ($inBody -or $inNote)) { continue }

        $new = Convert-Line $line
        if ($new -cne $line) {
            $fileHits++; $linesChanged++
            if ($new -cmatch '==\s*-1|!=\s*-1') {
                $flagged += "$($file.Name): $($new.Trim())"
            }
            if (-not $Apply) {
                Write-Host ("{0,-28} {1}" -f $file.Name, $line.Trim())
            }
            $lines[$i] = $new
        }
    }

    if ($fileHits -gt 0) {
        $filesChanged++
        if ($Apply) {
            [IO.File]::WriteAllText($file.FullName, ($lines -join ''))
            Write-Host ("{0,-28} {1} lines converted" -f $file.Name, $fileHits)
        }
    }
}

$verb = if ($Apply) { 'Converted' } else { 'WOULD convert' }
Write-Host "`n$verb $linesChanged lines in $filesChanged files."
if ($flagged.Count) {
    Write-Host "`nREVIEW BY HAND -- bool compared to -1 (tri-state idiom):"
    $flagged | ForEach-Object { Write-Host "  $_" }
}
if (-not $Apply) { Write-Host 'Dry run -- re-run with -Apply to perform the change.' }
else { Write-Host 'Now: in CB run Read Source (accept the prompt) and Save the model.' }
