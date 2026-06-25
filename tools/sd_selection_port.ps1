# sd_selection_port.ps1
# One-off: add the Qt in-Draw per-view selection overloads to the SequenceDiagram
# shapes, mirroring the ClassDiagram work. Pipe-driven model edits + write_source.
#
#   - SequenceDiagramShape::IsSelectedIn(vm)                 (int, Find-wrapper)
#   - SequenceDiagramShape::Draw(painter, vm, BOOL) = 0      (pure virtual base)
#   - LifeLineShape::Draw(painter, vm, BOOL) = 0             (re-expose; hides base otherwise)
#   - {Actor,Class}LifeLineShape / ChildActivationShape /
#     RootActivationShape / SignalShape / SDNoteShape :: Draw(painter, vm, BOOL)
#   - SequenceDiagram::Draw(painter, vm)                     (orchestrator)
#
# Then write_source. Build is done separately.

$ErrorActionPreference = 'Stop'

# ---- argument lists (hand-written JSON so single-element arrays stay arrays) ----
$ARGS_ISSEL = '[{"name":"pSequenceDiagramViewModel","type":"SequenceDiagramViewModel*"}]'
$ARGS_DRAW3 = '[{"name":"painter","type":"CbPainter&"},{"name":"pSequenceDiagramViewModel","type":"SequenceDiagramViewModel*"},{"name":"selected","type":"BOOL"}]'
$ARGS_DRAW2 = '[{"name":"painter","type":"CbPainter&"},{"name":"pSequenceDiagramViewModel","type":"SequenceDiagramViewModel*"}]'

# ---- bodies (single-quoted here-strings: no apostrophes/backticks inside) ----

$BODY_ISSEL = @'
    if (!pSequenceDiagramViewModel)
        return 0;
    if (FindSequenceDiagramViewModelSelection(pSequenceDiagramViewModel))
    {
        return 1;
    }
    return 0;
'@

$BODY_ORCH = @'
    struct DrawGuard
    {
        DataModelDoc* pDoc;
        DrawGuard(DataModelDoc* p) : pDoc(p) { if (pDoc) pDoc->BeginDraw(); }
        ~DrawGuard() { if (pDoc) pDoc->EndDraw(); }
    } drawGuard(GetDataModelDoc());

    RecalculateRects();

    LifeLineShapeIterator iLifeLineShape(this);
    while (++iLifeLineShape)
    {
        iLifeLineShape->Draw(painter, pSequenceDiagramViewModel,
            iLifeLineShape->IsSelectedIn(pSequenceDiagramViewModel));
    }

    ParentActivationShape::ChildActivationShapeIterator
        iChildActivationShape(GetRootActivationShape());
    while (++iChildActivationShape)
    {
        iChildActivationShape->Draw(painter, pSequenceDiagramViewModel,
            iChildActivationShape->IsSelectedIn(pSequenceDiagramViewModel));
    }

    SequenceDiagramShapeIterator iSequenceDiagramShape(this, &SequenceDiagramShape::IsSDNoteShape);
    while (++iSequenceDiagramShape)
    {
        iSequenceDiagramShape->Draw(painter, pSequenceDiagramViewModel,
            iSequenceDiagramShape->IsSelectedIn(pSequenceDiagramViewModel));
    }
'@

$BODY_ACTOR = @'
    CbRect rect = GetRect();
    rect.right = rect.left + RecalculateRectWidth();
    SetRect(rect);

    int save = painter.Save();
    painter.SetNullBrush();
    painter.FillSolidRect(_rect, painter.GetBkColor());
    painter.SetFont(SequenceDiagram::GetLifeLineFont());

    painter.SetTextAlign(TA_CENTER|TA_TOP|TA_NOUPDATECP);
    COLORREF oldBkColor = painter.GetBkColor();
    UINT options = ETO_CLIPPED;

    painter.SetTextColor(GetTextColor());
    if (selected)
    {
        painter.SetBkColor(CbPainter::GetSelectFillColor());
        options = ETO_CLIPPED | ETO_OPAQUE;
    }

    // Limit the coloured area.
    CbRect clipRect = GetRect();
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 2,
        options, clipRect, GetName() + " : ");
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 36,
        ETO_CLIPPED, clipRect, GetTypeName());

    painter.SetBkColor(oldBkColor);
    painter.SetTextColor(GetTextColor());

    COLORREF color = GetPenColor();
    int penWidth = 1;
    if (selected)
    {
        color = CbPainter::GetSelectColor();
        penWidth = 2;
    }
    painter.SetPen(PS_SOLID, penWidth, color);

    const int size = 5;
    CbPoint refPoint = _rect.CenterPoint();
    Shape::Round(refPoint);
    refPoint.y = _rect.top + 150;
    CbRect head(refPoint, refPoint);
    head.InflateRect(3*size, 3*size);
    painter.Ellipse(head);
    painter.DrawLine(refPoint+CbSize(0, -3*size),   refPoint+CbSize(0, -10*size));
    painter.DrawLine(refPoint+CbSize(-4*size, -5*size), refPoint+CbSize(4*size, -5*size));
    painter.DrawLine(refPoint+CbSize(0, -10*size),  refPoint+CbSize(-5*size, -15*size));
    painter.DrawLine(refPoint+CbSize(0, -10*size),  refPoint+CbSize(5*size, -15*size));

    painter.SetPen(PS_DOT, 1, color);
    painter.DrawLine(GetStartPoint(), GetEndPoint());

    painter.Restore(save);
'@

$BODY_CLASSLL = @'
    CbRect rect = GetRect();
    rect.right = rect.left + RecalculateRectWidth();

    if (GetFirstChildActivationShape() &&
        GetFirstChildActivationShape()->GetCreation())
    {
        rect.top = GetFirstChildActivationShape()->GetRect().bottom;
        rect.bottom = rect.top + SequenceDiagram::GetClassLifeLineHeight();
    }
    else
    {
        rect.top = -SequenceDiagram::GetClassLifeLineOffset();
        rect.bottom = rect.top + SequenceDiagram::GetClassLifeLineHeight();
    }
    SetRect(rect);

    int save = painter.Save();
    painter.SetNullBrush();
    painter.FillSolidRect(_rect, painter.GetBkColor());
    painter.SetFont(SequenceDiagram::GetLifeLineFont());

    painter.SetTextAlign(TA_CENTER|TA_TOP|TA_NOUPDATECP);
    COLORREF oldBkColor = painter.GetBkColor();
    UINT options = ETO_CLIPPED;

    painter.SetTextColor(GetTextColor());
    if (selected)
    {
        painter.SetBkColor(CbPainter::GetSelectFillColor());
        options = ETO_CLIPPED | ETO_OPAQUE;
    }

    // Limit the coloured area.
    CbRect clipRect = GetRect();
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 2,
        options, clipRect, GetName() + " : ");
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 36,
        ETO_CLIPPED, clipRect, GetTypeName());

    painter.SetBkColor(oldBkColor);

    painter.SetTextColor(GetTextColor());
    COLORREF penColor = GetPenColor(painter);
    painter.SetPen(PS_SOLID, 1, penColor);

    painter.DrawRect(_rect);

    CbPoint point(GetRect().right, GetRect().bottom);
    CbRect templateRect(point, point);
    if (!GetTemplate().IsEmpty())
    {
        templateRect = GetTemplateRect(painter, TRUE);
    }
    SetTemplateRect(templateRect);

    if (selected)
    {
        penColor = CbPainter::GetSelectColor();
        DrawSelectedRect(painter, penColor);
    }

    painter.SetPen(PS_DOT, 1, penColor);
    painter.DrawLine(GetStartPoint(), GetEndPoint());

    painter.Restore(save);
'@

$BODY_CHILDACT = @'
    int save = painter.Save();
    painter.SetNullBrush();
    painter.SetPen(PS_SOLID, 1, GetPenColor(painter));

    CbRect rect = GetMergeRect();

    if (selected)
    {
        painter.FillSolidRect(rect, painter.GetBkColor());
        painter.FillSolidRect(_rect, CbPainter::GetSelectFillColor());
        int saveSel = painter.Save();
        painter.SetPen(PS_SOLID, 2, CbPainter::GetSelectColor());
        painter.DrawRect(rect);
        painter.Restore(saveSel);
    }
    else if (!GetMerge() && GetLifeLineShape()->GetShowActivations())
    {
        painter.FillSolidRect(rect, painter.GetBkColor());
        painter.DrawRect(rect);
    }

    if (GetDestruction())
    {
        COLORREF color = GetPenColor(painter);
        if (GetLifeLineShape()->GetDestructionChildActivationShape() != this)
        {
            color = RGB(255, 0, 0);
        }
        int saveDestruct = painter.Save();
        painter.SetPen(PS_SOLID, 3, color);

        CbPoint crossPoint = _rect.CenterPoint();
        crossPoint.y = _rect.top;
        const int size = 20;
        CbSize offset1(size, size);
        CbSize offset2(size, -size);
        painter.DrawLine(crossPoint+offset1, crossPoint-offset1);
        painter.DrawLine(crossPoint+offset2, crossPoint-offset2);
        painter.Restore(saveDestruct);
    }

    painter.Restore(save);

    if (GetChildActivation() && GetChildActivation()->GetSender())
    {
        SignalShape* pSignalShape = GetChildActivation()->GetSender();
        pSignalShape->Draw(painter, pSequenceDiagramViewModel,
            pSignalShape->IsSelectedIn(pSequenceDiagramViewModel));
    }

    ChildActivationShapeIterator iChildActivationShape(this);
    while (++iChildActivationShape)
    {
        iChildActivationShape->Draw(painter, pSequenceDiagramViewModel,
            iChildActivationShape->IsSelectedIn(pSequenceDiagramViewModel));
    }
'@

$BODY_SIGNAL = @'
    int save = painter.Save();
    painter.SetNullBrush();

    COLORREF penColor = GetPenColor(painter);
    int penWidth = 1;
    painter.SetTextColor(GetTextColor());
    if (selected)
    {
        painter.SetBkColor(CbPainter::GetSelectFillColor());
        penColor = CbPainter::GetSelectColor();
        penWidth = 2;
    }

    CbRect rect(0, 0, 0, 0);

    if (!GetLabel().IsEmpty())
    {
        rect *= GetLabelRect(painter, TRUE);
    }

    if (GetEnableReturn() && !GetReturn().IsEmpty())
    {
        rect *= GetReturnRect(painter, TRUE);
    }

    rect *= GetNameRect(painter, TRUE);

    painter.SetPen(PS_SOLID, penWidth, penColor);
    CbPoint startPoint = GetStartPoint();

    CbSize inflate(0, 10);
    if (IsRecursiveActivation())
    {
        CbPoint point1 = GetStartPoint() + CbSize(SequenceDiagram::GetSignalLengthRecursive(), 0) + CbSize(0, -_duration/2);
        CbPoint point2 = point1 + CbSize(0, -SequenceDiagram::GetActivationSpaceRecursive());
        painter.DrawLine(GetStartPoint(), point1);
        painter.DrawLine(point1, point2);
        painter.DrawLine(point2, GetEndPoint());
        startPoint = point2;

        CbRect activeAreaRect(GetStartPoint()+inflate, point2-inflate);
        activeAreaRect.NormalizeRect();
        SetActiveAreaRect(activeAreaRect);
        rect *= activeAreaRect;
    }
    else
    {
        painter.DrawLine(GetStartPoint(), GetEndPoint());

        CbRect activeAreaRect(GetStartPoint()+inflate, GetEndPoint()-inflate);
        activeAreaRect.NormalizeRect();
        SetActiveAreaRect(activeAreaRect);
        rect *= activeAreaRect;
    }
    DrawArrow(painter, penColor, startPoint, GetEndPoint());

    if (GetEnableReturn())
    {
        int saveRet = painter.Save();
        painter.SetPen(PS_DASH, penWidth, penColor);
        painter.DrawLine(GetReturnStartPoint(), GetReturnEndPoint());

        CbRect returnActiveAreaRect(GetReturnStartPoint()+inflate, GetReturnEndPoint()-inflate);
        returnActiveAreaRect.NormalizeRect();
        SetReturnActiveAreaRect(returnActiveAreaRect);
        rect *= returnActiveAreaRect;

        DrawArrow(painter, penColor, GetReturnStartPoint(), GetReturnEndPoint(), TRUE);
        painter.Restore(saveRet);
    }

    SetRect(rect);

    painter.Restore(save);
'@

$BODY_SDNOTE = @'
    const int size = GetFontHeight() + 2;

    CbRect rect = GetRect();
    rect.top = rect.bottom;
    int y = rect.bottom-2;
    int x = (rect.left + rect.right)/2;

    int save = painter.Save();

    CFont font;
    font.CreateFont(GetFontHeight(), 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH|FF_SWISS, "Arial");
    painter.SetFont(&font);

    painter.SetTextAlign(TA_CENTER|TA_TOP|TA_NOUPDATECP);
    painter.SetTextColor(GetTextColor());
    if (selected)
    {
        painter.SetBkColor(CbPainter::GetSelectFillColor());
    }

    CbString remaining = GetNote();
    int width = rect.Width() - size * 2;
    const int yStart = y;
    BOOL moreSegments = TRUE;
    while (moreSegments)
    {
        int nlIndex = remaining.Find(NL);
        CbString note;
        if (nlIndex == -1)
        {
            note = remaining;
            moreSegments = FALSE;
        }
        else
        {
            note = remaining.Left(nlIndex);
            remaining = remaining.Mid(nlIndex + 2);
        }

        int index = note.Find("  ");
        while (index != -1)
        {
            note = note.Left(index) + note.Mid(index+1);
            index = note.Find("  ");
        }

        note.TrimLeft();
        note.TrimRight();
        const BOOL segmentWasEmpty = note.IsEmpty();
        if (painter.GetTextExtent(note).cx > width)
        {
            CbString line;
            CbString lineOk;

            index = note.FindOneOf(" \t");
            while (index != -1)
            {
                line += note.Left(index+1);

                if (painter.GetTextExtent(line).cx <= width || lineOk.IsEmpty())
                {
                    lineOk = line;
                    note = note.Mid(index+1);
                }
                else
                {
                    CbRect clipRect(rect.left, y-size, rect.right, y);
                    note.TrimLeft();
                    lineOk.TrimRight();
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, lineOk);
                    y -= size;
                    line.Empty();
                    lineOk.Empty();
                    width = rect.Width() - GetFontHeight()/2;
                }

                index = note.FindOneOf(" \t");
            }

            if (!lineOk.IsEmpty())
            {
                CbString rest = lineOk + note;
                if (painter.GetTextExtent(rest).cx <= width)
                {
                    CbRect clipRect(rect.left, y-size, rect.right, y);
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, rest);
                    y -= size;
                    note.Empty();
                }
                else
                {
                    CbRect clipRect(rect.left, y-size, rect.right, y);
                    note.TrimLeft();
                    lineOk.TrimRight();
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, lineOk);
                    y -= size;
                    width = rect.Width() - GetFontHeight()/2;
                }
            }
        }

        if (!note.IsEmpty())
        {
            CbRect clipRect(rect.left, y-size, rect.right, y);
            painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, note);
            y -= size;
        }
        else if (segmentWasEmpty && y != yStart)
        {
            CbRect clipRect(rect.left, y-size, rect.right, y);
            painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, CbString(""));
            y -= size;
        }
    }
    rect.top = y;
    if (rect.Height() < size * 2)
    {
        rect.top -= size * 2 - rect.Height();
    }
    SetRect(rect);

    painter.SetPen(PS_SOLID, 1, GetPenColor());
    painter.DrawLine({rect.left,       rect.top},         {rect.right,        rect.top});
    painter.DrawLine({rect.right,      rect.top},         {rect.right,        rect.bottom-size});
    painter.DrawLine({rect.right,      rect.bottom-size}, {rect.right-size,   rect.bottom});
    painter.DrawLine({rect.right-size, rect.bottom},      {rect.right-size,   rect.bottom-size});
    painter.DrawLine({rect.right-size, rect.bottom-size}, {rect.right,        rect.bottom-size});
    painter.DrawLine({rect.right-size, rect.bottom},      {rect.left,         rect.bottom});
    painter.DrawLine({rect.left,       rect.bottom},      {rect.left,         rect.top});

    painter.SetPen(PS_DASH, 1, GetPenColor());
    SDNoteShapePointIterator iSDNoteShapePoint(this);
    while (++iSDNoteShapePoint)
    {
        if (!iSDNoteShapePoint.IsLast())
        {
            painter.DrawLine(
                Shape::CrossPoint(GetRect(), iSDNoteShapePoint->GetPoint()),
                iSDNoteShapePoint->GetPoint());
        }
    }

    painter.Restore(save);

    if (selected)
    {
        DrawSelectedRect(painter, CbPainter::GetSelectColor());
    }
'@

# ---- request builder ----
function New-AddMethod {
    param(
        [string]$Class, [string]$Name, [string]$ReturnType, [string]$ArgsJson,
        [string]$Access = 'public', [bool]$Virtual = $false, [bool]$Pure = $false,
        $Body = $null
    )
    $v = if ($Virtual) { 'true' } else { 'false' }
    $p = if ($Pure)    { 'true' } else { 'false' }
    $j  = '{"cmd":"add_method","params":{'
    $j += '"class":"'       + $Class      + '",'
    $j += '"name":"'        + $Name       + '",'
    $j += '"return_type":"' + $ReturnType + '",'
    $j += '"access":"'      + $Access     + '",'
    $j += '"virtual":'      + $v          + ','
    $j += '"static":false,"const":false,'
    $j += '"pure":'         + $p          + ','
    $j += '"args":'         + $ArgsJson
    if ($null -ne $Body) {
        $crlf = ($Body -replace "`r`n", "`n") -replace "`n", "`r`n"
        $bj   = $crlf | ConvertTo-Json          # produces a JSON-escaped quoted string
        $j   += ',"body":' + $bj
    }
    $j += '}}'
    return $j
}

# ---- the request list, in order ----
$requests = @()
$requests += New-AddMethod -Class 'SequenceDiagramShape' -Name 'IsSelectedIn' -ReturnType 'int'  -ArgsJson $ARGS_ISSEL -Body $BODY_ISSEL
$requests += New-AddMethod -Class 'SequenceDiagramShape' -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Pure $true
$requests += New-AddMethod -Class 'LifeLineShape'        -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Pure $true
$requests += New-AddMethod -Class 'ActorLifeLineShape'   -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Body $BODY_ACTOR
$requests += New-AddMethod -Class 'ClassLifeLineShape'   -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Body $BODY_CLASSLL
$requests += New-AddMethod -Class 'ChildActivationShape' -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Body $BODY_CHILDACT
$requests += New-AddMethod -Class 'RootActivationShape'  -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Body "`r`n"
$requests += New-AddMethod -Class 'SignalShape'          -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Body $BODY_SIGNAL
$requests += New-AddMethod -Class 'SDNoteShape'          -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW3 -Virtual $true -Body $BODY_SDNOTE
$requests += New-AddMethod -Class 'SequenceDiagram'      -Name 'Draw' -ReturnType 'void' -ArgsJson $ARGS_DRAW2 -Body $BODY_ORCH
$requests += '{"cmd":"write_source"}'

# ---- send them all on one connection ----
$p = New-Object System.IO.Pipes.NamedPipeClientStream(".", "ClassBuilder", [System.IO.Pipes.PipeDirection]::InOut)
$p.Connect(5000)
$sw = New-Object System.IO.StreamWriter($p); $sw.AutoFlush = $true
$sr = New-Object System.IO.StreamReader($p)

$i = 0
foreach ($req in $requests) {
    $i++
    $sw.WriteLine($req)
    $reply = $sr.ReadLine()
    $tag = if ($reply -match '"ok"\s*:\s*true') { 'OK ' } else { 'ERR' }
    Write-Output ("[{0}] {1}  {2}" -f $i, $tag, $reply.Substring(0, [Math]::Min(160, $reply.Length)))
}
$p.Close()
