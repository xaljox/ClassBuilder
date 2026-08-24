/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassLifeLineShape.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassLifeLineShape'
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


/*@NOTE_32484
Constructor method.
*/
ClassLifeLineShape::ClassLifeLineShape(SequenceDiagram* pSequenceDiagram,
                                       BaseClass* pBaseClass,
                                       const CbPoint& point) //@INIT_32484
    : LifeLineShape(pSequenceDiagram, point)
    , _autoWidth(true)
    , _template(pBaseClass->GetTemplate())
    , _templateRect()
{//@CODE_32484
    ConstructorInclude(pBaseClass);

    // Put in your own code
    CbRect rect = GetRect();
    rect.right = rect.left + RecalculateRectWidth();
    rect.top = -SequenceDiagram::GetClassLifeLineOffset();
    rect.bottom = rect.top + SequenceDiagram::GetClassLifeLineHeight();

    SetRect(rect);
}//@CODE_32484


/*@NOTE_30665
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassLifeLineShape::ClassLifeLineShape() //@INIT_30665
    : LifeLineShape()
    , _autoWidth(true)
    , _template()
    , _templateRect()
{//@CODE_30665
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_30665


/*@NOTE_30663
Destructor method.
*/
ClassLifeLineShape::~ClassLifeLineShape()
{//@CODE_30663
    DestructorInclude();

    // Put in your own code
}//@CODE_30663


void ClassLifeLineShape::CopyShape(SequenceDiagram* pSequenceDiagram)
{//@CODE_35114
    ClassLifeLineShape* pClassLifeLineShape =
        new ClassLifeLineShape(pSequenceDiagram, GetBaseClass(), CbPoint(0, 0));
    pClassLifeLineShape->CopyState(this);
    _ptrIndex = intptr_t(pClassLifeLineShape);
}//@CODE_35114


void ClassLifeLineShape::Draw(CbPainter& painter,
                              SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              bool selected)
{//@CODE_40421
    CbRect rect = GetRect();
    rect.right = rect.left + RecalculateRectWidth();

    if (GetFirstChildActivationShape() &&
        GetFirstChildActivationShape()->GetCreation())
    {
        rect.top = GetFirstChildActivationShape()->GetRect().bottom;
        rect.bottom = rect.top + SequenceDiagram::GetClassLifeLineHeight();
    }
    else
    {
        rect.top = -SequenceDiagram::GetClassLifeLineOffset();
        rect.bottom = rect.top + SequenceDiagram::GetClassLifeLineHeight();
    }
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
    CbColorRef penColor = GetPenColor(painter);
    painter.SetPen(PS_SOLID, 1, penColor);

    painter.DrawRect(_rect);

    CbPoint point(GetRect().right, GetRect().bottom);
    CbRect templateRect(point, point);
    if (!GetTemplate().IsEmpty())
    {
        templateRect = GetTemplateRect(painter, true);
    }
    SetTemplateRect(templateRect);

    if (selected)
    {
        penColor = CbPainter::GetSelectColor();
        DrawSelectedRect(painter, penColor);
    }

    painter.SetPen(PS_DOT, 1, penColor);
    painter.DrawLine(GetStartPoint(), GetEndPoint());

    painter.Restore(save);
}//@CODE_40421


void ClassLifeLineShape::DrawSelectedRect(CbPainter& painter, CbColorRef color)
{//@CODE_33524
    painter.DrawSelectionHandle(GetLeftSelectedPoint(),  color);
    painter.DrawSelectionHandle(GetRightSelectedPoint(), color);
}//@CODE_33524


CbRect ClassLifeLineShape::GetBoundingRect()
{//@CODE_34115
    CbRect rect = GetRect();
    
    rect *= GetLifeLineRect();
    rect *= GetTemplateRect();
    
    return rect;
}//@CODE_34115


ClassLifeLineShape* ClassLifeLineShape::GetClassLifeLine()
{//@CODE_33553
    return this;
}//@CODE_33553


CbPoint ClassLifeLineShape::GetLeftSelectedPoint()
{//@CODE_33522
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.left;

    return value;
}//@CODE_33522


int ClassLifeLineShape::GetLifeLineLength()
{//@CODE_34342
    ChildActivationShape* pDestructionChildActivationShape = 
        GetDestructionChildActivationShape();
    if (pDestructionChildActivationShape)
    {
        return _rect.top - pDestructionChildActivationShape->GetRect().top;
    }
    else if (GetFirstChildActivationShape() && 
             GetFirstChildActivationShape()->GetCreation())
    {
        return LifeLineShape::GetLifeLineLength() + 
            _rect.top + SequenceDiagram::GetClassLifeLineOffset();
    }
    else
    {
        return LifeLineShape::GetLifeLineLength();
    }
}//@CODE_34342


CbColorRef ClassLifeLineShape::GetPenColor(CbPainter& painter)
{//@CODE_34392
    if (painter.IsScreen() && WrongCreationOrDestruction())
    {
        return Cb_RGB(255, 0, 0);
    }
    else
    {
        return Shape::GetPenColor();
    }
}//@CODE_34392


CbPoint ClassLifeLineShape::GetRightSelectedPoint()
{//@CODE_33523
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.right;

    return value;
}//@CODE_33523


CbRect ClassLifeLineShape::GetTemplateRect(CbPainter& painter, bool draw)
{//@CODE_35264
    int save = painter.Save();

    painter.SetPen(PS_DASH, 1, GetPenColor(painter));

    CbString templateText = GetTemplate();
    templateText.Replace('<', ' ');
    templateText.Replace('>', ' ');

    painter.SetFont(CBF_SIGNAL);
    painter.SetTextAlign(TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);

    CbSize textSize = painter.GetTextExtent(templateText);
    CbPoint point(GetRect().right-textSize.cx/2, GetRect().bottom-textSize.cy/2);
    CbRect rect(point, textSize);
    rect.NormalizeRect();

    if (draw)
    {
        painter.ExtTextOut(rect.left, rect.top,
            ETO_CLIPPED | ETO_OPAQUE, rect, templateText);

        painter.DrawRect(rect);
    }

    painter.Restore(save);
    
    return rect;
}//@CODE_35264


CbString ClassLifeLineShape::GetTypeName()
{//@CODE_33536
    return GetBaseClass()->Type::GetName();
}//@CODE_33536


int ClassLifeLineShape::OnEditAttributes(bool checkOnly)
{//@CODE_33279
    return GetBaseClass()->OnEditAttributes(checkOnly);
}//@CODE_33279


bool ClassLifeLineShape::PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                      const CbPoint& pointLP)
{//@CODE_40877
    if (SequenceDiagramShape::PointInShape(pSequenceDiagramViewModel, pointLP))
    {
        return true;
    }

    return GetLifeLineRect().PtInRect(pointLP);
}//@CODE_40877


int ClassLifeLineShape::RecalculateRectWidth()
{//@CODE_33491
    int width = 100; // the minimum width
    if (GetAutoWidth())
        width = 250; // The miniumum if autoWidth is on
    
    CbPainter* pMeasure = CbPainter::GetMeasurePainter();
    if (pMeasure && GetAutoWidth())
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
        if (width < tmpWidth)
            width = tmpWidth;
    }
    
    return width;
}//@CODE_33491


bool ClassLifeLineShape::WrongCreationOrDestruction()
{//@CODE_34395
    ChildActivationShapeIterator iChildActivationShape(this);
    while (++iChildActivationShape)
    {
        if (iChildActivationShape->WrongCreationOrDestruction())
        {
            return true;
        }
    }

    return false;
}//@CODE_34395


/*@NOTE_35262
Set the value of member '_templateRect' to 'rTemplateRect'.
*/
void ClassLifeLineShape::SetTemplateRect(const CbRect& rTemplateRect)
{//@CODE_35262
    if (_templateRect != rTemplateRect)
    {
        // _templateRect is the auto-width measured header box -- DERIVED, re-
        // measured/re-positioned every paint to track the lifeline (the user
        // never sets it directly). Serialized but redundant. OptimizePlacement's
        // shift is already captured by the preceding SetRectNoSort snapshot
        // (SequenceDiagram.cpp:1118 / 860), so this SaveState was redundant there
        // too -- removed so the derived recompute records no undo.
        _templateRect = rTemplateRect;
    }
}//@CODE_35262


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_30672
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassLifeLineShape::CleanupReferences()
{
    LifeLineShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
}


/*@NOTE_30662
Method which must be called first in a constructor.
*/
void ClassLifeLineShape::ConstructorInclude(BaseClass* pBaseClass)
{
    INIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
}


/*@NOTE_30664
Method which must be called first in a destructor.
*/
void ClassLifeLineShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
}


/*@NOTE_30673
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassLifeLineShape::RemoveReferences()
{
    LifeLineShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
}


/*@NOTE_30674
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassLifeLineShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassLifeLineShape* pClassLifeLineShape = (ClassLifeLineShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
    LifeLineShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_30676
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassLifeLineShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    LifeLineShape::SaveReferences(pDataModelDocObject);
    ClassLifeLineShape* pClassLifeLineShape = (ClassLifeLineShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
}


/*@NOTE_30667
Serialize the members only to a CbObject object.
*/
void ClassLifeLineShape::Serialize(CbArchive& archive)
{
    LifeLineShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _autoWidth;
        archive << _template;
        archive << _templateRect;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _autoWidth;
            archive >> _template;
            archive >> _templateRect;
        }
    }
}


/*@NOTE_30666
Method which must be called first in a serialize constructor.
*/
void ClassLifeLineShape::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)
}


/*@NOTE_30669
Serialize the relations to a CbObject object.
*/
void ClassLifeLineShape::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(ClassLifeLineShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassLifeLineShape, ClassLifeLineShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
