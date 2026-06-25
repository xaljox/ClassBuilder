# Recreates the Matrix data model + diagram via the pipe API.
#
# Model:
#   Matrix (document, serialize) — constructor (rows, columns) creates the grid
#   Row    (serialize) — _id:int
#   Column (serialize) — _id:int
#   Cell   (serialize) — _value:CString
#
# Relations (all multi, all owned — CB intrusive lists support multi-ownership):
#   Matrix --owns--> Row
#   Matrix --owns--> Column
#   Row    --owns--> Cell
#   Column --owns--> Cell
#
# A Cell is owned by both its Row and its Column. ~Cell unlinks itself from
# both lists. Whichever parent's cascade fires first deletes the Cell; the
# other parent's cascade simply walks an already-shortened container.
#
# Diagram: all four classes, auto_place:true.

$ErrorActionPreference = 'Stop'

$pipe = [System.IO.Pipes.NamedPipeClientStream]::new(
    '.', 'ClassBuilder', [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(2000)
$w = [System.IO.StreamWriter]::new($pipe); $w.AutoFlush = $true
$r = [System.IO.StreamReader]::new($pipe)

function Cb([string]$cmd, [hashtable]$params = @{}) {
    $j = @{ cmd = $cmd; params = $params } | ConvertTo-Json -Compress -Depth 10
    $w.WriteLine($j)
    $line = $r.ReadLine()
    $resp = $line | ConvertFrom-Json
    if (-not $resp.ok) { throw "command '$cmd' failed: $($resp.error)" }
    return $resp.result
}

# --- Document ---
Cb 'new_model_serialize' @{
    name           = 'Matrix'
    document_class = 'Matrix'
    h_file         = 'Matrix.h'
}
Write-Host "Created Matrix doc"

# CString might not exist as a Type yet — add if missing.
$types = Cb 'list_types'
if (-not ($types | Where-Object { $_.name -eq 'CString' })) {
    Cb 'add_type' @{ name = 'CString' } | Out-Null
}

# --- Classes ---
foreach ($c in @('Row','Column','Cell')) {
    Cb 'add_class' @{ name = $c; serialize = $true } | Out-Null
}

# --- Members ---
# Row/Column ids are private with a public getter (no setter — set once via
# the constructor, then immutable). Cell value stays public for simplicity.
Cb 'add_member' @{ class='Row';    name='id';    type='int';     access='private'; getter='public' } | Out-Null
Cb 'add_member' @{ class='Column'; name='id';    type='int';     access='private'; getter='public' } | Out-Null
Cb 'add_member' @{ class='Cell';   name='value'; type='CString'; access='public' } | Out-Null

# --- Relations: all owned multi ---
Cb 'add_relation' @{ from_class='Matrix'; to_class='Row';    kind='multi'; owned=$true } | Out-Null
Cb 'add_relation' @{ from_class='Matrix'; to_class='Column'; kind='multi'; owned=$true } | Out-Null
Cb 'add_relation' @{ from_class='Row';    to_class='Cell';   kind='multi'; owned=$true } | Out-Null
Cb 'add_relation' @{ from_class='Column'; to_class='Cell';   kind='multi'; owned=$true } | Out-Null

# --- Constructors ---
# `add_constructor` auto-derives args from relations + members:
#   Row      → (Matrix* pMatrix, int id)
#   Column   → (Matrix* pMatrix, int id)
#   Cell     → (Row* pRow, Column* pColumn)
# We only supply additional args (Matrix's `rows`/`columns`); the rest comes
# from CreateArguments and is appended-after.
#
# Row and Column ctors are SYMMETRIC: each walks the cross-dimension on its
# parent Matrix and creates one Cell per existing item on that side. When
# called from Matrix(rows, columns), Columns are built first → no Rows
# exist yet → Column ctor creates 0 Cells. Then Rows are built → Columns
# exist → Row ctor creates one Cell per Column. Total = rows × columns
# Cells, exactly once each. If the user later adds a Column or Row by
# hand, the symmetric ctor fills in the missing cross-row of Cells
# automatically.

# Bodies contain ONLY what goes below "// Put in your own code". CB
# auto-emits ConstructorInclude(...) with the correct parent args above
# that marker — re-including it here would duplicate the call.

$columnBody = @'
    Matrix::RowIterator iRow(pMatrix);
    while (++iRow)
    {
        (void)new Cell(iRow, this);
    }
'@
$columnBody = $columnBody -replace "`r?`n", "`r`n"

Cb 'add_constructor' @{
    class  = 'Column'
    access = 'public'
    body   = $columnBody
} | Out-Null

$rowBody = @'
    Matrix::ColumnIterator iColumn(pMatrix);
    while (++iColumn)
    {
        (void)new Cell(this, iColumn);
    }
'@
$rowBody = $rowBody -replace "`r?`n", "`r`n"

Cb 'add_constructor' @{
    class  = 'Row'
    access = 'public'
    body   = $rowBody
} | Out-Null

# Cell needs an explicit public ctor so Row/Column can call `new Cell(...)`.
# Auto-derived args = (Row* pRow, Column* pColumn). No body needed; the
# auto-emitted ConstructorInclude(pRow, pColumn) does the splicing into
# both intrusive lists.
Cb 'add_constructor' @{
    class  = 'Cell'
    access = 'public'
} | Out-Null

$matrixBody = @'
    for (int c = 0; c < columns; c++)
    {
        (void)new Column(this, c);
    }

    for (int r = 0; r < rows; r++)
    {
        (void)new Row(this, r);
    }
'@
$matrixBody = $matrixBody -replace "`r?`n", "`r`n"

Cb 'add_constructor' @{
    class  = 'Matrix'
    args   = @( @{name='rows'; type='int'}, @{name='columns'; type='int'} )
    access = 'public'
    body   = $matrixBody
} | Out-Null

# --- Diagram ---
Cb 'add_class_diagram' @{
    name              = 'Matrix Overview'
    classes           = @('Matrix','Row','Column','Cell')
    auto_place        = $true
    public_members    = $true
    public_methods    = $true
    protected_members = $false
    protected_methods = $false
    private_members   = $false
    private_methods   = $false
    get_set_methods   = $false
} | Out-Null

Write-Host "Done. Open the 'Matrix Overview' diagram."

$pipe.Close()
