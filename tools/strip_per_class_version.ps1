# strip_per_class_version.ps1
#
# Removes the per-class version literal write/read from every
# `<ClassName>::Serialize(CbArchive& archive)` body EXCEPT DataModelDoc's,
# which keeps the single global version (read into _objectVersion).
#
# Removes these lines (whitespace-tolerant) from each affected body:
#     int version = 1;
#     archive << version;
#     archive >> version;
#     ASSERT(version == 1);
#
# The format is CBZ v1; existing .cbz files generated before this strip are
# no longer compatible — re-save from a .cbd source.

param(
    [string]$SourceDir = 'c:\Users\jimmy\Projects\Src\ClassBuilder\ClassBuilder',
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

$total = 0
$touched = @()

Get-ChildItem -Path $SourceDir -Filter *.cpp | ForEach-Object {
    $path = $_.FullName
    $lines = [System.IO.File]::ReadAllText($path) -split "`r`n", 0, "RegexMatch"
    # Track which class we're inside; toggle when we see a Serialize signature.
    $insideCb = $false
    $currentClass = $null
    $depth = 0
    $changed = $false

    $out = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -lt $lines.Length; $i++) {
        $L = $lines[$i]

        # Detect entry into a CbArchive Serialize body.
        if (-not $insideCb) {
            $m = [regex]::Match($L, '(\w+)::Serialize\(CbArchive&')
            if ($m.Success -and $m.Groups[1].Value -ne 'DataModelDoc') {
                $insideCb = $true
                $currentClass = $m.Groups[1].Value
                $depth = 0
            }
        }

        $skip = $false
        if ($insideCb) {
            $strip = $L -match '^\s*int\s+version\s*=\s*1\s*;\s*$' -or
                     $L -match '^\s*archive\s*<<\s*version\s*;\s*$' -or
                     $L -match '^\s*archive\s*>>\s*version\s*;\s*$' -or
                     $L -match '^\s*ASSERT\s*\(\s*version\s*==\s*1\s*\)\s*;\s*$'
            if ($strip) {
                $skip = $true
                $changed = $true
            }
            # Track brace depth to know when the body ends.
            foreach ($c in $L.ToCharArray()) {
                if ($c -eq '{') { $depth++ }
                elseif ($c -eq '}') {
                    $depth--
                    if ($depth -le 0 -and $insideCb) {
                        $insideCb = $false
                        $currentClass = $null
                    }
                }
            }
        }

        if (-not $skip) { $out.Add($L) | Out-Null }
        else            { $total++ }
    }

    if ($changed) {
        $touched += $_.Name
        if (-not $DryRun) {
            [System.IO.File]::WriteAllText($path, ($out -join "`r`n"))
        }
    }
}

Write-Host "Files touched: $($touched.Count)"
$touched | ForEach-Object { Write-Host "  $_" }
Write-Host "Lines removed: $total $(if ($DryRun) { '(dry run)' })"
