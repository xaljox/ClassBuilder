# clear_dll_export.ps1 -- turn OFF the per-class "DLL export" flag on every
# class in the open model, via the ClassBuilder pipe API.
#
# Codegen emits `class AFX_EXT_CLASS Foo` only when Class::GetDllExport() is
# true. Since ClassBuilder is now a single static EXE (no extension DLL), no
# class needs exporting -- clearing the flag on all classes makes a regen stop
# emitting AFX_EXT_CLASS entirely, after which the NODLL / #undef AFX_EXT_CLASS
# hacks can be removed.
#
# Prerequisite: ClassBuilder.exe must be RUNNING with the model open (the pipe
# server lives in the process). After this script: regenerate in CB.

$ErrorActionPreference = 'Stop'

$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.', 'ClassBuilder',
            [System.IO.Pipes.PipeDirection]::InOut)
try   { $pipe.Connect(3000) }
catch { Write-Error "Cannot connect to \\.\pipe\ClassBuilder -- is ClassBuilder.exe running with a model open?"; exit 1 }

$reader = New-Object System.IO.StreamReader($pipe)
$writer = New-Object System.IO.StreamWriter($pipe)
$writer.AutoFlush = $true
$writer.NewLine   = "`n"            # server splits on LF

function Send-Cmd($obj) {
    $writer.WriteLine(($obj | ConvertTo-Json -Compress -Depth 10))
    return ($reader.ReadLine() | ConvertFrom-Json)
}

$r = Send-Cmd @{ cmd = 'list_classes' }
if (-not $r.ok) { Write-Error "list_classes failed: $($r.error)"; exit 1 }
$classes = @($r.result)
Write-Host "Classes in model: $($classes.Count)"

$cleared = 0; $alreadyOff = 0; $failed = @()
foreach ($name in $classes) {
    $r = Send-Cmd @{ cmd = 'set_class_dll_export'; params = @{ name = $name; value = $false } }
    if (-not $r.ok)          { $failed += "$name : $($r.error)" }
    elseif ($r.result.dll_export) { $failed += "$name : still true after set" }
    else                     { $cleared++ }
}

$pipe.Dispose()
Write-Host "dll_export cleared on $cleared class(es)."
if ($failed.Count) { Write-Host "FAILED:"; $failed | ForEach-Object { Write-Host "  $_" } }
else { Write-Host "All classes done. Now regenerate in ClassBuilder." }
