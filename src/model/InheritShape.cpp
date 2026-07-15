/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          InheritShape.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'InheritShape'
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


InheritShape::InheritShape(ClassDiagram* pClassDiagram, Inherit* pInherit,
                           ClassShape* pFromClassShape,
                           ClassShape* pToClassShape) //@INIT_3833
    : ConnectionShape(pClassDiagram, pFromClassShape, pToClassShape,
        pClassDiagram->GetDataModelDoc()->GetInheritPenColor())
{//@CODE_3833
    ConstructorInclude(pInherit);

    // Put in your own code
    MakeNewRouting();
    RecalculateRect();
}//@CODE_3833


/*@NOTE_3849
Constructor needed for serialization, not meant to use for other purposes!
*/
InheritShape::InheritShape() //@INIT_3849
    : ConnectionShape()
{//@CODE_3849
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3849


/*@NOTE_4582
Copy constructor, needed for temporary copy, while modifying shape.
*/
InheritShape::InheritShape(InheritShape* pInheritShape) //@INIT_4582
    : ConnectionShape(pInheritShape)
{//@CODE_4582
    ConstructorInclude(pInheritShape->GetInherit());

    // Put in your own code
    ConvertRouting();
}//@CODE_4582


/*@NOTE_35302
Constructor method.
*/
InheritShape::InheritShape(ClassDiagram* pClassDiagram,
                           InheritShape* pInheritShape) //@INIT_35302
    : ConnectionShape(pClassDiagram, pInheritShape)
{//@CODE_35302
    ConstructorInclude(pInheritShape->GetInherit());

    // Put in your own code
    ConvertRouting();
}//@CODE_35302


/*@NOTE_3380
Destructor method.
*/
InheritShape::~InheritShape()
{//@CODE_3380
    DestructorInclude();

    // Put in your own code
}//@CODE_3380


/*@NOTE_4504
Convert segments, to draw correct start and end styles.
*/
void InheritShape::ConvertRouting()
{//@CODE_4504
    // Replace first and last segment with appropiate segments
    ConnectionSegment* pFirst = GetFirstConnectionSegment();
    if (pFirst && !pFirst->IsReplaced())
        (void)new InheritStartSegment(pFirst);
 
    ConnectionSegment* pLast = GetLastConnectionSegment();
    if (pLast && !pLast->IsReplaced())
        (void)new InheritEndSegment(pLast);
}//@CODE_4504


void InheritShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35134
    InheritShape* pInheritShape = new InheritShape(pClassDiagram, this);
    pInheritShape->CopyState(this);
    _ptrIndex = intptr_t(pInheritShape);
}//@CODE_35134


void InheritShape::Draw(CbPainter& painter,
                        ClassDiagramViewModel* pClassDiagramViewModel,
                        bool selected)
{//@CODE_40379
    if (!GetHidden())
    {
        if (GetFromClassShape() != GetToClassShape())
        {
            painter.Save();
            CbColorRef penColor = selected ? CbPainter::GetSelectColor() : GetPenColor();
            painter.SetPen(PS_SOLID, selected ? 3 : 1, penColor);
            painter.SetSolidBrush(penColor);
            ConnectionShape::Draw(painter, pClassDiagramViewModel, selected);
            painter.Restore();
        }
    }
}//@CODE_40379


InheritShape* InheritShape::GetInheritShape()
{//@CODE_3969
    return this;
}//@CODE_3969


/*@NOTE_4506
Make a brand new routing, delete old routing first.
*/
void InheritShape::MakeNewRouting()
{//@CODE_4506
    ConnectionShape::MakeNewRouting();
    ConvertRouting();
}//@CODE_4506


int InheritShape::OnDelete(bool checkOnly)
{//@CODE_4549
    return GetInherit()->OnDelete(checkOnly);
}//@CODE_4549


int InheritShape::OnEditAttributes(bool checkOnly)
{//@CODE_3938
    return GetInherit()->OnEditAttributes(checkOnly);
}//@CODE_3938


int InheritShape::UsesTextColor() const
{//@CODE_19894
    return 0;
}//@CODE_19894


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5702
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void InheritShape::CleanupReferences()
{
    ConnectionShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
}


/*@NOTE_3379
Method which must be called first in a constructor.
*/
void InheritShape::ConstructorInclude(Inherit* pInherit)
{
    INIT_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
}


/*@NOTE_3381
Method which must be called first in a destructor.
*/
void InheritShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
}


/*@NOTE_5703
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void InheritShape::RemoveReferences()
{
    ConnectionShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
}


/*@NOTE_5704
Bring the current object relations into the same state as pDataModelDocObject.
*/
void InheritShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    InheritShape* pInheritShape = (InheritShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
    ConnectionShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5706
Save the state of the current object relations to pDataModelDocObject.
*/
void InheritShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape::SaveReferences(pDataModelDocObject);
    InheritShape* pInheritShape = (InheritShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
}


/*@NOTE_3851
Serialize the members only to a CbObject object.
*/
void InheritShape::Serialize(CbArchive& archive)
{
    ConnectionShape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_3850
Method which must be called first in a serialize constructor.
*/
void InheritShape::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)
}


/*@NOTE_3853
Serialize the relations to a CbObject object.
*/
void InheritShape::SerializeRelations(CbArchive& archive,
                                      DataModelDocObject* pointerArray[])
{
    ConnectionShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(InheritShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Inherit, Inherit, InheritShape, InheritShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
