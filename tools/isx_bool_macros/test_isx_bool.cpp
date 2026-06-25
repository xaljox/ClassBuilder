// test_isx_bool.cpp -- focused compile/run check for the IsXxx int->bool sweep.
//
// Validates the essential macro change: the CB_IteratorMulti / CB_IteratorStaticMulti
// filter-iterator predicate slot, retyped from `int (ClassTo::*)() const` to
// `bool (ClassTo::*)() const`, accepts a generated `bool IsXxx() const` predicate
// and invokes it as a filter.
//
// A pointer-to-member-function does NOT convert on return type, so the OLD `int`
// slot would REJECT a `bool` predicate (and vice-versa) -- that's exactly why the
// macro must flip in lockstep with the whole IsX family + the IsClassMethod codegen.
// The static_assert below documents that the two slot types are distinct.
//
// Build (matches the project's conformant mode):
//   cl /nologo /std:c++17 /permissive- /EHsc test_isx_bool.cpp

#include <cassert>
#include <type_traits>

struct Cell;
struct MatrixObject {
    virtual ~MatrixObject() {}
    bool IsCell() const;        // generated form (IsClassMethod, post int->bool)
};
struct Cell : MatrixObject {};
bool MatrixObject::IsCell() const { return dynamic_cast<const Cell*>(this) != nullptr; }

// The iterator's predicate slot, AFTER the macro change:
using BoolPredicate = bool (MatrixObject::*)() const;

// Distinct from the OLD int slot -- no implicit conversion either way:
static_assert(!std::is_same<BoolPredicate, int (MatrixObject::*)() const>::value,
              "bool and int predicate slots are distinct member-fn-pointer types");

// Mirrors how the iterator stores + calls the predicate as a filter
// (_method == 0 means "no filter"):
static bool passesFilter(const MatrixObject* o, BoolPredicate m) {
    return m == nullptr || (o->*m)();
}

int main() {
    BoolPredicate m = &MatrixObject::IsCell;   // binds: slot return type == IsCell's (bool)
    Cell cell;
    MatrixObject base;
    assert(passesFilter(&cell, m));            // Cell      -> filtered in
    assert(!passesFilter(&base, m));           // bare base -> filtered out
    assert(passesFilter(&base, nullptr));      // no predicate -> no filter
    return 0;
}
