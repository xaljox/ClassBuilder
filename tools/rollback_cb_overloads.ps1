# rollback_cb_overloads.ps1
#
# Deletes every Serialize and SerializeRelations overload whose first
# argument has type CbArchive&. Used to wipe the wrong-signature methods
# created by the first run of duplicate_serialize.ps1 so the corrected
# version can re-create them.
#
# Idempotent: runs cleanly even if no such overloads exist.

param(
    [string[]]$MethodNames = @('Serialize', 'SerializeRelations')
)

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

$classes = Invoke-Cb 'list_classes'
$summary = @{}

foreach ($methodName in $MethodNames) {
    Write-Host "==== $methodName ====" -ForegroundColor Cyan
    $deleted = 0
    $failed  = @()

    foreach ($className in $classes) {
        $methods = Invoke-Cb 'list_class_methods' @{ class = $className }
        if (-not $methods) { continue }

        foreach ($m in $methods) {
            if ($m.name -ne $methodName) { continue }
            if ($m.args.Count -lt 1)     { continue }
            if ($m.args[0].type.Trim() -ne 'CbArchive&') { continue }

            try {
                $null = Invoke-Cb 'delete_method' @{ class = $className; id = $m.id }
                $deleted++
                Write-Host "  - $className.$($m.name)  (id=$($m.id))"
            } catch {
                $failed += "$className.$($m.name) : $_"
                Write-Host "  ! $className.$($m.name) failed: $_" -ForegroundColor Red
            }
        }
    }

    $summary[$methodName] = [pscustomobject]@{ Deleted = $deleted; Failed = $failed.Count }
}

Write-Host ""
Write-Host "==== Summary ====" -ForegroundColor Cyan
$summary.GetEnumerator() | ForEach-Object {
    Write-Host ("  {0,-22}  deleted={1,-4}  failed={2}" -f $_.Key, $_.Value.Deleted, $_.Value.Failed)
}

$pipe.Dispose()
