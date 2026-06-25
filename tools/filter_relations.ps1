# Relations/inheritance filter: 5 per-view flags on TreeViewModel + ShownByFilter
# on FromRelation/ToRelation (cardinality via Relation::GetMulti/GetSingle +
# aggregation via GetOwned) and Inherit (inheritance flag). All default true.
# NO write_source -- the 5 members aren't in the ctor init list until the
# TreeViewModel constructor's init part is regenerated (reinit / the new
# reinit_constructor pipe command); do that, THEN write_source.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.name)$($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.name)$($obj.params.class).$($obj.params.name)" } }

function AddBool($name){
    Send @{ cmd='add_member'; params=@{ class='TreeViewModel'; name=$name; type='bool'; initialization='true'; serialize=$false; getter='public'; setter='public' } }
}
AddBool 'showInheritance'
AddBool 'showMultiRelations'
AddBool 'showSingleRelations'
AddBool 'showAggregationRelations'
AddBool 'showNonAggregationRelations'

$relBody = @(
'    Relation* pRelation = GetRelation();',
'    // Static relations are always shown -- there are few and they are easy to',
'    // spot when the rest is filtered out, so they bypass the cardinality/',
'    // aggregation gate entirely.',
'    if (pRelation && !pRelation->GetStatic())',
'    {',
'        bool cardOk = pRelation->GetMulti()',
'            ? pTreeViewModel->GetShowMultiRelations()',
'            : pTreeViewModel->GetShowSingleRelations();',
'        bool aggOk = pRelation->GetOwned()',
'            ? pTreeViewModel->GetShowAggregationRelations()',
'            : pTreeViewModel->GetShowNonAggregationRelations();',
'        if (!cardOk || !aggOk)',
'        {',
'            return false;',
'        }',
'    }',
'',
'    return Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='add_method'; params=@{ class='FromRelation'; name='ShownByFilter'; return_type='bool'; virtual=$true; access='public';
    args=@(@{ name='pTreeViewModel'; type='TreeViewModel*' }); body=$relBody } }
Send @{ cmd='add_method'; params=@{ class='ToRelation'; name='ShownByFilter'; return_type='bool'; virtual=$true; access='public';
    args=@(@{ name='pTreeViewModel'; type='TreeViewModel*' }); body=$relBody } }

$inhBody = @(
'    if (!pTreeViewModel->GetShowInheritance())',
'    {',
'        return false;',
'    }',
'',
'    return Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='add_method'; params=@{ class='Inherit'; name='ShownByFilter'; return_type='bool'; virtual=$true; access='public';
    args=@(@{ name='pTreeViewModel'; type='TreeViewModel*' }); body=$inhBody } }

$pipe.Close()
Write-Output "DONE -- reinit the TreeViewModel constructor init part, then write_source"
