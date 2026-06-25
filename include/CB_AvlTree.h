#ifndef CB_AVLTREE_H
#define CB_AVLTREE_H

#include "CB_IteratorMulti.h"

// defines for include files
#define RELATION_TEMPLATE_AVLTREE_ACTIVE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
private:\
    CB_PTR(ClassTo) _top##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo(ClassTo* item)\
    {\
        METHOD_AVLTREE_ADD(member, ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Remove##NameTo(ClassTo* item)\
    {\
        METHOD_AVLTREE_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void RemoveAll##NameTo()\
    {\
        METHOD_AVLTREE_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void DeleteAll##NameTo()\
    {\
        METHOD_AVLTREE_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem)\
    {\
        METHOD_AVLTREE_REPLACE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetFirst##NameTo() const\
    {\
        METHOD_AVLTREE_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetLast##NameTo() const\
    {\
        METHOD_AVLTREE_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetNext##NameTo(ClassTo* pos) const\
    {\
        METHOD_AVLTREE_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const\
    {\
        METHOD_AVLTREE_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    int Get##NameTo##Count() const\
    {\
        METHOD_AVLTREE_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ITERATOR_TEMPLATE_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_TEMPLATE_NOFILTER_AVLTREE_ACTIVE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
private:\
    CB_PTR(ClassTo) _top##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo(ClassTo* item)\
    {\
        METHOD_AVLTREE_ADD(member, ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Remove##NameTo(ClassTo* item)\
    {\
        METHOD_AVLTREE_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void RemoveAll##NameTo()\
    {\
        METHOD_AVLTREE_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void DeleteAll##NameTo()\
    {\
        METHOD_AVLTREE_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem)\
    {\
        METHOD_AVLTREE_REPLACE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetFirst##NameTo() const\
    {\
        METHOD_AVLTREE_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetLast##NameTo() const\
    {\
        METHOD_AVLTREE_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetNext##NameTo(ClassTo* pos) const\
    {\
        METHOD_AVLTREE_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const\
    {\
        METHOD_AVLTREE_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    int Get##NameTo##Count() const\
    {\
        METHOD_AVLTREE_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    }\
    ITERATOR_TEMPLATE_NOFILTER_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
private:\
    CB_PTR(ClassTo) _top##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo(ClassTo* item);\
    void Remove##NameTo(ClassTo* item);\
    void RemoveAll##NameTo();\
    void DeleteAll##NameTo();\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem);\
    ClassTo* GetFirst##NameTo() const;\
    ClassTo* GetLast##NameTo() const;\
    ClassTo* GetNext##NameTo(ClassTo* pos) const;\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const;\
    int Get##NameTo##Count() const;\
    ITERATOR_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_NOFILTER_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
private:\
    CB_PTR(ClassTo) _top##NameTo;\
    int _count##NameTo;\
\
    static bool IsCritical##NameTo()  { return false; }\
\
public:\
    void Add##NameTo(ClassTo* item);\
    void Remove##NameTo(ClassTo* item);\
    void RemoveAll##NameTo();\
    void DeleteAll##NameTo();\
    void Replace##NameTo(ClassTo* item, ClassTo* newItem);\
    ClassTo* GetFirst##NameTo() const;\
    ClassTo* GetLast##NameTo() const;\
    ClassTo* GetNext##NameTo(ClassTo* pos) const;\
    ClassTo* GetPrev##NameTo(ClassTo* pos) const;\
    int Get##NameTo##Count() const;\
    ITERATOR_NOFILTER_MULTI_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#define RELATION_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
public:\
    CB_PTR(ClassFrom) _ref##NameFrom;\
    CB_PTR(ClassTo) _parent##NameFrom;\
    CB_PTR(ClassTo) _left##NameFrom;\
    CB_PTR(ClassTo) _right##NameFrom;\
    intptr_t _bal##NameFrom;\
\
public:\
    ClassFrom* Get##NameFrom() const { return _ref##NameFrom; };

// defines implementation
#define INIT_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _top##NameTo = (ClassTo*)0;\
    _count##NameTo = 0;

#define EXIT_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    {\
        ClassTo* firstItem = GetFirst##NameTo();\
		ClassTo* item = firstItem;\
		ClassTo* nextItem;\
		while (item)\
	    {\
			nextItem = GetNext##NameTo(item);\
		    item->_bal##NameFrom = intptr_t(nextItem);\
		    item = nextItem;\
	    }\
\
	    ClassFrom::NameTo##Iterator::CheckAll(this);\
		item = firstItem;\
		while (item)\
		{\
			nextItem = (ClassTo*)item->_bal##NameFrom;\
            item->_ref##NameFrom = (ClassFrom*)0;\
            item->_parent##NameFrom = (ClassTo*)0;\
            item->_left##NameFrom = (ClassTo*)0;\
            item->_right##NameFrom = (ClassTo*)0;\
            item->_bal##NameFrom = 0;\
			item = nextItem;\
		}\
		_top##NameTo = (ClassTo*)0;\
		_count##NameTo = 0;\
	}

#define REPLACE_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _top##NameTo = pOld->_top##NameTo;\
    _count##NameTo = pOld->_count##NameTo;\
    pOld->_top##NameTo = (ClassTo*)0;\
    { for (ClassTo* item = GetFirst##NameTo(); item; item = GetNext##NameTo(item))\
          item->_ref##NameFrom = this; }

#define INIT_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _ref##NameFrom = (ClassFrom*)0;\
    _parent##NameFrom = (ClassTo*)0;\
    _left##NameFrom = (ClassTo*)0;\
    _right##NameFrom = (ClassTo*)0;\
    _bal##NameFrom = 0;

#define EXIT_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (_ref##NameFrom)\
        _ref##NameFrom->Remove##NameTo(this);

#define REPLACE_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _ref##NameFrom = (ClassFrom*)0;\
    _parent##NameFrom = (ClassTo*)0;\
    _left##NameFrom = (ClassTo*)0;\
    _right##NameFrom = (ClassTo*)0;\
    _bal##NameFrom = 0;\
    if (pOld->_ref##NameFrom)\
        pOld->_ref##NameFrom->Replace##NameTo(pOld, this);

#define REMOVE_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    while (_top##NameTo)\
    {\
        (void)new UndoSubChange(_top##NameTo);\
        Remove##NameTo(_top##NameTo);\
    }

#define SAVE_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    p##ClassTo->_ref##NameFrom = _ref##NameFrom;

#define RESTORE_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    {\
        ClassFrom* p##ClassFrom = p##ClassTo->_ref##NameFrom;\
        _ref##NameFrom = (ClassFrom*)0;\
        _bal##NameFrom = 0;\
        p##ClassFrom->Add##NameTo(this);\
    }\

#define REMOVE_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    if (_ref##NameFrom)\
    {\
        ClassFrom* p##ClassFrom = _ref##NameFrom;\
        _ref##NameFrom->Remove##NameTo(this);\
        _ref##NameFrom = p##ClassFrom;\
    }

#define CLEANUP_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    _ref##NameFrom = (ClassFrom*)0;\
    _parent##NameFrom = (ClassTo*)0;\
    _left##NameFrom = (ClassTo*)0;\
    _right##NameFrom = (ClassTo*)0;\
    _bal##NameFrom = 0;

#define METHODS_AVLTREE_ACTIVE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
void ClassFrom::Add##NameTo(ClassTo* item)\
{\
    METHOD_AVLTREE_ADD(member, ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Remove##NameTo(ClassTo* item)\
{\
    METHOD_AVLTREE_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::RemoveAll##NameTo()\
{\
    METHOD_AVLTREE_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::DeleteAll##NameTo()\
{\
    METHOD_AVLTREE_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
void ClassFrom::Replace##NameTo(ClassTo* item, ClassTo* newItem)\
{\
    METHOD_AVLTREE_REPLACE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetFirst##NameTo() const\
{\
    METHOD_AVLTREE_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetLast##NameTo() const\
{\
    METHOD_AVLTREE_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetNext##NameTo(ClassTo* pos) const\
{\
    METHOD_AVLTREE_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
ClassTo* ClassFrom::GetPrev##NameTo(ClassTo* pos) const\
{\
    METHOD_AVLTREE_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
}\
\
int ClassFrom::Get##NameTo##Count() const\
{\
    METHOD_AVLTREE_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
}

#define METHOD_AVLTREE_ADD(member, ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == (ClassFrom*)0);\
\
    _count##NameTo++;\
\
    item->_ref##NameFrom = this;\
\
    if (_top##NameTo)\
    {\
        /* Find insertion point via binary search */\
        ClassTo* current = _top##NameTo;\
        while (1)\
        {\
            if (item->member < current->member)\
            {\
                if (current->_left##NameFrom)\
                {\
                    current = current->_left##NameFrom;\
                }\
                else\
                {\
                    /* Insert as left child */\
                    current->_left##NameFrom = item;\
                    item->_parent##NameFrom = current;\
                    current->_bal##NameFrom--;  /* Left subtree grew */\
                    break;\
                }\
            }\
            else\
            {\
                if (current->_right##NameFrom)\
                {\
                    current = current->_right##NameFrom;\
                }\
                else\
                {\
                    /* Insert as right child */\
                    current->_right##NameFrom = item;\
                    item->_parent##NameFrom = current;\
                    current->_bal##NameFrom++;  /* Right subtree grew */\
                    break;\
                }\
            }\
        }\
\
        /* Rebalance tree: propagate height changes and fix imbalances */\
        ClassTo* parent;\
        while (current && current->_bal##NameFrom)\
        {\
            parent = current->_parent##NameFrom;\
            if (parent)\
            {\
                /* Update parent's balance factor based on which child changed */\
                if (parent->_left##NameFrom == current)\
                {\
                    parent->_bal##NameFrom--;  /* Left child height changed */\
                }\
                else\
                {\
                    parent->_bal##NameFrom++;  /* Right child height changed */\
                }\
\
                /* Balance factor: -1/+1 = acceptable, -2/+2 = imbalanced, 0 = balanced */\
                if (parent->_bal##NameFrom == 2)\
                {\
                    /* Right subtree is too heavy: need left rotation(s) */\
                    if (current->_bal##NameFrom == -1)\
                    {\
                        /* Right-Left case: right child is left-heavy */\
                        /* Perform right rotation on current, then left on parent */\
                        ClassTo* sub = current->_left##NameFrom;\
                        parent->_right##NameFrom = sub->_left##NameFrom;\
                        if (sub->_left##NameFrom)\
                        {\
                            sub->_left##NameFrom->_parent##NameFrom = parent;\
                        }\
                        current->_left##NameFrom = sub->_right##NameFrom;\
                        if (sub->_right##NameFrom)\
                        {\
                            sub->_right##NameFrom->_parent##NameFrom = current;\
                        }\
                        sub->_parent##NameFrom = parent->_parent##NameFrom;\
                        sub->_left##NameFrom = parent;\
                        parent->_parent##NameFrom = sub;\
                        sub->_right##NameFrom = current;\
                        current->_parent##NameFrom = sub;\
                        /* Update parent pointer in grandparent */\
                        if (sub->_parent##NameFrom)\
                        {\
                            if (sub->_parent##NameFrom->_left##NameFrom == parent)\
                            {\
                                sub->_parent##NameFrom->_left##NameFrom = sub;\
                            }\
                            else\
                            {\
                                sub->_parent##NameFrom->_right##NameFrom = sub;\
                            }\
                        }\
                        else\
                        {\
                            _top##NameTo = sub;\
                        }\
                        /* Recalculate balance factors after rotation */\
                        parent->_bal##NameFrom = (sub->_bal##NameFrom == 1? -1: 0);\
                        current->_bal##NameFrom = (sub->_bal##NameFrom == -1? 1: 0);\
                        sub->_bal##NameFrom = 0;\
                        current = sub;\
                        break;  /* Tree is rebalanced */\
                    }\
                    else\
                    {\
                        /* Right-Right case: right child is right-heavy */\
                        /* Single left rotation */\
                        parent->_right##NameFrom = current->_left##NameFrom;\
                        if (current->_left##NameFrom)\
                        {\
                            current->_left##NameFrom->_parent##NameFrom = parent;\
                        }\
                        current->_left##NameFrom = parent;\
                        current->_parent##NameFrom = parent->_parent##NameFrom;\
                        parent->_parent##NameFrom = current;\
                        /* Update parent pointer in grandparent */\
                        if (current->_parent##NameFrom)\
                        {\
                            if (current->_parent##NameFrom->_left##NameFrom == parent)\
                            {\
                                current->_parent##NameFrom->_left##NameFrom = current;\
                            }\
                            else\
                            {\
                                current->_parent##NameFrom->_right##NameFrom = current;\
                            }\
                        }\
                        else\
                        {\
                            _top##NameTo = current;\
                        }\
                        parent->_bal##NameFrom = 0;\
                        current->_bal##NameFrom = 0;\
                        break;  /* Tree is rebalanced */\
                    }\
                }\
                else if (parent->_bal##NameFrom == -2)\
                {\
                    /* Left subtree is too heavy: need right rotation(s) */\
                    if (current->_bal##NameFrom == 1)\
                    {\
                        /* Left-Right case: left child is right-heavy */\
                        /* Perform left rotation on current, then right on parent */\
                        ClassTo* sub = current->_right##NameFrom;\
                        parent->_left##NameFrom = sub->_right##NameFrom;\
                        if (sub->_right##NameFrom)\
                        {\
                            sub->_right##NameFrom->_parent##NameFrom = parent;\
                        }\
                        current->_right##NameFrom = sub->_left##NameFrom;\
                        if (sub->_left##NameFrom)\
                        {\
                            sub->_left##NameFrom->_parent##NameFrom = current;\
                        }\
                        sub->_parent##NameFrom = parent->_parent##NameFrom;\
                        sub->_right##NameFrom = parent;\
                        parent->_parent##NameFrom = sub;\
                        sub->_left##NameFrom = current;\
                        current->_parent##NameFrom = sub;\
                        /* Update parent pointer in grandparent */\
                        if (sub->_parent##NameFrom)\
                        {\
                            if (sub->_parent##NameFrom->_right##NameFrom == parent)\
                            {\
                                sub->_parent##NameFrom->_right##NameFrom = sub;\
                            }\
                            else\
                            {\
                                sub->_parent##NameFrom->_left##NameFrom = sub;\
                            }\
                        }\
                        else\
                        {\
                            _top##NameTo = sub;\
                        }\
                        /* Recalculate balance factors after rotation */\
                        parent->_bal##NameFrom = (sub->_bal##NameFrom == -1? 1: 0);\
                        current->_bal##NameFrom = (sub->_bal##NameFrom == 1? -1: 0);\
                        sub->_bal##NameFrom = 0;\
                        current = sub;\
                        break;  /* Tree is rebalanced */\
                    }\
                    else\
                    {\
                        /* Left-Left case: left child is left-heavy */\
                        /* Single right rotation */\
                        parent->_left##NameFrom = current->_right##NameFrom;\
                        if (current->_right##NameFrom)\
                        {\
                            current->_right##NameFrom->_parent##NameFrom = parent;\
                        }\
                        current->_right##NameFrom = parent;\
                        current->_parent##NameFrom = parent->_parent##NameFrom;\
                        parent->_parent##NameFrom = current;\
                        /* Update parent pointer in grandparent */\
                        if (current->_parent##NameFrom)\
                        {\
                            if (current->_parent##NameFrom->_right##NameFrom == parent)\
                            {\
                                current->_parent##NameFrom->_right##NameFrom = current;\
                            }\
                            else\
                            {\
                                current->_parent##NameFrom->_left##NameFrom = current;\
                            }\
                        }\
                        else\
                        {\
                            _top##NameTo = current;\
                        }\
                        parent->_bal##NameFrom = 0;\
                        current->_bal##NameFrom = 0;\
                        break;  /* Tree is rebalanced */\
                    }\
                }\
                else if (parent->_bal##NameFrom == 0)\
                    break;  /* Height stable, no further propagation needed */\
                else\
                    current = parent;  /* Continue propagating balance changes up */\
            }\
            else\
                break;  /* Reached root */\
        }\
    }\
    else\
    {\
        /* Tree is empty, new item becomes root */\
        _top##NameTo = item;\
    }

#define METHOD_AVLTREE_REMOVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == this);\
\
    ClassFrom::NameTo##Iterator::Check(item);\
\
    _count##NameTo--;\
\
    ClassTo* subParent = item->_parent##NameFrom;\
    ClassTo* sub = item;\
    \
    /* === PHASE 1: Remove node from tree === */\
    /* Case 1: Leaf node (no children) */\
    if (!item->_left##NameFrom && !item->_right##NameFrom)\
    {\
        if (subParent)\
        {\
            /* Remove from parent and update balance factor */\
            if (subParent->_left##NameFrom == item)\
            {\
                subParent->_left##NameFrom = (ClassTo*)0;\
                subParent->_bal##NameFrom++;  /* Left subtree shrank */\
            }\
            else\
            {\
                subParent->_right##NameFrom = (ClassTo*)0;\
                subParent->_bal##NameFrom--;  /* Right subtree shrank */\
            }\
        }\
        else\
        {\
            /* Item was root, tree is now empty */\
            _top##NameTo = (ClassTo*)0;\
        }\
    }\
    /* Case 2: Node with children - use successor/predecessor replacement */\
    else\
    {\
        /* Choose which child's subtree is heavier to minimize rebalancing */\
        if (item->_bal##NameFrom > 0)\
        {\
            /* Right subtree is heavier: find in-order successor (min in right subtree) */\
            sub = item->_right##NameFrom;\
            while (sub->_left##NameFrom)\
            {\
                sub = sub->_left##NameFrom;\
            }\
            subParent = sub->_parent##NameFrom;\
            if (subParent != item)\
            {\
                /* Successor is not direct right child: unlink it from its parent */\
                subParent->_left##NameFrom = sub->_right##NameFrom;\
                if (subParent->_left##NameFrom)\
                {\
                    subParent->_left##NameFrom->_parent##NameFrom = subParent;\
                }\
                subParent->_bal##NameFrom++;  /* Left subtree of subParent shrank */\
            }\
            else\
            {\
                /* Successor is direct right child: just adjust item's balance */\
                item->_bal##NameFrom--;\
            }\
        }\
        else\
        {\
            /* Left subtree is heavier: find in-order predecessor (max in left subtree) */\
            sub = item->_left##NameFrom;\
            while (sub->_right##NameFrom)\
            {\
                sub = sub->_right##NameFrom;\
            }\
            subParent = sub->_parent##NameFrom;\
            if (subParent != item)\
            {\
                /* Predecessor is not direct left child: unlink it from its parent */\
                subParent->_right##NameFrom = sub->_left##NameFrom;\
                if (subParent->_right##NameFrom)\
                {\
                    subParent->_right##NameFrom->_parent##NameFrom = subParent;\
                }\
                subParent->_bal##NameFrom--;  /* Right subtree of subParent shrank */\
            }\
            else\
            {\
                /* Predecessor is direct left child: just adjust item's balance */\
                item->_bal##NameFrom++;\
            }\
        }\
        \
        /* Link successor/predecessor into item's position */\
        sub->_parent##NameFrom = item->_parent##NameFrom;\
        if (item->_parent##NameFrom)\
        {\
            /* Update parent's child pointer */\
            if (item->_parent##NameFrom->_left##NameFrom == item)\
            {\
                item->_parent##NameFrom->_left##NameFrom = sub;\
            }\
            else\
            {\
                item->_parent##NameFrom->_right##NameFrom = sub;\
            }\
        }\
        else\
        {\
            /* Item was root */\
            _top##NameTo = sub;\
        }\
        \
        /* Attach item's children to successor/predecessor */\
        if (item->_left##NameFrom != sub)\
        {\
            sub->_left##NameFrom = item->_left##NameFrom;\
            if (item->_left##NameFrom)\
            {\
                item->_left##NameFrom->_parent##NameFrom = sub;\
            }\
        }\
        if (item->_right##NameFrom != sub)\
        {\
            sub->_right##NameFrom = item->_right##NameFrom;\
            if (item->_right##NameFrom)\
            {\
                item->_right##NameFrom->_parent##NameFrom = sub;\
            }\
        }\
        /* Copy balance factor from removed item to its replacement */\
        sub->_bal##NameFrom = item->_bal##NameFrom;\
\
        /* Adjust starting point for rebalancing */\
        if (subParent == item)\
        {\
            subParent = sub;\
        }\
    }\
\
    /* Clear removed item's references */\
    item->_ref##NameFrom = (ClassFrom*)0;\
    item->_parent##NameFrom = (ClassTo*)0;\
    item->_left##NameFrom = (ClassTo*)0;\
    item->_right##NameFrom = (ClassTo*)0;\
    item->_bal##NameFrom = 0;\
\
    /* === PHASE 2: Rebalance tree by propagating height changes up === */\
    /* Unlike insertion, removal can create cascading imbalances up to root */\
    ClassTo* parent = subParent;\
    while (parent && parent->_bal##NameFrom != -1 && parent->_bal##NameFrom != 1)\
    {\
        if (parent->_bal##NameFrom == 2)\
        {\
            /* Right subtree is too heavy: need left rotation(s) */\
            ClassTo* current = parent->_right##NameFrom;\
            if (current->_bal##NameFrom == -1)\
            {\
                /* Right-Left case: right child is left-heavy */\
                ClassTo* sub = current->_left##NameFrom;\
                parent->_right##NameFrom = sub->_left##NameFrom;\
                if (sub->_left##NameFrom)\
                {\
                    sub->_left##NameFrom->_parent##NameFrom = parent;\
                }\
                current->_left##NameFrom = sub->_right##NameFrom;\
                if (sub->_right##NameFrom)\
                {\
                    sub->_right##NameFrom->_parent##NameFrom = current;\
                }\
                sub->_parent##NameFrom = parent->_parent##NameFrom;\
                sub->_left##NameFrom = parent;\
                parent->_parent##NameFrom = sub;\
                sub->_right##NameFrom = current;\
                current->_parent##NameFrom = sub;\
                if (sub->_parent##NameFrom)\
                {\
                    if (sub->_parent##NameFrom->_left##NameFrom == parent)\
                    {\
                        sub->_parent##NameFrom->_left##NameFrom = sub;\
                    }\
                    else\
                    {\
                        sub->_parent##NameFrom->_right##NameFrom = sub;\
                    }\
                }\
                else\
                {\
                    _top##NameTo = sub;\
                }\
                parent->_bal##NameFrom = (sub->_bal##NameFrom == 1? -1: 0);\
                current->_bal##NameFrom = (sub->_bal##NameFrom == -1? 1: 0);\
                sub->_bal##NameFrom = 0;\
                parent = sub;\
            }\
            else if (current->_bal##NameFrom == 1)\
            {\
                /* Right-Right case: right child is right-heavy */\
                parent->_right##NameFrom = current->_left##NameFrom;\
                if (current->_left##NameFrom)\
                {\
                    current->_left##NameFrom->_parent##NameFrom = parent;\
                }\
                current->_left##NameFrom = parent;\
                current->_parent##NameFrom = parent->_parent##NameFrom;\
                parent->_parent##NameFrom = current;\
                if (current->_parent##NameFrom)\
                {\
                    if (current->_parent##NameFrom->_left##NameFrom == parent)\
                    {\
                        current->_parent##NameFrom->_left##NameFrom = current;\
                    }\
                    else\
                    {\
                        current->_parent##NameFrom->_right##NameFrom = current;\
                    }\
                }\
                else\
                {\
                    _top##NameTo = current;\
                }\
                parent->_bal##NameFrom = 0;\
                current->_bal##NameFrom = 0;\
                parent = current;\
            }\
            else\
            {\
                /* Right-Right with neutral child: rotation doesn't fully fix balance */\
                parent->_right##NameFrom = current->_left##NameFrom;\
                if (current->_left##NameFrom)\
                {\
                    current->_left##NameFrom->_parent##NameFrom = parent;\
                }\
                current->_left##NameFrom = parent;\
                current->_parent##NameFrom = parent->_parent##NameFrom;\
                parent->_parent##NameFrom = current;\
                if (current->_parent##NameFrom)\
                {\
                    if (current->_parent##NameFrom->_left##NameFrom == parent)\
                    {\
                        current->_parent##NameFrom->_left##NameFrom = current;\
                    }\
                    else\
                    {\
                        current->_parent##NameFrom->_right##NameFrom = current;\
                    }\
                }\
                else\
                {\
                    _top##NameTo = current;\
                }\
                /* Mark as partially balanced (±1 means height is stable) */\
                parent->_bal##NameFrom = 1;\
                current->_bal##NameFrom = -1;\
                break;  /* Height is stable, no further propagation needed */\
            }\
        }\
        else if (parent->_bal##NameFrom == -2)\
        {\
            /* Left subtree is too heavy: need right rotation(s) */\
            ClassTo* current = parent->_left##NameFrom;\
            if (current->_bal##NameFrom == 1)\
            {\
                /* Left-Right case: left child is right-heavy */\
                ClassTo* sub = current->_right##NameFrom;\
                parent->_left##NameFrom = sub->_right##NameFrom;\
                if (sub->_right##NameFrom)\
                {\
                    sub->_right##NameFrom->_parent##NameFrom = parent;\
                }\
                current->_right##NameFrom = sub->_left##NameFrom;\
                if (sub->_left##NameFrom)\
                {\
                    sub->_left##NameFrom->_parent##NameFrom = current;\
                }\
                sub->_parent##NameFrom = parent->_parent##NameFrom;\
                sub->_right##NameFrom = parent;\
                parent->_parent##NameFrom = sub;\
                sub->_left##NameFrom = current;\
                current->_parent##NameFrom = sub;\
                if (sub->_parent##NameFrom)\
                {\
                    if (sub->_parent##NameFrom->_right##NameFrom == parent)\
                    {\
                        sub->_parent##NameFrom->_right##NameFrom = sub;\
                    }\
                    else\
                    {\
                        sub->_parent##NameFrom->_left##NameFrom = sub;\
                    }\
                }\
                else\
                {\
                    _top##NameTo = sub;\
                }\
                parent->_bal##NameFrom = (sub->_bal##NameFrom == -1? 1: 0);\
                current->_bal##NameFrom = (sub->_bal##NameFrom == 1? -1: 0);\
                sub->_bal##NameFrom = 0;\
                parent = sub;\
            }\
            else if (current->_bal##NameFrom == -1)\
            {\
                /* Left-Left case: left child is left-heavy */\
                parent->_left##NameFrom = current->_right##NameFrom;\
                if (current->_right##NameFrom)\
                {\
                    current->_right##NameFrom->_parent##NameFrom = parent;\
                }\
                current->_right##NameFrom = parent;\
                current->_parent##NameFrom = parent->_parent##NameFrom;\
                parent->_parent##NameFrom = current;\
                if (current->_parent##NameFrom)\
                {\
                    if (current->_parent##NameFrom->_right##NameFrom == parent)\
                    {\
                        current->_parent##NameFrom->_right##NameFrom = current;\
                    }\
                    else\
                    {\
                        current->_parent##NameFrom->_left##NameFrom = current;\
                    }\
                }\
                else\
                {\
                    _top##NameTo = current;\
                }\
                parent->_bal##NameFrom = 0;\
                current->_bal##NameFrom = 0;\
                parent = current;\
            }\
            else\
            {\
                /* Left-Left with neutral child: rotation doesn't fully fix balance */\
                parent->_left##NameFrom = current->_right##NameFrom;\
                if (current->_right##NameFrom)\
                {\
                    current->_right##NameFrom->_parent##NameFrom = parent;\
                }\
                current->_right##NameFrom = parent;\
                current->_parent##NameFrom = parent->_parent##NameFrom;\
                parent->_parent##NameFrom = current;\
                if (current->_parent##NameFrom)\
                {\
                    if (current->_parent##NameFrom->_right##NameFrom == parent)\
                    {\
                        current->_parent##NameFrom->_right##NameFrom = current;\
                    }\
                    else\
                    {\
                        current->_parent##NameFrom->_left##NameFrom = current;\
                    }\
                }\
                else\
                {\
                    _top##NameTo = current;\
                }\
                /* Mark as partially balanced (±1 means height is stable) */\
                parent->_bal##NameFrom = -1;\
                current->_bal##NameFrom = 1;\
                break;  /* Height is stable, no further propagation needed */\
            }\
        }\
\
        /* Propagate height changes up to ancestors */\
        if (parent->_parent##NameFrom)\
        {\
            if (parent->_parent##NameFrom->_left##NameFrom == parent)\
                parent->_parent##NameFrom->_bal##NameFrom++;  /* Left child height changed */\
            else\
                parent->_parent##NameFrom->_bal##NameFrom--;  /* Right child height changed */\
            parent = parent->_parent##NameFrom;\
            /* Stop if balance becomes stable (±1) */\
            if (parent->_bal##NameFrom == 1 || parent->_bal##NameFrom == -1)\
                break;\
        }\
        else\
            break;  /* Reached root */\
    }\

#define METHOD_AVLTREE_REMOVEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    {\
        ClassTo* lastItem = GetLast##NameTo();\
		ClassTo* item = lastItem;\
		ClassTo* prevItem;\
		while (item)\
	    {\
			prevItem = GetPrev##NameTo(item);\
		    item->_bal##NameFrom = intptr_t(prevItem);\
		    item = prevItem;\
	    }\
\
	    ClassFrom::NameTo##Iterator::CheckAll(this);\
		item = lastItem;\
		while (item)\
		{\
			prevItem = (ClassTo*)item->_bal##NameFrom;\
            item->_ref##NameFrom = (ClassFrom*)0;\
            item->_parent##NameFrom = (ClassTo*)0;\
            item->_left##NameFrom = (ClassTo*)0;\
            item->_right##NameFrom = (ClassTo*)0;\
            item->_bal##NameFrom = 0;\
			item = prevItem;\
		}\
		_top##NameTo = (ClassTo*)0;\
		_count##NameTo = 0;\
	}

#define METHOD_AVLTREE_DELETEALL(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    {\
        ClassTo* lastItem = GetLast##NameTo();\
		ClassTo* item = lastItem;\
		ClassTo* prevItem;\
		while (item)\
	    {\
			prevItem = GetPrev##NameTo(item);\
		    item->_bal##NameFrom = intptr_t(prevItem);\
		    item = prevItem;\
	    }\
\
	    ClassFrom::NameTo##Iterator::CheckAll(this);\
		item = lastItem;\
		while (item)\
		{\
			prevItem = (ClassTo*)item->_bal##NameFrom;\
            item->_ref##NameFrom = (ClassFrom*)0;\
            item->_parent##NameFrom = (ClassTo*)0;\
            item->_left##NameFrom = (ClassTo*)0;\
            item->_right##NameFrom = (ClassTo*)0;\
            item->_bal##NameFrom = 0;\
            delete item;\
			item = prevItem;\
		}\
		_top##NameTo = (ClassTo*)0;\
		_count##NameTo = 0;\
	}

#define METHOD_AVLTREE_REPLACE(member, ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    assert(item);\
    assert(item->_ref##NameFrom == this);\
\
    assert(newItem);\
    assert(newItem->_ref##NameFrom == (ClassFrom*)0);\
\
    if (item->member == newItem->member)\
    {\
        ClassFrom::NameTo##Iterator::Check(item, newItem);\
        if (_top##NameTo == item)\
        {\
            _top##NameTo = newItem;\
        }\
        if (item->_parent##NameFrom)\
        {\
            if (item->_parent##NameFrom->_left##NameFrom == item)\
            {\
                item->_parent##NameFrom->_left##NameFrom = newItem;\
            }\
            else if (item->_parent##NameFrom->_right##NameFrom == item)\
            {\
                item->_parent##NameFrom->_right##NameFrom = newItem;\
            }\
        }\
		if (item->_left##NameFrom)\
		{\
			item->_left##NameFrom->_parent##NameFrom = newItem;\
		}\
		if (item->_right##NameFrom)\
		{\
			item->_right##NameFrom->_parent##NameFrom = newItem;\
		}\
        newItem->_ref##NameFrom = this;\
        newItem->_parent##NameFrom = item->_parent##NameFrom;\
        newItem->_left##NameFrom = item->_left##NameFrom;\
        newItem->_right##NameFrom = item->_right##NameFrom;\
        newItem->_bal##NameFrom = item->_bal##NameFrom;\
        item->_ref##NameFrom = (ClassFrom*)0;\
        item->_parent##NameFrom = (ClassTo*)0;\
        item->_left##NameFrom = (ClassTo*)0;\
        item->_right##NameFrom = (ClassTo*)0;\
        item->_bal##NameFrom = 0;\
    }\
    else\
    {\
        ClassFrom::NameTo##Iterator::Check(item);\
        Remove##NameTo(item);\
        Add##NameTo(newItem);\
    }

#define METHOD_AVLTREE_GETFIRST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    /* Find minimum (leftmost node): follow left pointers to leaf */\
    ClassTo* result = _top##NameTo;\
    if (result)\
    {\
        while (result->_left##NameFrom)\
        {\
            result = result->_left##NameFrom;\
        }\
    }\
\
    return result;

#define METHOD_AVLTREE_GETLAST(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    /* Find maximum (rightmost node): follow right pointers to leaf */\
    ClassTo* result = _top##NameTo;\
    if (result)\
    {\
        while (result->_right##NameFrom)\
        {\
            result = result->_right##NameFrom;\
        }\
    }\
\
    return result;

#define METHOD_AVLTREE_GETNEXT(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    /* Null position returns first element */\
    if (pos == (ClassTo*)0)\
        return GetFirst##NameTo();\
\
    assert(pos->_ref##NameFrom == this);\
\
    ClassTo* result;\
    /* In-order successor has two cases: */\
    if (pos->_right##NameFrom)\
    {\
        /* Case 1: Node has right child */\
        /* Successor is minimum of right subtree (leftmost in right subtree) */\
        result = pos->_right##NameFrom;\
        while (result->_left##NameFrom)\
        {\
            result = result->_left##NameFrom;\
        }\
    }\
    else\
    {\
        /* Case 2: Node has no right child */\
        /* Successor is first ancestor where we're in the left subtree */\
        /* Walk up tree until we find node that's a left child of its parent */\
        result = pos->_parent##NameFrom;\
        while (result && result->_right##NameFrom == pos)\
        {\
            pos = result;\
            result = pos->_parent##NameFrom;\
        }\
    }\
\
    return result;

#define METHOD_AVLTREE_GETPREV(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
\
    /* Null position returns last element */\
    if (pos == (ClassTo*)0)\
        return GetLast##NameTo();\
\
    assert(pos->_ref##NameFrom == this);\
\
    ClassTo* result;\
    /* In-order predecessor has two cases: */\
    if (pos->_left##NameFrom)\
    {\
        /* Case 1: Node has left child */\
        /* Predecessor is maximum of left subtree (rightmost in left subtree) */\
        result = pos->_left##NameFrom;\
        while (result->_right##NameFrom)\
        {\
            result = result->_right##NameFrom;\
        }\
    }\
    else\
    {\
        /* Case 2: Node has no left child */\
        /* Predecessor is first ancestor where we're in the right subtree */\
        /* Walk up tree until we find node that's a right child of its parent */\
        result = pos->_parent##NameFrom;\
        while (result && result->_left##NameFrom == pos)\
        {\
            pos = result;\
            result = pos->_parent##NameFrom;\
        }\
    }\
\
    return result;

#define METHOD_AVLTREE_GETCOUNT(ClassFrom, NameFrom, ClassTo, NameTo) \
    assert(this);\
    return _count##NameTo;

#define METHODS_AVLTREE_PASSIVE(ClassFrom, NameFrom, ClassTo, NameTo)

#ifndef _BODY_AVLTREE_FIND
/* Internal helper: performs basic binary search without duplicate handling */
#define _BODY_AVLTREE_FIND(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    ClassTo* result = 0;\
    {\
        ClassTo* item = _top##NameTo;\
        /* Standard binary search: follow left/right based on comparison */\
        while (item)\
        {\
            if (item->member == value)\
            { result = item; break; }\
            /* Navigate tree: go left if smaller, right if larger */\
            item = (value < item->member) ? item->_left##NameFrom : item->_right##NameFrom;\
        }\
    }
#endif

/* Find first occurrence: returns earliest (smallest) value matching search key */\
#define BODY_AVLTREE_FIND(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    _BODY_AVLTREE_FIND(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    /* For duplicates: walk backward to find first occurrence */\
    if (result)\
    {\
        ClassTo* prev##NameTo = GetPrev##NameTo(result);\
        while (prev##NameTo && prev##NameTo->member == value)\
        {\
            result = prev##NameTo;\
            prev##NameTo = GetPrev##NameTo(result);\
        }\
    }\
    return result;

/* Find last occurrence: returns latest (largest) value matching search key */\
#define BODY_AVLTREE_FINDREVERSE(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    _BODY_AVLTREE_FIND(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    /* For duplicates: walk forward to find last occurrence */\
    if (result)\
    {\
        ClassTo* next##NameTo = GetNext##NameTo(result);\
        while (next##NameTo && next##NameTo->member == value)\
        {\
            result = next##NameTo;\
            next##NameTo = GetNext##NameTo(result);\
        }\
    }\
    return result;

#define BODY_AVLTREE_FINDEQUALORBIGGER(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    ClassTo* result = 0;\
    {\
        ClassTo* item = _top##NameTo;\
        ClassTo* bigger = (ClassTo*)0;  /* Smallest node >= value (saved as fallback) */\
        /* Binary search for exact match or closest bigger value */\
        while (item)\
        {\
            if (item->member == value)\
            {\
                /* Found exact match: walk backward to get first occurrence (handle duplicates) */\
                result = item;\
                ClassTo* prev##NameTo = GetPrev##NameTo(result);\
                while (prev##NameTo && prev##NameTo->member == value)\
                {\
                    result = prev##NameTo;\
                    prev##NameTo = GetPrev##NameTo(result);\
                }\
                break;\
            }\
            /* Save candidate when we go left (value is bigger than current) */\
            if (value < item->member)\
            { bigger = item; item = item->_left##NameFrom; }\
            else\
            { item = item->_right##NameFrom; }\
        }\
        /* Use saved candidate if exact match not found */\
        if (!result) result = bigger;\
    }\
\
    return result;


#define BODY_AVLTREE_FINDEQUALORSMALLER(member, value, ClassFrom, NameFrom, ClassTo, NameTo) \
    ClassTo* result = 0;\
    {\
        ClassTo* item = _top##NameTo;\
        ClassTo* smaller = (ClassTo*)0;  /* Largest node <= value (saved as fallback) */\
        /* Binary search for exact match or closest smaller value */\
        while (item)\
        {\
            if (item->member == value)\
            {\
                /* Found exact match: walk forward to get last occurrence (handle duplicates) */\
                result = item;\
                ClassTo* next##NameTo = GetNext##NameTo(result);\
                while (next##NameTo && next##NameTo->member == value)\
                {\
                    result = next##NameTo;\
                    next##NameTo = GetNext##NameTo(result);\
                }\
                break;\
            }\
            /* Save candidate when we go right (value is smaller than current) */\
            if (value < item->member)\
            { item = item->_left##NameFrom; }\
            else\
            { smaller = item; item = item->_right##NameFrom; }\
        }\
        /* Use saved candidate if exact match not found */\
        if (!result) result = smaller;\
    }\
\
    return result;

#define WRITE_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    archive << Get##NameTo##Count();\
    { for (ClassTo* item = GetFirst##NameTo(); item; item = GetNext##NameTo(item))\
          archive << item->_index; }

#define READ_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    {\
        int count;\
        int index;\
\
        archive >> count;\
        for (int i = 0; i < count; i++)\
        {\
            archive >> index;\
            Add##NameTo(static_cast<ClassTo*>(pointerArray[index]));\
        }\
    }

#define DYNAMIC_READ_AVLTREE_ACTIVE(ClassFrom, NameFrom, ClassTo, NameTo) \
    {\
        int count;\
        int index;\
\
        archive >> count;\
        for (int i = 0; i < count; i++)\
        {\
            archive >> index;\
            Add##NameTo(dynamic_cast<ClassTo*>(pointerArray[index]));\
        }\
    }

#endif
