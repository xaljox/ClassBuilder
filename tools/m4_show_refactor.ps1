# M4 "both sides": refactor the MFC tree's Gti::Show to gate on the shared model
# method ShownByFilter (reading the view's own TreeViewModel) instead of its
# inline phase gate + the per-type access gates. Member::Show / Method::Show then
# collapse to a plain Gti::Show call (their access gate now lives in the virtual
# ShownByFilter override). Class::Show is left as-is (its niche ctor filter still
# reads the view flag) -- so Class.cpp isn't regenerated.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.class).$($obj.params.name)" } }

$show = @(
'    assert(pCClassBuilderView);',
'',
'    if (ShownByFilter(pCClassBuilderView->GetTreeViewModel()))',
'    {',
'        if (_added)',
'        {',
'            CTreeCtrl& rCTreeCtrl = pCClassBuilderView->GetTreeCtrl();',
'            HTREEITEM item = rCTreeCtrl.InsertItem(',
'                TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT|TVIF_STATE|TVIF_PARAM,',
'                GetItemText(), GetIcon(), GetIcon(),',
'                INDEXTOSTATEIMAGEMASK(GetStateIcon()), TVIS_STATEIMAGEMASK,',
'                DWORD_PTR(this), parentItem, TVI_LAST);',
'',
'            ChildIterator gti(this);',
'            while (++gti)',
'                gti->Show(pCClassBuilderView, item);',
'',
'            pCClassBuilderView->Sort(parentItem);',
'        }',
'    }'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='Gti'; name='Show'; body=$show } }

$delegate = '    Gti::Show(pCClassBuilderView, parentItem);'
Send @{ cmd='set_method_body'; params=@{ class='Member'; name='Show'; body=$delegate } }
Send @{ cmd='set_method_body'; params=@{ class='Method'; name='Show'; body=$delegate } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
