#ifndef CB_STATIC_MULTI_H
#define CB_STATIC_MULTI_H

#include "CB_IteratorStaticMulti.h"

// defines for include files
#define RELATION_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    static CB_PTR(ClassTo) _first##NameTo;\
    static CB_PTR(ClassTo) _last##NameTo;\
    static int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    static void Add##NameTo##First(ClassTo* item);\
    static void Add##NameTo##Last(ClassTo* item);\
    static void Add##NameTo##After(ClassTo* item, ClassTo* pos);\
    static void Add##NameTo##Before(ClassTo* item, ClassTo* pos);\
    static void Remove##NameTo(ClassTo* item);\
    static void RemoveAll##NameTo();\
    static void DeleteAll##NameTo();\
    static void Replace##NameTo(ClassTo* item, ClassTo* newItem);\
    static ClassTo* GetFirst##NameTo();\
    static ClassTo* GetLast##NameTo();\
    static ClassTo* GetNext##NameTo(ClassTo* pos);\
    static ClassTo* GetPrev##NameTo(ClassTo* pos);\
    static int Get##NameTo##Count();\
    static int Includes##NameTo(ClassTo* item);\
    static void Move##NameTo##First(ClassTo* item);\
    static void Move##NameTo##Last(ClassTo* item);\
    static void Move##NameTo##After(ClassTo* item, ClassTo* pos);\
    static void Move##NameTo##Before(ClassTo* item, ClassTo* pos);\
    static void Sort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    static void MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    ITERATOR_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_NOFILTER_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    static CB_PTR(ClassTo) _first##NameTo;\
    static CB_PTR(ClassTo) _last##NameTo;\
    static int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    static void Add##NameTo##First(ClassTo* item);\
    static void Add##NameTo##Last(ClassTo* item);\
    static void Add##NameTo##After(ClassTo* item, ClassTo* pos);\
    static void Add##NameTo##Before(ClassTo* item, ClassTo* pos);\
    static void Remove##NameTo(ClassTo* item);\
    static void RemoveAll##NameTo();\
    static void DeleteAll##NameTo();\
    static void Replace##NameTo(ClassTo* item, ClassTo* newItem);\
    static ClassTo* GetFirst##NameTo();\
    static ClassTo* GetLast##NameTo();\
    static ClassTo* GetNext##NameTo(ClassTo* pos);\
    static ClassTo* GetPrev##NameTo(ClassTo* pos);\
    static int Get##NameTo##Count();\
    static int Includes##NameTo(ClassTo* item);\
    static void Move##NameTo##First(ClassTo* item);\
    static void Move##NameTo##Last(ClassTo* item);\
    static void Move##NameTo##After(ClassTo* item, ClassTo* pos);\
    static void Move##NameTo##Before(ClassTo* item, ClassTo* pos);\
    static void Sort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    static void MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    ITERATOR_NOFILTER_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_STATIC_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassTo) _prev##NameFrom;\
    CB_PTR(ClassTo) _next##NameFrom;

// defines implementation
#define INIT_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) 

#define EXIT_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define REPLACE_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define INIT_STATIC_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _prev##NameFrom = (ClassTo*)0;\
    _next##NameFrom = (ClassTo*)0;

#define EXIT_STATIC_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    if (_prev##NameFrom || _next##NameFrom || ClassFrom::GetFirst##NameTo() == this)\
    {\
        ClassFrom::NameTo##Iterator::Check(this);\
\
        ClassFrom::_count##NameTo--;\
\
        if (_next##NameFrom)\
            _next##NameFrom->_prev##NameFrom = _prev##NameFrom;\
        else\
            ClassFrom::_last##NameTo = _prev##NameFrom;\
\
        if (_prev##NameFrom)\
            _prev##NameFrom->_next##NameFrom = _next##NameFrom;\
        else\
            ClassFrom::_first##NameTo = _next##NameFrom;\
\
        _prev##NameFrom = (ClassTo*)0;\
        _next##NameFrom = (ClassTo*)0;\
    }

#define REPLACE_STATIC_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    assert(pOld);\
    if ((pOld->_prev##NameFrom != (ClassTo*)0) ||\
        (pOld->_next##NameFrom != (ClassTo*)0) ||\
        (ClassFrom::GetFirst##NameTo() == pOld))\
    {\
        ClassFrom::NameTo##Iterator::Check(pOld, this);\
\
        if (pOld->_next##NameFrom)\
            pOld->_next##NameFrom->_prev##NameFrom = this;\
        else\
            ClassFrom::_last##NameTo = this;\
\
        if (pOld->_prev##NameFrom)\
            pOld->_prev##NameFrom->_next##NameFrom = this;\
        else\
            ClassFrom::_first##NameTo = this;\
\
        _next##NameFrom = pOld->_next##NameFrom;\
        _prev##NameFrom = pOld->_prev##NameFrom;\
\
        pOld->_next##NameFrom = (ClassTo*)0;\
        pOld->_prev##NameFrom = (ClassTo*)0;\
    }\
    else\
    {\
        _prev##NameFrom = (ClassTo*)0;\
        _next##NameFrom = (ClassTo*)0;\
    }

#define METHODS_STATIC_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    ClassTo* ClassFrom::_first##NameTo = (ClassTo*)0;\
    ClassTo* ClassFrom::_last##NameTo = (ClassTo*)0;\
    int ClassFrom::_count##NameTo = 0;\
void ClassFrom::Add##NameTo##First(ClassTo* item)\
{\
    METHOD_STATIC_MULTI_ADDFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Add##NameTo##Last(ClassTo* item)\
{\
    METHOD_STATIC_MULTI_ADDLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Add##NameTo##After(ClassTo* item, ClassTo* pos)\
{\
    METHOD_STATIC_MULTI_ADDAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Add##NameTo##Before(ClassTo* item, ClassTo* pos)\
{\
    METHOD_STATIC_MULTI_ADDBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Remove##NameTo(ClassTo* item)\
{\
    METHOD_STATIC_MULTI_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::RemoveAll##NameTo()\
{\
    METHOD_STATIC_MULTI_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::DeleteAll##NameTo()\
{\
    METHOD_STATIC_MULTI_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Replace##NameTo(ClassTo* item, ClassTo* newItem)\
{\
    METHOD_STATIC_MULTI_REPLACE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetFirst##NameTo()\
{\
    METHOD_STATIC_MULTI_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetLast##NameTo()\
{\
    METHOD_STATIC_MULTI_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetNext##NameTo(ClassTo* pos)\
{\
    METHOD_STATIC_MULTI_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetPrev##NameTo(ClassTo* pos)\
{\
    METHOD_STATIC_MULTI_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
int ClassFrom::Get##NameTo##Count()\
{\
    METHOD_STATIC_MULTI_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
int ClassFrom::Includes##NameTo(ClassTo* item)\
{\
    METHOD_STATIC_MULTI_INCLUDES(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##First(ClassTo* item)\
{\
    METHOD_STATIC_MULTI_MOVEFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##Last(ClassTo* item)\
{\
    METHOD_STATIC_MULTI_MOVELAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##After(ClassTo* item, ClassTo* pos)\
{\
    METHOD_STATIC_MULTI_MOVEAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##Before(ClassTo* item, ClassTo* pos)\
{\
    METHOD_STATIC_MULTI_MOVEBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Sort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
{\
    METHOD_STATIC_MULTI_SORT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
{\
    METHOD_STATIC_MULTI_MERGESORT(ClassFrom, NameFrom, ClassTo, NameTo) \
}

#define METHOD_STATIC_MULTI_ADDFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert(item->_prev##NameFrom == (ClassTo*)0 &&\
            item->_next##NameFrom == (ClassTo*)0 &&\
            (_first##NameTo != item));\
\
    _count##NameTo++;\
\
    if (_first##NameTo)\
    {\
        _first##NameTo->_prev##NameFrom = item;\
        item->_next##NameFrom = _first##NameTo;\
        _first##NameTo = item;\
    }\
    else\
        _first##NameTo = _last##NameTo = item;

#define METHOD_STATIC_MULTI_ADDLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert(item->_prev##NameFrom == (ClassTo*)0 &&\
            item->_next##NameFrom == (ClassTo*)0 &&\
            (_first##NameTo != item));\
\
    _count##NameTo++;\
\
    if (_last##NameTo)\
    {\
        _last##NameTo->_next##NameFrom = item;\
        item->_prev##NameFrom = _last##NameTo;\
        _last##NameTo = item;\
    }\
    else\
        _first##NameTo = _last##NameTo = item;

#define METHOD_STATIC_MULTI_ADDAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(pos);\
    assert((pos->_prev##NameFrom != (ClassTo*)0) ||\
            (pos->_next##NameFrom != (ClassTo*)0) ||\
            (_first##NameTo == pos));\
\
    assert(item);\
    assert(item->_prev##NameFrom == (ClassTo*)0 &&\
            item->_next##NameFrom == (ClassTo*)0 &&\
            (_first##NameTo != item));\
\
    _count##NameTo++;\
\
    item->_prev##NameFrom = pos;\
    item->_next##NameFrom = pos->_next##NameFrom;\
    pos->_next##NameFrom  = item;\
\
    if (item->_next##NameFrom)\
        item->_next##NameFrom->_prev##NameFrom = item;\
    else\
        _last##NameTo = item;

#define METHOD_STATIC_MULTI_ADDBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(pos);\
    assert((pos->_prev##NameFrom != (ClassTo*)0) ||\
            (pos->_next##NameFrom != (ClassTo*)0) ||\
            (_first##NameTo == pos));\
\
    assert(item);\
\
    assert(item->_prev##NameFrom == (ClassTo*)0 &&\
            item->_next##NameFrom == (ClassTo*)0 &&\
            (_first##NameTo != item));\
\
    _count##NameTo++;\
\
    item->_next##NameFrom = pos;\
    item->_prev##NameFrom = pos->_prev##NameFrom;\
    pos->_prev##NameFrom  = item;\
\
    if (item->_prev##NameFrom)\
        item->_prev##NameFrom->_next##NameFrom = item;\
    else\
        _first##NameTo = item;

#define METHOD_STATIC_MULTI_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert((item->_prev##NameFrom != (ClassTo*)0) ||\
            (item->_next##NameFrom != (ClassTo*)0) ||\
            (_first##NameTo == item));\
\
    ClassFrom::NameTo##Iterator::Check(item);\
\
    _count##NameTo--;\
\
    if (item->_next##NameFrom)\
        item->_next##NameFrom->_prev##NameFrom = item->_prev##NameFrom;\
    else\
        _last##NameTo = item->_prev##NameFrom;\
\
    if (item->_prev##NameFrom)\
        item->_prev##NameFrom->_next##NameFrom = item->_next##NameFrom;\
    else\
        _first##NameTo = item->_next##NameFrom;\
\
    item->_prev##NameFrom = (ClassTo*)0;\
    item->_next##NameFrom = (ClassTo*)0;

#define METHOD_STATIC_MULTI_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    { ClassTo* item = _first##NameTo;\
      ClassFrom::NameTo##Iterator::CheckAll();\
      _first##NameTo = (ClassTo*)0;\
      _last##NameTo = (ClassTo*)0;\
      _count##NameTo = 0;\
      while (item)\
      { ClassTo* next = item->_next##NameFrom;\
        item->_prev##NameFrom = (ClassTo*)0;\
        item->_next##NameFrom = (ClassTo*)0;\
        item = next;\
      }\
    }

#define METHOD_STATIC_MULTI_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    while (ClassTo* item = GetFirst##NameTo())\
        delete item;

#define METHOD_STATIC_MULTI_REPLACE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert((item->_prev##NameFrom != (ClassTo*)0) ||\
            (item->_next##NameFrom != (ClassTo*)0) ||\
            (_first##NameTo == item));\
\
    assert(newItem);\
    assert(newItem->_prev##NameFrom == (ClassTo*)0 &&\
            newItem->_next##NameFrom == (ClassTo*)0 &&\
            (_first##NameTo != newItem));\
\
    ClassFrom::NameTo##Iterator::Check(item, newItem);\
\
    if (item->_next##NameFrom)\
        item->_next##NameFrom->_prev##NameFrom = newItem;\
    else\
        _last##NameTo = newItem;\
\
    if (item->_prev##NameFrom)\
        item->_prev##NameFrom->_next##NameFrom = newItem;\
    else\
        _first##NameTo = newItem;\
\
    newItem->_next##NameFrom = item->_next##NameFrom;\
    newItem->_prev##NameFrom = item->_prev##NameFrom;\
    item->_next##NameFrom = (ClassTo*)0;\
    item->_prev##NameFrom = (ClassTo*)0;

#define METHOD_STATIC_MULTI_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    return _first##NameTo;

#define METHOD_STATIC_MULTI_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    return _last##NameTo;

#define METHOD_STATIC_MULTI_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (pos == (ClassTo*)0)\
        return _first##NameTo;\
\
    assert((pos->_prev##NameFrom != (ClassTo*)0) ||\
            (pos->_next##NameFrom != (ClassTo*)0) ||\
            (_first##NameTo == pos));\
\
    return pos->_next##NameFrom;

#define METHOD_STATIC_MULTI_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (pos == (ClassTo*)0)\
        return _last##NameTo;\
\
    assert((pos->_prev##NameFrom != (ClassTo*)0) ||\
            (pos->_next##NameFrom != (ClassTo*)0) ||\
            (_first##NameTo == pos));\
\
    return pos->_prev##NameFrom;

#define METHOD_STATIC_MULTI_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    return _count##NameTo;

#define METHOD_STATIC_MULTI_INCLUDES(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    if (item->_prev##NameFrom || item->_next##NameFrom || (_first##NameTo == item))\
        return 1;\
\
    return 0;

#define METHOD_STATIC_MULTI_MOVEFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    Remove##NameTo(item);\
    Add##NameTo##First(item);

#define METHOD_STATIC_MULTI_MOVELAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    Remove##NameTo(item);\
    Add##NameTo##Last(item);

#define METHOD_STATIC_MULTI_MOVEAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    Remove##NameTo(item);\
    Add##NameTo##After(item, pos);

#define METHOD_STATIC_MULTI_MOVEBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    Remove##NameTo(item);\
    Add##NameTo##Before(item, pos);

#define METHOD_STATIC_MULTI_SORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (_first##NameTo && _first##NameTo->_next##NameFrom)\
    {\
        ClassFrom::NameTo##Iterator::CheckAll();\
        /* Bubble sort: adjacent-pair compare-and-swap. Each iteration */\
        /* performs at most one discrete list mutation. Static MULTI */\
        /* items aren't part of the undo system, so this variant has no */\
        /* undo-related hooks — see MergeSort##NameTo (METHOD_STATIC_- */\
        /* MULTI_MERGESORT) for the O(N log N) equivalent. */\
        bool changed = true;\
        while (changed)\
        {\
            changed = false;\
            ClassTo* node = _first##NameTo;\
            while (node && node->_next##NameFrom)\
            {\
                ClassTo* next = node->_next##NameFrom;\
                if (comp(node, next) > 0)\
                {\
                    ClassTo* before = node->_prev##NameFrom;\
                    ClassTo* after  = next->_next##NameFrom;\
                    if (before) before->_next##NameFrom = next;\
                    else _first##NameTo = next;\
                    next->_prev##NameFrom = before;\
                    next->_next##NameFrom = node;\
                    node->_prev##NameFrom = next;\
                    node->_next##NameFrom = after;\
                    if (after) after->_prev##NameFrom = node;\
                    else _last##NameTo = node;\
                    changed = true;\
                }\
                else\
                {\
                    node = next;\
                }\
            }\
        }\
    }

/* O(N log N) bottom-up merge sort for static MULTI lists. */
#define METHOD_STATIC_MULTI_MERGESORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    /* Only sort if list has 2+ elements */\
    if (_first##NameTo && _first##NameTo->_next##NameFrom)\
    {\
        ClassFrom::NameTo##Iterator::CheckAll();\
        /* Bottom-up merge sort: iterate through doubling run sizes (1, 2, 4, 8, ...) */\
        for (int runSize = 1; runSize < _count##NameTo; runSize <<= 1)\
        {\
            ClassTo* unmerged = _first##NameTo;        /* Unsorted portion head */\
            ClassTo* sortedHead = (ClassTo*)0;         /* Head of sorted list */\
            ClassTo** insertPoint = &sortedHead;       /* Where to append next node */\
            \
            /* Merge runs of size runSize */\
            while (unmerged)\
            {\
                ClassTo* leftNode = unmerged;          /* Left segment start */\
                ClassTo* rightNode = unmerged;         /* Right segment start */\
                int leftCount = 0, rightCount = 0;     /* Elements remaining in each segment */\
                \
                /* Advance rightNode to start of right segment, count left elements */\
                for (int idx = 0; idx < runSize && rightNode; idx++, leftCount++) \
                    rightNode = rightNode->_next##NameFrom;\
                unmerged = rightNode;\
                \
                /* Advance unmerged to end of right segment, count right elements */\
                for (int idx = 0; idx < runSize && unmerged; idx++, rightCount++) \
                    unmerged = unmerged->_next##NameFrom;\
                \
                /* Merge left and right segments in sorted order */\
                while (leftCount > 0 || rightCount > 0)\
                {\
                    ClassTo* selected;\
                    /* Both segments depleted: shouldn't happen, handled by counts */\
                    if (leftCount == 0)\
                        { selected = rightNode; rightNode = rightNode->_next##NameFrom; rightCount--; }\
                    /* Left depleted or left <= right: take from left */\
                    else if (rightCount == 0 || comp(leftNode, rightNode) <= 0)\
                        { selected = leftNode; leftNode = leftNode->_next##NameFrom; leftCount--; }\
                    /* Right has smaller element: take from right */\
                    else\
                        { selected = rightNode; rightNode = rightNode->_next##NameFrom; rightCount--; }\
                    *insertPoint = selected;\
                    insertPoint = &selected->_next##NameFrom;\
                }\
            }\
            *insertPoint = (ClassTo*)0;\
            _first##NameTo = sortedHead;\
        }\
        /* Restore backward pointers throughout the sorted list */\
        ClassTo* node = _first##NameTo;\
        node->_prev##NameFrom = (ClassTo*)0;\
        while (node->_next##NameFrom)\
        { node->_next##NameFrom->_prev##NameFrom = node; node = node->_next##NameFrom; }\
        _last##NameTo = node;\
    }

#define METHODS_STATIC_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#endif
