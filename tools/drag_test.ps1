# drag_test.ps1 -- simulate a mouse drag (screen coords) via SendInput.
#   powershell tools/drag_test.ps1 -X1 545 -Y1 390 -X2 545 -Y2 560
param([int]$X1, [int]$Y1, [int]$X2, [int]$Y2)
$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class CbDrag {
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint flags, int dx, int dy, uint data, UIntPtr extra);
    public const uint LEFTDOWN = 0x0002, LEFTUP = 0x0004;
}
'@
[CbDrag]::SetCursorPos($X1, $Y1) | Out-Null
Start-Sleep -Milliseconds 150
[CbDrag]::mouse_event([CbDrag]::LEFTDOWN, 0, 0, 0, [UIntPtr]::Zero)
Start-Sleep -Milliseconds 150
# move in steps so the app sees a drag, not a jump
$steps = 12
for ($i = 1; $i -le $steps; $i++) {
    $x = $X1 + [int](($X2 - $X1) * $i / $steps)
    $y = $Y1 + [int](($Y2 - $Y1) * $i / $steps)
    [CbDrag]::SetCursorPos($x, $y) | Out-Null
    Start-Sleep -Milliseconds 40
}
Start-Sleep -Milliseconds 200
[CbDrag]::mouse_event([CbDrag]::LEFTUP, 0, 0, 0, [UIntPtr]::Zero)
Write-Host "dragged ($X1,$Y1) -> ($X2,$Y2)"
