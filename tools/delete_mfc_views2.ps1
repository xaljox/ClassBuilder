# Phase A+C redo: delete the 4 doomed classes (delete_class wants 'name').
# Selected* first (cascade their Selected relations + accessors), then the view
# classes (cascade their View relations + own methods). Then re-verify.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 10)); return ($rd.ReadLine() | ConvertFrom-Json) }
function Find($type){ $r = Send @{ cmd='find_methods_using_type'; params=@{ type=$type } }; if ($r.ok) { return @($r.result) } else { return @() } }

foreach ($c in @('SelectedSequenceDiagramShape','SelectedClassDiagramShape','SequenceDiagramView','CClassDiagramView')) {
    $r = Send @{ cmd='delete_class'; params=@{ name=$c } }
    Write-Output ("delete_class {0,-30} ok={1} {2}" -f $c, $r.ok, $r.error)
}

Write-Output "`n== verify =="
foreach ($t in @('SequenceDiagramView','CClassDiagramView','SelectedSequenceDiagramShape','SelectedClassDiagramShape','CDC')) {
    $r = Send @{ cmd='find_methods_using_type'; params=@{ type=$t } }
    if ($r.ok) { Write-Output ("  {0,-30} {1}" -f $t, @($r.result).Count) }
    else       { Write-Output ("  {0,-30} {1}" -f $t, $r.error) }
}
foreach ($t in @('SequenceDiagramViewModel','ClassDiagramViewModel')) {
    $rows = @(Find $t) | Sort-Object -Property id -Unique
    Write-Output ("  {0,-26} {1} distinct (keep)" -f $t, $rows.Count)
}
$pipe.Close()
Write-Output "DONE2 (no write_source yet)"
