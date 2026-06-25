# convert_qt_bool.ps1 -- rewrite TRUE/FALSE/BOOL to true/false/bool in the
# hand-written qt/ sources (model calls take bool since the 2026-06-10 model
# conversion). Skips: string literals, genuine Win32 lines (::EnableWindow,
# IsWindowEnabled, MessageBox, mouse_event...), and the "windows.h before the
# model headers (BOOL / UINT / COLORREF)" boilerplate comments.
#
#   powershell tools/convert_qt_bool.ps1           # dry run
#   powershell tools/convert_qt_bool.ps1 -Apply

param([switch]$Apply)
$ErrorActionPreference = 'Stop'
$qtDir = Join-Path $PSScriptRoot '..\qt'

function Convert-Line([string]$line) {
    $segments = [regex]::Split($line, '("(?:[^"\\]|\\.)*")')
    for ($i = 0; $i -lt $segments.Count; $i++) {
        if ($i % 2 -eq 0) {
            $segments[$i] = $segments[$i] -creplace '\bBOOL\b', 'bool' `
                                          -creplace '\bTRUE\b', 'true' `
                                          -creplace '\bFALSE\b', 'false'
        }
    }
    return ($segments -join '')
}

$filesChanged = 0; $linesChanged = 0
foreach ($file in Get-ChildItem (Join-Path $qtDir '*.cpp'), (Join-Path $qtDir '*.h')) {
    $text = [IO.File]::ReadAllText($file.FullName)
    if ($text -cnotmatch '\b(TRUE|FALSE|BOOL)\b') { continue }

    # qt/ files are a CRLF/LF mix -- split on either, keep the separators.
    $lines = $text -split "(\r?\n)"
    $fileHits = 0
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        if ($line -cnotmatch '\b(TRUE|FALSE|BOOL)\b') { continue }
        # Win32 boundary + the windows.h boilerplate comment stay as-is.
        if ($line -match '::EnableWindow|IsWindowEnabled|::MessageBox|mouse_event|COLORREF') { continue }

        $new = Convert-Line $line
        if ($new -cne $line) {
            $fileHits++; $linesChanged++
            if (-not $Apply) { Write-Host ("{0,-30} {1}" -f $file.Name, $line.Trim()) }
            $lines[$i] = $new
        }
    }
    if ($fileHits -gt 0) {
        $filesChanged++
        if ($Apply) {
            [IO.File]::WriteAllText($file.FullName, ($lines -join ''))
            Write-Host ("{0,-30} {1} lines converted" -f $file.Name, $fileHits)
        }
    }
}
$verb = if ($Apply) { 'Converted' } else { 'WOULD convert' }
Write-Host "`n$verb $linesChanged lines in $filesChanged files."
