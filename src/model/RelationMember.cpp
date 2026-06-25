/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationMember.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RelationMember'
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


/*@NOTE_1681
Constructor needed for serialization, not meant to use for other purposes!
*/
RelationMember::RelationMember() //@INIT_1681
    : DataModelDocObject()
{//@CODE_1681
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_1681


RelationMember::RelationMember(Relation* pRelation,
                               Member* pMember) //@INIT_1698
    : DataModelDocObject(pRelation->GetDataModelDoc())
{//@CODE_1698
    ConstructorInclude(pRelation, pMember);

    // Put in your own code
    if (pMember->GetSetMemberMethod())
    {
        pMember->GetSetMemberMethod()->SaveState(1);
        pMember->GetSetMemberMethod()->InitCode();
        pMember->GetSetMemberMethod()->SetInline(0);
    }
}//@CODE_1698


/*@NOTE_1679
Destructor method
*/
RelationMember::~RelationMember()
{//@CODE_1679
    DestructorInclude();

    // Put in your own code
}//@CODE_1679


/*@NOTE_5825
Use this method instead of calling delete. This method will make the
appropriate actions to put the object on the undo stack, so the delete can be
undone. It will also take care of  the associations and the aggregations.
*/
void RelationMember::Delete()
{//@CODE_5825
    Member* pMember = GetMember();

    DataModelDocObject::Delete();
    
    if (pMember->GetSetMemberMethod())
    {
        pMember->GetSetMemberMethod()->SaveState(1);
        pMember->GetSetMemberMethod()->InitCode();
    }
}//@CODE_5825


CbString RelationMember::EpilogSetMemberMethod()
{//@CODE_1718
    CbString value;

    value += GetIndent(2) + "ref" + GetRelation()->GetFromName() + "->Add" + 
             GetRelation()->GetToName() + "(this);" NL;

    return value;
}//@CODE_1718


int RelationMember::GetImplementation()
{//@CODE_1719
    return 0;
}//@CODE_1719


CbString RelationMember::InitCodeFindMethod(FindMethod* pFindMethod,
                                            Argument* pArgument)
{//@CODE_1787
    CbString value;

    return value;
}//@CODE_1787


CbString RelationMember::PrologueSetMemberMethod()
{//@CODE_1717
    CbString value;
    
    //value += GetIndent(2) + GetRelation()->GetFromClassName() + "::" +
    //         GetRelation()->GetToName() + "Iterator::Check(this);" NL;
    value += GetIndent(2) + GetRelation()->GetFromClassName() + "* " +
             "ref" + GetRelation()->GetFromName() + " = " + 
             "_ref" + GetRelation()->GetFromName() + ";" NL;
    value += GetIndent(2) + "ref" + GetRelation()->GetFromName() + "->Remove" + 
             GetRelation()->GetToName() + "(this);" NL;

    return value;
}//@CODE_1717


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5582
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RelationMember::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    CLEANUP_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)
}


/*@NOTE_1678
Method which must be called first in a constructor
*/
void RelationMember::ConstructorInclude(Relation* pRelation, Member* pMember)
{
    INIT_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    INIT_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)
}


/*@NOTE_1680
Method which must be called first in a destructor
*/
void RelationMember::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    EXIT_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)
}


/*@NOTE_41460
Method that returns true if it is actually a AvlTree Object.
*/
bool RelationMember::IsAvlTree() const
{
    return (dynamic_cast<const AvlTree*>(this) != nullptr);
}


/*@NOTE_41458
Method that returns true if it is actually a UniqueValueTree Object.
*/
bool RelationMember::IsUniqueValueTree() const
{
    return (dynamic_cast<const UniqueValueTree*>(this) != nullptr);
}


/*@NOTE_41459
Method that returns true if it is actually a ValueTree Object.
*/
bool RelationMember::IsValueTree() const
{
    return (dynamic_cast<const ValueTree*>(this) != nullptr);
}


/*@NOTE_5583
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RelationMember::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)
    REMOVE_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
}


/*@NOTE_5584
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RelationMember::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationMember* pRelationMember = (RelationMember*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    RESTORE_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5586
Save the state of the current object relations to pDataModelDocObject.
*/
void RelationMember::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
    RelationMember* pRelationMember = (RelationMember*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    SAVE_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)
}


/*@NOTE_1683
Serialize the members only to a CbObject object
*/
void RelationMember::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_1682
Method which must be called first in a serialize constructor
*/
void RelationMember::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(Relation, Relation, RelationMember, RelationMember)
    INIT_MULTI_PASSIVE(Member, Member, RelationMember, RelationMember)
}


/*@NOTE_1685
Serialize the relations to a CbObject object
*/
void RelationMember::SerializeRelations(CbArchive& archive,
                                        DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(RelationMember)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(Relation, Relation, RelationMember, RelationMember)
METHODS_MULTI_OWNED_PASSIVE(Member, Member, RelationMember, RelationMember)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
