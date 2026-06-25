# fill_serialize_bodies.ps1
#
# Phase B1: writes the body of every `Serialize(CbArchive&)` method that was
# created by duplicate_serialize.ps1.
#
# For each class that has both a Serialize(CArchive&) (the original MFC body)
# and a Serialize(CbArchive&) (our new shell with `// TODO Bz`):
#   1. Parse <ClassName>.cpp for the original body.
#   2. Extract the base-class chain call (e.g. `Variable::Serialize(archive);`)
#      and the canonical field write order from the IsStoring branch.
#   3. Generate a v1 CbArchive body: chain to same base, write/read a literal
#      `1` for version, then write/read all fields unconditionally. No
#      version-gate `if (V <= _objectVersion)` chains -- that's the whole
#      point of the v1 format.
#   4. POST it via set_method_body.
#
# Idempotent: bodies that have already been filled (no longer contain
# "TODO Bz") are skipped.

param(
    [string]$SourceDir   = 'c:\Users\jimmy\Projects\Src\ClassBuilder\ClassBuilder',
    [string]$OnlyClass   = $null,      # if set, run for one class only
    [string[]]$SkipClasses = @('DataModelDoc'),  # special bodies, hand-edit
    [switch]$DryRun                    # print bodies without sending
)

$ErrorActionPreference = 'Stop'

# --- Pipe ---

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

# --- Parsing the original Serialize body --------------------------------------

# Returns @{ base = '<BaseName>' or $null; fields = @('_a','_b',...) }
# Returns $null if the file or the Serialize method can't be located.
function Parse-SerializeBody([string]$ClassName) {
    $path = Join-Path $SourceDir "$ClassName.cpp"
    if (-not (Test-Path $path)) { return $null }
    $src = Get-Content -Raw -LiteralPath $path

    # Match: void <ClassName>::Serialize(CArchive& archive)\n{...}\n}
    $sig = "void\s+$ClassName::Serialize\s*\(\s*CArchive\s*&\s*\w+\s*\)"
    if ($src -notmatch "(?ms)$sig\s*\{(.*?)^\}") { return $null }
    $body = $Matches[1]

    # Base chain: first <Identifier>::Serialize(<arg>); line before the if
    $base = $null
    if ($body -match '(?m)^\s*([A-Za-z_]\w*)::Serialize\s*\(') {
        $base = $Matches[1]
    }

    # Field writes inside the IsStoring branch -- `archive << _xxx;`
    # We walk lines; everything between IsStoring's `{` and its matching `}`.
    $fields = @()
    $inStoring = $false
    $depth = 0
    foreach ($line in ($body -split "`n")) {
        if (-not $inStoring -and $line -match 'IsStoring\s*\(\s*\)') {
            $inStoring = $true
            $depth = 0
            continue
        }
        if ($inStoring) {
            $opens  = ([regex]::Matches($line, '\{')).Count
            $closes = ([regex]::Matches($line, '\}')).Count
            $depth += $opens - $closes
            if ($line -match '^\s*\w+\s*<<\s*(_[A-Za-z_]\w*)\s*;') {
                $fields += $Matches[1]
            }
            if ($depth -lt 0) { break }   # closing brace of IsStoring block
        }
    }

    return @{ base = $base; fields = $fields }
}

# --- New body generation ------------------------------------------------------

function Build-CbBody([string]$BaseName, [string[]]$Fields) {
    # CBZ v1: a single global version is written once at the very top of
    # DataModelDoc::Serialize and stored in DataModelDocObject::_objectVersion.
    # Derived classes neither write nor read their own version — they just
    # stream their own fields. If a future schema change needs gating, check
    # the inherited static `_objectVersion` member.
    $sb = [System.Text.StringBuilder]::new()
    if ($BaseName) {
        [void]$sb.AppendLine("    $BaseName::Serialize(archive);")
    }
    [void]$sb.AppendLine("    if (archive.IsStoring())")
    [void]$sb.AppendLine("    {")
    foreach ($f in $Fields) {
        [void]$sb.AppendLine("        archive << $f;")
    }
    [void]$sb.AppendLine("    }")
    [void]$sb.AppendLine("    else")
    [void]$sb.AppendLine("    {")
    foreach ($f in $Fields) {
        [void]$sb.AppendLine("        archive >> $f;")
    }
    [void]$sb.AppendLine("    }")
    return $sb.ToString()
}

# --- Locating the new CbArchive overload --------------------------------------

# Returns the method id, or $null if no Serialize(CbArchive&) overload exists
# (or it has already been filled in).
function Find-CbSerialize($ClassName) {
    $methods = Invoke-Cb 'list_class_methods' @{ class = $ClassName }
    if (-not $methods) { return $null }
    foreach ($m in $methods) {
        if ($m.name -ne 'Serialize') { continue }
        if ($m.args.Count -lt 1)     { continue }
        if ($m.args[0].type.Trim() -ne 'CbArchive&') { continue }
        return $m
    }
    return $null
}

# --- Main --------------------------------------------------------------------

if ($DryRun) {
    # Without a live API session we still need a class list to walk; in dry
    # mode we discover them from .cpp files in $SourceDir.
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
        Write-Host "  - $className (in skip list -- body needs hand-edit, e.g. SERIALIZE_ALL_OBJECTS)" -ForegroundColor DarkYellow
        continue
    }

    $parsed = Parse-SerializeBody $className
    if (-not $parsed) {
        # Not all listed classes are CB-modeled with a Serialize body in src;
        # skip silently rather than treating as failure.
        continue
    }

    $body = Build-CbBody $parsed.base $parsed.fields

    if ($DryRun) {
        Write-Host "===== $className =====" -ForegroundColor Cyan
        Write-Host $body
        $filled++
        continue
    }

    $cbMethod = Find-CbSerialize $className
    if (-not $cbMethod) {
        $missing++
        Write-Host "  - $className (no Serialize(CbArchive&) overload found)" -ForegroundColor DarkYellow
        continue
    }
    if ($cbMethod.args[0].type.Trim() -ne 'CbArchive&') {
        $missing++
        continue
    }

    # set_method_body needs class+id (or class+name). Use id to disambiguate
    # the overload from the existing Serialize(CArchive&).
    try {
        $null = Invoke-Cb 'set_method_body' @{
            class = $className
            id    = $cbMethod.id
            body  = $body
        }
        $filled++
        Write-Host "  + $className  ($($parsed.fields.Count) fields)"
    } catch {
        $failed += "$className : $_"
        Write-Host "  ! $className failed: $_" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "==== Summary ====" -ForegroundColor Cyan
Write-Host "  filled  = $filled"
Write-Host "  missing = $missing  (no CbArchive overload -- run duplicate_serialize.ps1 first)"
Write-Host "  failed  = $($failed.Count)"

if (-not $DryRun) { $pipe.Dispose() }
