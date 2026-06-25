# Sort-undo fix, part 2: same SaveState()-before-SetOrder fix (validated on
# DataModel) applied to the remaining sort methods -- Class (member/method groups),
# ClassGroup (classes), MetaGroup (class groups).
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.params.class).$($obj.params.name) -> $r" } else { Write-Output "ok:   $($obj.params.class).$($obj.params.name)" } }

function Body($sortCall, $iterType, $iterVar, $ret) {
    return (@(
'    if (!checkOnly)',
'    {',
"        $sortCall",
'        ',
'        int i = 0;',
"        $iterType $iterVar(this);",
"        while (++$iterVar)",
'        {',
"            $iterVar->SaveState();",
"            $iterVar->SetOrder(i++);",
'        }',
'        ',
'        GetDataModelDoc()->GetDocument()->UpdateAllViews(NULL, MOD_SORT, (CObject*)this);',
'    }',
'    ',
"    return $ret;"
) -join "`r`n")
}

Send @{ cmd='set_method_body'; params=@{ class='Class'; name='SortOnName';
    body=(Body 'SortMemberAndMethodGroup(MemberAndMethodGroup::CompareName);' 'MemberAndMethodGroupIterator' 'memberAndMethodGroup' '1') } }
Send @{ cmd='set_method_body'; params=@{ class='Class'; name='SortOnPhase';
    body=(Body 'SortMemberAndMethodGroup(MemberAndMethodGroup::ComparePhase);' 'MemberAndMethodGroupIterator' 'memberAndMethodGroup' 'GetDataModel()->GetPhaseSupport()') } }

Send @{ cmd='set_method_body'; params=@{ class='ClassGroup'; name='SortOnName';
    body=(Body 'SortClass(Class::CompareName);' 'ClassIterator' 'iClass' '1') } }
Send @{ cmd='set_method_body'; params=@{ class='ClassGroup'; name='SortOnPhase';
    body=(Body 'SortClass(Class::ComparePhase);' 'ClassIterator' 'iClass' 'GetDataModelDoc()->GetDataModel()->GetPhaseSupport()') } }

Send @{ cmd='set_method_body'; params=@{ class='MetaGroup'; name='SortOnName';
    body=(Body 'SortClassGroup(ClassGroup::CompareName);' 'ClassGroupIterator' 'iClassGroup' '1') } }
Send @{ cmd='set_method_body'; params=@{ class='MetaGroup'; name='SortOnPhase';
    body=(Body 'SortClassGroup(ClassGroup::ComparePhase);' 'ClassGroupIterator' 'iClassGroup' 'GetDataModel()->GetPhaseSupport()') } }

Send @{ cmd='write_source'; params=@{ modified_only=$true } }
$pipe.Close()
Write-Output "DONE"
