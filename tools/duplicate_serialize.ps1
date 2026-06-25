# duplicate_serialize.ps1
#
# Bulk-creates `Serialize(CbArchive& archive)` and
# `SerializeRelations(CbArchive& archive, DataModelDocObject* pointerArray[])`
# overloads on every class that already has the corresponding `(CArchive&)`
# version.
#
# Argument naming and shape mirror the MFC versions exactly so that the
# existing SERIALIZE_ALL_OBJECTS / WRITE_MULTI_ACTIVE / READ_MULTI_ACTIVE
# macros — which hardcode `archive` as the variable name — work unchanged
# inside the new bodies.
#
# Idempotent: classes that already have a CbArchive overload (matched on
# first-arg type) are skipped.

$ErrorActionPreference = 'Stop'

$pipe = [System.IO.Pipes.NamedPipeClientStream]::new(
    '.', 'ClassBuilder', [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(2000)
$writer = [System.IO.StreamWriter]::new($pipe); $writer.AutoFlush = $true
$reader = [System.IO.StreamReader]::new($pipe)

function Invoke-Cb([string]$cmd, [hashtable]$params = @{}) {
    $payload = @{ cmd = $cmd; params = $params } | ConvertTo-Json -Compress -Depth 10
    $writer.WriteLine($payload)
    $line = $reader.ReadLine()
    $resp = $line | ConvertFrom-Json
    if (-not $resp.ok) { throw "command '$cmd' failed: $($resp.error)" }
    return $resp.result
}

# Per-method-name configuration: which arguments to add. Keeps the script
# extensible if more CbArchive-flavoured methods need duplicating later.
$methodSpecs = @(
    [pscustomobject]@{
        Name = 'Serialize'
        Args = @( @{ name = 'archive'; type = 'CbArchive&' } )
    },
    [pscustomobject]@{
        Name = 'SerializeRelations'
        Args = @(
            @{ name = 'archive';      type = 'CbArchive&' },
            @{ name = 'pointerArray'; type = 'DataModelDocObject*[]' }
        )
    }
)

# True when $methods (full method records) already contains an overload of
# $methodName whose first arg has type CbArchive&. We only need to look at
# the first arg — the second one for SerializeRelations is bookkeeping that
# adds nothing to the dedupe key.
function Has-CbOverload($methods, [string]$methodName) {
    foreach ($m in $methods) {
        if ($m.name -ne $methodName) { continue }
        if ($m.args.Count -lt 1)     { continue }
        if ($m.args[0].type.Trim() -eq 'CbArchive&') { return $true }
    }
    return $false
}

$summary = @{}

foreach ($spec in $methodSpecs) {
    $methodName = $spec.Name
    Write-Host "==== $methodName ====" -ForegroundColor Cyan

    $hits = Invoke-Cb 'list_methods_named' @{ name = $methodName }
    Write-Host "  $($hits.Count) classes carry $methodName(CArchive&)"

    $added   = 0
    $skipped = 0
    $failed  = @()

    foreach ($hit in $hits) {
        $className = $hit.class

        $allMethods = Invoke-Cb 'list_class_methods' @{ class = $className }
        if (Has-CbOverload $allMethods $methodName) {
            $skipped++
            Write-Host "  skip $className ($methodName(CbArchive&) already present)" -ForegroundColor Yellow
            continue
        }

        try {
            $params = @{
                class       = $className
                name        = $methodName
                return_type = 'void'
                args        = $spec.Args
                access      = 'public'
                virtual     = $true
                body        = "    // TODO Bz`n"
            }
            $null = Invoke-Cb 'add_method' $params
            $added++
            Write-Host "  + $className"
        }
        catch {
            $failed += "$className : $_"
            Write-Host "  ! $className failed: $_" -ForegroundColor Red
        }
    }

    $summary[$methodName] = [pscustomobject]@{
        Added = $added; Skipped = $skipped; Failed = $failed.Count
    }
}

Write-Host ""
Write-Host "==== Summary ====" -ForegroundColor Cyan
$summary.GetEnumerator() | ForEach-Object {
    Write-Host ("  {0,-22}  added={1,-4}  skipped={2,-4}  failed={3}" -f `
        $_.Key, $_.Value.Added, $_.Value.Skipped, $_.Value.Failed)
}

$pipe.Dispose()
