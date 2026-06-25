# update-stable.ps1 -- promote the freshly built Release ClassBuilder.exe to
# the stable install at C:\Program Files (x86)\ClassBuilder.
#
# The build is a STATIC Qt build: the EXE is fully self-contained, so this
# also strips any leftover windeployqt artifacts (Qt6*.dll, the plugin dirs,
# dxcompiler/dxil/opengl32sw) from a previous dynamic-Qt install.
#
# Writing to Program Files needs admin -- the script self-elevates (UAC
# prompt). Just run it; no need to open an admin shell yourself.
#
# Run AFTER:  cmake --build --preset x64-release
# and AFTER closing any running ClassBuilder.exe (else the copy is blocked).

$ErrorActionPreference = "Stop"

# --- self-elevate ----------------------------------------------------------
$isAdmin = ([Security.Principal.WindowsPrincipal]`
    [Security.Principal.WindowsIdentity]::GetCurrent()`
    ).IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "Not elevated -- relaunching as administrator..."
    Start-Process powershell -Verb RunAs -ArgumentList `
        "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`""
    exit
}

# --- paths -----------------------------------------------------------------
$src = "C:\Users\jimmy\Projects\ClassBuilder\out\build\x64\bin\Release\ClassBuilder.exe"
$dst = "C:\Program Files (x86)\ClassBuilder"

if (-not (Test-Path $src)) {
    Write-Host "ERROR: $src not found. Build x64-release first." -ForegroundColor Red
    Read-Host "Press Enter to close"; exit 1
}

# --- back up current stable EXE as a numbered fallback ---------------------
# The previous ClassBuilder.exe becomes ClassBuilder_<n>.exe -- n increments
# each promotion, so older versions accumulate (highest n = most recent
# fallback). Prune old ClassBuilder_*.exe by hand when no longer wanted.
# NOTE: a backup is only a usable fallback if it is itself a static build;
# the very first promotion backs up the old dynamic-Qt EXE, which will not
# run once the Qt6*.dll are stripped below.
try {
    if (Test-Path "$dst\ClassBuilder.exe") {
        $nums = Get-ChildItem "$dst\ClassBuilder_*.exe" -ErrorAction SilentlyContinue |
            ForEach-Object { if ($_.BaseName -match '^ClassBuilder_(\d+)$') { [int]$Matches[1] } }
        $next = 1
        if ($nums) { $next = ($nums | Measure-Object -Maximum).Maximum + 1 }
        Move-Item "$dst\ClassBuilder.exe" "$dst\ClassBuilder_$next.exe" -Force
        Write-Host "Previous EXE kept as ClassBuilder_$next.exe (fallback)"
    }
} catch {
    Write-Host "ERROR backing up EXE: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "(Is ClassBuilder.exe still running? Close it and retry.)"
    Read-Host "Press Enter to close"; exit 1
}

# --- copy the EXE ----------------------------------------------------------
try {
    Copy-Item $src "$dst\ClassBuilder.exe" -Force
    $size = [math]::Round((Get-Item "$dst\ClassBuilder.exe").Length / 1MB, 1)
    Write-Host "ClassBuilder.exe updated ($size MB)" -ForegroundColor Green
} catch {
    Write-Host "ERROR copying EXE: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "(Is ClassBuilder.exe still running? Close it and retry.)"
    Read-Host "Press Enter to close"; exit 1
}

# --- sync bundled runtime headers + sources --------------------------------
# Generated code #includes the CB_* headers in Include\ and the serialization
# runtime in SRC\. This script historically copied ONLY the EXE, so those
# bundled copies drifted from the EXE's codegen (e.g. the int IsX -> bool
# iterator-macro flip + a _count zero-init, 2026-06). Mirror them from the repo
# on every promotion so the EXE and its headers can never disagree.
$repoRoot = "C:\Users\jimmy\Projects\ClassBuilder"
try {
    Copy-Item "$repoRoot\include\*" "$dst\Include" -Force
    $nInc = (Get-ChildItem "$dst\Include" -File).Count
    Write-Host "Include\ mirrored from repo ($nInc headers)." -ForegroundColor Green

    # SRC\ serialization runtime (leave bundled *.CBZ sample models alone).
    # Post-restructure the runtime sources live in tier dirs: serialization in
    # serialize\, value types (CbString) in value\ -- not the old ClassBuilder\.
    $srcFiles = @{
        "serialize\CbSerialize.cpp"  = "CbSerialize.cpp"
        "serialize\CbSerialize.h"    = "CbSerialize.h"
        "serialize\CbZstdStream.cpp" = "CbZstdStream.cpp"
        "serialize\CbZstdStream.h"   = "CbZstdStream.h"
        "value\CbString.h"           = "CbString.h"
    }
    foreach ($rel in $srcFiles.Keys) {
        Copy-Item "$repoRoot\$rel" "$dst\SRC\$($srcFiles[$rel])" -Force
    }
    Write-Host "SRC\ runtime sources synced ($($srcFiles.Count) files)." -ForegroundColor Green
} catch {
    Write-Host "ERROR syncing headers/sources: $($_.Exception.Message)" -ForegroundColor Red
    Read-Host "Press Enter to close"; exit 1
}

# --- strip windeployqt artifacts (static build needs none) -----------------
$dlls = "Qt6Core.dll","Qt6Gui.dll","Qt6Network.dll","Qt6Svg.dll","Qt6Widgets.dll",
        "dxcompiler.dll","dxil.dll","opengl32sw.dll"
$dirs = "generic","iconengines","imageformats","networkinformation",
        "platforms","styles","tls"
$removed = 0
foreach ($f in $dlls) {
    if (Test-Path "$dst\$f") { Remove-Item "$dst\$f" -Force; $removed++ }
}
foreach ($d in $dirs) {
    if (Test-Path "$dst\$d") { Remove-Item "$dst\$d" -Recurse -Force; $removed++ }
}
Write-Host "Removed $removed leftover Qt deployment item(s)."

# --- report ----------------------------------------------------------------
Write-Host "`n=== $dst ===" -ForegroundColor Cyan
Get-ChildItem $dst | Select-Object Mode, Length, Name | Format-Table -AutoSize

Read-Host "Done. Press Enter to close"
