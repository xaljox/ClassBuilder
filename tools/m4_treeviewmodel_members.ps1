# M4 TreeViewModel: add the per-view filter state (11 bool flags, init true) +
# the sub-tree root Gti* (where a sub-window's tree starts). Non-serialized
# (the class already is); public get/set so the GUI flips them and Gti::
# ShownByFilter reads them. The base class (callbacks, ctor, Refresh, the
# DataModelDoc relation) was created in the GUI already.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.name)" } }

function AddBool($name){
    Send @{ cmd='add_member'; params=@{ class='TreeViewModel'; name=$name; type='bool'; initialization='true'; serialize=$false; getter='public'; setter='public' } }
}

AddBool 'showPublicMembers'
AddBool 'showProtectedMembers'
AddBool 'showPrivateMembers'
AddBool 'showPublicMethods'
AddBool 'showProtectedMethods'
AddBool 'showPrivateMethods'
AddBool 'showAnalysisPhase'
AddBool 'showDesignPhase'
AddBool 'showImplementationPhase'
AddBool 'showTestPhase'
AddBool 'showCompletePhase'

# Sub-tree root: which Gti a sub-window's tree starts at (null = the whole tree).
Send @{ cmd='add_member'; params=@{ class='TreeViewModel'; name='subTree'; type='Gti*'; initialization='0'; serialize=$false; getter='public'; setter='public' } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
