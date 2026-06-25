/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'MemberShape'
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


MemberShape::MemberShape(ClassShape* pClassShape, Member* pMember) //@INIT_3836
    : ClassDiagramShape(pClassShape->GetClassDiagram(), Cb_RGB(0, 0, 0),
        pClassShape->GetClassDiagram()->GetDataModelDoc()->GetMemberTextColor())
{//@CODE_3836
    ConstructorInclude(pMember, pClassShape);

    // Put in your own code
}//@CODE_3836


/*@NOTE_3870
Constructor needed for serialization, not meant to use for other purposes!
*/
MemberShape::MemberShape() //@INIT_3870
    : ClassDiagramShape()
{//@CODE_3870
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3870


/*@NOTE_3385
Destructor method.
*/
MemberShape::~MemberShape()
{//@CODE_3385
    DestructorInclude();

    // Put in your own code
}//@CODE_3385


void MemberShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35126
    MemberShape* pMemberShape =
        new MemberShape((ClassShape*)GetClassShape()->_ptrIndex, GetMember());
    pMemberShape->CopyState(this);
    _ptrIndex = intptr_t(pMemberShape);
}//@CODE_35126


void MemberShape::Draw(CbPainter& painter,
                       ClassDiagramViewModel* pClassDiagramViewModel,
                       bool selected)
{//@CODE_40391
    painter.Save();
    if (GetMember()->GetStatic())
        painter.SetFont(CBF_STATIC_MEMBER);
    else
        painter.SetFont(CBF_MEMBER);
    painter.SetTextAlign(TA_LEFT|TA_TOP|TA_NOUPDATECP);

    CbString text = GetMember()->GetShapeText();

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
}//@CODE_40391


Gti* MemberShape::GetGti()
{//@CODE_3995
    return GetMember();
}//@CODE_3995


MemberShape* MemberShape::GetMemberShape()
{//@CODE_4484
    return this;
}//@CODE_4484


ClassDiagramShape* MemberShape::GetOuterClassDiagramShape()
{//@CODE_3977
    return GetClassShape()->GetOuterClassDiagramShape();
}//@CODE_3977


int MemberShape::OnEditAttributes(bool checkOnly)
{//@CODE_3939
    return GetMember()->OnEditAttributes(checkOnly);
}//@CODE_3939


int MemberShape::UsesPenColor() const
{//@CODE_19895
    return 0;
}//@CODE_19895


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5708
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void MemberShape::CleanupReferences()
{
    ClassDiagramShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
    CLEANUP_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
}


/*@NOTE_3384
Method which must be called first in a constructor.
*/
void MemberShape::ConstructorInclude(Member* pMember, ClassShape* pClassShape)
{
    INIT_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
    INIT_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
}


/*@NOTE_3386
Method which must be called first in a destructor.
*/
void MemberShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
    EXIT_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
}


/*@NOTE_5709
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void MemberShape::RemoveReferences()
{
    ClassDiagramShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    REMOVE_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
}


/*@NOTE_5710
Bring the current object relations into the same state as pDataModelDocObject.
*/
void MemberShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    MemberShape* pMemberShape = (MemberShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
    RESTORE_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    ClassDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5712
Save the state of the current object relations to pDataModelDocObject.
*/
void MemberShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape::SaveReferences(pDataModelDocObject);
    MemberShape* pMemberShape = (MemberShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
    SAVE_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
}


/*@NOTE_3872
Serialize the members only to a CbObject object.
*/
void MemberShape::Serialize(CbArchive& archive)
{
    ClassDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_3871
Method which must be called first in a serialize constructor.
*/
void MemberShape::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Member, Member, MemberShape, MemberShape)
    INIT_MULTI_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)
}


/*@NOTE_3874
Serialize the relations to a CbObject object.
*/
void MemberShape::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(MemberShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
METHODS_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
