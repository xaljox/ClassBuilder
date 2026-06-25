# M4 refresh migration: the model fires each open tree view's Refresh callback,
# replacing the Qt-side openTrees registry. DataModelDoc::UpdateTreeViews()
# mirrors ClassDiagram::UpdateClassDiagramViews (iterate the owned ViewModels,
# call Refresh()). GetRefreshCtx() exposes the VM's ctx (the Qt window) so the
# MFC->Qt selection echo can reach each window via its VM instead of openTrees.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.class).$($obj.params.name)" } }

Send @{ cmd='add_method'; params=@{ class='DataModelDoc'; name='UpdateTreeViews'; return_type='void' } }
$body = @(
'    TreeViewModelIterator iTreeViewModel(this);',
'    while (++iTreeViewModel)',
'    {',
'        iTreeViewModel->Refresh();',
'    }'
) -join "`r`n"
Send @{ cmd='set_method_body'; params=@{ class='DataModelDoc'; name='UpdateTreeViews'; body=$body } }

Send @{ cmd='add_method'; params=@{ class='TreeViewModel'; name='GetRefreshCtx'; return_type='void*'; const=$true } }
Send @{ cmd='set_method_body'; params=@{ class='TreeViewModel'; name='GetRefreshCtx'; body='    return _refreshCtx;' } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
