/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassDiagramShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassDiagramShape'
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


ClassDiagramShape::ClassDiagramShape(ClassDiagram* pClassDiagram,
                                     const CbPoint& point, CbColorRef penColor,
                                     CbColorRef textColor) //@INIT_3819
    : Shape(pClassDiagram->GetDataModelDoc(), point, penColor, textColor)
{//@CODE_3819
    ConstructorInclude(pClassDiagram);

    // Put in your own code
}//@CODE_3819


ClassDiagramShape::ClassDiagramShape(ClassDiagram* pClassDiagram,
                                     CbColorRef penColor,
                                     CbColorRef textColor) //@INIT_4179
    : Shape(pClassDiagram->GetDataModelDoc(), penColor, textColor)
{//@CODE_4179
    ConstructorInclude(pClassDiagram);

    // Put in your own code
}//@CODE_4179


/*@NOTE_3881
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassDiagramShape::ClassDiagramShape() //@INIT_3881
    : Shape()
{//@CODE_3881
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3881


/*@NOTE_3365
Destructor method.
*/
ClassDiagramShape::~ClassDiagramShape()
{//@CODE_3365
    DestructorInclude();

    // Put in your own code
}//@CODE_3365


void ClassDiagramShape::AlignBottom(Shape* pShape)
{//@CODE_35340
    CbRect rect = GetRect();
    CbSize offset(0, pShape->GetRect().top - rect.top);
    Shape::Round(offset);
    SetRect(rect + offset);

    rect.InflateRect(10, 10, 11, 11);
    GetClassDiagram()->MoveNoteShapePoints(rect, offset);
}//@CODE_35340


void ClassDiagramShape::AlignCenter(Shape* pShape)
{//@CODE_35332
    CbRect rect = GetRect();
    CbSize offset(pShape->GetRect().CenterPoint().x - rect.CenterPoint().x, 0);
    Shape::Round(offset);
    SetRect(rect + offset);

    rect.InflateRect(10, 10, 11, 11);
    GetClassDiagram()->MoveNoteShapePoints(rect, offset);
}//@CODE_35332


void ClassDiagramShape::AlignLeft(Shape* pShape)
{//@CODE_35330
    CbRect rect = GetRect();
    CbSize offset(pShape->GetRect().left - rect.left, 0);
    SetRect(rect + offset);

    rect.InflateRect(10, 10, 11, 11);
    GetClassDiagram()->MoveNoteShapePoints(rect, offset);
}//@CODE_35330


void ClassDiagramShape::AlignMiddle(Shape* pShape)
{//@CODE_35338
    CbRect rect = GetRect();
    CbSize offset(0, pShape->GetRect().CenterPoint().y - rect.CenterPoint().y);
    Shape::Round(offset);
    SetRect(rect + offset);

    rect.InflateRect(10, 10, 11, 11);
    GetClassDiagram()->MoveNoteShapePoints(rect, offset);
}//@CODE_35338


void ClassDiagramShape::AlignRight(Shape* pShape)
{//@CODE_35334
    CbRect rect = GetRect();
    CbSize offset(pShape->GetRect().right - rect.right, 0);
    SetRect(rect + offset);

    rect.InflateRect(10, 10, 11, 11);
    GetClassDiagram()->MoveNoteShapePoints(rect, offset);
}//@CODE_35334


void ClassDiagramShape::AlignTop(Shape* pShape)
{//@CODE_35336
    CbRect rect = GetRect();
    CbSize offset(0, pShape->GetRect().bottom - rect.bottom);
    SetRect(rect + offset);

    rect.InflateRect(10, 10, 11, 11);
    GetClassDiagram()->MoveNoteShapePoints(rect, offset);
}//@CODE_35336


void ClassDiagramShape::Draw(CbPainter& painter,
                             ClassDiagramViewModel* pClassDiagramViewModel,
                             bool selected)
{//@CODE_40403
}//@CODE_40403


/*@NOTE_27463
Returns a non zero value if this shape can be drawn directly. Returns a zero value if
this item is drawn indirectly as sub part of another item.
*/
bool ClassDiagramShape::DrawDirect() const
{//@CODE_27463
    return (const_cast<ClassDiagramShape*>(this)->
        GetOuterClassDiagramShape() == this);
}//@CODE_27463


void ClassDiagramShape::DrawSelectBox(CbPainter& painter)
{//@CODE_40361
    painter.Save();
    painter.SetPen(PS_SOLID, 2, CbPainter::GetSelectColor());
    painter.SetNullBrush();
    painter.Rectangle(GetRect());
    painter.Restore();
}//@CODE_40361


ClassDiagramViewModelSelection* ClassDiagramShape::FindClassDiagramViewModelSelection(ClassDiagramViewModel* pClassDiagramViewModel)
{//@CODE_40336
    ClassDiagramViewModelSelectionIterator iClassDiagramViewModelSelection(this);
    while (++iClassDiagramViewModelSelection)
    {
        if (pClassDiagramViewModel == iClassDiagramViewModelSelection->GetClassDiagramViewModel())
        {
            return iClassDiagramViewModelSelection;
        }
    }

    return 0;
}//@CODE_40336


CbRect ClassDiagramShape::GetBoundingRect()
{//@CODE_35258
    return GetRect();
}//@CODE_35258


ClassShape* ClassDiagramShape::GetClassShape() const
{//@CODE_27548
    return 0;
}//@CODE_27548


ConnectionShape* ClassDiagramShape::GetConnectionShape()
{//@CODE_27549
    return 0;
}//@CODE_27549


DependencyShape* ClassDiagramShape::GetDependencyShape()
{//@CODE_27550
    return 0;
}//@CODE_27550


Gti* ClassDiagramShape::GetGti()
{//@CODE_27518
    return 0;
}//@CODE_27518


ClassDiagramShape* ClassDiagramShape::GetHitShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                                  CbPoint pointLP, bool nested)
{//@CODE_40859
    return this;
}//@CODE_40859


InheritShape* ClassDiagramShape::GetInheritShape()
{//@CODE_27551
    return 0;
}//@CODE_27551


MemberShape* ClassDiagramShape::GetMemberShape()
{//@CODE_27552
    return 0;
}//@CODE_27552


MethodShape* ClassDiagramShape::GetMethodShape()
{//@CODE_27553
    return 0;
}//@CODE_27553


NoteShape* ClassDiagramShape::GetNoteShape()
{//@CODE_27554
    return 0;
}//@CODE_27554


/*@NOTE_3976
Returns a pointer to the outer ClassDiagramShape, is only overruled at the Member and 
Method Shape since it are embeded shapes and thus have an outer ClassDiagramShape. 
In the other case the current object pointer is returned.
*/
ClassDiagramShape* ClassDiagramShape::GetOuterClassDiagramShape()
{//@CODE_3976
    return this;
}//@CODE_3976


RelationDiagramOnlyShape* ClassDiagramShape::GetRelationDiagramOnlyShape()
{//@CODE_27555
    return 0;
}//@CODE_27555


RelationShape* ClassDiagramShape::GetRelationShape()
{//@CODE_27556
    return 0;
}//@CODE_27556


int ClassDiagramShape::IsAlignShape() const
{//@CODE_35325
    return 0;
}//@CODE_35325


int ClassDiagramShape::IsSelectedIn(ClassDiagramViewModel* pClassDiagramViewModel)
{//@CODE_40334
    if (!pClassDiagramViewModel)
        return 0;
    if (FindClassDiagramViewModelSelection(pClassDiagramViewModel))
    {
        return 1;
    }
    return 0;
}//@CODE_40334


int ClassDiagramShape::OnDelete(bool checkOnly)
{//@CODE_27520
    if (GetGti())
    {
        return GetGti()->OnDelete(checkOnly);
    }
    else
    {
        return false;
    }
}//@CODE_27520


int ClassDiagramShape::OnOpen(bool checkOnly)
{//@CODE_27524
    int result = 0;
    if (GetGti())
        result = GetGti()->OnOpen(checkOnly);
    else if (checkOnly == false)
        result = OnEditAttributes();
    
    return result;
}//@CODE_27524


bool ClassDiagramShape::PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                     CbPoint pointLP)
{//@CODE_40841
    return Shape::PointInShape(pointLP);
}//@CODE_40841


int ClassDiagramShape::UsesPenColor() const
{//@CODE_19892
    return 1;
}//@CODE_19892


int ClassDiagramShape::UsesTextColor() const
{//@CODE_19893
    return 1;
}//@CODE_19893


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5684
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassDiagramShape::CleanupReferences()
{
    Shape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
}


/*@NOTE_3364
Method which must be called first in a constructor.
*/
void ClassDiagramShape::ConstructorInclude(ClassDiagram* pClassDiagram)
{
    INIT_MULTI_OWNED_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
    INIT_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
}


/*@NOTE_3366
Method which must be called first in a destructor.
*/
void ClassDiagramShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
    EXIT_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
}


/*@NOTE_41470
Method that returns true if it is actually a ClassShape Object.
*/
bool ClassDiagramShape::IsClassShape() const
{
    return (dynamic_cast<const ClassShape*>(this) != nullptr);
}


/*@NOTE_41472
Method that returns true if it is actually a ConnectionShape Object.
*/
bool ClassDiagramShape::IsConnectionShape() const
{
    return (dynamic_cast<const ConnectionShape*>(this) != nullptr);
}


/*@NOTE_41476
Method that returns true if it is actually a DependencyShape Object.
*/
bool ClassDiagramShape::IsDependencyShape() const
{
    return (dynamic_cast<const DependencyShape*>(this) != nullptr);
}


/*@NOTE_41474
Method that returns true if it is actually a InheritShape Object.
*/
bool ClassDiagramShape::IsInheritShape() const
{
    return (dynamic_cast<const InheritShape*>(this) != nullptr);
}


/*@NOTE_41478
Method that returns true if it is actually a MemberShape Object.
*/
bool ClassDiagramShape::IsMemberShape() const
{
    return (dynamic_cast<const MemberShape*>(this) != nullptr);
}


/*@NOTE_41477
Method that returns true if it is actually a MethodShape Object.
*/
bool ClassDiagramShape::IsMethodShape() const
{
    return (dynamic_cast<const MethodShape*>(this) != nullptr);
}


/*@NOTE_41471
Method that returns true if it is actually a NoteShape Object.
*/
bool ClassDiagramShape::IsNoteShape() const
{
    return (dynamic_cast<const NoteShape*>(this) != nullptr);
}


/*@NOTE_41475
Method that returns true if it is actually a RelationDiagramOnlyShape Object.
*/
bool ClassDiagramShape::IsRelationDiagramOnlyShape() const
{
    return (dynamic_cast<const RelationDiagramOnlyShape*>(this) != nullptr);
}


/*@NOTE_41473
Method that returns true if it is actually a RelationShape Object.
*/
bool ClassDiagramShape::IsRelationShape() const
{
    return (dynamic_cast<const RelationShape*>(this) != nullptr);
}


/*@NOTE_5685
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassDiagramShape::RemoveReferences()
{
    EXIT_MULTI_OWNED_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
    Shape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
}


/*@NOTE_5686
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassDiagramShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape* pClassDiagramShape = (ClassDiagramShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
    Shape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5688
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassDiagramShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Shape::SaveReferences(pDataModelDocObject);
    ClassDiagramShape* pClassDiagramShape = (ClassDiagramShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
}


/*@NOTE_3883
Serialize the members only to a CbObject object.
*/
void ClassDiagramShape::Serialize(CbArchive& archive)
{
    Shape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_3882
Method which must be called first in a serialize constructor.
*/
void ClassDiagramShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
    INIT_MULTI_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
}


/*@NOTE_3885
Serialize the relations to a CbObject object.
*/
void ClassDiagramShape::SerializeRelations(CbArchive& archive,
                                           DataModelDocObject* pointerArray[])
{
    Shape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
METHODS_ITERATOR_MULTI_ACTIVE(ClassDiagramShape, ClassDiagramShape, ClassDiagramViewModelSelection, ClassDiagramViewModelSelection)
METHODS_MULTI_OWNED_PASSIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
