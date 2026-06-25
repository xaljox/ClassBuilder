# SD additive split: GetHitShape/PointInShape overloads parameterised by
# SequenceDiagramViewModel* alongside the (SequenceDiagramView*, ...) ones, so the
# Qt SD canvas stops sharing the view-param hit-test path. MFC versions kept ->
# build stays green. SignalShape KEEPS its headless GetMeasurePainter text-hit
# (SD signal-text hit-test is implemented), only dropping the CClientDC(view)
# branch. NO write_source here.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 10)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.params.class).$($obj.params.name)" } }
function B($lines){ return ($lines -join "`r`n") }
$pisArgs = @(@{name='pSequenceDiagramViewModel'; type='SequenceDiagramViewModel*'}, @{name='pointLP'; type='CbPoint'})
$ghsArgs = @(@{name='pSequenceDiagramViewModel'; type='SequenceDiagramViewModel*'}, @{name='pointLP'; type='CbPoint'}, @{name='nested'; type='BOOL'})
function AddPIS($cls,$body){ Send @{ cmd='add_method'; params=@{ class=$cls; name='PointInShape'; return_type='BOOL'; virtual=$true; access='public'; args=$pisArgs; body=$body } } }
function AddGHS($cls,$body,$virtual){ Send @{ cmd='add_method'; params=@{ class=$cls; name='GetHitShape'; return_type='SequenceDiagramShape*'; virtual=$virtual; access='public'; args=$ghsArgs; body=$body } } }

AddPIS 'SequenceDiagramShape' (B @('    return Shape::PointInShape(pointLP);'))
AddPIS 'ChildActivationShape' (B @('    return SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP);'))
AddPIS 'ClassLifeLineShape' (B @(
'    if (SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP))',
'    {',
'        return TRUE;',
'    }',
'',
'    return GetLifeLineRect().PtInRect(pointLP);'))
AddPIS 'ActorLifeLineShape' (B @(
'    if (SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP))',
'    {',
'        return TRUE;',
'    }',
'',
'    return GetActorRect().PtInRect(pointLP) || GetLifeLineRect().PtInRect(pointLP);'))
AddPIS 'SDNoteShape' (B @(
'    BOOL result = SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP);',
'',
'    SDNoteShapePointIterator iSDNoteShapePoint(this);',
'    while (result == FALSE && ++iSDNoteShapePoint)',
'    {',
'        result = iSDNoteShapePoint->PointInShape(pointLP);',
'    }',
'',
'    return result;'))
AddPIS 'SignalShape' (B @(
'    if (_activeAreaRect.PtInRect(pointLP))',
'    {',
'        return TRUE;',
'    }',
'',
'    if (GetEnableReturn() && _returnActiveAreaRect.PtInRect(pointLP))',
'    {',
'        return TRUE;',
'    }',
'',
'    auto hitText = [&](CbPainter& painter) -> BOOL',
'    {',
'        if (GetNameRect(painter).PtInRect(pointLP))',
'            return TRUE;',
'        if (!GetLabel().IsEmpty() && GetLabelRect(painter).PtInRect(pointLP))',
'            return TRUE;',
'        if (GetEnableReturn() && !GetReturn().IsEmpty() &&',
'            GetReturnRect(painter).PtInRect(pointLP))',
'            return TRUE;',
'        return FALSE;',
'    };',
'',
'    if (CbPainter* pMeasure = GetMeasurePainter())',
'        return hitText(*pMeasure);',
'    return FALSE;'))

AddGHS 'SequenceDiagramShape' (B @('    return this;')) $true
AddGHS 'SequenceDiagram' (B @(
'    SequenceDiagramShape* pSequenceDiagramShape = 0;',
'',
'    SequenceDiagramShapeIterator iSequenceDiagramShape(this);',
'    while (!pSequenceDiagramShape && --iSequenceDiagramShape)',
'    {',
'        if (iSequenceDiagramShape->PointInShape(pSequenceDiagramViewModel, pointLP))',
'        {',
'            pSequenceDiagramShape = iSequenceDiagramShape->GetHitShape(pSequenceDiagramViewModel, pointLP, nested);',
'        }',
'    }',
'',
'    return pSequenceDiagramShape;')) $false

$pipe.Close()
Write-Output "DONE-SD"
