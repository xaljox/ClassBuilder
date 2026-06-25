# Move the "show only classes without constructor" filter off the MFC view and
# into the per-view TreeViewModel, AND into Class::ShownByFilter (so the Qt mirror
# honours it too -- it was previously only in Class::Show, the MFC-only path).
#
# Class.cpp carries a hand edit (Class::InitPhase sets the destructor's phase) that
# is NOT pipe-managed; write_source would regenerate Class.cpp from the model, so
# we first push that exact body into the model to protect it.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.class).$($obj.params.name)" } }

# 1. Protect the hand-edited Class::InitPhase by writing its exact body to the model.
$initPhase = @(
'    if (GetDataModel()->GetPhase() > Design_Phase)',
'    {',
'        SetPhase(Implementation_Phase);',
'    }',
'    else',
'    {',
'        SetPhase(GetDataModel()->GetPhase());',
'    }',
'    Destructor* pDestructor = dynamic_cast<Destructor*>(GetFirstMethod());',
'    if (pDestructor)',
'    {',
'        pDestructor->SetPhase(GetPhase());',
'    }'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Class'; name='InitPhase'; body=$initPhase } }

# 2. The per-view flag (defaults OFF -- unlike the other 11 which default ON).
Send @{ cmd='add_member'; params=@{ class='TreeViewModel'; name='showOnlyClassesWithoutConstructor';
    type='bool'; initialization='false'; serialize=$false; getter='public'; setter='public' } }

# 3. Fold the ctor filter into Class::ShownByFilter (preserving the exact old
#    Class::Show gate), then defer to the base (phase) gate.
$shownByFilter = @(
'    bool show = true;',
'    if (pTreeViewModel->GetShowOnlyClassesWithoutConstructor())',
'    {',
'        if (GetDocument())',
'        {',
'            show = false;',
'        }',
'',
'        BaseClass::MethodIterator iMethod(this, &Method::IsNormalConstructor);',
'        while (show && ++iMethod)',
'        {',
'            show = false;',
'        }',
'    }',
'',
'    return show && Gti::ShownByFilter(pTreeViewModel);'
) -join "`r`n"
Send @{ cmd='add_method'; params=@{ class='Class'; name='ShownByFilter'; return_type='bool'; virtual=$true; access='public';
    args=@(@{ name='pTreeViewModel'; type='TreeViewModel*' }); body=$shownByFilter } }

# 4. Class::Show now just delegates -- the filter lives in ShownByFilter, which
#    Gti::Show gates on.
Send @{ cmd='set_method_body'; params=@{ class='Class'; name='Show'; body='    Gti::Show(pCClassBuilderView, parentItem);' } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
