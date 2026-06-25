# M0 main-tree port: add the tree sibling-order comparator as a real model
# method, Gti::CompareTreeOrder(Gti*, Gti*) -- static, public, returns int.
# It was the body of the MFC-only CClassBuilderView::Compare; moving it onto Gti
# (the model) lets both the MFC tree and the Qt mirror share one comparator.
# Driven over the pipe so it lives in the model and regenerates with the source.
$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.','ClassBuilder',[System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(4000)
$rd = New-Object System.IO.StreamReader($pipe)
$wr = New-Object System.IO.StreamWriter($pipe); $wr.AutoFlush = $true
function Send($obj){ $wr.WriteLine(($obj | ConvertTo-Json -Compress -Depth 8)); $r = $rd.ReadLine(); if ($r -notmatch '"ok":true') { Write-Output "FAIL: $($obj.cmd) -> $r" } else { Write-Output "ok:   $($obj.cmd) $($obj.params.name)" } }

# 1) Create the method (no body yet). Args are two Gti* (bare type 'Gti' already
#    exists in the model). Static + public so both callers can reach it.
Send @{ cmd='add_method'; params=@{
    class       = 'Gti'
    name        = 'CompareTreeOrder'
    return_type = 'int'
    static      = $true
    access      = 'public'
    args        = @(
        @{ name='pGti1'; type='Gti*' },
        @{ name='pGti2'; type='Gti*' }
    )
} }

# 2) Body -- the former CClassBuilderView::Compare logic, with pGti1/pGti2 as the
#    parameters and a symmetric default rank (11/11) so it is a valid strict-weak
#    ordering for both SortChildrenCB and std::stable_sort. Explicit CRLF join.
$body = @(
'    DataModel* pDataModel1             = dynamic_cast<DataModel*>(pGti1);',
'    ClassDiagram* pClassDiagram1       = dynamic_cast<ClassDiagram*>(pGti1);',
'    SequenceDiagram* pSequenceDiagram1 = dynamic_cast<SequenceDiagram*>(pGti1);',
'    ClassGroup* pClassGroup1           = dynamic_cast<ClassGroup*>(pGti1);',
'    MetaGroup* pMetaGroup1             = dynamic_cast<MetaGroup*>(pGti1);',
'    Group* pGroup1                     = dynamic_cast<Group*>(pGti1);',
'    Class* pClass1                     = dynamic_cast<Class*>(pGti1);',
'    Inherit* pInherit1                 = dynamic_cast<Inherit*>(pGti1);',
'    FromRelation* pFromRelation1       = dynamic_cast<FromRelation*>(pGti1);',
'    ToRelation* pToRelation1           = dynamic_cast<ToRelation*>(pGti1);',
'    Member* pMember1                   = dynamic_cast<Member*>(pGti1);',
'    Method* pMethod1                   = dynamic_cast<Method*>(pGti1);',
'    Type* pType1                       = dynamic_cast<Type*>(pGti1);',
'    Argument* pArgument1               = dynamic_cast<Argument*>(pGti1);',
'    Actors* pActors1                   = dynamic_cast<Actors*>(pGti1);',
'    Actor* pActor1                     = dynamic_cast<Actor*>(pGti1);',
'',
'    DataModel* pDataModel2             = dynamic_cast<DataModel*>(pGti2);',
'    ClassDiagram* pClassDiagram2       = dynamic_cast<ClassDiagram*>(pGti2);',
'    SequenceDiagram* pSequenceDiagram2 = dynamic_cast<SequenceDiagram*>(pGti2);',
'    ClassGroup* pClassGroup2           = dynamic_cast<ClassGroup*>(pGti2);',
'    MetaGroup* pMetaGroup2             = dynamic_cast<MetaGroup*>(pGti2);',
'    Group* pGroup2                     = dynamic_cast<Group*>(pGti2);',
'    Class* pClass2                     = dynamic_cast<Class*>(pGti2);',
'    Inherit* pInherit2                 = dynamic_cast<Inherit*>(pGti2);',
'    FromRelation* pFromRelation2       = dynamic_cast<FromRelation*>(pGti2);',
'    ToRelation* pToRelation2           = dynamic_cast<ToRelation*>(pGti2);',
'    Member* pMember2                   = dynamic_cast<Member*>(pGti2);',
'    Method* pMethod2                   = dynamic_cast<Method*>(pGti2);',
'    Type* pType2                       = dynamic_cast<Type*>(pGti2);',
'    Argument* pArgument2               = dynamic_cast<Argument*>(pGti2);',
'    Actors* pActors2                   = dynamic_cast<Actors*>(pGti2);',
'    Actor* pActor2                     = dynamic_cast<Actor*>(pGti2);',
'',
'    int order1 = 11; // Default extern, other types',
'    if (pDataModel1)',
'        order1 = 0;',
'    else if (pClassDiagram1 || pSequenceDiagram1)',
'        order1 = 1;',
'    else if (pClassGroup1 || pMetaGroup1)',
'        order1 = 2;',
'    else if (pInherit1 || pActors1)',
'        order1 = 3;',
'    else if (pToRelation1)',
'        order1 = 4;',
'    else if (pFromRelation1)',
'        order1 = 5;',
'    else if (pGroup1)',
'        order1 = 6;',
'    else if (pMethod1)',
'        order1 = 7;',
'    else if (pMember1)',
'        order1 = 8;',
'    else if (pClass1)',
'        order1 = 9;',
'    else if (pArgument1)',
'        order1 = 10;',
'',
'    int order2 = 11; // Default extern, other types',
'    if (pDataModel2)',
'        order2 = 0;',
'    else if (pClassDiagram2 || pSequenceDiagram2)',
'        order2 = 1;',
'    else if (pClassGroup2 || pMetaGroup2)',
'        order2 = 2;',
'    else if (pInherit2 || pActors2)',
'        order2 = 3;',
'    else if (pToRelation2)',
'        order2 = 4;',
'    else if (pFromRelation2)',
'        order2 = 5;',
'    else if (pGroup2)',
'        order2 = 6;',
'    else if (pMethod2)',
'        order2 = 7;',
'    else if (pMember2)',
'        order2 = 8;',
'    else if (pClass2)',
'        order2 = 9;',
'    else if (pArgument2)',
'        order2 = 10;',
'',
'    if (pGroup1 && pGroup2)',
'        return pGroup1->GetOrder()-pGroup2->GetOrder();',
'    else if (pClass1 && pClass2)',
'        return pClass1->GetOrder()-pClass2->GetOrder();',
'    else if ((pClassDiagram1 || pSequenceDiagram1) && (pClassDiagram2 || pSequenceDiagram2))',
'        return pGti1->GetOrder()-pGti2->GetOrder();',
'    else if (pType1 && pType2)',
'        return pType1->GetName().CompareNoCase(pType2->GetName());',
'    else if (pActor1 && pActor2)',
'        return pActor1->GetName().CompareNoCase(pActor2->GetName());',
'    else if (pInherit1 && pInherit2)',
'        return pInherit1->GetBaseName().CompareNoCase(pInherit2->GetBaseName());',
'    else if (pFromRelation1 && pFromRelation2)',
'        return pFromRelation1->GetItemText().CompareNoCase(pFromRelation2->GetItemText());',
'    else if (pToRelation1 && pToRelation2)',
'        return pToRelation1->GetItemText().CompareNoCase(pToRelation2->GetItemText());',
'    else if (pMember1 && pMember2)',
'    {',
'        return Member::CompareTree(pMember1, pMember2);',
'    }',
'    else if (pMethod1 && pMethod2)',
'    {',
'        return Method::CompareTree(pMethod1, pMethod2);',
'    }',
'    else if (pArgument1 && pArgument2)',
'    {',
'        if (pArgument1 == pArgument2)',
'            return 0;',
'',
'        Argument* pArgument = pArgument1->GetMethod()->GetNextArgument(pArgument1);',
'        while (pArgument)',
'        {',
'            if (pArgument == pArgument2)',
'                return -1;',
'',
'            pArgument = pArgument1->GetMethod()->GetNextArgument(pArgument);',
'        }',
'',
'        return 1;',
'    }',
'    else',
'    {',
'        return order1-order2;',
'    }'
) -join "`r`n"

Send @{ cmd='set_method_body'; params=@{ class='Gti'; name='CompareTreeOrder'; body=$body } }

# 3) Flush the model to disk so the build picks up the new Gti method.
Send @{ cmd='write_source'; params=@{ modified_only=$true } }

$pipe.Close()
Write-Output "DONE"
