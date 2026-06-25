/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ClassShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'ClassShape'
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
#include "MainFrm.h"
//@END_USER2


// Static members
bool ClassShape::_tracking = false;


ClassShape::ClassShape(ClassDiagram* pClassDiagram, BaseClass* pBaseClass,
                       const CbPoint& point) //@INIT_3826
    : ClassDiagramShape(pClassDiagram, point,
        pClassDiagram->GetDataModelDoc()->GetClassPenColor(),
        pClassDiagram->GetDataModelDoc()->GetClassTextColor())
    , _line1Point1(point)
    , _line1Point2(point)
    , _line2Point1(point)
    , _line2Point2(point)
    , _verbosity(0)
    , _autoWidth(true)
    , _templateRect()
{//@CODE_3826
    ConstructorInclude(pBaseClass);
    
    // Put in your own code
    RecalculateRect();

    Class* pClass = dynamic_cast<Class*>(pBaseClass);
    if (pClass)
    {
        Class::FromRelationIterator iFromRelation(pClass);
        while (++iFromRelation)
        {
            ClassShape* pToClassShape = iFromRelation->GetToClass()->FindClassShape(pClassDiagram);
            while (pToClassShape)
            {
                (void)new RelationShape(pClassDiagram, iFromRelation, this, pToClassShape);

				pToClassShape = iFromRelation->GetToClass()->FindClassShape(pClassDiagram, pToClassShape);
            }
        }
        Class::ToRelationIterator iToRelation(pClass);
        while (++iToRelation)
        {
            ClassShape* pFromClassShape = iToRelation->GetFromClass()->FindClassShape(pClassDiagram);
            while (pFromClassShape && pFromClassShape != this)
            {
                (void)new RelationShape(pClassDiagram, iToRelation, pFromClassShape, this);

				pFromClassShape = iToRelation->GetFromClass()->FindClassShape(pClassDiagram, pFromClassShape);
            }
        }
    }

    ExternClass* pExternClass = dynamic_cast<ExternClass*>(pBaseClass);
    if (pExternClass)
    {
        ExternClass::InheritIterator iInherit(pExternClass);
        while (++iInherit)
        {
            ClassShape* pBaseClassShape = iInherit->GetBaseClass()->FindClassShape(pClassDiagram);
            while (pBaseClassShape)
            {
                (void)new InheritShape(pClassDiagram, iInherit, pBaseClassShape, this);

				pBaseClassShape = iInherit->GetBaseClass()->FindClassShape(pClassDiagram, pBaseClassShape);
            }
        }
    }

    BaseClass::InheritIterator iInherit(pBaseClass);
    while (++iInherit)
    {
        ClassShape* pExternClassShape = iInherit->GetExternClass()->FindClassShape(pClassDiagram);
        while (pExternClassShape)
        {
            (void)new InheritShape(pClassDiagram, iInherit, this, pExternClassShape);

			pExternClassShape = iInherit->GetExternClass()->FindClassShape(pClassDiagram, pExternClassShape);
        }
    }

    // Auto-populate Member/MethodShapes per the diagram's visibility flags.
    // No-op for the click-to-create-new-class path (BaseClass has no members
    // yet); meaningful when an existing class is added to the diagram.
    PopulateFromDiagramFlags();
}//@CODE_3826


/*@NOTE_3856
Constructor needed for serialization, not meant to use for other purposes!
*/
ClassShape::ClassShape() //@INIT_3856
    : ClassDiagramShape()
    , _verbosity(0)
    , _autoWidth(false)
    , _templateRect()
{//@CODE_3856
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3856


/*@NOTE_35278
Constructor method.
*/
ClassShape::ClassShape(ClassDiagram* pClassDiagram,
                       ClassShape* pClassShape) //@INIT_35278
    : ClassDiagramShape(pClassDiagram,
        pClassDiagram->GetDataModelDoc()->GetClassPenColor(),
        pClassDiagram->GetDataModelDoc()->GetClassTextColor())
    , _line1Point1()
    , _line1Point2()
    , _line2Point1()
    , _line2Point2()
    , _verbosity(0)
    , _autoWidth(true)
    , _templateRect()
{//@CODE_35278
    ConstructorInclude(pClassShape->GetBaseClass());

    // Put in your own code
}//@CODE_35278


/*@NOTE_3370
Destructor method.
*/
ClassShape::~ClassShape()
{//@CODE_3370
    DestructorInclude();

    // Put in your own code
}//@CODE_3370


/*@NOTE_4387
Calculates the connection point, when connected to other ClassShape.
*/
CbPoint ClassShape::ConnectionPoint(ClassShape* pClassShape)
{//@CODE_4387
    return Shape::CrossPoint(GetRect(), pClassShape->GetRect().CenterPoint());
}//@CODE_4387


void ClassShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35124
    ClassShape* pClassShape = new ClassShape(pClassDiagram, this);
    pClassShape->CopyState(this);
    _ptrIndex = intptr_t(pClassShape);

    MemberShapeIterator iMemberShape(this);
    while (++iMemberShape)
    {
        iMemberShape->CopyShape(pClassDiagram);
    }
    
    MethodShapeIterator iMethodShape(this);
    while (++iMethodShape)
    {
        iMethodShape->CopyShape(pClassDiagram);
    }
}//@CODE_35124


void ClassShape::Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected)
{//@CODE_40387
    // Paint-neutrality: a Draw must not mutate the model. Geometry (box size +
    // the connection reroute a resize triggers) is recomputed at the EDIT, in
    // DataModelDoc::MarkLastUndo. The serialized _rect / _line points / sub-rects
    // are already correct here, so Draw just paints.
    painter.Save();
    painter.SetNullBrush();
    painter.FillSolidRect(_rect, painter.GetBkColor());

    if (painter.IsScreen() && selected)
        painter.FillSolidRect(CbRect(_rect.left, _line1Point1.y, _rect.right, _rect.bottom),
                              CbPainter::GetSelectFillColor());

    BaseClass::MethodIterator iMethod(GetBaseClass(), &Method::GetPure);
    if (++iMethod)
        painter.SetFont(CBF_ABSTRACT_CLASS);
    else
        painter.SetFont(CBF_CLASS_NAME);
    painter.SetTextAlign(TA_CENTER|TA_TOP|TA_NOUPDATECP);
    painter.SetTextColor(GetTextColor());

    CbRect clipRect = GetRect();
    if (clipRect.top < _line1Point1.y)
        clipRect.top = _line1Point1.y;
    painter.ExtTextOut(_rect.CenterPoint().x, GetRect().bottom - 2,
        ETO_CLIPPED, clipRect, GetBaseClass()->Type::GetName());

    // Members/methods are drawn unselected here; the selected ones are redrawn
    // on top from ClassDiagram::Draw's second pass (the ViewModel Selected list
    // is the single source of truth -- no per-member IsSelectedIn here).
    MemberShapeIterator iMemberShape(this);
    while (++iMemberShape)
        iMemberShape->Draw(painter, pClassDiagramViewModel, false);

    MethodShapeIterator iMethodShape(this);
    while (++iMethodShape)
        iMethodShape->Draw(painter, pClassDiagramViewModel, false);

    int penStyle = PS_DOT;
    if (GetBaseClass()->IsClass())
        penStyle = PS_SOLID;

    painter.SetTextColor(GetTextColor());
    painter.SetPen(penStyle, 1, GetPenColor());
    painter.DrawRect(_rect);

    if (GetMemberShapeCount())
        painter.DrawLine(_line1Point1, _line1Point2);
    if (GetMethodShapeCount())
        painter.DrawLine(_line2Point1, _line2Point2);

    CbPoint point(GetRect().right, GetRect().bottom);
    CbRect templateRect(point, point);
    if (!GetBaseClass()->GetTemplate().IsEmpty())
        templateRect = GetTemplateRect(painter, true);
    SetTemplateRect(templateRect);

    painter.Restore();

    // Selection resize handles (left/right edge) -- mirrors the SD shapes' Qt
    // Draw; the affordance for the upcoming drag-resize. Screen-only.
    if (painter.IsScreen() && selected)
        DrawSelectedRect(painter, CbPainter::GetSelectColor());
}//@CODE_40387


void ClassShape::DrawSelectedRect(CbPainter& painter, CbColorRef color)
{//@CODE_4923
    painter.DrawSelectionHandle(GetLeftSelectedPoint(),  color);
    painter.DrawSelectionHandle(GetRightSelectedPoint(), color);
}//@CODE_4923


MemberShape* ClassShape::FindMemberShape(Member* pMember)
{//@CODE_35156
    MemberShapeIterator iMemberShape(this);
    while (++iMemberShape)
    {
        if (pMember == iMemberShape->GetMember())
        {
            return iMemberShape;
        }
    }

    return 0;
}//@CODE_35156


MethodShape* ClassShape::FindMethodShape(Method* pMethod)
{//@CODE_35154
    MethodShapeIterator iMethodShape(this);
    while (++iMethodShape)
    {
        if (pMethod == iMethodShape->GetMethod())
        {
            return iMethodShape;
        }
    }

    return 0;
}//@CODE_35154


CbRect ClassShape::GetBoundingRect()
{//@CODE_35259
    CbRect rect = GetRect();
    
    rect *= GetTemplateRect();
    
    return rect;
}//@CODE_35259


ClassShape* ClassShape::GetClassShape() const
{//@CODE_3967
    return const_cast<ClassShape*>(this);
}//@CODE_3967


Gti* ClassShape::GetGti()
{//@CODE_3994
    return GetBaseClass();
}//@CODE_3994


ClassDiagramShape* ClassShape::GetHitShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                           CbPoint pointLP, bool nested)
{//@CODE_40867
    ClassDiagramShape* pClassDiagramShape = 0;

    if (nested)
    {
        MemberShapeIterator iMemberShape(this);
        while (!pClassDiagramShape && ++iMemberShape)
        {
            if (iMemberShape->PointInShape(pClassDiagramViewModel, pointLP))
            {
                pClassDiagramShape = iMemberShape;
            }
        }

        MethodShapeIterator iMethodShape(this);
        while (!pClassDiagramShape && ++iMethodShape)
        {
            if (iMethodShape->PointInShape(pClassDiagramViewModel, pointLP))
            {
                pClassDiagramShape = iMethodShape;
            }
        }
    }

    if (!pClassDiagramShape)
    {
        pClassDiagramShape = this;
    }

    return pClassDiagramShape;
}//@CODE_40867


CbPoint ClassShape::GetLeftSelectedPoint()
{//@CODE_4950
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.left;

    return value;
}//@CODE_4950


CbPoint ClassShape::GetRightSelectedPoint()
{//@CODE_4951
    CbRect rect = GetRect();
    CbPoint value = rect.CenterPoint();
    value.x = rect.right;

    return value;
}//@CODE_4951


CbRect ClassShape::GetTemplateRect(CbPainter& painter, bool draw)
{//@CODE_35271
    painter.Save();

    CbString templateText = GetBaseClass()->GetTemplate();
    templateText.Replace('<', ' ');
    templateText.Replace('>', ' ');

    painter.SetPen(PS_DASH, 1, GetPenColor());
    painter.SetFont(CBF_CLASS_NAME);
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

    painter.Restore();

    return rect;
}//@CODE_35271


int ClassShape::IsAlignShape() const
{//@CODE_35326
    return 1;
}//@CODE_35326


void ClassShape::NotifyAddMember(Member* pMember)
{//@CODE_35787
    // The MFC views are gone; nothing suppresses the model-driven auto-add now
    // (the old check skipped it while an MFC view had focus).
    bool addShapePossible = true;

    if (addShapePossible)
    {
		if ((pMember->IsPublic()    && GetClassDiagram()->GetPublicMembers()       == true) ||
			(pMember->IsProtected() && GetClassDiagram()->GetProtectedMembers()    == true) ||
			(pMember->IsPrivate()   && GetClassDiagram()->GetPrivateMembers()      == true) )
		{
			(void)new MemberShape(this, pMember);
		}
    }
}//@CODE_35787


void ClassShape::NotifyAddMethod(Method* pMethod)
{//@CODE_35789
    bool addShapePossible = true;
    
    if (addShapePossible)
    {
		if ((pMethod->IsPublicMethod()    && GetClassDiagram()->GetPublicMethods()    == true) ||
			(pMethod->IsProtectedMethod() && GetClassDiagram()->GetProtectedMethods() == true) ||
			(pMethod->IsPrivateMethod()   && GetClassDiagram()->GetPrivateMethods()   == true) )
		{
			if ((pMethod->IsGetMemberMethod() && GetClassDiagram()->GetGetSetMethods() == true) ||
				(pMethod->IsSetMemberMethod() && GetClassDiagram()->GetGetSetMethods() == true) ||
				(!pMethod->IsGetMemberMethod() && !pMethod->IsSetMemberMethod()))
			{
				(void)new MethodShape(this, pMethod);
			}
		}
    }
}//@CODE_35789


int ClassShape::OnEditAttributes(bool checkOnly)
{//@CODE_3936
    return GetBaseClass()->OnEditAttributes(checkOnly);
}//@CODE_3936


void ClassShape::OptimizeConnectionBottomPlacement()
{//@CODE_14319
    int x = 0;
    int cnt = 0;
    FromConnectionShapeIterator iFromConnectionShape(this, &ConnectionShape::IsFromAtBottom);
    while (++iFromConnectionShape)
    {
        if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
        {
            cnt++;
            x += iFromConnectionShape->GetStartPoint().x;
        }
    }
    
    if (cnt > 1)
    {
        CbPoint point(x/cnt, GetRect().bottom);
		point = Shape::CrossPoint(GetRect(), point);

        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
            {
				iFromConnectionShape->SetInitial(false);
                iFromConnectionShape->UpdateStartPoint(point);
            }
        }
    }
}//@CODE_14319


void ClassShape::OptimizeConnectionLeftPlacement()
{//@CODE_14320
    int y = 0;
    int cnt = 0;
    FromConnectionShapeIterator iFromConnectionShape(this, &ConnectionShape::IsFromAtLeft);
    while (++iFromConnectionShape)
    {
        if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
        {
            cnt++;
            y += iFromConnectionShape->GetStartPoint().y;
        }
    }
    
    if (cnt > 1)
    {
        CbPoint point(GetRect().left, y/cnt);
		point = Shape::CrossPoint(GetRect(), point);

        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
            {
				iFromConnectionShape->SetInitial(false);
                iFromConnectionShape->UpdateStartPoint(point);
            }
        }
    }
}//@CODE_14320


void ClassShape::OptimizeConnectionPlacement()
{//@CODE_14318
    OptimizeConnectionTopPlacement();
    OptimizeConnectionBottomPlacement();
    OptimizeConnectionLeftPlacement();
    OptimizeConnectionRightPlacement();
}//@CODE_14318


void ClassShape::OptimizeConnectionRightPlacement()
{//@CODE_14321
    int y = 0;
    int cnt = 0;
    FromConnectionShapeIterator iFromConnectionShape(this, &ConnectionShape::IsFromAtRight);
    while (++iFromConnectionShape)
    {
        if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
        {
            cnt++;
            y += iFromConnectionShape->GetStartPoint().y;
        }
    }
    
    if (cnt > 1)
    {
        CbPoint point(GetRect().right, y/cnt);
		point = Shape::CrossPoint(GetRect(), point);

        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
            {
				iFromConnectionShape->SetInitial(false);
                iFromConnectionShape->UpdateStartPoint(point);
            }
        }
    }
}//@CODE_14321


void ClassShape::OptimizeConnectionTopPlacement()
{//@CODE_14305
    int x = 0;
    int cnt = 0;
    FromConnectionShapeIterator iFromConnectionShape(this, &ConnectionShape::IsFromAtTop);
    while (++iFromConnectionShape)
    {
        if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
        {
            cnt++;
            x += iFromConnectionShape->GetStartPoint().x;
        }
    }
    
    if (cnt > 1)
    {
        CbPoint point(x/cnt, GetRect().top);
		point = Shape::CrossPoint(GetRect(), point);

        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->IsInheritShape() && iFromConnectionShape->GetInitial())
            {
				iFromConnectionShape->SetInitial(false);
                iFromConnectionShape->UpdateStartPoint(point);
            }
        }
    }
}//@CODE_14305


void ClassShape::PopulateFromDiagramFlags()
{//@CODE_38301
BaseClass* pBaseClass = GetBaseClass();
    if (!pBaseClass) return;
    ClassDiagram* pCD = GetClassDiagram();
    if (!pCD) return;

    BaseClass::MemberIterator iMember(pBaseClass);
    while (++iMember)
    {
        bool show = false;
        if      (iMember->IsPublic()    && pCD->GetPublicMembers())    show = true;
        else if (iMember->IsProtected() && pCD->GetProtectedMembers()) show = true;
        else if (iMember->IsPrivate()   && pCD->GetPrivateMembers())   show = true;
        if (!show) continue;

        bool already = false;
        MemberShapeIterator iMS(this);
        while (++iMS)
        {
            if (iMS->GetMember() == iMember.Get()) { already = true; break; }
        }
        if (already) continue;

        (void)new MemberShape(this, iMember);
    }

    BaseClass::MethodIterator iMethod(pBaseClass);
    while (++iMethod)
    {
        if (!iMethod->IsNonMacroMethod()) continue;
        if (iMethod->IsConstructor() || iMethod->IsDestructor()) continue;
        if ((iMethod->IsGetMemberMethod() || iMethod->IsSetMemberMethod())
            && !pCD->GetGetSetMethods())
            continue;

        bool show = false;
        if      (iMethod->IsPublicMethod()    && pCD->GetPublicMethods())    show = true;
        else if (iMethod->IsProtectedMethod() && pCD->GetProtectedMethods()) show = true;
        else if (iMethod->IsPrivateMethod()   && pCD->GetPrivateMethods())   show = true;
        if (!show) continue;

        bool already = false;
        MethodShapeIterator iMS(this);
        while (++iMS)
        {
            if (iMS->GetMethod() == iMethod.Get()) { already = true; break; }
        }
        if (already) continue;

        (void)new MethodShape(this, iMethod);
    }
}//@CODE_38301


void ClassShape::RecalculateRect()
{//@CODE_3931
    CbRect rect = GetRect();
    int right = rect.left + RecalculateRectWidth();
    int left = rect.left;
    int bottom = rect.bottom;
    int top = bottom - (32+4);

    // Width changed by autowith feature
    if (right != rect.right)
    {
        CbSize offset(right - rect.right, 0);
        
        CbRect neighbourhood(rect.right, rect.top, rect.right, rect.bottom);
        neighbourhood.NormalizeRect();
        neighbourhood.InflateRect(10, 10, 11, 11);

        GetClassDiagram()->MoveNoteShapePoints(neighbourhood, offset);

        rect.right = right;
    }
    
    _line1Point1.x = left;
    _line1Point1.y = top;
    _line1Point2.x = right;
    _line1Point2.y = top;
    MemberShapeIterator iMemberShape(this);
    while (++iMemberShape)
    {
        bottom = top;
        top -= 34;
        iMemberShape->SetRect(CbRect(left, bottom, right, top));
        
        if (iMemberShape.IsLast())
            top -= 2;
    }
    
    _line2Point1.x = left;
    _line2Point1.y = top;
    _line2Point2.x = right;
    _line2Point2.y = top;
    MethodShapeIterator iMethodShape(this);
    while (++iMethodShape)
    {
        bottom = top;
        top -= 34;
        iMethodShape->SetRect(CbRect(left, bottom, right, top));
        
        if (iMethodShape.IsLast())
            top -= 2;
    }
    
    if (rect.top != top)
    {
        CbSize offset(0, top - rect.top);

        CbRect neighbourhood(rect.left, rect.top, rect.right, rect.top);
        neighbourhood.NormalizeRect();
        neighbourhood.InflateRect(10, 10, 11, 11);

        GetClassDiagram()->MoveNoteShapePoints(neighbourhood, offset);

        rect.top = top;
    }
    rect.NormalizeRect();
    
    SetRect(rect);
}//@CODE_3931


int ClassShape::RecalculateRectWidth()
{//@CODE_7523
    int width = 100; // the minimum width
	if (GetAutoWidth())
		width = 250; // The miniumum if autoWidth is on
        
    // The app-wide CbPainter_QFontMetrics (installed by the Qt app, see
    // CbPainter::GetMeasurePainter) measures headlessly -- so this works at
    // pipe-creation time before any view, like the old CClientDC(NULL) desktop
    // DC did. Grid::Place needs accurate text widths to place shapes correctly.
    CbPainter* pMeasure = CbPainter::GetMeasurePainter();
    if (GetAutoWidth() && pMeasure)
    {
        int saved = pMeasure->Save();
        pMeasure->SetFont(CBF_CLASS_NAME);

        CbSize textSize = pMeasure->GetTextExtent(GetBaseClass()->Type::GetName());
        int tmpWidth = ((textSize.cx+textSize.cx/20+10)/10)*10;
        if (width < tmpWidth)
            width = tmpWidth;

        pMeasure->SetFont(CBF_MEMBER);
        MemberShapeIterator iMemberShape(this);
        while (++iMemberShape)
        {
            textSize = pMeasure->GetTextExtent(iMemberShape->GetMember()->GetShapeText());
            tmpWidth = ((textSize.cx+textSize.cx/20+18)/10)*10;
            if (width < tmpWidth)
                width = tmpWidth;
        }

        pMeasure->SetFont(CBF_METHOD);
        MethodShapeIterator iMethodShape(this);
        while (++iMethodShape)
        {
            textSize = pMeasure->GetTextExtent(iMethodShape->GetMethod()->GetShapeText(VerbosityType(GetVerbosity())));
            tmpWidth = ((textSize.cx+textSize.cx/20+18)/10)*10;
            if (width < tmpWidth)
                width = tmpWidth;
        }

        pMeasure->Restore(saved);
    }
    else
    {
        int tmpWidth = GetRect().Width();
        if (width < tmpWidth)
            width = tmpWidth;
    }
    
    return width;
}//@CODE_7523


/*@NOTE_4543
Set the value of member '_rect' to 'rRect'.
*/
void ClassShape::SetRect(const CbRect& rRect)
{//@CODE_4543
    CbRect newRect = rRect;
    CbRect oldRect = _rect;
    
    if (oldRect != newRect)
    {
        SaveState();
        _rect = newRect;
        
        newRect.NormalizeRect();
        oldRect.NormalizeRect();
        CbSize move = newRect.CenterPoint() - oldRect.CenterPoint();
        int oldHeight = oldRect.Height();
        int newHeight = newRect.Height();
        int oldWidth = oldRect.Width();
        int newWidth = newRect.Width();
        
        FromConnectionShapeIterator iFromConnectionShape(this);
        while (++iFromConnectionShape)
        {
            if (iFromConnectionShape->GetInitial())
            {
                ClassShape* pToClassShape = iFromConnectionShape->GetToClassShape();
                iFromConnectionShape->SetStartPoint(ConnectionPoint(pToClassShape));
                iFromConnectionShape->SetEndPoint(pToClassShape->ConnectionPoint(this));
                iFromConnectionShape->MakeNewRouting();
            }
            else
            {
                CbPoint point = iFromConnectionShape->GetStartPoint();
            
                if (oldHeight == newHeight && oldWidth == newWidth)
                {
                    // Move only
                    point += move;
                    if (iFromConnectionShape->GetToClassShape() != this)
                    {
                        iFromConnectionShape->UpdateStartPoint(point);
                    }
                    else
                    {
                        iFromConnectionShape->SetStartPoint(point);
                    }
                }
                else
                {
                    // Resize
                    point -= oldRect.TopLeft();
                    point.x = int(double(point.x)/double(oldWidth)*double(newWidth));
                    point.y = int(double(point.y)/double(oldHeight)*double(newHeight));
                    point += newRect.TopLeft();
                    point = Shape::CrossPoint(newRect, point);
                    iFromConnectionShape->UpdateStartPoint(point);
                }
            }
        }
        
        ToConnectionShapeIterator iToConnectionShape(this);
        while (++iToConnectionShape)
        {
            if (iToConnectionShape->GetInitial())
            {
                ClassShape* pFromClassShape = iToConnectionShape->GetFromClassShape();
                iToConnectionShape->SetStartPoint(pFromClassShape->ConnectionPoint(this));
                iToConnectionShape->SetEndPoint(ConnectionPoint(pFromClassShape));
                iToConnectionShape->MakeNewRouting();
            }
            else
            {
                CbPoint point = iToConnectionShape->GetEndPoint();
            
                if (oldHeight == newHeight && oldWidth == newWidth)
                {
                    // Move only
                    point += move;
                    if (iToConnectionShape->GetFromClassShape() != this)
                    {
                        iToConnectionShape->UpdateEndPoint(point);
                    }
                    else
                    {
                        iToConnectionShape->SetEndPoint(point);
                    }
                }
                else
                {
                    // Resize
                    point -= oldRect.TopLeft();
                    point.x = int(double(point.x)/double(oldWidth)*double(newWidth));
                    point.y = int(double(point.y)/double(oldHeight)*double(newHeight));
                    point += newRect.TopLeft();
                    point = Shape::CrossPoint(newRect, point);
                    iToConnectionShape->UpdateEndPoint(point);
                }
            }
        }
    }
}//@CODE_4543


/*@NOTE_35269
Set the value of member '_templateRect' to 'rTemplateRect'.
*/
void ClassShape::SetTemplateRect(const CbRect& rTemplateRect)
{//@CODE_35269
    if (_templateRect != rTemplateRect)
    {
        // _templateRect is DERIVED -- re-measured/re-positioned every paint in
        // ClassShape::Draw (the ONLY caller) from the box corner + template text.
        // Recording undo for a derived paint recompute used to push an undo entry
        // during paint and corrupt the redo stack (CD got "stuck", redo disabled).
        // The SD twin ClassLifeLineShape::SetTemplateRect was stripped for exactly
        // this; do the same here. (Serialized but redundant.)
        _templateRect = rTemplateRect;
    }
}//@CODE_35269


/*@NOTE_4614
Returns the value of member '_tracking'.
*/
bool ClassShape::GetTracking()
{//@CODE_4614
    return _tracking;
}//@CODE_4614


/*@NOTE_4616
Returns the value of member '_verbosity'.
*/
int ClassShape::GetVerbosity()
{//@CODE_4616
    return _verbosity;
}//@CODE_4616


/*@NOTE_4617
Set the value of member '_verbosity' to 'verbosity'.
*/
void ClassShape::SetVerbosity(int verbosity)
{//@CODE_4617
    _verbosity = verbosity;
}//@CODE_4617


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5690
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void ClassShape::CleanupReferences()
{
    ClassDiagramShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
}


/*@NOTE_3369
Method which must be called first in a constructor.
*/
void ClassShape::ConstructorInclude(BaseClass* pBaseClass)
{
    INIT_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    INIT_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    INIT_MULTI_OWNED_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    INIT_MULTI_OWNED_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    INIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
}


/*@NOTE_3371
Method which must be called first in a destructor.
*/
void ClassShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    EXIT_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    EXIT_MULTI_OWNED_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    EXIT_MULTI_OWNED_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    EXIT_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
}


/*@NOTE_5691
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void ClassShape::RemoveReferences()
{
    REMOVE_MULTI_OWNED_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    REMOVE_MULTI_OWNED_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    REMOVE_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    REMOVE_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    ClassDiagramShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
}


/*@NOTE_5692
Bring the current object relations into the same state as pDataModelDocObject.
*/
void ClassShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassShape* pClassShape = (ClassShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
    ClassDiagramShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5694
Save the state of the current object relations to pDataModelDocObject.
*/
void ClassShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ClassDiagramShape::SaveReferences(pDataModelDocObject);
    ClassShape* pClassShape = (ClassShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
}


/*@NOTE_3858
Serialize the members only to a CbObject object.
*/
void ClassShape::Serialize(CbArchive& archive)
{
    ClassDiagramShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _line1Point1;
        archive << _line1Point2;
        archive << _line2Point1;
        archive << _line2Point2;
        archive << _verbosity;
        archive << _autoWidth;
        archive << _templateRect;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _line1Point1;
            archive >> _line1Point2;
            archive >> _line2Point1;
            archive >> _line2Point2;
            archive >> _verbosity;
            archive >> _autoWidth;
            archive >> _templateRect;
        }
    }
}


/*@NOTE_3857
Method which must be called first in a serialize constructor.
*/
void ClassShape::SerializeConstructorInclude()
{
    INIT_MULTI_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
    INIT_MULTI_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
    INIT_MULTI_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
    INIT_MULTI_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    INIT_MULTI_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)
}


/*@NOTE_3860
Serialize the relations to a CbObject object.
*/
void ClassShape::SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[])
{
    ClassDiagramShape::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
        WRITE_MULTI_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
        WRITE_MULTI_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
        WRITE_MULTI_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
        WRITE_MULTI_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
    }
    else
    {
        if (1 <= _objectVersion)
        {
            READ_MULTI_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
            READ_MULTI_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
            READ_MULTI_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
            READ_MULTI_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
        }
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(ClassShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
METHODS_ITERATOR_MULTI_ACTIVE(ClassShape, ClassShape, MemberShape, MemberShape)
METHODS_MULTI_OWNED_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
METHODS_ITERATOR_MULTI_ACTIVE(ClassShape, ClassShape, MethodShape, MethodShape)
METHODS_MULTI_OWNED_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
METHODS_ITERATOR_MULTI_ACTIVE(ClassShape, FromClassShape, ConnectionShape, FromConnectionShape)
METHODS_MULTI_OWNED_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
METHODS_ITERATOR_MULTI_ACTIVE(ClassShape, ToClassShape, ConnectionShape, ToConnectionShape)
METHODS_MULTI_OWNED_PASSIVE(BaseClass, BaseClass, ClassShape, ClassShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
