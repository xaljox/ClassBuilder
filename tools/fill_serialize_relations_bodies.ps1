# fill_serialize_relations_bodies.ps1
#
# Phase B2: writes the body of every `SerializeRelations(CbArchive&,
# DataModelDocObject* pointerArray[])` overload.
#
# Because the new method shares the parameter names `archive` and
# `pointerArray` with the MFC version, the WRITE_MULTI_ACTIVE /
# READ_MULTI_ACTIVE / WRITE_SINGLE_ACTIVE / etc. macros (which reference
# those names literally) work unchanged. So the body is simply:
#
#   1. Chain to base class (`Foo::SerializeRelations(archive, pointerArray);`)
#   2. The if (IsStoring) / else block, copied verbatim from the MFC body.
#
# Idempotent: bodies that no longer contain "TODO Bz" are skipped.

param(
    [string]$SourceDir   = 'c:\Users\jimmy\Projects\Src\ClassBuilder\ClassBuilder',
    [string]$OnlyClass   = $null,
    [string[]]$SkipClasses = @('DataModelDoc'),  # special, hand-edit
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

if (-not $DryRun) {
    $pipe = [System.IO.Pipes.NamedPipeClientStream]::new(
        '.', 'ClassBuilder', [System.IO.Pipes.PipeDirection]::InOut)
    $pipe.Connect(2000)
    $writer = [System.IO.StreamWriter]::new($pipe); $writer.AutoFlush = $true
    $reader = [System.IO.StreamReader]::new($pipe)
}

function Invoke-Cb([string]$cmd, [hashtable]$params = @{}) {
    if ($DryRun) { return $null }
    $payload = @{ cmd = $cmd; params = $params } | ConvertTo-Json -Compress -Depth 10
    $writer.WriteLine($payload)
    $line = $reader.ReadLine()
    $resp = $line | ConvertFrom-Json
    if (-not $resp.ok) { throw "command '$cmd' failed: $($resp.error)" }
    return $resp.result
}

# Returns @{ base = '<BaseName>' or $null; storing = '<text>'; loading = '<text>' }
# storing/loading hold the contents of the if (IsStoring) and else blocks
# verbatim (whitespace preserved, no surrounding braces).
function ParseSerializeRelationsBody([string]$ClassName) {
    $path = Join-Path $SourceDir "$ClassName.cpp"
    if (-not (Test-Path $path)) { return $null }
    $src = Get-Content -Raw -LiteralPath $path

    $sig = "void\s+$ClassName::SerializeRelations\s*\(\s*CArchive\s*&\s*\w+\s*,[^)]+\)"
    if ($src -notmatch "(?ms)$sig\s*\{(.*?)^\}") { return $null }
    $body = $Matches[1]

    # Base chain (skip the SerializeRelations recursion to self if present)
    $base = $null
    foreach ($m in [regex]::Matches($body, '(?m)^\s*([A-Za-z_]\w*)::SerializeRelations\s*\(')) {
        if ($m.Groups[1].Value -ne $ClassName) { $base = $m.Groups[1].Value; break }
    }

    # Walk lines, capturing the contents of the if (IsStoring) { ... } block
    # and the else { ... } block. Brace-counting per block.
    $lines = $body -split "`r?`n"
    $storing = New-Object System.Collections.Generic.List[string]
    $loading = New-Object System.Collections.Generic.List[string]

    $state = 'pre'    # pre | inIfHead | inStoring | betweenElseHead | inLoading | done
    $depth = 0

    foreach ($line in $lines) {
        switch ($state) {
            'pre' {
                if ($line -match 'IsStoring\s*\(\s*\)') { $state = 'inIfHead' }
            }
            'inIfHead' {
                # Skip until we see the opening brace
                if ($line -match '\{') {
                    $state = 'inStoring'
                    $depth = ([regex]::Matches($line, '\{')).Count - ([regex]::Matches($line, '\}')).Count
                }
            }
            'inStoring' {
                $opens  = ([regex]::Matches($line, '\{')).Count
                $closes = ([regex]::Matches($line, '\}')).Count
                $depth += $opens - $closes
                if ($depth -le 0) {
                    $state = 'betweenElseHead'
                } else {
                    $storing.Add($line)
                }
            }
            'betweenElseHead' {
                if ($line -match '^\s*else\b') { $state = 'inIfHead2' }
            }
            'inIfHead2' {
                if ($line -match '\{') {
                    $state = 'inLoading'
                    $depth = ([regex]::Matches($line, '\{')).Count - ([regex]::Matches($line, '\}')).Count
                }
            }
            'inLoading' {
                $opens  = ([regex]::Matches($line, '\{')).Count
                $closes = ([regex]::Matches($line, '\}')).Count
                $depth += $opens - $closes
                if ($depth -le 0) { $state = 'done'; break }
                $loading.Add($line)
            }
        }
    }

    # v1 format: drop `if (N <= _objectVersion) { ... }` wrappers from the
    # load branch — every relation read is unconditional in the fresh format.
    # Keep the inner content, de-indented one level.
    $loadingText = ($loading -join "`r`n")
    $loadingText = [regex]::Replace($loadingText,
        '(?ms)^\s*if\s*\(\s*\d+\s*<=\s*_objectVersion\s*\)\s*\r?\n\s*\{\s*\r?\n(.*?)^\s*\}\s*$',
        {
            param($m)
            # De-indent inner block by 4 spaces if uniformly indented.
            $inner = $m.Groups[1].Value
            ($inner -split "`r?`n" | ForEach-Object {
                if ($_ -match '^    (.*)$') { $Matches[1] } else { $_ }
            }) -join "`r`n"
        })

    # Drop blank/whitespace-only lines left over by the unwrapping pass so the
    # body reads as a flat list of macro calls.
    $loadingText = ($loadingText -split "`r?`n" |
        Where-Object { $_.Trim() -ne '' }) -join "`r`n"

    return @{
        base    = $base
        storing = ($storing -join "`r`n")
        loading = $loadingText
    }
}

function BuildCbBody([string]$BaseName, [string]$StoringText, [string]$LoadingText) {
    $sb = [System.Text.StringBuilder]::new()
    if ($BaseName) {
        [void]$sb.AppendLine("    $BaseName::SerializeRelations(archive, pointerArray);")
    }
    [void]$sb.AppendLine("    if (archive.IsStoring())")
    [void]$sb.AppendLine("    {")
    if ($StoringText) { [void]$sb.AppendLine($StoringText) }
    [void]$sb.AppendLine("    }")
    [void]$sb.AppendLine("    else")
    [void]$sb.AppendLine("    {")
    if ($LoadingText) { [void]$sb.AppendLine($LoadingText) }
    [void]$sb.AppendLine("    }")
    return $sb.ToString()
}

# Returns the new SerializeRelations(CbArchive&,...) method record.
function FindCbSerializeRelations($ClassName) {
    $methods = Invoke-Cb 'list_class_methods' @{ class = $ClassName }
    if (-not $methods) { return $null }
    foreach ($m in $methods) {
        if ($m.name -ne 'SerializeRelations') { continue }
        if ($m.args.Count -lt 1)               { continue }
        if ($m.args[0].type.Trim() -ne 'CbArchive&') { continue }
        return $m
    }
    return $null
}

# --- Main ---

if ($DryRun) {
    $classes = Get-ChildItem -LiteralPath $SourceDir -Filter '*.cpp' |
        ForEach-Object { [IO.Path]::GetFileNameWithoutExtension($_.Name) }
} else {
    $classes = Invoke-Cb 'list_classes'
}

if ($OnlyClass) { $classes = @($OnlyClass) }

$filled  = 0
$skipped = 0
$missing = 0
$failed  = @()

foreach ($className in $classes) {
    if ($SkipClasses -contains $className) {
        $skipped++
        Write-Host "  - $className (in skip list)" -ForegroundColor DarkYellow
        continue
    }

    $parsed = ParseSerializeRelationsBody $className
    if (-not $parsed) { continue }

    $body = BuildCbBody $parsed.base $parsed.storing $parsed.loading

    if ($DryRun) {
        Write-Host "===== $className =====" -ForegroundColor Cyan
        Write-Host $body
        $filled++
        continue
    }

    $cbMethod = FindCbSerializeRelations $className
    if (-not $cbMethod) {
        $missing++
        Write-Host "  - $className (no SerializeRelations(CbArchive&) overload)" -ForegroundColor DarkYellow
        continue
    }

    try {
        $null = Invoke-Cb 'set_method_body' @{
            class = $className
            id    = $cbMethod.id
            body  = $body
        }
        $filled++
        $nLines = if ($parsed.storing) { ($parsed.storing -split "`n").Count } else { 0 }
        Write-Host "  + $className  ($nLines stored lines)"
    } catch {
        $failed += "$className : $_"
        Write-Host "  ! $className failed: $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "==== Summary ====" -ForegroundColor Cyan
Write-Host "  filled  = $filled"
Write-Host "  skipped = $skipped"
Write-Host "  missing = $missing"
Write-Host "  failed  = $($failed.Count)"

if (-not $DryRun) { $pipe.Dispose() }
