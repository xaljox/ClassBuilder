/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MacroMethods.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MacroMethods'
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
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
//@END_USER2


// Static members


/*@NOTE_1853
Constructor needed for serialization, not meant to use for other purposes!
*/
MacroMethods::MacroMethods() //@INIT_1853
    : Gti()
{//@CODE_1853
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_1853


MacroMethods::MacroMethods(DataModelDoc* pDataModelDoc) //@INIT_1974
    : Gti(pDataModelDoc)
{//@CODE_1974
    ConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_1974


/*@NOTE_2003
Constructor needed for putting a new object in the old one's context
*/
MacroMethods::MacroMethods(MacroMethods* pOld) //@INIT_2003
    : Gti(pOld)
{//@CODE_2003
    ReplaceConstructorInclude(pOld);

    // Put in your own code
}//@CODE_2003


/*@NOTE_1851
Destructor method
*/
MacroMethods::~MacroMethods()
{//@CODE_1851
    DestructorInclude();

    // Put in your own code
}//@CODE_1851


void MacroMethods::Add()
{//@CODE_2011
    if (!GetAdded())
    {
        if (!GetParent())
        {
            SaveState(1);
            GetGtiParent()->AddChildLast(this);
        }

        SetItemText("Relation methods");
        SetIcon(ICON_METHODGROUP);

        Gti::Add();

        MacroMethods::MacroMethodIterator iMacroMethod(this);
        while (++iMacroMethod)
            iMacroMethod->Add();
    }
}//@CODE_2011


int MacroMethods::OnDelete(bool checkOnly)
{//@CODE_2012
    return 0;
}//@CODE_2012


int MacroMethods::OnEditAttributes(bool checkOnly)
{//@CODE_2013
    return 0;
}//@CODE_2013


int MacroMethods::OnOpen(bool checkOnly)
{//@CODE_2014
    return 0;
}//@CODE_2014


void MacroMethods::Update()
{//@CODE_2015
    if (GetAdded())
    {
        SetItemText("Relation methods");
        SetIcon(ICON_METHODGROUP);

        Gti::Update();
        
        MacroMethods::MacroMethodIterator iMacroMethod(this);
        while (++iMacroMethod)
            iMacroMethod->Update();
    }
}//@CODE_2015


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5624
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MacroMethods::CleanupReferences()
{
    Gti::CleanupReferences();
}


/*@NOTE_1850
Method which must be called first in a constructor
*/
void MacroMethods::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_1852
Method which must be called first in a destructor
*/
void MacroMethods::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_5625
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MacroMethods::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
    Gti::RemoveReferences();
}


/*@NOTE_2005
Method which must be called first in a replace constructor
*/
void MacroMethods::ReplaceConstructorInclude(MacroMethods* pOld)
{
    REPLACE_MULTI_OWNED_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_5626
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MacroMethods::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5628
Save the state of the current object relations to pDataModelDocObject.
*/
void MacroMethods::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
}


/*@NOTE_1855
Serialize the members only to a CbObject object
*/
void MacroMethods::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1854
Method which must be called first in a serialize constructor
*/
void MacroMethods::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_1857
Serialize the relations to a CbObject object
*/
void MacroMethods::SerializeRelations(CbArchive& archive,
                                      DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
METHODS_ITERATOR_MULTI_ACTIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
