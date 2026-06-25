/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ExternClasses.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ExternClasses'
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


ExternClasses::ExternClasses(DataModelDoc* pDataModelDoc) //@INIT_952
    : Gti(pDataModelDoc)
{//@CODE_952
    ConstructorInclude(pDataModelDoc);

    // Put in your own code
}//@CODE_952


/*@NOTE_334
Constructor needed for serialization, not meant to use for other purposes!
*/
ExternClasses::ExternClasses() //@INIT_334
    : Gti()
{//@CODE_334
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_334


/*@NOTE_332
Destructor method
*/
ExternClasses::~ExternClasses()
{//@CODE_332
    DestructorInclude();

    // Put in your own code
}//@CODE_332


void ExternClasses::Add()
{//@CODE_954
    if (!GetAdded())
    {
        SetItemText(CbString("Extern Classes"));
        SetIcon(ICON_FILE);

        Gti::Add();

        DataModelDoc::BaseClassIterator 
            externClass(GetDataModelDoc(), &BaseClass::IsExternClass);
        while (++externClass)
            externClass->Add();
    }
}//@CODE_954


Gti* ExternClasses::GetNext(Gti* pGti)
{//@CODE_35375
    Gti* pNextGti = Gti::GetNext(pGti);

    if (!pNextGti)
    {
        pNextGti = GetDataModelDoc()->GetOtherTypes();
    }

    return pNextGti;
}//@CODE_35375


int ExternClasses::OnAddExternClass(bool checkOnly)
{//@CODE_41498
    if (!checkOnly)
    {
        GetDataModelDoc()->MarkLastUndo();
        ExternClass* pExternClass = new ExternClass(GetDataModelDoc());

        if (pExternClass->OnEditAttributes())
            pExternClass->Add();
        else
            GetDataModelDoc()->RollBack();
    }

    return 1;
}//@CODE_41498


int ExternClasses::OnPaste(Gti* pGti, bool checkOnly)
{//@CODE_7587
    int result = 0;
    
    ExternClass* pExternClass = dynamic_cast<ExternClass*>(pGti);
    if (pExternClass && pExternClass->GetDataModelDoc() != GetDataModelDoc())
    {
        if (!checkOnly)
        {
            GetDataModelDoc()->FindOrDuplicateExternClass(pExternClass);
        }        
        result = 1;
    }
    
    DataModel* pDataModel = dynamic_cast<DataModel*>(pGti);
    if (pDataModel && pDataModel->GetDataModelDoc() != GetDataModelDoc())
    {
        if (!checkOnly)
        {
            DataModel::ClassIterator iClass(pDataModel);
            while (++iClass)
            {
                GetDataModelDoc()->FindOrDuplicateExternClass(iClass);
            }
        }        
        result = 1;
    }
    
    ExternClasses* pExternClasses = dynamic_cast<ExternClasses*>(pGti);
    if (pExternClasses && pExternClasses->GetDataModelDoc() != GetDataModelDoc())
    {
        if (!checkOnly)
        {
            DataModelDoc::BaseClassIterator iBaseClass(pExternClasses->GetDataModelDoc());
            while (++iBaseClass)
            {
                if (!iBaseClass->IsClass())
                {
                    GetDataModelDoc()->FindOrDuplicateType(iBaseClass);
                }
            }
        }        
        result = 1;
    }
    
    if (result)
    {
        if (!checkOnly)
        {
            DataModelDoc::TypeIterator iType(GetDataModelDoc());
            while (++iType)
            {
                if (!iType->GetAdded())
                {
                    iType->Add();
                }
            }
        }
    }
    else
    {
        result = Gti::OnPaste(pGti, checkOnly);
    }
          
    return result;
}//@CODE_7587


void ExternClasses::Update()
{//@CODE_955
    Gti::Update();
}//@CODE_955


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5408
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ExternClasses::CleanupReferences()
{
    Gti::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
}


/*@NOTE_331
Method which must be called first in a constructor
*/
void ExternClasses::ConstructorInclude(DataModelDoc* pDataModelDoc)
{
    INIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
}


/*@NOTE_333
Method which must be called first in a destructor
*/
void ExternClasses::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
}


/*@NOTE_5409
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ExternClasses::RemoveReferences()
{
    Gti::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
}


/*@NOTE_5410
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ExternClasses::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ExternClasses* pExternClasses = (ExternClasses*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
    Gti::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5412
Save the state of the current object relations to pDataModelDocObject.
*/
void ExternClasses::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Gti::SaveReferences(pDataModelDocObject);
    ExternClasses* pExternClasses = (ExternClasses*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
}


/*@NOTE_336
Serialize the members only to a CbObject object
*/
void ExternClasses::Serialize(CbArchive& archive)
{
    Gti::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_335
Method which must be called first in a serialize constructor
*/
void ExternClasses::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)
}


/*@NOTE_338
Serialize the relations to a CbObject object
*/
void ExternClasses::SerializeRelations(CbArchive& archive,
                                       DataModelDocObject* pointerArray[])
{
    Gti::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ExternClasses)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ExternClasses, ExternClasses)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
