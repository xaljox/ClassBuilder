/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RootActivationShape.cpp
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RootActivationShape'
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


/*@NOTE_31193
Constructor needed for serialization, not meant to use for other purposes!
*/
RootActivationShape::RootActivationShape() //@INIT_31193
    : ParentActivationShape()
{//@CODE_31193
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_31193


/*@NOTE_32493
Constructor method.
*/
RootActivationShape::RootActivationShape(SequenceDiagram* pSequenceDiagram) //@INIT_32493
    : ParentActivationShape(pSequenceDiagram)
{//@CODE_32493
    ConstructorInclude(pSequenceDiagram);

    // Put in your own code
}//@CODE_32493


/*@NOTE_31191
Destructor method.
*/
RootActivationShape::~RootActivationShape()
{//@CODE_31191
    DestructorInclude();

    // Put in your own code
}//@CODE_31191


void RootActivationShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35116
    RootActivationShape* pRootActivationShape =
        new RootActivationShape(pSequenceDiagram);
    pRootActivationShape->CopyState(this);
    _ptrIndex = intptr_t(pRootActivationShape);
}//@CODE_35116


void RootActivationShape::Draw(CbPainter& painter,
                               SequenceDiagramViewModel* pSequenceDiagramViewModel,
                               bool selected)
{//@CODE_40429

}//@CODE_40429


int RootActivationShape::OnEditAttributes(bool checkOnly)
{//@CODE_33221
    return 0;
}//@CODE_33221


int RootActivationShape::OnOpen(bool checkOnly)
{//@CODE_33223
    return 0;
}//@CODE_33223


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_31200
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RootActivationShape::CleanupReferences()
{
    ParentActivationShape::CleanupReferences();
    CLEANUP_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
}


/*@NOTE_31190
Method which must be called first in a constructor.
*/
void RootActivationShape::ConstructorInclude(SequenceDiagram* pSequenceDiagram)
{
    INIT_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
}


/*@NOTE_31192
Method which must be called first in a destructor.
*/
void RootActivationShape::DestructorInclude()
{
    EXIT_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
}


/*@NOTE_31201
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RootActivationShape::RemoveReferences()
{
    ParentActivationShape::RemoveReferences();
    REMOVE_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
}


/*@NOTE_31202
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RootActivationShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RootActivationShape* pRootActivationShape = (RootActivationShape*)pDataModelDocObject;
    RESTORE_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
    ParentActivationShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_31204
Save the state of the current object relations to pDataModelDocObject.
*/
void RootActivationShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ParentActivationShape::SaveReferences(pDataModelDocObject);
    RootActivationShape* pRootActivationShape = (RootActivationShape*)pDataModelDocObject;
    SAVE_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
}


/*@NOTE_31195
Serialize the members only to a CbObject object.
*/
void RootActivationShape::Serialize(CbArchive& archive)
{
    ParentActivationShape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_31194
Method which must be called first in a serialize constructor.
*/
void RootActivationShape::SerializeConstructorInclude()
{
    INIT_SINGLE_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
}


/*@NOTE_31197
Serialize the relations to a CbObject object.
*/
void RootActivationShape::SerializeRelations(CbArchive& archive,
                                             DataModelDocObject* pointerArray[])
{
    ParentActivationShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(RootActivationShape)


// Methods for the relation(s) of the class
METHODS_SINGLE_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
