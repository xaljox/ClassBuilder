/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MacroMethod.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MacroMethod'
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


MacroMethod::MacroMethod(MacroMethods* pMacroMethods, BaseClass* pBaseClass,
                         Type* pType) //@INIT_1970
    : Method(pBaseClass, pType)
{//@CODE_1970
    ConstructorInclude(pMacroMethods);

    // Put in your own code
	_untouched = 0;
	SetDeclare(false);
	SetImplement(false);
    SetPhase(Complete_Phase);
}//@CODE_1970


/*@NOTE_1840
Constructor needed for serialization, not meant to use for other purposes!
*/
MacroMethod::MacroMethod() //@INIT_1840
    : Method()
{//@CODE_1840
    SerializeConstructorInclude();

    // Put in your own code
    SetPhase(Complete_Phase);
}//@CODE_1840


/*@NOTE_1838
Destructor method
*/
MacroMethod::~MacroMethod()
{//@CODE_1838
    DestructorInclude();

    // Put in your own code
}//@CODE_1838


void MacroMethod::Add()
{//@CODE_1994
    if (!GetAdded())
    {
        if (!GetParent())
        {
            SaveState(1);
            GetMacroMethods()->AddChildLast(this);
        }

        SetItemText();
        SetIcon();

        Gti::Add();

        ArgumentIterator argument(this);
        while (++argument)
            argument->Add();
    }
}//@CODE_1994


bool MacroMethod::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_2030
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
    }

    return value;
}//@CODE_2030


void MacroMethod::InitCode()
{//@CODE_1999
}//@CODE_1999


int MacroMethod::IsFixed() const
{//@CODE_35423
    return 1;
}//@CODE_35423


bool MacroMethod::IsNonMacroMethod() const
{//@CODE_2002
    return 0;
}//@CODE_2002


int MacroMethod::OnAddArgument(bool checkOnly)
{//@CODE_3188
    return 0;
}//@CODE_3188


int MacroMethod::OnDelete(bool checkOnly)
{//@CODE_1997
    return 0;
}//@CODE_1997


int MacroMethod::OnEditAttributes(bool checkOnly)
{//@CODE_1998
    return 0;
}//@CODE_1998


int MacroMethod::OnEditContext(bool checkOnly)
{//@CODE_27311
    return 0;
}//@CODE_27311


int MacroMethod::OnEditExceptionSpecification(bool checkOnly)
{//@CODE_22714
    return 0;
}//@CODE_22714


int MacroMethod::OnOpen(bool checkOnly)
{//@CODE_2000
    return 0;
}//@CODE_2000


void MacroMethod::Update()
{//@CODE_1995
    if (GetAdded())
    {
        SetItemText();
        SetIcon();

        Gti::Update();
    }
}//@CODE_1995


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5618
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MacroMethod::CleanupReferences()
{
    Method::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_1837
Method which must be called first in a constructor
*/
void MacroMethod::ConstructorInclude(MacroMethods* pMacroMethods)
{
    INIT_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_1839
Method which must be called first in a destructor
*/
void MacroMethod::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_5619
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MacroMethod::RemoveReferences()
{
    Method::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_5620
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MacroMethod::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MacroMethod* pMacroMethod = (MacroMethod*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
    Method::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5622
Save the state of the current object relations to pDataModelDocObject.
*/
void MacroMethod::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::SaveReferences(pDataModelDocObject);
    MacroMethod* pMacroMethod = (MacroMethod*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_1842
Serialize the members only to a CbObject object
*/
void MacroMethod::Serialize(CbArchive& archive)
{
    Method::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1841
Method which must be called first in a serialize constructor
*/
void MacroMethod::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)
}


/*@NOTE_1844
Serialize the relations to a CbObject object
*/
void MacroMethod::SerializeRelations(CbArchive& archive,
                                     DataModelDocObject* pointerArray[])
{
    Method::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(MacroMethod)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(MacroMethods, MacroMethods, MacroMethod, MacroMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
