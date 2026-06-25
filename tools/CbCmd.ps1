# CbCmd.ps1 -- shared client for the ClassBuilder command server.
#
# The server listens on a localhost TCP socket (127.0.0.1:51777 by default;
# override with the CB_CMD_PORT env var). TCP replaced the old Windows named
# pipe so the same client works on Windows / macOS / Linux. Dot-source this:
#
#   . "$PSScriptRoot/CbCmd.ps1"
#   Connect-Cb                          # or: Connect-Cb -Port 51778
#   Send-Cb '{"cmd":"ping"}'            # -> raw reply line (JSON string)
#   (Invoke-Cb 'list_classes').result   # -> parsed; Invoke-Cb takes cmd + optional params hashtable
#   Invoke-Cb 'add_type' @{ name = 'Foo' }
#   Disconnect-Cb

$script:CbClient = $null
$script:CbReader = $null
$script:CbWriter = $null

function Connect-Cb {
    param(
        [int]    $Port = $(if ($env:CB_CMD_PORT) { [int]$env:CB_CMD_PORT } else { 51777 }),
        [string] $Address = '127.0.0.1',   # NB: not $Host -- that's a read-only PS automatic variable
        [int]    $TimeoutMs = 5000
    )
    Disconnect-Cb
    $client = [System.Net.Sockets.TcpClient]::new()
    $iar = $client.BeginConnect($Address, $Port, $null, $null)
    if (-not $iar.AsyncWaitHandle.WaitOne($TimeoutMs)) {
        $client.Close(); throw "CbCmd: could not connect to ${Address}:$Port within ${TimeoutMs}ms (is ClassBuilder running?)"
    }
    $client.EndConnect($iar)
    $stream = $client.GetStream()
    $script:CbClient = $client
    $script:CbReader = [System.IO.StreamReader]::new($stream)
    $script:CbWriter = [System.IO.StreamWriter]::new($stream)
    $script:CbWriter.AutoFlush = $true
}

# Send a raw JSON line, return the raw reply line.
function Send-Cb {
    param([Parameter(Mandatory)][string] $Json)
    if (-not $script:CbWriter) { throw 'CbCmd: not connected -- call Connect-Cb first.' }
    $script:CbWriter.WriteLine($Json)
    return $script:CbReader.ReadLine()
}

# Convenience: build the request from a command name + optional params hashtable,
# send it, and return the PARSED reply object ({ok, result} / {ok:false, error}).
function Invoke-Cb {
    param(
        [Parameter(Mandatory)][string] $Cmd,
        [hashtable] $Params
    )
    $req = @{ cmd = $Cmd }
    if ($Params) { $req.params = $Params }
    $json = $req | ConvertTo-Json -Compress -Depth 20
    return (Send-Cb $json) | ConvertFrom-Json
}

function Disconnect-Cb {
    if ($script:CbClient) { try { $script:CbClient.Close() } catch {} }
    $script:CbClient = $null
    $script:CbReader = $null
    $script:CbWriter = $null
}
