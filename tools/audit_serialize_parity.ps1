# audit_serialize_parity.ps1
#
# For each ClassBuilder .cpp file, find both Serialize(CArchive&) and
# Serialize(CbArchive&) storing branches, extract the set of underlying
# field names written, and report any field that appears in one but not
# the other.
#
# Catches bugs like Gti::_phase: present in the CArchive body inside an
# `int(_phase)` cast and inside a version gate, missed by the bulk-fill
# script when generating the CbArchive body.
#
# Read-side mismatches (CArchive uses gates, CbArchive collapses) are
# normal and not flagged. We compare the *store* branches, which always
# write every field unconditionally in both formats.

param(
    [string]$SourceDir = 'c:\Users\jimmy\Projects\Src\ClassBuilder\ClassBuilder'
)

$ErrorActionPreference = 'Stop'

# Returns the brace-balanced block content immediately after the given
# regex-matched header (e.g. "IsStoring()) {" or "else {").
function Extract-Block {
    param([string]$body, [string]$headerPattern)
    $m = [regex]::Match($body, $headerPattern, 'Singleline')
    if (-not $m.Success) { return $null }
    $start = $m.Index + $m.Length
    $depth = 1
    $i = $start
    while ($i -lt $body.Length -and $depth -gt 0) {
        $c = $body[$i]
        if ($c -eq '{') { $depth++ }
        elseif ($c -eq '}') { $depth-- }
        $i++
    }
    if ($depth -ne 0) { return $null }
    return $body.Substring($start, $i - $start - 1)
}

# Extract `<<` field names from a block.
function Extract-StoreFieldsFromBlock {
    param([string]$block)
    $fields = @()
    foreach ($m in [regex]::Matches($block, '(?:r?[Cc]b?[Aa]rchive|archive)\s*<<\s*([^;]+);')) {
        $expr = $m.Groups[1].Value
        $f = [regex]::Match($expr, '_\w+')
        if ($f.Success) { $fields += $f.Value }
        else {
            $lit = [regex]::Match($expr.Trim(), '^\d+$')
            if ($lit.Success) { $fields += "<literal:$($expr.Trim())>" }
        }
    }
    return $fields
}

# Extract `>>` field names from a block.
function Extract-LoadFieldsFromBlock {
    param([string]$block)
    $fields = @()
    foreach ($m in [regex]::Matches($block, '(?:r?[Cc]b?[Aa]rchive|archive)\s*>>\s*([^;]+);')) {
        $expr = $m.Groups[1].Value
        $f = [regex]::Match($expr, '_\w+')
        if ($f.Success) { $fields += $f.Value }
    }
    return $fields
}

function Extract-StoreFields {
    param([string]$body)
    $b = Extract-Block $body 'IsStoring\(\)\s*\)\s*\{'
    if (-not $b) { return @() }
    return Extract-StoreFieldsFromBlock $b
}

# Returns the body of the named overload from the file text, or $null if missing.
# `archiveType` is "CArchive" or "CbArchive". `methodName` defaults to "Serialize".
function Get-SerializeBody {
    param([string]$file, [string]$className, [string]$archiveType,
          [string]$methodName = 'Serialize')

    $sig = [regex]::Escape("$className`::$methodName($archiveType&")
    $m = [regex]::Match($file, "$sig[^)]*\)\s*\{", 'Singleline')
    if (-not $m.Success) { return $null }
    $start = $m.Index + $m.Length
    $depth = 1
    $i = $start
    while ($i -lt $file.Length -and $depth -gt 0) {
        $c = $file[$i]
        if ($c -eq '{') { $depth++ }
        elseif ($c -eq '}') { $depth-- }
        $i++
    }
    if ($depth -ne 0) { return $null }
    return $file.Substring($start, $i - $start - 1)
}

$cppFiles = Get-ChildItem -Path $SourceDir -Filter *.cpp
$mismatches = @()

# Also count WRITE_*/READ_* macros inside SerializeRelations branches.
function Count-RelationWrites {
    param([string]$block)
    return ([regex]::Matches($block, '\bWRITE_\w+_ACTIVE\b')).Count
}
function Count-RelationReads {
    param([string]$block)
    return ([regex]::Matches($block, '\bREAD_\w+_ACTIVE\b')).Count
}

foreach ($f in $cppFiles) {
    $text = Get-Content -Raw $f.FullName

    # Find every "ClassName::Serialize(CArchive&" — gives us candidate classes.
    $classNames = [regex]::Matches($text, '(\w+)::Serialize\(CArchive&') |
        ForEach-Object { $_.Groups[1].Value } |
        Sort-Object -Unique

    # Also check SerializeRelations(CbArchive&) for store/load count parity.
    $relClassNames = [regex]::Matches($text, '(\w+)::SerializeRelations\(CbArchive&') |
        ForEach-Object { $_.Groups[1].Value } |
        Sort-Object -Unique
    foreach ($cls in $relClassNames) {
        $relBody = Get-SerializeBody -file $text -className $cls -archiveType 'CbArchive' `
                   -methodName 'SerializeRelations'
        if (-not $relBody) { continue }
        $storeBlock = Extract-Block $relBody 'IsStoring\(\)\s*\)\s*\{'
        if (-not $storeBlock) { continue }
        $loadBlock = Extract-Block $relBody '\}\s*else\s*\{'
        if (-not $loadBlock) { continue }
        $sw = Count-RelationWrites $storeBlock
        $lr = Count-RelationReads  $loadBlock
        if ($sw -ne $lr) {
            $mismatches += [pscustomobject]@{
                File   = $f.Name
                Class  = $cls
                Kind   = 'cbz-relations-write-vs-read'
                Detail = "store WRITE_*=$sw vs load READ_*=$lr"
            }
        }
    }

    foreach ($cls in $classNames) {
        $cBody  = Get-SerializeBody -file $text -className $cls -archiveType 'CArchive'
        $cbBody = Get-SerializeBody -file $text -className $cls -archiveType 'CbArchive'
        if (-not $cBody -or -not $cbBody) { continue }

        $cFields  = Extract-StoreFields $cBody  | Where-Object { $_ -notlike '<literal:*>' }
        $cbFields = Extract-StoreFields $cbBody | Where-Object { $_ -notlike '<literal:*>' }

        $onlyInC  = $cFields  | Where-Object { $cbFields -notcontains $_ }
        $onlyInCb = $cbFields | Where-Object { $cFields  -notcontains $_ }

        if ($onlyInC -or $onlyInCb) {
            $mismatches += [pscustomobject]@{
                File         = $f.Name
                Class        = $cls
                Kind         = 'store-set'
                Detail       = "missing in CBZ: $($onlyInC -join ', '); extra in CBZ: $($onlyInCb -join ', ')"
            }
        }

        # Within CbArchive: store branch vs load branch must agree on
        # ordered field sequence. The bulk-fill script and any later
        # hand edits should keep them mirrored.
        $cbStoreBlock = Extract-Block $cbBody 'IsStoring\(\)\s*\)\s*\{'
        if ($cbStoreBlock) {
            $cbLoadBlock = Extract-Block $cbBody '\}\s*else\s*\{'
            if ($cbLoadBlock) {
                $storeOrdered = Extract-StoreFieldsFromBlock $cbStoreBlock |
                                Where-Object { $_ -notlike '<literal:*>' }
                $loadOrdered  = Extract-LoadFieldsFromBlock $cbLoadBlock
                $maxLen = [Math]::Max($storeOrdered.Count, $loadOrdered.Count)
                $diffIdx = -1
                for ($i = 0; $i -lt $maxLen; $i++) {
                    $s = if ($i -lt $storeOrdered.Count) { $storeOrdered[$i] } else { '<missing>' }
                    $l = if ($i -lt $loadOrdered.Count)  { $loadOrdered[$i]  } else { '<missing>' }
                    if ($s -ne $l) { $diffIdx = $i; break }
                }
                if ($diffIdx -ge 0) {
                    $sNeighbour = if ($diffIdx -lt $storeOrdered.Count) { $storeOrdered[$diffIdx] } else { '<missing>' }
                    $lNeighbour = if ($diffIdx -lt $loadOrdered.Count)  { $loadOrdered[$diffIdx]  } else { '<missing>' }
                    $mismatches += [pscustomobject]@{
                        File         = $f.Name
                        Class        = $cls
                        Kind         = 'cbz-store-vs-load'
                        Detail       = "first divergence at idx $diffIdx`: store='$sNeighbour' vs load='$lNeighbour' (store=$($storeOrdered.Count) fields, load=$($loadOrdered.Count) fields)"
                    }
                }
            }
        }
    }
}

if (-not $mismatches) {
    Write-Host "All Serialize parity checks passed." -ForegroundColor Green
    return
}

Write-Host "Field-set / order mismatches:" -ForegroundColor Yellow
$mismatches | Format-Table -AutoSize -Wrap
