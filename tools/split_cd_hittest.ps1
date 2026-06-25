# CD additive split: add GetHitShape/PointInShape overloads parameterised by
# ClassDiagramViewModel* alongside the existing (CClassDiagramView*, ...) ones, so
# the Qt CD canvas stops sharing the view-param hit-test path. The MFC versions are
# left intact -> build stays green (both frameworks live). Bodies mirror the view
# versions: view passed down -> VM passed down; the &&pView-guarded CClientDC text-
# measure branch (Relation/Dependency/RelationDiagramOnly) is dropped (already
# skipped headless, matching current Qt behaviour). NO write_source here.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 10)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.params.class).$($obj.params.name)" } }
function B($lines){ return ($lines -join "`r`n") }
$vmArgsPIS = @(@{name='pClassDiagramViewModel'; type='ClassDiagramViewModel*'}, @{name='pointLP'; type='CbPoint'})
$vmArgsGHS = @(@{name='pClassDiagramViewModel'; type='ClassDiagramViewModel*'}, @{name='pointLP'; type='CbPoint'}, @{name='nested'; type='BOOL'})
function AddPIS($cls,$body){ Send @{ cmd='add_method'; params=@{ class=$cls; name='PointInShape'; return_type='BOOL'; virtual=$true; access='public'; args=$vmArgsPIS; body=$body } } }
function AddGHS($cls,$body,$virtual){ Send @{ cmd='add_method'; params=@{ class=$cls; name='GetHitShape'; return_type='ClassDiagramShape*'; virtual=$virtual; access='public'; args=$vmArgsGHS; body=$body } } }

AddPIS 'ClassDiagramShape' (B @('    return Shape::PointInShape(pointLP);'))

AddPIS 'ConnectionShape' (B @(
'    if (!GetHidden())',
'    {',
'        ConnectionSegmentIterator iConnectionSegment(this);',
'        while (++iConnectionSegment)',
'        {',
'            if (iConnectionSegment->PointInShape(pointLP))',
'            {',
'                return TRUE;',
'            }',
'        }',
'    }',
'',
'    return FALSE;'))

AddPIS 'DependencyShape'          (B @('    return ConnectionShape::PointInShape(pClassDiagramViewModel, pointLP);'))
AddPIS 'RelationShape'            (B @('    return ConnectionShape::PointInShape(pClassDiagramViewModel, pointLP);'))
AddPIS 'RelationDiagramOnlyShape' (B @('    return ConnectionShape::PointInShape(pClassDiagramViewModel, pointLP);'))

AddPIS 'NoteShape' (B @(
'    BOOL result = ClassDiagramShape::PointInShape(pClassDiagramViewModel, pointLP);',
'',
'    NoteShapePointIterator iSDNoteShapePoint(this);',
'    while (result == FALSE && ++iSDNoteShapePoint)',
'    {',
'        result = iSDNoteShapePoint->PointInShape(pointLP);',
'    }',
'',
'    return result;'))

AddGHS 'ClassDiagramShape' (B @('    return this;')) $true

AddGHS 'ClassDiagram' (B @(
'    ClassDiagramShape* pClassDiagramShape = 0;',
'',
'    ClassDiagramShapeIterator iClassDiagramShape(this, &ClassDiagramShape::DrawDirect);',
'    while (!pClassDiagramShape && --iClassDiagramShape)',
'    {',
'        if (iClassDiagramShape->PointInShape(pClassDiagramViewModel, pointLP))',
'        {',
'            pClassDiagramShape = iClassDiagramShape->GetHitShape(pClassDiagramViewModel, pointLP, nested);',
'        }',
'    }',
'',
'    return pClassDiagramShape;')) $false

AddGHS 'ClassShape' (B @(
'    ClassDiagramShape* pClassDiagramShape = 0;',
'',
'    if (nested)',
'    {',
'        MemberShapeIterator iMemberShape(this);',
'        while (!pClassDiagramShape && ++iMemberShape)',
'        {',
'            if (iMemberShape->PointInShape(pClassDiagramViewModel, pointLP))',
'            {',
'                pClassDiagramShape = iMemberShape;',
'            }',
'        }',
'',
'        MethodShapeIterator iMethodShape(this);',
'        while (!pClassDiagramShape && ++iMethodShape)',
'        {',
'            if (iMethodShape->PointInShape(pClassDiagramViewModel, pointLP))',
'            {',
'                pClassDiagramShape = iMethodShape;',
'            }',
'        }',
'    }',
'',
'    if (!pClassDiagramShape)',
'    {',
'        pClassDiagramShape = this;',
'    }',
'',
'    return pClassDiagramShape;')) $true

$pipe.Close()
Write-Output "DONE-CD"
