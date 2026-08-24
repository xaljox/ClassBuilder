/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ActorLifeLineShape.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ActorLifeLineShape'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
//@START_USER1
//@END_USER1


// Master include file
#include "StdAfx.h"


//@START_USER2
#include "MainFrm.h"
#include "CbPainter.h"
//@END_USER2


// Static members


/*@NOTE_33873
Constructor method.
*/
ActorLifeLineShape::ActorLifeLineShape(SequenceDiagram* pSequenceDiagram,
                                       Actor* pActor,
                                       const CbPoint& point) //@INIT_33873
    : LifeLineShape(pSequenceDiagram, point)
{//@CODE_33873
    ConstructorInclude(pActor);

    // Put in your own code
    CbRect rect = GetRect();
    rect.right = rect.left + RecalculateRectWidth();
    rect.top = -SequenceDiagram::GetClassLifeLineOffset();
    rect.bottom = rect.top + SequenceDiagram::GetClassLifeLineHeight();

    SetRect(rect);
}//@CODE_33873


/*@NOTE_33778
Constructor needed for serialization, not meant to use for other purposes!
*/
ActorLifeLineShape::ActorLifeLineShape() //@INIT_33778
    : LifeLineShape()
{//@CODE_33778
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_33778


/*@NOTE_33776
Destructor method.
*/
ActorLifeLineShape::~ActorLifeLineShape()
{//@CODE_33776
    DestructorInclude();

    // Put in your own code
}//@CODE_33776


void ActorLifeLineShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35112
    ActorLifeLineShape* pActorLifeLineShape =
        new ActorLifeLineShape(pSequenceDiagram, GetActor(), CbPoint(0, 0));
    pActorLifeLineShape->CopyState(this);
    _ptrIndex = intptr_t(pActorLifeLineShape);
}//@CODE_35112


void ActorLifeLineShape::Draw(CbPainter& painter,
                              SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              bool selected)
{//@CODE_40417
    CbRect rect = GetRect();
    rect.right = rect.left + RecalculateRectWidth();
    SetRect(rect);

    int save = painter.Save();
    painter.SetNullBrush();
    painter.FillSolidRect(_rect, painter.GetBkColor());
    painter.SetFont(CBF_LIFELINE);

    painter.SetTextAlign(TA_CENTER|TA_TOP|TA_NOUPDATECP);
    CbColorRef oldBkColor = painter.GetBkColor();
    unsigned int options = ETO_CLIPPED;

    painter.SetTextColor(GetTextColor());
    if (selected)
    {
        painter.SetBkColor(CbPainter::GetSelectFillColor());
        options = ETO_CLIPPED | ETO_OPAQUE;
    }

    // Limit the coloured area.
    CbRect clipRect = GetRect();
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 2,
        options, clipRect, GetName() + " : ");
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 36,
        ETO_CLIPPED, clipRect, GetTypeName());

    painter.SetBkColor(oldBkColor);
    painter.SetTextColor(GetTextColor());

    CbColorRef color = GetPenColor();
    int penWidth = 1;
    if (selected)
    {
        color = CbPainter::GetSelectColor();
        penWidth = 2;
    }
    painter.SetPen(PS_SOLID, penWidth, color);

    const int size = 5;
    CbPoint refPoint = _rect.CenterPoint();
    Shape::Round(refPoint);
    refPoint.y = _rect.top + 150;
    CbRect head(refPoint, refPoint);
    head.InflateRect(3*size, 3*size);
    painter.Ellipse(head);
    painter.DrawLine(refPoint+CbSize(0, -3*size),   refPoint+CbSize(0, -10*size));
    painter.DrawLine(refPoint+CbSize(-4*size, -5*size), refPoint+CbSize(4*size, -5*size));
    painter.DrawLine(refPoint+CbSize(0, -10*size),  refPoint+CbSize(-5*size, -15*size));
    painter.DrawLine(refPoint+CbSize(0, -10*size),  refPoint+CbSize(5*size, -15*size));

    painter.SetPen(PS_DOT, 1, color);
    painter.DrawLine(GetStartPoint(), GetEndPoint());

    painter.Restore(save);
}//@CODE_40417


ActorLifeLineShape* ActorLifeLineShape::GetActorLifeLine()
{//@CODE_33882
    return this;
}//@CODE_33882


CbRect ActorLifeLineShape::GetActorRect()
{//@CODE_34119
    CbRect actorRect;
    CbPoint centerPoint = _rect.CenterPoint();
    Shape::Round(centerPoint);
    actorRect.left = centerPoint.x - 25;
    actorRect.right = centerPoint.x + 25;
    actorRect.bottom = _rect.top +165;
    actorRect.top = _rect.top + 75;

    return actorRect;
}//@CODE_34119


CbRect ActorLifeLineShape::GetBoundingRect()
{//@CODE_34116
    CbRect rect = GetRect();
    
    rect *= GetLifeLineRect();
    rect *= GetActorRect();
    
    return rect;
}//@CODE_34116


CbString ActorLifeLineShape::GetTypeName()
{//@CODE_33883
    return GetActor()->GetName();
}//@CODE_33883


int ActorLifeLineShape::OnEditAttributes(bool checkOnly)
{//@CODE_33884
    return GetActor()->OnEditAttributes(checkOnly);
}//@CODE_33884


bool ActorLifeLineShape::PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                      const CbPoint& pointLP)
{//@CODE_40880
    if (SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP))
    {
        return true;
    }

    return GetActorRect().PtInRect(pointLP) || GetLifeLineRect().PtInRect(pointLP);
}//@CODE_40880


int ActorLifeLineShape::RecalculateRectWidth()
{//@CODE_33899
    int width = 50; // The miniumum

    CbPainter* pMeasure = CbPainter::GetMeasurePainter();
    if (pMeasure)
    {
        int saved = pMeasure->Save();
        pMeasure->SetFont(CBF_LIFELINE);

        CbSize textSize = pMeasure->GetTextExtent(GetName() + " : ");
        int tmpWidth = ((textSize.cx+textSize.cx/20+10)/10)*10;
        if (width < tmpWidth)
            width = tmpWidth;
        textSize = pMeasure->GetTextExtent(GetTypeName());
        tmpWidth = ((textSize.cx+textSize.cx/20+10)/10)*10;
        if (width < tmpWidth)
            width = tmpWidth;

        pMeasure->Restore(saved);
    }
    else
    {
        int tmpWidth = GetRect().Width();
        if (tmpWidth == 0)
            tmpWidth = 250;

        if (width < tmpWidth)
            width = tmpWidth;
    }
    
    return width;
}//@CODE_33899


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_33785
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ActorLifeLineShape::CleanupReferences()
{
    LifeLineShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
}


/*@NOTE_33775
Method which must be called first in a constructor.
*/
void ActorLifeLineShape::ConstructorInclude(Actor* pActor)
{
    INIT_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
}


/*@NOTE_33777
Method which must be called first in a destructor.
*/
void ActorLifeLineShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
}


/*@NOTE_33786
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ActorLifeLineShape::RemoveReferences()
{
    LifeLineShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
}


/*@NOTE_33787
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ActorLifeLineShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ActorLifeLineShape* pActorLifeLineShape = (ActorLifeLineShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
    LifeLineShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_33789
Save the state of the current object relations to pDataModelDocObject.
*/
void ActorLifeLineShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    LifeLineShape::SaveReferences(pDataModelDocObject);
    ActorLifeLineShape* pActorLifeLineShape = (ActorLifeLineShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
}


/*@NOTE_33780
Serialize the members only to a CbObject object.
*/
void ActorLifeLineShape::Serialize(CbArchive& archive)
{
    LifeLineShape::Serialize(archive);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


/*@NOTE_33779
Method which must be called first in a serialize constructor.
*/
void ActorLifeLineShape::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)
}


/*@NOTE_33782
Serialize the relations to a CbObject object.
*/
void ActorLifeLineShape::SerializeRelations(CbArchive& archive,
                                            DataModelDocObject* pointerArray[])
{
    LifeLineShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ActorLifeLineShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Actor, Actor, ActorLifeLineShape, ActorLifeLineShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
