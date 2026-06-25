# _isDrawing retirement -- note step. Moves the note HEIGHT recompute OUT of paint.
#
# Adds to the self-host model via the pipe:
#   Shape::NoteCalcRect (protected static) -- wrap -> rect.top  (MEASURE, no paint)
#   Shape::NoteDraw     (protected static) -- wrap + text + box/corner (PAINT, read-only, NO SetRect)
#   NoteShape::RecalcHeight / SDNoteShape::RecalcHeight (public) -- edit-boundary entry (SetRect)
# and thins NoteShape::Draw / SDNoteShape::Draw to call NoteDraw + their own attach line.
#
# Notes are identical (text loop + box/corner) bar the select fill (unified to the CD
# form) and the attach-line iterator (note-specific, stays in each Draw).
# KEEP THE WRAP IN SYNC between NoteCalcRect and NoteDraw.
#
# Does NOT write_source -- run that separately after a read-back check.

$ErrorActionPreference = 'Stop'

$pipe = [System.IO.Pipes.NamedPipeClientStream]::new('.', 'ClassBuilder', [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(2000)
$w = [System.IO.StreamWriter]::new($pipe); $w.AutoFlush = $true
$r = [System.IO.StreamReader]::new($pipe)

function Invoke-Cb([string]$cmd, [hashtable]$params = @{}) {
    $j = @{ cmd = $cmd; params = $params } | ConvertTo-Json -Compress -Depth 20
    $w.WriteLine($j)
    $resp = $r.ReadLine() | ConvertFrom-Json
    if (-not $resp.ok) { Write-Host "  FAILED ($cmd): $($resp.error)" }
    else               { Write-Host "  ok ($cmd)" }
    return $resp
}

function To-Crlf([string]$s) { return ($s -replace "`r?`n", "`r`n") }

# ---------------------------------------------------------------- Shape::NoteCalcRect
$noteCalcBody = To-Crlf @'
    const int size = fontHeight + 2;
    CbRect result = rect;
    result.top = result.bottom;
    int y = result.bottom - 2;
    int width = result.Width() - size * 2;
    const int yStart = y;

    int save = painter.Save();
    painter.SetFontPx(fontHeight);

    CbString remaining = noteText;
    bool moreSegments = true;
    while (moreSegments)
    {
        int nlIndex = remaining.Find(NL);
        CbString note;
        if (nlIndex == -1)
        {
            note = remaining;
            moreSegments = false;
        }
        else
        {
            note = remaining.Left(nlIndex);
            remaining = remaining.Mid(nlIndex + 2);
        }

        int index = note.Find("  ");
        while (index != -1)
        {
            note = note.Left(index) + note.Mid(index + 1);
            index = note.Find("  ");
        }

        note.TrimLeft();
        note.TrimRight();
        const bool segmentWasEmpty = note.IsEmpty();
        if (painter.GetTextExtent(note).cx > width)
        {
            CbString line;
            CbString lineOk;

            index = note.FindOneOf(" \t");
            while (index != -1)
            {
                line += note.Left(index + 1);

                if (painter.GetTextExtent(line).cx <= width || lineOk.IsEmpty())
                {
                    lineOk = line;
                    note = note.Mid(index + 1);
                }
                else
                {
                    note.TrimLeft();
                    y -= size;
                    line.Empty();
                    lineOk.Empty();
                    width = result.Width() - fontHeight / 2;
                }

                index = note.FindOneOf(" \t");
            }

            if (!lineOk.IsEmpty())
            {
                CbString rest = lineOk + note;
                if (painter.GetTextExtent(rest).cx <= width)
                {
                    y -= size;
                    note.Empty();
                }
                else
                {
                    note.TrimLeft();
                    y -= size;
                    width = result.Width() - fontHeight / 2;
                }
            }
        }

        if (!note.IsEmpty())
        {
            y -= size;
        }
        else if (segmentWasEmpty && y != yStart)
        {
            y -= size;
        }
    }

    painter.Restore(save);

    result.top = y;
    if (result.Height() < size * 2)
    {
        result.top -= size * 2 - result.Height();
    }
    return result;
'@

# -------------------------------------------------------------------- Shape::NoteDraw
$noteDrawBody = To-Crlf @'
    const int size = fontHeight + 2;

    const CbRect fillRect = rect;
    rect.top = rect.bottom;
    int y = rect.bottom - 2;
    int x = (rect.left + rect.right) / 2;

    painter.SetFontPx(fontHeight);
    painter.SetTextAlign(TA_CENTER | TA_TOP | TA_NOUPDATECP);
    painter.SetTextColor(textColor);

    if (painter.IsScreen() && selected)
    {
        painter.FillSolidRect(fillRect, CbPainter::GetSelectFillColor());
        painter.SetBkColor(CbPainter::GetSelectFillColor());
    }

    CbString remaining = noteText;
    int width = rect.Width() - size * 2;
    const int yStart = y;
    bool moreSegments = true;
    while (moreSegments)
    {
        int nlIndex = remaining.Find(NL);
        CbString note;
        if (nlIndex == -1)
        {
            note = remaining;
            moreSegments = false;
        }
        else
        {
            note = remaining.Left(nlIndex);
            remaining = remaining.Mid(nlIndex + 2);
        }

        int index = note.Find("  ");
        while (index != -1)
        {
            note = note.Left(index) + note.Mid(index + 1);
            index = note.Find("  ");
        }

        note.TrimLeft();
        note.TrimRight();
        const bool segmentWasEmpty = note.IsEmpty();
        if (painter.GetTextExtent(note).cx > width)
        {
            CbString line;
            CbString lineOk;

            index = note.FindOneOf(" \t");
            while (index != -1)
            {
                line += note.Left(index + 1);

                if (painter.GetTextExtent(line).cx <= width || lineOk.IsEmpty())
                {
                    lineOk = line;
                    note = note.Mid(index + 1);
                }
                else
                {
                    CbRect clipRect(rect.left, y - size, rect.right, y);
                    note.TrimLeft();
                    lineOk.TrimRight();
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, lineOk);
                    y -= size;
                    line.Empty();
                    lineOk.Empty();
                    width = rect.Width() - fontHeight / 2;
                }

                index = note.FindOneOf(" \t");
            }

            if (!lineOk.IsEmpty())
            {
                CbString rest = lineOk + note;
                if (painter.GetTextExtent(rest).cx <= width)
                {
                    CbRect clipRect(rect.left, y - size, rect.right, y);
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, rest);
                    y -= size;
                    note.Empty();
                }
                else
                {
                    CbRect clipRect(rect.left, y - size, rect.right, y);
                    note.TrimLeft();
                    lineOk.TrimRight();
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, lineOk);
                    y -= size;
                    width = rect.Width() - fontHeight / 2;
                }
            }
        }

        if (!note.IsEmpty())
        {
            CbRect clipRect(rect.left, y - size, rect.right, y);
            painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, note);
            y -= size;
        }
        else if (segmentWasEmpty && y != yStart)
        {
            CbRect clipRect(rect.left, y - size, rect.right, y);
            painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, CbString(""));
            y -= size;
        }
    }
    rect.top = y;
    if (rect.Height() < size * 2)
    {
        rect.top -= size * 2 - rect.Height();
    }

    painter.SetPen(PS_SOLID, 1, penColor);
    painter.DrawLine(CbPoint(rect.left, rect.top),                   CbPoint(rect.right, rect.top));
    painter.DrawLine(CbPoint(rect.right, rect.top),                  CbPoint(rect.right, rect.bottom - size));
    painter.DrawLine(CbPoint(rect.right, rect.bottom - size),        CbPoint(rect.right - size, rect.bottom));
    painter.DrawLine(CbPoint(rect.right - size, rect.bottom),        CbPoint(rect.right - size, rect.bottom - size));
    painter.DrawLine(CbPoint(rect.right - size, rect.bottom - size), CbPoint(rect.right, rect.bottom - size));
    painter.DrawLine(CbPoint(rect.right - size, rect.bottom),        CbPoint(rect.left, rect.bottom));
    painter.DrawLine(CbPoint(rect.left, rect.bottom),                CbPoint(rect.left, rect.top));
'@

# ------------------------------------------------- RecalcHeight (CD + SD, same body)
$recalcBody = To-Crlf @'
    SetRect(Shape::NoteCalcRect(painter, GetRect(), GetNote(), GetFontHeight()));
'@

# ------------------------------------------------------------- thin NoteShape::Draw
$noteDrawThin = To-Crlf @'
    int save = painter.Save();
    Shape::NoteDraw(painter, GetRect(), GetNote(), GetFontHeight(), selected, GetPenColor(), GetTextColor());

    painter.SetPen(PS_DASH, 1, GetPenColor());
    NoteShapePointIterator iNoteShapePoint(this);
    while (++iNoteShapePoint)
    {
        if (!iNoteShapePoint.IsLast())
        {
            painter.DrawLine(
                Shape::CrossPoint(GetRect(), iNoteShapePoint->GetPoint()),
                iNoteShapePoint->GetPoint());
        }
    }

    painter.Restore(save);

    if (painter.IsScreen() && selected)
        DrawSelectedRect(painter, CbPainter::GetSelectColor());
'@

# ----------------------------------------------------------- thin SDNoteShape::Draw
$sdNoteDrawThin = To-Crlf @'
    int save = painter.Save();
    Shape::NoteDraw(painter, GetRect(), GetNote(), GetFontHeight(), selected, GetPenColor(), GetTextColor());

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

    if (painter.IsScreen() && selected)
        DrawSelectedRect(painter, CbPainter::GetSelectColor());
'@

Write-Host "add Shape::NoteCalcRect"
Invoke-Cb 'add_method' @{
    class = 'Shape'; name = 'NoteCalcRect'; return_type = 'CbRect';
    static = $true; access = 'protected';
    args = @(
        @{ name = 'painter';    type = 'CbPainter&' },
        @{ name = 'rect';       type = 'CbRect' },
        @{ name = 'noteText';   type = 'CbString' },
        @{ name = 'fontHeight'; type = 'int' }
    )
    body = $noteCalcBody
} | Out-Null

Write-Host "add Shape::NoteDraw"
Invoke-Cb 'add_method' @{
    class = 'Shape'; name = 'NoteDraw'; return_type = 'void';
    static = $true; access = 'protected';
    args = @(
        @{ name = 'painter';    type = 'CbPainter&' },
        @{ name = 'rect';       type = 'CbRect' },
        @{ name = 'noteText';   type = 'CbString' },
        @{ name = 'fontHeight'; type = 'int' },
        @{ name = 'selected';   type = 'bool' },
        @{ name = 'penColor';   type = 'CbColorRef' },
        @{ name = 'textColor';  type = 'CbColorRef' }
    )
    body = $noteDrawBody
} | Out-Null

Write-Host "add NoteShape::RecalcHeight"
Invoke-Cb 'add_method' @{
    class = 'NoteShape'; name = 'RecalcHeight'; return_type = 'void'; access = 'public';
    args = @( @{ name = 'painter'; type = 'CbPainter&' } )
    body = $recalcBody
} | Out-Null

Write-Host "add SDNoteShape::RecalcHeight"
Invoke-Cb 'add_method' @{
    class = 'SDNoteShape'; name = 'RecalcHeight'; return_type = 'void'; access = 'public';
    args = @( @{ name = 'painter'; type = 'CbPainter&' } )
    body = $recalcBody
} | Out-Null

Write-Host "thin NoteShape::Draw"
Invoke-Cb 'set_method_body' @{ class = 'NoteShape'; name = 'Draw'; body = $noteDrawThin } | Out-Null

Write-Host "thin SDNoteShape::Draw"
Invoke-Cb 'set_method_body' @{ class = 'SDNoteShape'; name = 'Draw'; body = $sdNoteDrawThin } | Out-Null

Write-Host "Done (model edited in memory; no write_source / no save yet)."
$pipe.Close()
