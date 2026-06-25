# Static / Non-static filter: 4 per-view flags on TreeViewModel + the gate in
# Member/Method::ShownByFilter. A member/method shows when its access AND its
# static-ness are both enabled (so: off Non-Static = only statics; off Static =
# hide statics). All default true (show everything). NO write_source here -- the
# 4 new members won't be in the ctor init list until InitInit ("reinit the init
# part") is run on the TreeViewModel constructor; do that, THEN write_source.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.name)$($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.name)$($obj.params.class).$($obj.params.name)" } }

function AddBool($name){
    Send @{ cmd='add_member'; params=@{ class='TreeViewModel'; name=$name; type='bool'; initialization='true'; serialize=$false; getter='public'; setter='public' } }
}
AddBool 'showStaticMembers'
AddBool 'showNonStaticMembers'
AddBool 'showStaticMethods'
AddBool 'showNonStaticMethods'

$memberBody = @(
'    AccessType access = GetAccess();',
'    bool show = (access == PUBLIC    && pTreeViewModel->GetShowPublicMembers())',
'             || (access == PROTECTED && pTreeViewModel->GetShowProtectedMembers())',
'             || (access == PRIVATE   && pTreeViewModel->GetShowPrivateMembers());',
'    if (!show)',
'    {',
'        return false;',
'    }',
'',
'    if (!(GetStatic() ? pTreeViewModel->GetShowStaticMembers()',
'                      : pTreeViewModel->GetShowNonStaticMembers()))',
'    {',
'        return false;',
'    }',
'',
'    return Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Member'; name='ShownByFilter'; body=$memberBody } }

$methodBody = @(
'    AccessType access = GetAccess();',
'    bool show = (access == PUBLIC    && pTreeViewModel->GetShowPublicMethods())',
'             || (access == PROTECTED && pTreeViewModel->GetShowProtectedMethods())',
'             || (access == PRIVATE   && pTreeViewModel->GetShowPrivateMethods());',
'    if (!show)',
'    {',
'        return false;',
'    }',
'',
'    if (!(GetStatic() ? pTreeViewModel->GetShowStaticMethods()',
'                      : pTreeViewModel->GetShowNonStaticMethods()))',
'    {',
'        return false;',
'    }',
'',
'    return Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Method'; name='ShownByFilter'; body=$methodBody } }

$pipe.Close()
Write-Output "DONE -- now reinit the TreeViewModel constructor init part, then write_source"
