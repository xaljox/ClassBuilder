/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          UndoBase.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'UndoBase'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
* Philips Digital Video Systems, Eindhoven, The Netherlands.
* Distributed under the GNU General Public License (GPL)
*
\******************************************************************************/
#ifndef _UNDOBASE_H
#define _UNDOBASE_H

//@START_USER1
//@END_USER1


/*@NOTE_5106
All different kind of undoable mutations to the datastructure are derived from this class.
*/

class UndoBase
{
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, UndoBase, UndoBase)

//@START_USER2
//@END_USER2

// Members
private:
    int _last;

protected:
    DataModelDocObject* _pDataModelDocObject;

public:

// Methods
private:
    bool TouchesCd();
    bool TouchesSd();
    bool TouchesTree();
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();

protected:

public:
    UndoBase(DataModelDocObject* pDataModelDocObject);
    UndoBase(RedoBase* pRedoBase);
    UndoBase(DataModelDoc* pDataModelDoc);
    virtual ~UndoBase();
    void AccumulateTouches(bool& tree, bool& cd, bool& sd);
    void ChangeDataModelDocObject(ConnectionSegment* pConnectionSegment);
    virtual void Restore() = 0;
    int GetLast();
    void SetLast(int last);
    DataModelDocObject* GetDataModelDocObject();
    bool IsUndoChangeDoc() const;
    bool IsUndoDelete() const;
    bool IsUndoSubDelete() const;
};

#endif


#ifdef CB_INLINES
#ifndef _UNDOBASE_H_INLINES
#define _UNDOBASE_H_INLINES

/*@NOTE_5120
Returns the value of member '_last'.
*/
inline int UndoBase::GetLast()
{//@CODE_5120
    return _last;
}//@CODE_5120



/*@NOTE_5121
Set the value of member '_last' to 'last'.
*/
inline void UndoBase::SetLast(int last)
{//@CODE_5121
    _last = last;
}//@CODE_5121



/*@NOTE_5118
Returns the value of member '_pDataModelDocObject'.
*/
inline DataModelDocObject* UndoBase::GetDataModelDocObject()
{//@CODE_5118
    return _pDataModelDocObject;
}//@CODE_5118



//@START_USER3
//@END_USER3

#endif
#endif
