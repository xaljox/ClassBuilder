/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Destructor.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Destructor'
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
#include "qt/QtDestructorDialog.h"
//@END_USER2


// Static members


/*@NOTE_399
Constructor needed for serialization, not meant to use for other purposes!
*/
Destructor::Destructor() //@INIT_399
    : Method()
{//@CODE_399
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_399


Destructor::Destructor(BaseClass* pBaseClass) //@INIT_982
    : Method(pBaseClass, pBaseClass->GetDataModelDoc()->FindType(""))
{//@CODE_982
    ConstructorInclude();

    // Put in your own code
	_untouched = 0;
    if (!pBaseClass->GetName().IsEmpty())
        SetName("~" + pBaseClass->Type::GetName());
    SetAccess(PUBLIC);
    SetVirtual(1);

    SetNote("Destructor method.");
}//@CODE_982


/*@NOTE_7581
Constructor method needed to copy desctructor from one project to another.
*/
Destructor::Destructor(BaseClass* pBaseClass,
                       Destructor* pDestructor) //@INIT_7581
    : Method(pBaseClass, pBaseClass->GetDataModelDoc()->FindType(""), pDestructor)
{//@CODE_7581
    ConstructorInclude();

    // Put in your own code
	_untouched = 0;
}//@CODE_7581


/*@NOTE_397
Destructor method
*/
Destructor::~Destructor()
{//@CODE_397
    DestructorInclude();

    // Put in your own code
}//@CODE_397


bool Destructor::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1368
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
    }
    else
    {
        value = Method::Drag(ctrlKeyDown, pGtiDropDefault);
    }

    return value;
}//@CODE_1368


void Destructor::InitCode()
{//@CODE_984
    _code.Empty();

    Class* pClass = dynamic_cast<Class*>(GetBaseClass());
    if (pClass)
    {
        _code += GetIndent() + "DestructorInclude();" NL;

        bool first = true;
        Class::MemberIterator member(pClass);
        while (++member)
        {
            if (member->GetDelete())
            {
                if (first)
                {
                    _code += NL;
                    first = false;
                }
                _code += GetIndent() + "delete ";
                _code += member->GetPrefixedName() + ";" NL;
            }
        }
        _code += NL + GetIndent() + "// Put in your own code" NL;
    }
}//@CODE_984


int Destructor::OnAddArgument(bool checkOnly)
{//@CODE_987
    return 0;
}//@CODE_987


int Destructor::OnDelete(bool checkOnly)
{//@CODE_986
    return 0;
}//@CODE_986


int Destructor::OnEditAttributes(bool checkOnly)
{//@CODE_985
	if (checkOnly)
		return 1;

    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowDestructorDialog(this, modelChanged, ownerHwnd))
    {
        if (modelChanged)
        {
            // Coalesce Update()'s tree/diagram refresh (CbViewLock also shows the wait cursor).
            CbViewLock lock(GetDataModelDoc());
            Update();
        }

        return 1;
    }

    return 0;
}//@CODE_985


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5390
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Destructor::CleanupReferences()
{
    Method::CleanupReferences();
}


/*@NOTE_396
Method which must be called first in a constructor
*/
void Destructor::ConstructorInclude()
{
    INIT_SINGLE_OWNED_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_398
Method which must be called first in a destructor
*/
void Destructor::DestructorInclude()
{
    EXIT_SINGLE_OWNED_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_5391
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Destructor::RemoveReferences()
{
    REMOVE_SINGLE_OWNED_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
    Method::RemoveReferences();
}


/*@NOTE_5392
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Destructor::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5394
Save the state of the current object relations to pDataModelDocObject.
*/
void Destructor::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Method::SaveReferences(pDataModelDocObject);
}


/*@NOTE_401
Serialize the members only to a CbObject object
*/
void Destructor::Serialize(CbArchive& archive)
{
    Method::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_400
Method which must be called first in a serialize constructor
*/
void Destructor::SerializeConstructorInclude()
{
    INIT_SINGLE_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
}


/*@NOTE_403
Serialize the relations to a CbObject object
*/
void Destructor::SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[])
{
    Method::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_SINGLE_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_SINGLE_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Destructor)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_ACTIVE(Destructor, Destructor, DestructorIncludeMethod, DestructorIncludeMethod)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
