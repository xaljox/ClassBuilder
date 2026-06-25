# M0a — ConnectionSegment family: flip Draw(CDC*) -> Draw(CbPainter&) + convert bodies,
# add one transitional Draw(CDC*) forwarder on the base. Driven over the pipe.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.params.class).$($obj.cmd) -> $r" } else { Write-Output "ok:   $($obj.params.class) $($obj.cmd)" } }

function FlipAndBody($cls, $body) {
    Send @{ cmd='set_argument_type'; params=@{ class=$cls; method='Draw'; arg_index=0; value='CbPainter&' } }
    Send @{ cmd='set_argument_name'; params=@{ class=$cls; method='Draw'; arg_index=0; value='painter' } }
    Send @{ cmd='set_method_body';   params=@{ class=$cls; name='Draw'; body=$body } }
}

$simple = @(
'    CbPoint start = GetStartPoint();',
'    CbPoint end = start + GetSize();',
'    painter.DrawLine(start, end);'
) -join "`r`n"

$inheritStart = @(
'    CbPoint start = GetStartPoint();',
'',
'    const int size = 30;',
'    CbSize a(0,0);',
'    CbSize b(0,0);',
'',
'    if (GetSize().cx == 0)',
'    {',
'        if (GetSize().cy < 0)',
'        {',
'            a.cy = -size;',
'            b.cx = size/2;',
'        }',
'        else',
'        {',
'            a.cy = size;',
'            b.cx = size/2;',
'        }',
'    }',
'    else',
'    {',
'        if (GetSize().cx < 0)',
'        {',
'            a.cx = -size;',
'            b.cy = size/2;',
'        }',
'        else',
'        {',
'            a.cx = size;',
'            b.cy = size/2;',
'        }',
'    }',
'',
'    painter.DrawLine(start, start+a-b);',
'    painter.DrawLine(start, start+a+b);',
'    painter.DrawLine(start+a-b, start+a+b);',
'    painter.DrawLine(start+a, start+GetSize());'
) -join "`r`n"

$aggStart = @(
'    CbPoint start = GetStartPoint();',
'',
'    const int size = 20;',
'    CbSize a(0,0);',
'    CbSize b(0,0);',
'',
'    if (GetSize().cx == 0)',
'    {',
'        if (GetSize().cy < 0)',
'        {',
'            a.cy = -size;',
'            b.cx = size/2;',
'        }',
'        else',
'        {',
'            a.cy = size;',
'            b.cx = size/2;',
'        }',
'    }',
'    else',
'    {',
'        if (GetSize().cx < 0)',
'        {',
'            a.cx = -size;',
'            b.cy = size/2;',
'        }',
'        else',
'        {',
'            a.cx = size;',
'            b.cy = size/2;',
'        }',
'    }',
'',
'    if (GetConnectionShape()->GetOwned() == 1)',
'    {',
'        painter.DrawLine(start+a+a, start+a-b);',
'        painter.DrawLine(start+a-b, start);',
'        painter.DrawLine(start, start+a+b);',
'        painter.DrawLine(start+a+b, start+a+a);',
'    }',
'    else if (GetConnectionShape()->GetOwned() == 2)',
'    {',
'        CbPoint points[4];',
'        points[0] = start+a+a;',
'        points[1] = start+a-b;',
'        points[2] = start;',
'        points[3] = start+a+b;',
'        painter.Polygon(points, 4);',
'    }',
'',
'    painter.DrawLine(start+a+a, start+GetSize());'
) -join "`r`n"

$depEnd = @(
'    CbPoint start = GetStartPoint();',
'    CbPoint end = start + GetSize();',
'',
'    const int size = 20;',
'    CbSize a(0,0);',
'    CbSize b(0,0);',
'',
'    if (GetSize().cx == 0)',
'    {',
'        if (GetSize().cy > 0)',
'        {',
'            a.cy = -size;',
'            b.cx = size/2;',
'        }',
'        else',
'        {',
'            a.cy = size;',
'            b.cx = size/2;',
'        }',
'    }',
'    else',
'    {',
'        if (GetSize().cx > 0)',
'        {',
'            a.cx = -size;',
'            b.cy = size/2;',
'        }',
'        else',
'        {',
'            a.cx = size;',
'            b.cy = size/2;',
'        }',
'    }',
'',
'    CbSize c(a.cx/2, a.cy/2);',
'',
'    painter.DrawLine(end+c, start);',
'',
'    // Arrowhead stays solid even when the dependency line is dotted.',
'    painter.Save();',
'    painter.SetPen(PS_SOLID, 1, GetConnectionShape()->GetPenColor());',
'    painter.DrawLine(end+a, end);',
'    painter.DrawLine(end, end+a-b);',
'    painter.DrawLine(end, end+a+b);',
'    painter.Restore();'
) -join "`r`n"

# Base first (single Draw -> unambiguous), then the forwarder, then the 8 overrides.
FlipAndBody 'ConnectionSegment' $simple

# Raw JSON so the single-element args array isn't unwrapped by ConvertTo-Json.
# \r\n in the single-quoted string is literal -> JSON CRLF escape in the body.
$forwardJson = '{"cmd":"add_method","params":{"class":"ConnectionSegment","name":"Draw","return_type":"void","virtual":false,"args":[{"name":"pDC","type":"CDC*"}],"body":"    CbPainter_Cdc painter(pDC);   // MFC_CDC_BRIDGE: delete with MFC view/Track (Qt port)\r\n    Draw(painter);"}}'
$wr.WriteLine($forwardJson); $fr = $rd.ReadLine()
if ($fr -notmatch '"ok":true') { Write-Output "FAIL: add_method forwarder -> $fr" } else { Write-Output "ok:   ConnectionSegment add_method forwarder" }

FlipAndBody 'DependencyStartSegment'         $simple
FlipAndBody 'InheritEndSegment'              $simple
FlipAndBody 'RelationSingleEndSegment'       $simple
FlipAndBody 'RelationMultiEndSegment'        $simple
FlipAndBody 'RelationAssociationStartSegment' $simple
FlipAndBody 'InheritStartSegment'            $inheritStart
FlipAndBody 'RelationAggregationStartSegment' $aggStart
FlipAndBody 'DependencyEndSegment'           $depEnd

# Flush model -> disk
Send @{ cmd='write_source'; params=@{ modified_only=$true } }
$pipe.Close()
Write-Output "DONE"
