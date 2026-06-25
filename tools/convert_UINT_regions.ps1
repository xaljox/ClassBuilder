# convert_UINT_regions.ps1 -- UINT -> unsigned int inside the round-trippable
# regions (@CODE / @INIT / @START_USER / @NOTE) of the generated sources.
# Model member/arg/return UINT TYPES are repointed separately via the pipe;
# this handles UINT used inside method bodies / user code. Win32-boundary UINT
# (CbCommandServer message handling, the parser) is intentionally left.
#
#   powershell tools/convert_UINT_regions.ps1           # dry run
#   powershell tools/convert_UINT_regions.ps1 -Apply

param([switch]$Apply)
$ErrorActionPreference = 'Stop'
$srcDir = Join-Path $PSScriptRoot '..\ClassBuilder'

function Convert-Line([string]$line) {
    $segments = [regex]::Split($line, '("(?:[^"\\]|\\.)*")')
    for ($i = 0; $i -lt $segments.Count; $i++) {
        if ($i % 2 -eq 0) {
            $segments[$i] = $segments[$i] -creplace '\bUINT\b', 'unsigned int'
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
        if ($Apply) { [IO.File]::WriteAllText($file.FullName, ($lines -join '')); Write-Host ("{0,-26} {1} lines" -f $file.Name, $fileHits) }
    }
}
$verb = if ($Apply) { 'Converted' } else { 'WOULD convert' }
Write-Host "`n$verb $linesChanged lines in $filesChanged files."
