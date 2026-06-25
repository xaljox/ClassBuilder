# capture_cb.ps1 -- screenshot the running dev ClassBuilder main window.
#   powershell tools/capture_cb.ps1 -Out out\shot.png [-W 720] [-H 600] [-NoMove]
param(
    [string]$Out = 'out\cb_capture.png',
    [int]$W = 720,
    [int]$H = 600,
    [switch]$NoMove
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class CbCap {
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr a, int x, int y, int cx, int cy, uint f);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
}
'@
$p = Get-Process ClassBuilder -ErrorAction SilentlyContinue |
     Where-Object { $_.Path -like '*\out\build\*' -and $_.MainWindowHandle -ne 0 } |
     Select-Object -First 1
if (-not $p) { Write-Error 'dev ClassBuilder not running (or no main window)'; return }

if (-not $NoMove) {
    [CbCap]::SetWindowPos($p.MainWindowHandle, [IntPtr]::Zero, 60, 60, $W - 20, $H + 60, 0x0040) | Out-Null
}
[CbCap]::SetForegroundWindow($p.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 800

$r = New-Object CbCap+RECT
[CbCap]::GetWindowRect($p.MainWindowHandle, [ref]$r) | Out-Null
$bmp = New-Object System.Drawing.Bitmap($W, $H)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($r.Left, $r.Top, 0, 0, $bmp.Size)
$outPath = if ([IO.Path]::IsPathRooted($Out)) { $Out } else { Join-Path (Join-Path $PSScriptRoot '..') $Out }
$bmp.Save($outPath)
$g.Dispose(); $bmp.Dispose()
Write-Host "captured -> $outPath  windowRect=($($r.Left),$($r.Top)) $($r.Right - $r.Left)x$($r.Bottom - $r.Top)"
