/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RedoBase.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'RedoBase'
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
#ifndef _REDOBASE_H
#define _REDOBASE_H

//@START_USER1
//@END_USER1


/*@NOTE_5126
All different kind of redoable mutations to the datastructure are derived from this class.
*/

class RedoBase
{
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, RedoBase, RedoBase)

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
    RedoBase(UndoBase* pUndoBase);
    virtual ~RedoBase();
    void AccumulateTouches(bool& tree, bool& cd, bool& sd);
    virtual void Restore() = 0;
    int GetLast();
    DataModelDocObject* GetDataModelDocObject();
    bool IsRedoChangeDoc() const;
};

#endif


#ifdef CB_INLINES
#ifndef _REDOBASE_H_INLINES
#define _REDOBASE_H_INLINES

/*@NOTE_5142
Returns the value of member '_last'.
*/
inline int RedoBase::GetLast()
{//@CODE_5142
    return _last;
}//@CODE_5142



/*@NOTE_5140
Returns the value of member '_pDataModelDocObject'.
*/
inline DataModelDocObject* RedoBase::GetDataModelDocObject()
{//@CODE_5140
    return _pDataModelDocObject;
}//@CODE_5140



//@START_USER3
//@END_USER3

#endif
#endif
