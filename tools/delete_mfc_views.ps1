# Combined SD+CD MFC-view deletion -- MODEL EDITS ONLY, NO write_source.
# Recoverable via Ctrl+Z / reload CBZ. Ends with a verification gate:
#   - forbidden types -> 0 / type-not-found
#   - ViewModel overloads -> UNCHANGED (33 SD + 37 CD)  <-- the safety proof
#
# Rule for the method delete-set (union of the 3 forbidden arg-type lists):
#   delete by id UNLESS owner class is a doomed view class, OR the name is a
#   view-relation accessor (those cascade with delete_class). The ViewModel /
#   painter-only overloads take non-forbidden types -> never in these lists ->
#   never touched.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 10)); return ($rd.ReadLine() | ConvertFrom-Json) }
function Find($type){ $r = Send @{ cmd='find_methods_using_type'; params=@{ type=$type } }; if ($r.ok) { return @($r.result) } else { return @() } }

# ---- Phase A: Selected* classes (clears the accessor noise) -----------------
Write-Output "== Phase A: delete Selected* classes =="
foreach ($c in @('SelectedSequenceDiagramShape','SelectedClassDiagramShape')) {
    $r = Send @{ cmd='delete_class'; params=@{ class=$c } }
    Write-Output ("  delete_class {0,-30} ok={1} {2}" -f $c, $r.ok, $r.error)
}

# ---- Phase B: real shape methods by id --------------------------------------
Write-Output "`n== Phase B: delete MFC shape methods (by id) =="
$skipClasses = @('SequenceDiagramView','CClassDiagramView')
$accessorRe  = '^(Add|Remove|Move|GetNext|GetPrev|Replace).*(SequenceDiagramView|CClassDiagramView)'
$delSet = @{}    # id -> @{class;method}
$skipAcc = @()
foreach ($type in @('SequenceDiagramView','CClassDiagramView','CDC')) {
    foreach ($h in (Find $type)) {
        if ($skipClasses -contains $h.class) { continue }       # cascades with delete_class view
        if ($h.method -match $accessorRe)    { $skipAcc += "$($h.class).$($h.method)"; continue }
        if (-not $delSet.ContainsKey([int]$h.id)) { $delSet[[int]$h.id] = @{ class=$h.class; method=$h.method } }
    }
}
Write-Output ("  {0} distinct methods to delete; {1} relation-accessor hits skipped (cascade)" -f $delSet.Count, ($skipAcc | Sort-Object -Unique).Count)
$fail = 0
foreach ($id in ($delSet.Keys | Sort-Object)) {
    $m = $delSet[$id]
    $r = Send @{ cmd='delete_method'; params=@{ class=$m.class; id=$id } }
    if ($r.ok) { Write-Output ("  del {0,-26} {1,-22} id={2}" -f $m.class, $m.method, $id) }
    else       { Write-Output ("  FAIL {0,-24} {1,-22} id={2} -> {3}" -f $m.class, $m.method, $id, $r.error); $fail++ }
}
Write-Output ("  Phase B done; failures=$fail")

# ---- Phase C: the view classes (cascades their relations + own methods) ------
Write-Output "`n== Phase C: delete view classes =="
foreach ($c in @('SequenceDiagramView','CClassDiagramView')) {
    $r = Send @{ cmd='delete_class'; params=@{ class=$c } }
    Write-Output ("  delete_class {0,-22} ok={1} {2}" -f $c, $r.ok, $r.error)
}

# ---- Phase D: verification gate ---------------------------------------------
Write-Output "`n== Phase D: verify =="
Write-Output "  forbidden types (expect 0 / type-not-found):"
foreach ($t in @('SequenceDiagramView','CClassDiagramView','SelectedSequenceDiagramShape','SelectedClassDiagramShape','CDC')) {
    $n = (Find $t).Count
    Write-Output ("    {0,-30} {1}" -f $t, $n)
}
Write-Output "  ViewModel overloads (expect 33 SD / 37 CD -- MUST be unchanged):"
foreach ($t in @('SequenceDiagramViewModel','ClassDiagramViewModel')) {
    $rows = @(Find $t) | Sort-Object -Property id -Unique
    Write-Output ("    {0,-26} {1} distinct" -f $t, $rows.Count)
}
$pipe.Close()
Write-Output "`nDONE (no write_source yet)"
