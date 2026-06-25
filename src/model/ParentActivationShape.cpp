/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ParentActivationShape.cpp
* Creation date: June 25, 2026 19:48
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ParentActivationShape'
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
#include <vector>
#include <utility>
//@END_USER2


// Static members


/*@NOTE_33012
Constructor method.
*/
ParentActivationShape::ParentActivationShape(SequenceDiagram* pSequenceDiagram,
                                             CbColorRef penColor,
                                             CbColorRef textColor) //@INIT_33012
    : SequenceDiagramShape(pSequenceDiagram, penColor, textColor)
{//@CODE_33012
    ConstructorInclude();

    // Put in your own code
}//@CODE_33012


/*@NOTE_30798
Constructor needed for serialization, not meant to use for other purposes!
*/
ParentActivationShape::ParentActivationShape() //@INIT_30798
    : SequenceDiagramShape()
{//@CODE_30798
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_30798


/*@NOTE_30796
Destructor method.
*/
ParentActivationShape::~ParentActivationShape()
{//@CODE_30796
    DestructorInclude();

    // Put in your own code
}//@CODE_30796


void ParentActivationShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35140
    ChildActivationShapeIterator iChildActivationShape(this);
    while (++iChildActivationShape)
    {
        iChildActivationShape->CopyShape(pSequenceDiagram);
    }
}//@CODE_35140


unsigned short ParentActivationShape::GetHeight()
{//@CODE_33569
    unsigned short height = GetOffset();
    if (GetChildActivation() && GetChildActivation()->GetCreation() && 
        GetChildActivation()->GetLifeLineShape()->GetFirstChildActivationShape() == this)
    {
        height += 30;
    }

    if (GetChildActivationShapeCount())
    {
        if (NeedExtraSpaceBefore())
        {
            height += SequenceDiagram::GetActivationSpaceBefore();
        }
            
        ChildActivationShapeIterator iChildActivationShape(this);
        while (++iChildActivationShape)
        {
            height += SequenceDiagram::GetActivationSpaceBefore();
            if (iChildActivationShape->IsRecursiveActivation())
            {
                height += SequenceDiagram::GetActivationSpaceRecursive();
            }
            height += iChildActivationShape->GetHeight();
            height += SequenceDiagram::GetActivationSpaceAfter();
        }
        
        if (NeedExtraSpaceAfter())
        {
            height += SequenceDiagram::GetActivationSpaceAfter();
        }
    }
    else
    {
        height += SequenceDiagram::GetActivationMinimalHeight();
    }
    
    return height;
}//@CODE_33569


LifeLineShape* ParentActivationShape::GetLifeLineShape() const
{//@CODE_33570
    return 0;
}//@CODE_33570


CbString ParentActivationShape::GetNumbering()
{//@CODE_34207
    CbString value;

    return value;
}//@CODE_34207


int ParentActivationShape::GetOffset() const
{//@CODE_34266
    return 0;
}//@CODE_34266


ParentActivationShape* ParentActivationShape::GetParentActivationShape() const
{//@CODE_33621
    return 0;
}//@CODE_33621


bool ParentActivationShape::IsDirectOrIndirectChild(ChildActivationShape* pChildActivationShape)
{//@CODE_34130
    if (this == pChildActivationShape)
    {
        return true;
    }

    if (GetParentActivationShape())
    {
        return GetParentActivationShape()->IsDirectOrIndirectChild(pChildActivationShape);
    }
    
    return false;
}//@CODE_34130


bool ParentActivationShape::IsRecursiveActivation()
{//@CODE_33590
    return false;
}//@CODE_33590


bool ParentActivationShape::NeedExtraSpaceAfter()
{//@CODE_33623
    return false;
}//@CODE_33623


bool ParentActivationShape::NeedExtraSpaceBefore()
{//@CODE_33622
    return false;
}//@CODE_33622


int ParentActivationShape::RecalculateRect(int start, int& sequenceNumber,
                                           std::vector<std::pair<CbRect,CbRect>>* moved)
{//@CODE_33587
    start += GetOffset();
    if (GetChildActivation() && GetChildActivation()->GetCreation() &&
        GetChildActivation()->GetLifeLineShape()->GetFirstChildActivationShape() == this)
    {
        start += 30;
    }
    // Snap each activation's start to the grid so its rect.bottom (-start) -- where
    // signal arrows attach -- lands ON a grid line. The base (272+25-2) and the
    // 25-unit spacings are off-grid, so arrows otherwise sit ~5 off; note-shape
    // connector points always round to multiples of 10, so grid-aligning the arrow
    // Y makes points coincide with lines by construction (no snap/drift). Layout
    // is derived (recomputed at paint, not serialized), so this is cosmetic only.
    start = ((start + 5) / 10) * 10;
    int end = start;

    if (GetChildActivation() && !GetParentActivationShape()->IsRootActivationShape())
    {
        GetChildActivation()->SetSequenceNumber(sequenceNumber++);
    }

    if (GetChildActivationShapeCount())
    {
        if (NeedExtraSpaceBefore())
        {
            end += SequenceDiagram::GetActivationSpaceBefore();
        }
        
        int sequenceSubNumber = 1;
        ChildActivationShapeIterator iChildActivationShape(this);
        while (++iChildActivationShape)
        {
            end += SequenceDiagram::GetActivationSpaceBefore();
            if (iChildActivationShape->IsRecursiveActivation())
            {
                end += SequenceDiagram::GetActivationSpaceRecursive();
            }
            end = iChildActivationShape->RecalculateRect(end, sequenceNumber, moved);
            end += SequenceDiagram::GetActivationSpaceAfter();
            
            iChildActivationShape->SetSequenceSubNumber(sequenceSubNumber++);
        }
        
        if (NeedExtraSpaceAfter())
        {
            end += SequenceDiagram::GetActivationSpaceAfter();
        }
    }
    else
    {
        end += SequenceDiagram::GetActivationMinimalHeight();
    }
    
    if (GetLifeLineShape())
    {
        int offset = 0;
        if (GetLifeLineShape()->GetShowActivations())
        {
            ParentActivationShape* pParentActivationShape = GetParentActivationShape();
            while (pParentActivationShape)
            {
                ChildActivationShape* pChildActivationShape = 
                    pParentActivationShape->GetChildActivation();
                if (pChildActivationShape && 
                    pChildActivationShape->GetLifeLineShape() == GetLifeLineShape())
                {
                    offset += SequenceDiagram::GetActivationOffsetRecursive();
                }
                
                pParentActivationShape = pParentActivationShape->GetParentActivationShape();
            }
        }
        
        CbRect rect;
        rect.left = GetLifeLineShape()->GetLeftActivation()+offset;
        rect.right = rect.left + SequenceDiagram::GetActivationWidth();
        rect.bottom = -start;
        rect.top = -end;
        CbRect oldRect = GetRect();
        SetRect(rect);
        if (moved && oldRect != rect)               // record the move for note-follow
            moved->push_back(std::make_pair(oldRect, rect));
    }
    
    return end;
    
}//@CODE_33587


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_30805
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ParentActivationShape::CleanupReferences()
{
    SequenceDiagramShape::CleanupReferences();
}


/*@NOTE_30795
Method which must be called first in a constructor.
*/
void ParentActivationShape::ConstructorInclude()
{
    INIT_MULTI_OWNED_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_30797
Method which must be called first in a destructor.
*/
void ParentActivationShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_30806
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ParentActivationShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    SequenceDiagramShape::RemoveReferences();
}


/*@NOTE_30807
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ParentActivationShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_30809
Save the state of the current object relations to pDataModelDocObject.
*/
void ParentActivationShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    SequenceDiagramShape::SaveReferences(pDataModelDocObject);
}


/*@NOTE_30800
Serialize the members only to a CbObject object.
*/
void ParentActivationShape::Serialize(CbArchive& archive)
{
    SequenceDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_30799
Method which must be called first in a serialize constructor.
*/
void ParentActivationShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
}


/*@NOTE_30802
Serialize the relations to a CbObject object.
*/
void ParentActivationShape::SerializeRelations(CbArchive& archive,
                                               DataModelDocObject* pointerArray[])
{
    SequenceDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
METHODS_ITERATOR_NOFILTER_MULTI_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
