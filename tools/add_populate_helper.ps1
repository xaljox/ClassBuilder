# Adds ClassShape::PopulateFromDiagramFlags() to the active model via the
# pipe. Idempotent — re-runs cleanly because add_method rejects duplicates;
# the script tolerates that.

$ErrorActionPreference = 'Stop'

$pipe = [System.IO.Pipes.NamedPipeClientStream]::new(
    '.', 'ClassBuilder', [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(2000)
$w = [System.IO.StreamWriter]::new($pipe); $w.AutoFlush = $true
$r = [System.IO.StreamReader]::new($pipe)

function Invoke-Cb([string]$cmd, [hashtable]$params = @{}) {
    $j = @{ cmd = $cmd; params = $params } | ConvertTo-Json -Compress -Depth 10
    $w.WriteLine($j)
    $line = $r.ReadLine()
    $resp = $line | ConvertFrom-Json
    if (-not $resp.ok) { throw "command '$cmd' failed: $($resp.error)" }
    return $resp.result
}

# Body: walk this class's members/methods, create a MemberShape/MethodShape
# for each item that passes the diagram's visibility filter and isn't
# already shown. Same predicate as NotifyAddMember / NotifyAddMethod.
# Methods skip macros, constructors, destructors; Get/Set methods are
# additionally gated on GetGetSetMethods.
$body = @'
BaseClass* pBaseClass = GetBaseClass();
    if (!pBaseClass) return;
    ClassDiagram* pCD = GetClassDiagram();
    if (!pCD) return;

    BaseClass::MemberIterator iMember(pBaseClass);
    while (++iMember)
    {
        bool show = false;
        if      (iMember->IsPublic()    && pCD->GetPublicMembers())    show = true;
        else if (iMember->IsProtected() && pCD->GetProtectedMembers()) show = true;
        else if (iMember->IsPrivate()   && pCD->GetPrivateMembers())   show = true;
        if (!show) continue;

        bool already = false;
        MemberShapeIterator iMS(this);
        while (++iMS)
        {
            if (iMS->GetMember() == iMember.Get()) { already = true; break; }
        }
        if (already) continue;

        (void)new MemberShape(this, iMember);
    }

    BaseClass::MethodIterator iMethod(pBaseClass);
    while (++iMethod)
    {
        if (!iMethod->IsNonMacroMethod()) continue;
        if (iMethod->IsConstructor() || iMethod->IsDestructor()) continue;
        if ((iMethod->IsGetMemberMethod() || iMethod->IsSetMemberMethod())
            && !pCD->GetGetSetMethods())
            continue;

        bool show = false;
        if      (iMethod->IsPublicMethod()    && pCD->GetPublicMethods())    show = true;
        else if (iMethod->IsProtectedMethod() && pCD->GetProtectedMethods()) show = true;
        else if (iMethod->IsPrivateMethod()   && pCD->GetPrivateMethods())   show = true;
        if (!show) continue;

        bool already = false;
        MethodShapeIterator iMS(this);
        while (++iMS)
        {
            if (iMS->GetMethod() == iMethod.Get()) { already = true; break; }
        }
        if (already) continue;

        (void)new MethodShape(this, iMethod);
    }
'@

# CB stores bodies with CRLF.
$body = $body -replace "`r?`n", "`r`n"

$result = Invoke-Cb 'add_method' @{
    class       = 'ClassShape'
    name        = 'PopulateFromDiagramFlags'
    return_type = 'void'
    access      = 'public'
    body        = $body
}

Write-Host "Added method:" ($result | ConvertTo-Json -Compress -Depth 5)

$pipe.Close()
