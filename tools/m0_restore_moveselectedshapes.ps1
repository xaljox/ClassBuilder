# Restore ClassDiagram::MoveSelectedShapes as a real MODEL method.
# It had only ever existed as a hand-edit in the generated source (never in any
# .cbz), so write_source wiped it. Re-add it via the pipe so it lives in the
# model and round-trips. Body = the MFC drag-move-selected logic
# (CClassDiagramView::OnMouseMove) with the selection iterators swapped to the
# Qt view's ViewModel selection (ClassDiagramViewModel::SelectedIterator +
# ClassDiagramShape::IsSelectedIn), exactly the adaptation the user described.
# The note-point bounding rect (the MFC code took it from the drag tracker) is
# computed here from the selected shapes' pre-move rects.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.name)" } }

Send @{ cmd='add_method'; params=@{
    class       = 'ClassDiagram'
    name        = 'MoveSelectedShapes'
    return_type = 'void'
    access      = 'public'
    args        = @(
        @{ name='pClassDiagramViewModel'; type='ClassDiagramViewModel*' },
        @{ name='offset'; type='CbSize' }
    )
} }

$body = @(
'    CbRect rect;',
'    BOOL any = FALSE;',
'',
'    ClassDiagramViewModel::SelectedIterator iSelected(pClassDiagramViewModel);',
'    while (++iSelected)',
'    {',
'        ClassDiagramShape* pShape = iSelected->GetClassDiagramShape();',
'',
'        // Accumulate the pre-move bounding rect (the MFC code used the drag',
'        // tracking rectangle; here it comes from the selected shapes).',
'        CbRect shapeRect = pShape->GetRect();',
'        if (!any)',
'        {',
'            rect = shapeRect;',
'            any = TRUE;',
'        }',
'        else',
'        {',
'            if (shapeRect.left < rect.left)     rect.left = shapeRect.left;',
'            if (shapeRect.top < rect.top)       rect.top = shapeRect.top;',
'            if (shapeRect.right > rect.right)   rect.right = shapeRect.right;',
'            if (shapeRect.bottom > rect.bottom) rect.bottom = shapeRect.bottom;',
'        }',
'',
'        NoteShape* pNoteShape = dynamic_cast<NoteShape*>(pShape);',
'        if (pNoteShape)',
'        {',
'            pNoteShape->SetRect(pNoteShape->GetRect() + offset);',
'        }',
'',
'        ClassShape* pClassShape = dynamic_cast<ClassShape*>(pShape);',
'        if (pClassShape)',
'        {',
'            pClassShape->SaveState();',
'            pClassShape->Shape::SetRect(pClassShape->GetRect() + offset);',
'',
'            ClassShape::FromConnectionShapeIterator iFromConnectionShape(pClassShape);',
'            while (++iFromConnectionShape)',
'            {',
'                if (iFromConnectionShape->GetToClassShape()->IsSelectedIn(pClassDiagramViewModel))',
'                {',
'                    iFromConnectionShape->SetStartPoint(iFromConnectionShape->GetStartPoint() + offset);',
'                }',
'                else',
'                {',
'                    if (iFromConnectionShape->GetInitial())',
'                    {',
'                        ClassShape* pToClassShape = iFromConnectionShape->GetToClassShape();',
'                        iFromConnectionShape->SetStartPoint(pClassShape->ConnectionPoint(pToClassShape));',
'                        iFromConnectionShape->SetEndPoint(pToClassShape->ConnectionPoint(pClassShape));',
'                        iFromConnectionShape->MakeNewRouting();',
'                    }',
'                    else',
'                    {',
'                        iFromConnectionShape->UpdateStartPoint(iFromConnectionShape->GetStartPoint() + offset);',
'                    }',
'                }',
'            }',
'',
'            ClassShape::ToConnectionShapeIterator iToConnectionShape(pClassShape);',
'            while (++iToConnectionShape)',
'            {',
'                if (iToConnectionShape->GetFromClassShape()->IsSelectedIn(pClassDiagramViewModel))',
'                {',
'                    iToConnectionShape->SetEndPoint(iToConnectionShape->GetEndPoint() + offset);',
'                }',
'                else',
'                {',
'                    if (iToConnectionShape->GetInitial())',
'                    {',
'                        ClassShape* pFromClassShape = iToConnectionShape->GetFromClassShape();',
'                        iToConnectionShape->SetStartPoint(pFromClassShape->ConnectionPoint(pClassShape));',
'                        iToConnectionShape->SetEndPoint(pClassShape->ConnectionPoint(pFromClassShape));',
'                        iToConnectionShape->MakeNewRouting();',
'                    }',
'                    else',
'                    {',
'                        iToConnectionShape->UpdateEndPoint(iToConnectionShape->GetEndPoint() + offset);',
'                    }',
'                }',
'            }',
'        }',
'    }',
'',
'    if (any)',
'    {',
'        rect.InflateRect(10, 10, 11, 11);',
'        MoveNoteShapePoints(rect, offset);',
'    }'
) -join "`r`n"

Send @{ cmd='set_method_body'; params=@{ class='ClassDiagram'; name='MoveSelectedShapes'; body=$body } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
