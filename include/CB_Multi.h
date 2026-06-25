#ifndef CB_MULTI_H
#define CB_MULTI_H

#include "CB_IteratorMulti.h"

// defines for include files
#define RELATION_TEMPLATE_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassTo) _first##NameTo;\
    CB_PTR(ClassTo) _last##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo##First(ClassTo* item)\
    {\
        METHOD_MULTI_ADDFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Add##NameTo##Last(ClassTo* item)\
    {\
        METHOD_MULTI_ADDLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Add##NameTo##After(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_ADDAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Add##NameTo##Before(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_ADDBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Remove##NameTo(ClassTo* item)\
    {\
        METHOD_MULTI_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem)\
    {\
        METHOD_MULTI_REPLACE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void DeleteAll##NameTo()\
    {\
        METHOD_MULTI_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetFirst##NameTo() const\
    {\
        METHOD_MULTI_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetLast##NameTo() const\
    {\
        METHOD_MULTI_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetNext##NameTo(ClassTo* pos) const\
    {\
        METHOD_MULTI_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const\
    {\
        METHOD_MULTI_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    int Get##NameTo##Count() const\
    {\
        METHOD_MULTI_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##First(ClassTo* item)\
    {\
        METHOD_MULTI_MOVEFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##Last(ClassTo* item)\
    {\
        METHOD_MULTI_MOVELAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##After(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_MOVEAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##Before(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_MOVEBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Sort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
    {\
        METHOD_MULTI_SORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
    {\
        METHOD_MULTI_MERGESORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ITERATOR_TEMPLATE_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_TEMPLATE_NOFILTER_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassTo) _first##NameTo;\
    CB_PTR(ClassTo) _last##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo##First(ClassTo* item)\
    {\
        METHOD_MULTI_ADDFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Add##NameTo##Last(ClassTo* item)\
    {\
        METHOD_MULTI_ADDLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Add##NameTo##After(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_ADDAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Add##NameTo##Before(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_ADDBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Remove##NameTo(ClassTo* item)\
    {\
        METHOD_MULTI_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem)\
    {\
        METHOD_MULTI_REPLACE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void DeleteAll##NameTo()\
    {\
        METHOD_MULTI_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetFirst##NameTo() const\
    {\
        METHOD_MULTI_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetLast##NameTo() const\
    {\
        METHOD_MULTI_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetNext##NameTo(ClassTo* pos) const\
    {\
        METHOD_MULTI_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const\
    {\
        METHOD_MULTI_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    int Get##NameTo##Count() const\
    {\
        METHOD_MULTI_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##First(ClassTo* item)\
    {\
        METHOD_MULTI_MOVEFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##Last(ClassTo* item)\
    {\
        METHOD_MULTI_MOVELAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##After(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_MOVEAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Move##NameTo##Before(ClassTo* item, ClassTo* pos)\
    {\
        METHOD_MULTI_MOVEBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Sort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
    {\
        METHOD_MULTI_SORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
    {\
        METHOD_MULTI_MERGESORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ITERATOR_TEMPLATE_NOFILTER_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassTo) _first##NameTo;\
    CB_PTR(ClassTo) _last##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo##First(ClassTo* item);\
    void Add##NameTo##Last(ClassTo* item);\
    void Add##NameTo##After(ClassTo* item, ClassTo* pos);\
    void Add##NameTo##Before(ClassTo* item, ClassTo* pos);\
    void Remove##NameTo(ClassTo* item);\
    void RemoveAll##NameTo();\
    void DeleteAll##NameTo();\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem);\
    ClassTo* GetFirst##NameTo() const;\
    ClassTo* GetLast##NameTo() const;\
    ClassTo* GetNext##NameTo(ClassTo* pos) const;\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const;\
    int Get##NameTo##Count() const;\
    void Move##NameTo##First(ClassTo* item);\
    void Move##NameTo##Last(ClassTo* item);\
    void Move##NameTo##After(ClassTo* item, ClassTo* pos);\
    void Move##NameTo##Before(ClassTo* item, ClassTo* pos);\
    void Sort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    void MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    ITERATOR_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_NOFILTER_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassTo) _first##NameTo;\
    CB_PTR(ClassTo) _last##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo##First(ClassTo* item);\
    void Add##NameTo##Last(ClassTo* item);\
    void Add##NameTo##After(ClassTo* item, ClassTo* pos);\
    void Add##NameTo##Before(ClassTo* item, ClassTo* pos);\
    void Remove##NameTo(ClassTo* item);\
    void RemoveAll##NameTo();\
    void DeleteAll##NameTo();\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem);\
    ClassTo* GetFirst##NameTo() const;\
    ClassTo* GetLast##NameTo() const;\
    ClassTo* GetNext##NameTo(ClassTo* pos) const;\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const;\
    int Get##NameTo##Count() const;\
    void Move##NameTo##First(ClassTo* item);\
    void Move##NameTo##Last(ClassTo* item);\
    void Move##NameTo##After(ClassTo* item, ClassTo* pos);\
    void Move##NameTo##Before(ClassTo* item, ClassTo* pos);\
    void Sort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    void MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*));\
    ITERATOR_NOFILTER_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassFrom) _ref##NameFrom;\
    CB_PTR(ClassTo) _prev##NameFrom;\
    CB_PTR(ClassTo) _next##NameFrom;\
\
public:\
    ClassFrom* Get##NameFrom() const { return _ref##NameFrom; };

// defines implementation
#define INIT_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _first##NameTo = (ClassTo*)0;\
    _last##NameTo = (ClassTo*)0;\
    _count##NameTo = 0;

#define EXIT_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    {\
        ClassFrom::NameTo##Iterator::CheckAll(this);\
        ClassTo* item = _first##NameTo;\
        while (item)\
        {\
            ClassTo* next = item->_next##NameFrom;\
            item->_ref##NameFrom  = (ClassFrom*)0;\
            item->_prev##NameFrom = (ClassTo*)0;\
            item->_next##NameFrom = (ClassTo*)0;\
            item = next;\
        }\
        _first##NameTo = (ClassTo*)0;\
        _last##NameTo  = (ClassTo*)0;\
        _count##NameTo = 0;\
    }

#define REPLACE_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _first##NameTo = pOld->_first##NameTo;\
    _last##NameTo = pOld->_last##NameTo;\
    _count##NameTo = pOld->_count##NameTo;\
    pOld->_first##NameTo = (ClassTo*)0;\
    pOld->_last##NameTo = (ClassTo*)0;\
    pOld->_count##NameTo = 0;\
    { for (ClassTo* item = GetFirst##NameTo(); item; item = GetNext##NameTo(item))\
          item->_ref##NameFrom = this; }

#define INIT_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _ref##NameFrom = (ClassFrom*)0;\
    _prev##NameFrom = (ClassTo*)0;\
    _next##NameFrom = (ClassTo*)0;

#define EXIT_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    if (_ref##NameFrom)\
    {\
        ClassFrom::NameTo##Iterator::Check(this);\
\
        _ref##NameFrom->_count##NameTo--;\
\
        if (_next##NameFrom)\
            _next##NameFrom->_prev##NameFrom = _prev##NameFrom;\
        else\
            _ref##NameFrom->_last##NameTo = _prev##NameFrom;\
\
        if (_prev##NameFrom)\
            _prev##NameFrom->_next##NameFrom = _next##NameFrom;\
        else\
            _ref##NameFrom->_first##NameTo = _next##NameFrom;\
\
        _prev##NameFrom = (ClassTo*)0;\
        _next##NameFrom = (ClassTo*)0;\
        _ref##NameFrom = (ClassFrom*)0;\
    }

#define REPLACE_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    assert(pOld);\
    if (pOld->_ref##NameFrom)\
    {\
        ClassFrom::NameTo##Iterator::Check(pOld, this);\
\
        _ref##NameFrom = pOld->_ref##NameFrom;\
\
        if (pOld->_next##NameFrom)\
            pOld->_next##NameFrom->_prev##NameFrom = this;\
        else\
            _ref##NameFrom->_last##NameTo = this;\
\
        if (pOld->_prev##NameFrom)\
            pOld->_prev##NameFrom->_next##NameFrom = this;\
        else\
            _ref##NameFrom->_first##NameTo = this;\
\
        _next##NameFrom = pOld->_next##NameFrom;\
        _prev##NameFrom = pOld->_prev##NameFrom;\
\
        pOld->_ref##NameFrom = (ClassFrom*)0;\
        pOld->_next##NameFrom = (ClassTo*)0;\
        pOld->_prev##NameFrom = (ClassTo*)0;\
    }\
    else\
    {\
        _ref##NameFrom = (ClassFrom*)0;\
        _prev##NameFrom = (ClassTo*)0;\
        _next##NameFrom = (ClassTo*)0;\
    }

#define REMOVE_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    { for (ClassTo* item = GetFirst##NameTo(); item; item = GetFirst##NameTo())\
      {\
          (void)new UndoSubChange(item);\
          Remove##NameTo(item);\
      } }

#define SAVE_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    p##ClassTo->_ref##NameFrom = _ref##NameFrom;\
    p##ClassTo->_prev##NameFrom = _prev##NameFrom;\
    p##ClassTo->_next##NameFrom = _next##NameFrom;

#define RESTORE_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (p##ClassTo == this)\
    {\
        if (_ref##NameFrom)\
        {\
            _ref##NameFrom->_count##NameTo++;\
\
            assert(\
            (_prev##NameFrom && _next##NameFrom && \
            _prev##NameFrom->_next##NameFrom == _next##NameFrom && \
            _next##NameFrom->_prev##NameFrom == _prev##NameFrom) || \
            (!_prev##NameFrom && _next##NameFrom && \
            _ref##NameFrom->_first##NameTo == _next##NameFrom) || \
            (_prev##NameFrom && !_next##NameFrom && \
            _ref##NameFrom->_last##NameTo == _prev##NameFrom) || \
            (!_prev##NameFrom && !_next##NameFrom && \
            !_ref##NameFrom->_first##NameTo && !_ref##NameFrom->_last##NameTo));\
            if (_prev##NameFrom)\
                _prev##NameFrom->_next##NameFrom = this;\
            else\
                _ref##NameFrom->_first##NameTo = this;\
\
            if (_next##NameFrom)\
                _next##NameFrom->_prev##NameFrom = this;\
            else\
                _ref##NameFrom->_last##NameTo = this;\
        }\
    }\
    else if (_ref##NameFrom != p##ClassTo->_ref##NameFrom || \
             _prev##NameFrom != p##ClassTo->_prev##NameFrom || \
             _next##NameFrom != p##ClassTo->_next##NameFrom )\
    {\
        if (_ref##NameFrom)\
        {\
            ClassFrom::NameTo##Iterator::Check(this);\
\
            _ref##NameFrom->_count##NameTo--;\
\
            if (_next##NameFrom)\
                _next##NameFrom->_prev##NameFrom = _prev##NameFrom;\
            else\
                _ref##NameFrom->_last##NameTo = _prev##NameFrom;\
\
            if (_prev##NameFrom)\
                _prev##NameFrom->_next##NameFrom = _next##NameFrom;\
            else\
                _ref##NameFrom->_first##NameTo = _next##NameFrom;\
        }\
        _ref##NameFrom = p##ClassTo->_ref##NameFrom;\
        _prev##NameFrom = p##ClassTo->_prev##NameFrom;\
        _next##NameFrom = p##ClassTo->_next##NameFrom;\
        if (_ref##NameFrom)\
        {\
            _ref##NameFrom->_count##NameTo++;\
\
            assert(\
            (_prev##NameFrom && _next##NameFrom && \
            _prev##NameFrom->_next##NameFrom == _next##NameFrom && \
            _next##NameFrom->_prev##NameFrom == _prev##NameFrom) || \
            (!_prev##NameFrom && _next##NameFrom && \
            _ref##NameFrom->_first##NameTo == _next##NameFrom) || \
            (_prev##NameFrom && !_next##NameFrom && \
            _ref##NameFrom->_last##NameTo == _prev##NameFrom) || \
            (!_prev##NameFrom && !_next##NameFrom && \
            !_ref##NameFrom->_first##NameTo && !_ref##NameFrom->_last##NameTo));\
            if (_prev##NameFrom)\
                _prev##NameFrom->_next##NameFrom = this;\
            else\
                _ref##NameFrom->_first##NameTo = this;\
        \
            if (_next##NameFrom)\
                _next##NameFrom->_prev##NameFrom = this;\
            else\
                _ref##NameFrom->_last##NameTo = this;\
        }\
    }

#define REMOVE_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (_ref##NameFrom)\
    {\
        ClassFrom::NameTo##Iterator::Check(this);\
\
        _ref##NameFrom->_count##NameTo--;\
\
        if (_next##NameFrom)\
            _next##NameFrom->_prev##NameFrom = _prev##NameFrom;\
        else\
            _ref##NameFrom->_last##NameTo = _prev##NameFrom;\
\
        if (_prev##NameFrom)\
            _prev##NameFrom->_next##NameFrom = _next##NameFrom;\
        else\
            _ref##NameFrom->_first##NameTo = _next##NameFrom;\
    }

#define CLEANUP_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _ref##NameFrom = (ClassFrom*)0;\
    _prev##NameFrom = (ClassTo*)0;\
    _next##NameFrom = (ClassTo*)0;

#define METHODS_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
void ClassFrom::Add##NameTo##First(ClassTo* item)\
{\
    METHOD_MULTI_ADDFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Add##NameTo##Last(ClassTo* item)\
{\
    METHOD_MULTI_ADDLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Add##NameTo##After(ClassTo* item, ClassTo* pos)\
{\
    METHOD_MULTI_ADDAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Add##NameTo##Before(ClassTo* item, ClassTo* pos)\
{\
    METHOD_MULTI_ADDBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Remove##NameTo(ClassTo* item)\
{\
    METHOD_MULTI_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::RemoveAll##NameTo()\
{\
    METHOD_MULTI_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::DeleteAll##NameTo()\
{\
    METHOD_MULTI_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Replace##NameTo(ClassTo* item, ClassTo* newItem)\
{\
    METHOD_MULTI_REPLACE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetFirst##NameTo() const\
{\
    METHOD_MULTI_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetLast##NameTo() const\
{\
    METHOD_MULTI_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetNext##NameTo(ClassTo* pos) const\
{\
    METHOD_MULTI_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetPrev##NameTo(ClassTo* pos) const\
{\
    METHOD_MULTI_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
int ClassFrom::Get##NameTo##Count() const\
{\
    METHOD_MULTI_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##First(ClassTo* item)\
{\
    METHOD_MULTI_MOVEFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##Last(ClassTo* item)\
{\
    METHOD_MULTI_MOVELAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##After(ClassTo* item, ClassTo* pos)\
{\
    METHOD_MULTI_MOVEAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Move##NameTo##Before(ClassTo* item, ClassTo* pos)\
{\
    METHOD_MULTI_MOVEBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Sort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
{\
    METHOD_MULTI_SORT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::MergeSort##NameTo(int (*comp)(ClassTo*, ClassTo*))\
{\
    METHOD_MULTI_MERGESORT(ClassFrom, NameFrom, ClassTo, NameTo) \
}

#define METHOD_MULTI_ADDFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == (ClassFrom*)0);\
\
    _count##NameTo++;\
\
    item->_ref##NameFrom = this;\
\
    if (_first##NameTo)\
    {\
        _first##NameTo->_prev##NameFrom = item;\
        item->_next##NameFrom = _first##NameTo;\
        _first##NameTo = item;\
    }\
    else\
        _first##NameTo = _last##NameTo = item;

#define METHOD_MULTI_ADDLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == (ClassFrom*)0);\
\
    _count##NameTo++;\
\
    item->_ref##NameFrom = this;\
\
    if (_last##NameTo)\
    {\
        _last##NameTo->_next##NameFrom = item;\
        item->_prev##NameFrom = _last##NameTo;\
        _last##NameTo = item;\
    }\
    else\
        _first##NameTo = _last##NameTo = item;

#define METHOD_MULTI_ADDAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == (ClassFrom*)0);\
\
    assert(pos);\
    assert(pos->_ref##NameFrom == this);\
\
    _count##NameTo++;\
\
    item->_ref##NameFrom = this;\
    item->_prev##NameFrom = pos;\
    item->_next##NameFrom = pos->_next##NameFrom;\
    pos->_next##NameFrom  = item;\
\
    if (item->_next##NameFrom)\
        item->_next##NameFrom->_prev##NameFrom = item;\
    else\
        _last##NameTo = item;

#define METHOD_MULTI_ADDBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == (ClassFrom*)0);\
\
    assert(pos);\
    assert(pos->_ref##NameFrom == this);\
\
    _count##NameTo++;\
\
    item->_ref##NameFrom = this;\
    item->_next##NameFrom = pos;\
    item->_prev##NameFrom = pos->_prev##NameFrom;\
    pos->_prev##NameFrom  = item;\
\
    if (item->_prev##NameFrom)\
        item->_prev##NameFrom->_next##NameFrom = item;\
    else\
        _first##NameTo = item;

#define METHOD_MULTI_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == this);\
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
    item->_next##NameFrom = (ClassTo*)0;\
    item->_ref##NameFrom = (ClassFrom*)0;

#define METHOD_MULTI_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    ClassFrom::NameTo##Iterator::CheckAll(this);\
    {\
        ClassTo* item = _first##NameTo;\
        while (item)\
        {\
            ClassTo* next = item->_next##NameFrom;\
            item->_ref##NameFrom  = (ClassFrom*)0;\
            item->_prev##NameFrom = (ClassTo*)0;\
            item->_next##NameFrom = (ClassTo*)0;\
            item = next;\
        }\
    }\
    _first##NameTo = (ClassTo*)0;\
    _last##NameTo  = (ClassTo*)0;\
    _count##NameTo = 0;

#define METHOD_MULTI_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    while (ClassTo* item = GetLast##NameTo())\
        delete item;

#define METHOD_MULTI_REPLACE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == this);\
\
    assert(newItem);\
    assert(newItem->_ref##NameFrom == (ClassFrom*)0);\
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
    item->_prev##NameFrom = (ClassTo*)0;\
\
    item->_ref##NameFrom = (ClassFrom*)0;\
    newItem->_ref##NameFrom = this;

#define METHOD_MULTI_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    return _first##NameTo;

#define METHOD_MULTI_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    return _last##NameTo;

#define METHOD_MULTI_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    if (pos == (ClassTo*)0)\
        return _first##NameTo;\
\
    assert(pos);\
    assert(pos->_ref##NameFrom == this);\
\
    return pos->_next##NameFrom;

#define METHOD_MULTI_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    if (pos == (ClassTo*)0)\
        return _last##NameTo;\
\
    assert(pos);\
    assert(pos->_ref##NameFrom == this);\
\
    return pos->_prev##NameFrom;

#define METHOD_MULTI_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    return _count##NameTo;

#define METHOD_MULTI_MOVEFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert(item->_ref##NameFrom);\
    item->_ref##NameFrom->Remove##NameTo(item);\
    Add##NameTo##First(item);

#define METHOD_MULTI_MOVELAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert(item->_ref##NameFrom);\
    item->_ref##NameFrom->Remove##NameTo(item);\
    Add##NameTo##Last(item);

#define METHOD_MULTI_MOVEAFTER(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert(item->_ref##NameFrom);\
    item->_ref##NameFrom->Remove##NameTo(item);\
    Add##NameTo##After(item, pos);

#define METHOD_MULTI_MOVEBEFORE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(item);\
    assert(item->_ref##NameFrom);\
    item->_ref##NameFrom->Remove##NameTo(item);\
    Add##NameTo##Before(item, pos);

#define METHOD_MULTI_SORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (_count##NameTo > 1)\
    {\
        ClassFrom::NameTo##Iterator::CheckAll(this);\
        /* Bubble sort: adjacent-pair compare-and-swap. Each iteration */\
        /* performs at most one discrete list mutation, so a comparator */\
        /* that records its own undo state on swap (typically SaveState */\
        /* on the right operand plus a project-specific "close this */\
        /* sub-batch" call when result > 0) makes the reorder undoable */\
        /* as a sequence of one-pair sub-batches — the only multi- */\
        /* element reorder shape RESTORE_MULTI_PASSIVE can faithfully */\
        /* reverse. The macro itself stays project-agnostic and does */\
        /* not depend on any model-specific undo machinery. */\
        /* See MergeSort##NameTo (METHOD_MULTI_MERGESORT) for an */\
        /* O(N log N) variant that does NOT preserve undoability. */\
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
                    /* Swap adjacent pair (node, next): */\
                    /*   before <-> node <-> next <-> after  becomes */\
                    /*   before <-> next <-> node <-> after */\
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
                    /* node moved one position right; loop continues at */\
                    /* same node pointer (now positioned where next was), */\
                    /* so the next iteration compares (node, node->_next = after). */\
                }\
                else\
                {\
                    node = next;\
                }\
            }\
        }\
    }

/* Bottom-up merge sort — O(N log N) but NOT undo-safe for multi-element */
/* reorders. Use this for one-off sorts where undo isn't required (e.g. */
/* canonicalising on load) or where N is large enough that the bubble */
/* sort's O(N^2) is a real cost. The vast majority of CB-managed lists */
/* are small and benefit more from Sort##NameTo's undo guarantee. */
#define METHOD_MULTI_MERGESORT(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (_count##NameTo > 1)\
    {\
        ClassFrom::NameTo##Iterator::CheckAll(this);\
        /* Bottom-up merge sort: iterate through doubling run sizes (1, 2, 4, 8, ...) */\
        for (int runSize = 1; runSize < _count##NameTo; runSize <<= 1)\
        {\
            ClassTo* unmerged = _first##NameTo;            /* Unsorted portion */\
            ClassTo* sortedHead = (ClassTo*)0;             /* Head of sorted list */\
            ClassTo** insertPoint = &sortedHead;           /* Where to append next */\
            \
            /* Merge runs of size runSize */\
            while (unmerged)\
            {\
                ClassTo* leftNode = unmerged;              /* Left segment start */\
                ClassTo* rightNode = unmerged;             /* Right segment start */\
                int leftCount = 0, rightCount = 0;         /* Elements remaining in each segment */\
                \
                /* Advance right to start of right segment, count left elements */\
                for (int idx = 0; idx < runSize && rightNode; idx++, leftCount++)\
                    rightNode = rightNode->_next##NameFrom;\
                unmerged = rightNode;\
                \
                /* Advance to end of right segment, count right elements */\
                for (int idx = 0; idx < runSize && unmerged; idx++, rightCount++)\
                    unmerged = unmerged->_next##NameFrom;\
                \
                /* Merge left and right segments in sorted order using counters */\
                while (leftCount > 0 || rightCount > 0)\
                {\
                    ClassTo* selected;\
                    /* Left depleted: take from right */\
                    if (leftCount == 0)\
                    { selected = rightNode; rightNode = rightNode->_next##NameFrom; rightCount--; }\
                    /* Right depleted or left <= right: take from left */\
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
        { ClassTo* prevNode = (ClassTo*)0;\
          for (ClassTo* node = _first##NameTo; node; node = node->_next##NameFrom)\
          { node->_prev##NameFrom = prevNode; prevNode = node; _last##NameTo = node; }\
        }\
    }

#define METHODS_MULTI_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define SERIALIZE_ALL_OBJECTS(ObjectType, Document, DocumentName, DocumentBase, DocumentBaseName) \
    if (archive.IsStoring())\
    {\
        /* Write the number of objects to store.*/\
        archive << Get##DocumentBaseName##Count();\
\
        /* Reserve index = 0; for the NULL pointer*/\
        int index = 1;\
\
        /* Write the data of the objects*/\
        DocumentBase* object;\
        for (object = GetFirst##DocumentBaseName(); object; object = GetNext##DocumentBaseName(object))\
        {\
            object->_index = index++;\
            archive << object;\
        }\
\
        /* Write relations of Document*/\
        SerializeRelations(archive, NULL);\
\
        /* Write all other objects relations*/\
        for (object = GetFirst##DocumentBaseName(); object; object = GetNext##DocumentBaseName(object))\
        {\
            object->SerializeRelations(archive, NULL);\
        }\
   }\
   else\
   {\
        /* Read the number of objects stored*/\
        int objectCount;\
        archive >> objectCount;\
\
        /* Make an array for the pointer mapping*/\
        DocumentBase** pointerArray = new DocumentBase*[objectCount+1];\
        pointerArray[0] = NULL;\
\
        /* Read the data of the objects*/\
		int index ;\
\
        for (index = 1; index <= objectCount; index++)\
        {\
            ObjectType* pObjectType = 0;\
            archive >> pObjectType;\
            pointerArray[index] = (DocumentBase*)pObjectType;\
        }\
\
        /* Read relations of Document*/\
        SerializeRelations(archive, pointerArray);\
\
        /* Read all other objects relations*/\
        for (index = 1; index <= objectCount; index++)\
            pointerArray[index]->SerializeRelations(archive, pointerArray);\
\
        delete[] pointerArray;\
    }

#define WRITE_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
        archive << Get##NameTo##Count();\
        { for (ClassTo* item = GetFirst##NameTo(); item; item = GetNext##NameTo(item))\
              archive << item->_index; }

#define READ_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
            {\
                int count;\
                int index;\
                \
                archive >> count;\
                for (int i = 0; i < count; i++)\
                {\
                    archive >> index;\
                    Add##NameTo##Last(static_cast<ClassTo*>(pointerArray[index]));\
                }\
            }

#define DYNAMIC_READ_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
            {\
                int count;\
                int index;\
                \
                archive >> count;\
                for (int i = 0; i < count; i++)\
                {\
                    archive >> index;\
                    Add##NameTo##Last(dynamic_cast<ClassTo*>(pointerArray[index]));\
                }\
            }

#endif
