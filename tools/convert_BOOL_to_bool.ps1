# convert_BOOL_to_bool.ps1 -- migrate every BOOL member / argument / return
# type in the ACTIVE ClassBuilder document to C++ bool, via the pipe API.
#
# SAFE BY DESIGN:
#   * Dry-run by default -- lists what WOULD change, touches nothing. Pass
#     -Apply to actually convert.
#   * Wire-compatible: CbArchive serialises bool as a 4-byte int (== BOOL), so a
#     serialized member's .cbz / generated-code byte stream is UNCHANGED. (See
#     CbSerialize.h operator<<(bool); verified no member was ever a 1-byte bool.)
#   * Operates on whatever model is the active doc -- open the throwaway (Matrix)
#     first to rehearse; only run -Apply against the real model when you're ready,
#     then write_source + rebuild.
#
# After -Apply:  call write_source (and save the .cbz) to get bool onto disk,
# then rebuild. The Qt tree refreshes itself (full rebuild on each model update).
#
#   pwsh tools/convert_BOOL_to_bool.ps1            # dry run
#   pwsh tools/convert_BOOL_to_bool.ps1 -Apply     # do it

param([switch]$Apply)

$ErrorActionPreference = 'Stop'
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',
            [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(5000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true

function Send($cmd, $params) {
    $msg = @{ cmd = $cmd }
    if ($params) { $msg.params = $params }
    $wr.WriteLine(($msg | ConvertTo-Json -Compress -Depth 8))
    $rd.ReadLine() | ConvertFrom-Json
}

# Strip pointer/ref/array modifiers to get the bare type name.
function BareType($t) { ($t -replace '[\*\&\[\]0-9]','').Trim() }

$doc = (Send 'active_doc' $null).result
if (-not $doc) { Write-Error "No active document. Open a model first."; return }
Write-Host "Active doc: $($doc.title)`n"

$members = 0; $args_ = 0; $returns = 0
$classes = (Send 'list_classes' $null).result

foreach ($cls in $classes) {
    # --- members ---  (-ceq / -creplace: case-SENSITIVE, so only BOOL, not bool)
    $det = (Send 'get_class' @{ name = $cls }).result
    foreach ($m in $det.members) {
        if ((BareType $m.type) -ceq 'BOOL') {
            $newType = $m.type -creplace 'BOOL','bool'
            Write-Host "member  $cls.$($m.name) : $($m.type) -> $newType"
            $members++
            if ($Apply) { Send 'set_member_type' @{ class=$cls; name=$m.name; value=$newType } | Out-Null }
        }
    }
    # --- methods: return type + arguments ---
    $methods = (Send 'list_class_methods' @{ class = $cls }).result
    foreach ($mt in $methods) {
        if ((BareType $mt.return_type) -ceq 'BOOL') {
            $newRt = $mt.return_type -creplace 'BOOL','bool'
            Write-Host "return  $cls.$($mt.name)() : $($mt.return_type) -> $newRt"
            $returns++
            if ($Apply) { Send 'set_method_return_type' @{ class=$cls; id=$mt.id; value=$newRt } | Out-Null }
        }
        foreach ($a in $mt.args) {
            if ((BareType $a.type) -ceq 'BOOL') {
                $newAt = $a.type -creplace 'BOOL','bool'
                Write-Host "arg     $cls.$($mt.name)($($a.name)) : $($a.type) -> $newAt"
                $args_++
                if ($Apply) { Send 'set_argument_type' @{ class=$cls; method_id=$mt.id; arg=$a.name; value=$newAt } | Out-Null }
            }
        }
    }
}

$pipe.Close()
$verb = if ($Apply) { 'Converted' } else { 'WOULD convert' }
Write-Host "`n$verb  $members members, $returns returns, $args_ arguments (BOOL -> bool)."
if (-not $Apply) { Write-Host "Dry run -- re-run with -Apply to perform the change." }
else { Write-Host "Now: write_source + save the .cbz, then rebuild." }
