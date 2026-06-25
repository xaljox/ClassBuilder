# Report (read-only) of every method taking a forbidden MFC-view type as an
# argument, WITH the @CODE id (find_methods_using_type now emits "id").
# Drives the combined SD+CD deletion: I classify each hit (real shape method ->
# delete by id; relation accessor -> skip, cascades with delete_class) and
# generate the delete script. NO model changes here.
#
# REQUIRES the NEW dev build (with find-id) owning the pipe -- close the stable
# CB, open out/build/x64-release/ClassBuilder.exe on the saved .cbz first.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 10)); return $rd.ReadLine() }

$types = @('SequenceDiagramView','CClassDiagramView','SelectedSequenceDiagramShape','SelectedClassDiagramShape','CbPainter_Cdc','CDC','CClientDC')
foreach ($t in $types) {
    $resp = Send @{ cmd='find_methods_using_type'; params=@{ type=$t } }
    Write-Output "==== $t ===="
    try {
        $j = $resp | ConvertFrom-Json
        if (-not $j.ok) { Write-Output "  (none/error: $resp)"; Write-Output ""; continue }
        # dedup by method id (a method with N args of the type repeats N times)
        $rows = @($j.result) | Sort-Object -Property id -Unique | Sort-Object class, method
        Write-Output ("  {0} distinct method(s)" -f $rows.Count)
        foreach ($h in $rows) {
            Write-Output ("    {0,-28} {1,-26} id={2,-6} {3}" -f $h.class, $h.method, $h.id, $h.arg_type)
        }
    } catch { Write-Output "  RAW: $resp" }
    Write-Output ""
}
$pipe.Close()
Write-Output "DONE-REPORT"
