/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MethodShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MethodShape'
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
#include "CbPainter.h"
//@END_USER2


// Static members


MethodShape::MethodShape(ClassShape* pClassShape, Method* pMethod) //@INIT_3839
    : ClassDiagramShape(pClassShape->GetClassDiagram(), Cb_RGB(0, 0, 0),
        pClassShape->GetClassDiagram()->GetDataModelDoc()->GetMethodTextColor())
{//@CODE_3839
    ConstructorInclude(pMethod, pClassShape);

    // Put in your own code


}//@CODE_3839


/*@NOTE_3863
Constructor needed for serialization, not meant to use for other purposes!
*/
MethodShape::MethodShape() //@INIT_3863
    : ClassDiagramShape()
{//@CODE_3863
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3863


/*@NOTE_3390
Destructor method.
*/
MethodShape::~MethodShape()
{//@CODE_3390
    DestructorInclude();

    // Put in your own code
}//@CODE_3390


void MethodShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35128
    MethodShape* pMethodShape =
        new MethodShape((ClassShape*)GetClassShape()->_ptrIndex, GetMethod());
    pMethodShape->CopyState(this);
    _ptrIndex = intptr_t(pMethodShape);
}//@CODE_35128


void MethodShape::Draw(CbPainter& painter,
                       ClassDiagramViewModel* pClassDiagramViewModel,
                       bool selected)
{//@CODE_40395
    painter.Save();
    if (GetMethod()->GetStatic())
        painter.SetFont(CBF_STATIC_METHOD);
    else
        painter.SetFont(CBF_METHOD);
    painter.SetTextAlign(TA_LEFT|TA_TOP|TA_NOUPDATECP);

    CbString text = GetMethod()->GetShapeText(VerbosityType(GetClassShape()->GetVerbosity()));

    unsigned int options = ETO_CLIPPED;
    painter.SetTextColor(GetTextColor());
    if (painter.IsScreen() && selected)
    {
        painter.SetBkColor(CbPainter::GetSelectFillColor());
        options = ETO_CLIPPED | ETO_OPAQUE;
    }

    painter.ExtTextOut(GetRect().left + 8, GetRect().bottom - 2,
                       options, GetRect(), text);

    painter.Restore();
}//@CODE_40395


Gti* MethodShape::GetGti()
{//@CODE_3996
    return GetMethod();
}//@CODE_3996


MethodShape* MethodShape::GetMethodShape()
{//@CODE_4485
    return this;
}//@CODE_4485


ClassDiagramShape* MethodShape::GetOuterClassDiagramShape()
{//@CODE_3978
    return GetClassShape()->GetOuterClassDiagramShape();
}//@CODE_3978


int MethodShape::OnEditAttributes(bool checkOnly)
{//@CODE_3940
    return GetMethod()->OnEditAttributes(checkOnly);
}//@CODE_3940


int MethodShape::UsesPenColor() const
{//@CODE_19896
    return 0;
}//@CODE_19896


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5714
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MethodShape::CleanupReferences()
{
    ClassDiagramShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
    CLEANUP_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
}


/*@NOTE_3389
Method which must be called first in a constructor.
*/
void MethodShape::ConstructorInclude(Method* pMethod, ClassShape* pClassShape)
{
    INIT_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
    INIT_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
}


/*@NOTE_3391
Method which must be called first in a destructor.
*/
void MethodShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
    EXIT_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
}


/*@NOTE_5715
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MethodShape::RemoveReferences()
{
    ClassDiagramShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    REMOVE_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
}


/*@NOTE_5716
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MethodShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MethodShape* pMethodShape = (MethodShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
    RESTORE_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    ClassDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5718
Save the state of the current object relations to pDataModelDocObject.
*/
void MethodShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape::SaveReferences(pDataModelDocObject);
    MethodShape* pMethodShape = (MethodShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
    SAVE_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
}


/*@NOTE_3865
Serialize the members only to a CbObject object.
*/
void MethodShape::Serialize(CbArchive& archive)
{
    ClassDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_3864
Method which must be called first in a serialize constructor.
*/
void MethodShape::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Method, Method, MethodShape, MethodShape)
    INIT_MULTI_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)
}


/*@NOTE_3867
Serialize the relations to a CbObject object.
*/
void MethodShape::SerializeRelations(CbArchive& archive,
                                     DataModelDocObject* pointerArray[])
{
    ClassDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(MethodShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Method, Method, MethodShape, MethodShape)
METHODS_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MethodShape, MethodShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
