# IsXxx `int` -> `bool` sweep — staged artifacts

Staging for converting CB's `int IsX() const` type-predicates to `bool`
(see auto-memory `project_classbuilder_int_isx_to_bool`). The pieces:

1. **Codegen (DONE, live on disk):** `ClassBuilder/IsClassMethod.cpp` now generates
   `bool IsXxx()` — ctor return type `FindType("bool")` and `InitCode()` body
   `return (dynamic_cast<const Xxx*>(this) != nullptr);`. Verified by generating
   `Matrix_IsXxx.CBZ` -> `ClassBuilder/test/MatrixObject.{h,cpp}` shows
   `bool IsCell()/IsColumn()/IsRow()`. (Not yet folded into `new3.cbz`.)

2. **Filter-iterator macro (STAGED here, NOT applied to live `Include/`):**
   - `CB_IteratorMulti.bool.h`  (11 predicate slots flipped)
   - `CB_IteratorStaticMulti.bool.h`  (5 predicate slots flipped)
   The only change vs the live `Include/` headers is the predicate slot type
   `int (ClassTo::*...)() const` -> `bool (ClassTo::*...)() const`.
   `*.int.h` are the untouched originals (for diff/revert).

   **Why staged, not applied:** these macros are used by CB itself, whose `IsX`
   family is still `int`. Flipping the live macros now would break every
   `Iterator(&Class::IsX)` call site until the family flips. So apply these
   `*.bool.h` to `Include/` ONLY as part of the full conversion below.

3. **Test:** `test_isx_bool.cpp` — focused compile/run check that a `bool IsXxx()`
   predicate binds to the retyped (bool) iterator slot and invokes as a filter.

## To complete the conversion (when ready)

In ONE pass (don't flip piecemeal):
1. Flip CB's whole `IsX` family return types `int` -> `bool` (the stored return
   type on each existing IsClassMethod + hand-written ones like
   `CClassBuilderDoc::IsModified`). Bodies already emit a bool expression.
2. Apply the `*.bool.h` macros over `Include/CB_IteratorMulti.h` +
   `Include/CB_IteratorStaticMulti.h`.
3. Fold the IsClassMethod codegen change into `new3.cbz`.
4. Build CB; fix any caller that used the int value as an int (`== 1`, arithmetic).
   Call-site search: `Select-String 'Iterator \w+\([^)]*&\w+::'`.
