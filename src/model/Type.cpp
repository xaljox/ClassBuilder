/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Type.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Type'
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


Type::Type(DataModelDoc* pDataModelDoc) //@INIT_702
    : Gti(pDataModelDoc)
    , _name("")
{//@CODE_702
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_702


/*@NOTE_1560
Constructor needed for putting a new object in the old one's context
*/
Type::Type(Type* pOld) //@INIT_1560
    : Gti(pOld)
{//@CODE_1560
    ReplaceConstructorInclude(pOld);

    _name = pOld->_name;

}//@CODE_1560


/*@NOTE_7537
Constructor method used to copy from one project to the other.
*/
Type::Type(DataModelDoc* pDataModelDoc, Type* pType) //@INIT_7537
    : Gti(pDataModelDoc)
    , _name(pType->_name)
{//@CODE_7537
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_7537


/*@NOTE_74
Constructor needed for serialization, not meant to use for other purposes!
*/
Type::Type() //@INIT_74
    : Gti()
{//@CODE_74
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_74


/*@NOTE_72
Destructor method
*/
Type::~Type()
{//@CODE_72
    DestructorInclude();

    // Put in your own code
}//@CODE_72


bool Type::Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault)
{//@CODE_1581
    bool value = false;
    pGtiDropDefault = NULL;

    if (ctrlKeyDown)
    {
        value = true;
    }

    return value;
}//@CODE_1581


void Type::Drop(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1584
    DropOnClass(ctrlKeyDown, pGtiDrop);
    DropOnMethod(ctrlKeyDown, pGtiDrop);
    DropOnOtherTypes(ctrlKeyDown, pGtiDrop);
}//@CODE_1584


void Type::DropOnClass(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1643
    BaseClass* pDropClass = dynamic_cast<BaseClass*>(pGtiDrop);
    MemberAndMethodGroup* pDropMemberAndMethodGroup = 
        dynamic_cast<MemberAndMethodGroup*>(pGtiDrop);
        
    if (ctrlKeyDown)
    {
        if (pDropClass || pDropMemberAndMethodGroup)
        {
            if (pDropMemberAndMethodGroup)
                pDropClass = pDropMemberAndMethodGroup->GetBaseClass();
            
            Member* pNewMember = new Member(pDropClass, this);
            
            if (pNewMember->OnEditAttributes())
            {
                if (pDropMemberAndMethodGroup)
                    pDropMemberAndMethodGroup->AddMemberLast(pNewMember);
                
                pNewMember->Add();
                pDropClass->NotifyAddMember(pNewMember);
            }
            else
                GetDataModelDoc()->RollBack();
        }
    }
    else
    {
    }
}//@CODE_1643


void Type::DropOnMethod(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1646
    Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
        
    if (ctrlKeyDown)
    {
        if (pDropMethod)
        {
            int argumentCount = 0;
            Argument** pArgumentArray = 0;

            if (pDropMethod->GetVirtual())
            {
                pArgumentArray = new Argument*[GetDataModelDoc()->GetTypeCount()];

                DataModelDoc::TypeIterator iType(GetDataModelDoc(), &Type::IsExternClass);
                while (++iType)
                {
                    ExternClass* pDerivedClass = (ExternClass*)iType.Get();

                    if (pDerivedClass->IsBaseClass(pDropMethod->GetBaseClass()))
                    {
                        Method* pMethod = pDerivedClass->FindSimilarMethod(pDropMethod);
                        if (pMethod)
                            pArgumentArray[argumentCount++] = new Argument(pMethod, this);
                    }
                }
            }
            Argument* pNewArgument = new Argument(pDropMethod, this);
            
            if (pNewArgument->OnEditAttributes())
            {
                pNewArgument->Add();
            
                for (int i = 0; i < argumentCount; i++)
                    pArgumentArray[i]->Add();
            }
            else
            {
                GetDataModelDoc()->RollBack();
            }

            delete pArgumentArray;
        }
    }
    else
    {
    }
}//@CODE_1646


void Type::DropOnOtherTypes(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1649
    OtherTypes* pOtherTypes = dynamic_cast<OtherTypes*>(pGtiDrop);
    OtherType* pOtherType = dynamic_cast<OtherType*>(pGtiDrop);

    if (ctrlKeyDown)
    {
        if (pOtherType)
            pOtherTypes = pGtiDrop->GetDataModelDoc()->GetOtherTypes();

        if (pOtherTypes)
        {
            pOtherType = new OtherType(pGtiDrop->GetDataModelDoc());
            pOtherType->SetName(GetName());
            pOtherType->Add();
        }
    }
    else
    {
    }
}//@CODE_1649


bool Type::DropTarget(bool ctrlKeyDown, Gti* pGtiDrop)
{//@CODE_1587
    bool value = false;

    if (GetDataModelDoc() != pGtiDrop->GetDataModelDoc())
    {
        if (ctrlKeyDown)
        {
            OtherTypes* pOtherTypes = dynamic_cast<OtherTypes*>(pGtiDrop);
            OtherType* pOtherType = dynamic_cast<OtherType*>(pGtiDrop);
            if (pOtherType)
                pOtherTypes = pGtiDrop->GetDataModelDoc()->GetOtherTypes();

            if (pOtherTypes && !pGtiDrop->GetDataModelDoc()->FindType(GetName()))
            {
                value = true;
            }
        }
    }
    else
    {
        if (ctrlKeyDown)
        {
            Method* pDropMethod = dynamic_cast<Method*>(pGtiDrop);
            if (pDropMethod && 
                !pDropMethod->IsFixed() &&
                !pDropMethod->IsMemberMethod() &&
                !pDropMethod->IsFindMethod() &&
                !pDropMethod->IsDestructor() &&
                !pDropMethod->IsSerializeConstructor() &&
                !pDropMethod->IsReplaceConstructor())
            {
                value = true;
            }
            if (pGtiDrop->IsExternClass() ||
                pGtiDrop->IsMemberAndMethodGroup())
            {
                value = true;
            }
        }
    }

    return value;
}//@CODE_1587


CbString Type::GetFirstLowerName()
{//@CODE_1391
    CbString firstLowerName = _name.Left(1);
    firstLowerName.MakeLower();
    firstLowerName += _name.Right(_name.GetLength()-1);

    return firstLowerName;
}//@CODE_1391


CbString Type::GetFirstUpperName()
{//@CODE_1392
    CbString firstUpperName = _name.Left(1);
    firstUpperName.MakeUpper();
    firstUpperName += _name.Right(_name.GetLength()-1);

    return firstUpperName;
}//@CODE_1392


CbString Type::GetTemplate()
{//@CODE_7449
    CbString value;

    return value;
}//@CODE_7449


/*@NOTE_23039
This method is a hook to update the view in case the object changes state because
of an Undo/Redo. It is called after the object changed state. This method calls
OnUndoRedoAdded(), so overwrite this virtual method at derived classes if needed,
or change the default behaviour. 
*/
void Type::OnUndoRedoChanged(DataModelDocObject* pOldState)
{//@CODE_23039
    Gti::OnUndoRedoChanged(pOldState);

    Type* pType = (Type*)pOldState;

    if (pType && pType->GetName() != GetName())
    {
        Update();
    }
}//@CODE_23039


/*@NOTE_4791
Returns the value of member '_name'.
*/
CbString Type::GetName()
{//@CODE_4791
    return _name;
}//@CODE_4791


/*@NOTE_4792
Set the value of member '_name' to 'rName'.
*/
void Type::SetName(const CbString& rName)
{//@CODE_4792
    if (_name != rName)
    {
        SaveState();

        if (!_name.IsEmpty())
        {
            CbString oldName[5];
            CbString newName[5];

            int cnt = 0;
            oldName[cnt]   = _name;
            newName[cnt++] = rName;
            oldName[cnt]   = "p" + _name;
            newName[cnt++] = "p" + rName;
            oldName[cnt]   = "r" + _name;
            newName[cnt++] = "r" + rName;
            oldName[cnt]   = "pr" + _name;
            newName[cnt++] = "pr" + rName;
            oldName[cnt]   = "pp" + _name;
            newName[cnt++] = "pp" + rName;

            // Renaming a type rewrites the name in EVERY object that references
            // it; each change fires NotifyStructureChanged (a full Qt tree rebuild +
            // diagram repaints). Lock once so all those refreshes coalesce into
            // a single rebuild at the end -- this is what made the rename take
            // tens of seconds (the MFC tree updated incrementally; the Qt tree
            // rebuilds wholesale).
            CbViewLock lock(GetDataModelDoc());

            VariableIterator iVariable(this);//, &Variable::IsArgument);
            while (++iVariable)
            {
                for (int i = 0; i < cnt; i++)
                {
                    if (oldName[i] == iVariable->GetName())
                    {
                        iVariable->SetName(newName[i]);
                    }
                }
            }

            DataModelDoc::DataModelDocObjectIterator iDataModelDocObject(GetDataModelDoc());
            while (++iDataModelDocObject)
            {
                for (int i = 0; i < cnt; i++)
                {
                    iDataModelDocObject->ReplaceInX(oldName[i], newName[i]);
                }
            }

        }

        _name = rName;
    }
}//@CODE_4792


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5564
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Type::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_71
Method which must be called first in a constructor
*/
void Type::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_MULTI_OWNED_ACTIVE(Type, Type, Variable, Variable)
    INIT_MULTI_OWNED_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    INIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_73
Method which must be called first in a destructor
*/
void Type::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(Type, Type, Variable, Variable)
    EXIT_MULTI_OWNED_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    EXIT_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_5565
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Type::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    REMOVE_MULTI_OWNED_ACTIVE(Type, Type, Variable, Variable)
    Gti::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_1562
Method which must be called first in a replace constructor
*/
void Type::ReplaceConstructorInclude(Type* pOld)
{
    REPLACE_MULTI_OWNED_ACTIVE(Type, Type, Variable, Variable)
    REPLACE_MULTI_OWNED_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    REPLACE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_5566
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Type::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    Type* pType = (Type*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5568
Save the state of the current object relations to pDataModelDocObject.
*/
void Type::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    Type* pType = (Type*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_76
Serialize the members only to a CbObject object
*/
void Type::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _name;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _name;
        }
    }
}


/*@NOTE_75
Method which must be called first in a serialize constructor
*/
void Type::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(Type, Type, Variable, Variable)
    INIT_MULTI_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    INIT_MULTI_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)
}


/*@NOTE_78
Serialize the relations to a CbObject object
*/
void Type::SerializeRelations(CbArchive& archive,
                              DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(Type, Type, Variable, Variable)
        WRITE_MULTI_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(Type, Type, Variable, Variable)
            READ_MULTI_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Type)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(Type, Type, Variable, Variable)
METHODS_ITERATOR_MULTI_ACTIVE(Type, Type, Variable, Variable)
METHODS_MULTI_OWNED_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
METHODS_ITERATOR_MULTI_ACTIVE(Type, Type, ExceptionSpecificationType, ExceptionSpecificationType)
METHODS_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, Type, Type)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
