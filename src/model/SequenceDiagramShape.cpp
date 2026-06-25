/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SequenceDiagramShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'SequenceDiagramShape'
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


/*@NOTE_30449
Constructor method.
*/
SequenceDiagramShape::SequenceDiagramShape(SequenceDiagram* pSequenceDiagram,
                                           const CbPoint& point,
                                           CbColorRef penColor,
                                           CbColorRef textColor) //@INIT_30449
    : Shape(pSequenceDiagram->GetDataModelDoc(), point, penColor, textColor)
{//@CODE_30449
    ConstructorInclude(pSequenceDiagram);

    // Put in your own code
}//@CODE_30449


/*@NOTE_32504
Constructor method.
*/
SequenceDiagramShape::SequenceDiagramShape(SequenceDiagram* pSequenceDiagram,
                                           CbColorRef penColor,
                                           CbColorRef textColor) //@INIT_32504
    : Shape(pSequenceDiagram->GetDataModelDoc(), penColor, textColor)
{//@CODE_32504
    ConstructorInclude(pSequenceDiagram);

    // Put in your own code
}//@CODE_32504


/*@NOTE_29848
Constructor needed for serialization, not meant to use for other purposes!
*/
SequenceDiagramShape::SequenceDiagramShape() //@INIT_29848
    : Shape()
{//@CODE_29848
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_29848


/*@NOTE_29846
Destructor method.
*/
SequenceDiagramShape::~SequenceDiagramShape()
{//@CODE_29846
    DestructorInclude();

    // Put in your own code
}//@CODE_29846


void SequenceDiagramShape::Draw(CbPainter& painter,
                                SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                bool selected)
{//@CODE_40409
}//@CODE_40409


SequenceDiagramViewModelSelection* SequenceDiagramShape::FindSequenceDiagramViewModelSelection(SequenceDiagramViewModel* pSequenceDiagramViewModel)
{//@CODE_40338
    SequenceDiagramViewModelSelectionIterator iSequenceDiagramViewModelSelection(this);
    while (++iSequenceDiagramViewModelSelection)
    {
        if (pSequenceDiagramViewModel == iSequenceDiagramViewModelSelection->GetSequenceDiagramViewModel())
        {
            return iSequenceDiagramViewModelSelection;
        }
    }

    return 0;
}//@CODE_40338


ActorLifeLineShape* SequenceDiagramShape::GetActorLifeLine()
{//@CODE_33881
    return 0;
}//@CODE_33881


CbRect SequenceDiagramShape::GetBoundingRect()
{//@CODE_34112
    return GetRect();
}//@CODE_34112


ChildActivationShape* SequenceDiagramShape::GetChildActivation()
{//@CODE_33548
    return 0;
}//@CODE_33548


ClassLifeLineShape* SequenceDiagramShape::GetClassLifeLine()
{//@CODE_33550
    return 0;
}//@CODE_33550


SequenceDiagramShape* SequenceDiagramShape::GetHitShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                                        const CbPoint& pointLP,
                                                        bool nested)
{//@CODE_40889
    return this;
}//@CODE_40889


LifeLineShape* SequenceDiagramShape::GetLifeLine()
{//@CODE_33549
    return 0;
}//@CODE_33549


SDNoteShape* SequenceDiagramShape::GetNoteShape()
{//@CODE_34789
    return 0;
}//@CODE_34789


SequenceDiagramShape* SequenceDiagramShape::GetOuterSequenceDiagramShape()
{//@CODE_30491
    return this;
}//@CODE_30491


SignalShape* SequenceDiagramShape::GetSignal()
{//@CODE_33551
    return 0;
}//@CODE_33551


int SequenceDiagramShape::IsSelectedIn(SequenceDiagramViewModel* pSequenceDiagramViewModel)
{//@CODE_40407
    if (!pSequenceDiagramViewModel)
        return 0;
    if (FindSequenceDiagramViewModelSelection(pSequenceDiagramViewModel))
    {
        return 1;
    }
    return 0;
}//@CODE_40407


int SequenceDiagramShape::OnDelete(bool checkOnly)
{//@CODE_30460
    if (!checkOnly)
    {
        Delete();
    }
    
    return true;

}//@CODE_30460


bool SequenceDiagramShape::PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                        const CbPoint& pointLP)
{//@CODE_40871
    return Shape::PointInShape(pointLP);
}//@CODE_40871


int SequenceDiagramShape::UsesPenColor() const
{//@CODE_30475
    return 1;
}//@CODE_30475


int SequenceDiagramShape::UsesTextColor() const
{//@CODE_30476
    return 1;
}//@CODE_30476


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_29855
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void SequenceDiagramShape::CleanupReferences()
{
    Shape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
}


/*@NOTE_29845
Method which must be called first in a constructor.
*/
void SequenceDiagramShape::ConstructorInclude(SequenceDiagram* pSequenceDiagram)
{
    INIT_MULTI_OWNED_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
    INIT_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
}


/*@NOTE_29847
Method which must be called first in a destructor.
*/
void SequenceDiagramShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
    EXIT_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
}


/*@NOTE_41490
Method that returns true if it is actually a ActorLifeLineShape Object.
*/
bool SequenceDiagramShape::IsActorLifeLineShape() const
{
    return (dynamic_cast<const ActorLifeLineShape*>(this) != nullptr);
}


/*@NOTE_41494
Method that returns true if it is actually a ChildActivationShape Object.
*/
bool SequenceDiagramShape::IsChildActivationShape() const
{
    return (dynamic_cast<const ChildActivationShape*>(this) != nullptr);
}


/*@NOTE_41489
Method that returns true if it is actually a ClassLifeLineShape Object.
*/
bool SequenceDiagramShape::IsClassLifeLineShape() const
{
    return (dynamic_cast<const ClassLifeLineShape*>(this) != nullptr);
}


/*@NOTE_41488
Method that returns true if it is actually a LifeLineShape Object.
*/
bool SequenceDiagramShape::IsLifeLineShape() const
{
    return (dynamic_cast<const LifeLineShape*>(this) != nullptr);
}


/*@NOTE_41492
Method that returns true if it is actually a ParentActivationShape Object.
*/
bool SequenceDiagramShape::IsParentActivationShape() const
{
    return (dynamic_cast<const ParentActivationShape*>(this) != nullptr);
}


/*@NOTE_41493
Method that returns true if it is actually a RootActivationShape Object.
*/
bool SequenceDiagramShape::IsRootActivationShape() const
{
    return (dynamic_cast<const RootActivationShape*>(this) != nullptr);
}


/*@NOTE_41495
Method that returns true if it is actually a SDNoteShape Object.
*/
bool SequenceDiagramShape::IsSDNoteShape() const
{
    return (dynamic_cast<const SDNoteShape*>(this) != nullptr);
}


/*@NOTE_41491
Method that returns true if it is actually a SignalShape Object.
*/
bool SequenceDiagramShape::IsSignalShape() const
{
    return (dynamic_cast<const SignalShape*>(this) != nullptr);
}


/*@NOTE_29856
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void SequenceDiagramShape::RemoveReferences()
{
    EXIT_MULTI_OWNED_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
    Shape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
}


/*@NOTE_29857
Bring the current object relations into the same state as pDataModelDocObject.
*/
void SequenceDiagramShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape* pSequenceDiagramShape = (SequenceDiagramShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
    Shape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_29859
Save the state of the current object relations to pDataModelDocObject.
*/
void SequenceDiagramShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    Shape::SaveReferences(pDataModelDocObject);
    SequenceDiagramShape* pSequenceDiagramShape = (SequenceDiagramShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
}


/*@NOTE_29850
Serialize the members only to a CbObject object.
*/
void SequenceDiagramShape::Serialize(CbArchive& archive)
{
    Shape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_29849
Method which must be called first in a serialize constructor.
*/
void SequenceDiagramShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
    INIT_MULTI_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
}


/*@NOTE_29852
Serialize the relations to a CbObject object.
*/
void SequenceDiagramShape::SerializeRelations(CbArchive& archive,
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
METHODS_MULTI_OWNED_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
METHODS_ITERATOR_MULTI_ACTIVE(SequenceDiagramShape, SequenceDiagramShape, SequenceDiagramViewModelSelection, SequenceDiagramViewModelSelection)
METHODS_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
