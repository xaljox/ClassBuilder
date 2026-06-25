# M4: the visibility decision as a MODEL method -- virtual bool
# Gti::ShownByFilter(TreeViewModel*), overridden by Member/Method. Reads the
# per-view filter bools off the ViewModel (no statics, no bitmasks). Gti base =
# phase gate; Member/Method = access gate + base. Both the MFC tree's Show() and
# the Qt mirror call this one source. (Show wiring is a later step.)
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.class) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.class).$($obj.params.name)" } }

$arg = @( @{ name='pTreeViewModel'; type='TreeViewModel*' } )

Send @{ cmd='add_method'; params=@{ class='Gti';    name='ShownByFilter'; return_type='bool'; virtual=$true; args=$arg } }
Send @{ cmd='add_method'; params=@{ class='Member'; name='ShownByFilter'; return_type='bool'; virtual=$true; args=$arg } }
Send @{ cmd='add_method'; params=@{ class='Method'; name='ShownByFilter'; return_type='bool'; virtual=$true; args=$arg } }

# Gti base: phase gate (no phase / phase support off -> always shown).
$gti = @(
'    if (GetDataModelDoc()->GetDataModel()->GetPhaseSupport())',
'    {',
'        Phase phase = GetPhase();',
'        bool show = phase == None_Phase',
'                 || (phase == Analysis_Phase       && pTreeViewModel->GetShowAnalysisPhase())',
'                 || (phase == Design_Phase         && pTreeViewModel->GetShowDesignPhase())',
'                 || (phase == Implementation_Phase && pTreeViewModel->GetShowImplementationPhase())',
'                 || (phase == Test_Phase           && pTreeViewModel->GetShowTestPhase())',
'                 || (phase == Complete_Phase       && pTreeViewModel->GetShowCompletePhase());',
'        if (!show)',
'        {',
'            return false;',
'        }',
'    }',
'',
'    return true;'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Gti'; name='ShownByFilter'; body=$gti } }

$member = @(
'    AccessType access = GetAccess();',
'    bool show = (access == PUBLIC    && pTreeViewModel->GetShowPublicMembers())',
'             || (access == PROTECTED && pTreeViewModel->GetShowProtectedMembers())',
'             || (access == PRIVATE   && pTreeViewModel->GetShowPrivateMembers());',
'    if (!show)',
'    {',
'        return false;',
'    }',
'',
'    return Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Member'; name='ShownByFilter'; body=$member } }

$method = @(
'    AccessType access = GetAccess();',
'    bool show = (access == PUBLIC    && pTreeViewModel->GetShowPublicMethods())',
'             || (access == PROTECTED && pTreeViewModel->GetShowProtectedMethods())',
'             || (access == PRIVATE   && pTreeViewModel->GetShowPrivateMethods());',
'    if (!show)',
'    {',
'        return false;',
'    }',
'',
'    return Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Method'; name='ShownByFilter'; body=$method } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
