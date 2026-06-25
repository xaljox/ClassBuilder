# Sort-undo fix: the bubble SortClass/SortClassGroup is undo-safe (the comparator
# SaveStates the swapped operand + MarkLastUndo(2) per swap), but the trailing
# SetOrder(i++) loop rewrites _order on EVERY sibling with no snapshot -- only the
# swapped operands were captured. Siblings that merely shifted keep their new _order
# on undo -> duplicate/stale _order -> wrong tree order (and MFC vs Qt diverge on the
# ties). Fix: SaveState() each sibling right before SetOrder in the loop (dedup, so
# the comparator-covered ones aren't double-snapshotted; UndoChange just serializes a
# sibling copy, so it's iterator-safe). Validating on DataModel first.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.params.class).$($obj.params.name)" } }

$sortOnName = @(
'    if (!checkOnly)',
'    {',
'        SortClass(Class::CompareName);',
'        SortClassGroup(ClassGroup::CompareName);',
'    ',
'        int i = 0;',
'        ClassIterator iClass(this);',
'        while (++iClass)',
'        {',
'            if (!iClass->GetClassGroup())',
'            {',
'                iClass->SaveState();',
'                iClass->SetOrder(i++);',
'            }',
'        }',
'    ',
'        i = 0;',
'        ClassGroupIterator classGroup(this);',
'        while (++classGroup)',
'        {',
'            classGroup->SaveState();',
'            classGroup->SetOrder(i++);',
'        }',
'    ',
'        GetDataModelDoc()->GetDocument()->UpdateAllViews(NULL, MOD_SORT, (CObject*)this);',
'    }',
'    ',
'    return 1;'
) -join "`r`n"

$sortOnPhase = @(
'    if (!checkOnly)',
'    {',
'        SortClass(Class::ComparePhase);',
'        SortClassGroup(ClassGroup::ComparePhase);',
'    ',
'        int i = 0;',
'        ClassIterator iClass(this);',
'        while (++iClass)',
'        {',
'            if (!iClass->GetClassGroup())',
'            {',
'                iClass->SaveState();',
'                iClass->SetOrder(i++);',
'            }',
'        }',
'    ',
'        i = 0;',
'        ClassGroupIterator classGroup(this);',
'        while (++classGroup)',
'        {',
'            classGroup->SaveState();',
'            classGroup->SetOrder(i++);',
'        }',
'    ',
'        GetDataModelDoc()->GetDocument()->UpdateAllViews(NULL, MOD_SORT, (CObject*)this);',
'    }',
'    ',
'    return GetPhaseSupport();'
) -join "`r`n"

Send @{ cmd='set_method_body'; params=@{ class='DataModel'; name='SortOnName';  body=$sortOnName } }
Send @{ cmd='set_method_body'; params=@{ class='DataModel'; name='SortOnPhase'; body=$sortOnPhase } }
Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
