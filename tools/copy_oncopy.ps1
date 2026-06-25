# Add a virtual Gti::OnCopy(BOOL checkOnly) mirroring OnPaste: it pulls the
# "which node types are copyable" decision out of the MFC view's OnUpdateEditCopy
# and into the model. Base Gti::OnCopy covers the IsX() types; Method::OnCopy
# overrides for the method-specific gate. On !checkOnly it stashes the node in the
# shared static via SetGtiCopy(this). Both MFC + Qt then call OnCopy(checkOnly).
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.class).$($obj.params.name)" } }

$gtiBody = @(
'    BOOL enable = (IsExternClass() || IsDataModel() ||',
'        IsExternClasses() || IsMember() ||',
'        IsClassDiagram() || IsSequenceDiagram());',
'',
'    if (enable && !checkOnly)',
'    {',
'        SetGtiCopy(this);',
'    }',
'',
'    return enable;'
) -join "`r`n"

$methodBody = @(
'    BOOL enable = (!IsFindMethod() && !IsDestructor() &&',
'        !IsFixed() && !IsMemberMethod());',
'',
'    if (enable && !checkOnly)',
'    {',
'        SetGtiCopy(this);',
'    }',
'',
'    return enable;'
) -join "`r`n"

Send @{ cmd='add_method'; params=@{ class='Gti'; name='OnCopy'; return_type='int'; virtual=$true; access='public';
    args=@(@{ name='checkOnly'; type='BOOL'; default='FALSE' }); body=$gtiBody } }
Send @{ cmd='add_method'; params=@{ class='Method'; name='OnCopy'; return_type='int'; virtual=$true; access='public';
    args=@(@{ name='checkOnly'; type='BOOL'; default='FALSE' }); body=$methodBody } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
