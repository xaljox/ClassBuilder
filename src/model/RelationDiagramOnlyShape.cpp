/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          RelationDiagramOnlyShape.cpp
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RelationDiagramOnlyShape'
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
#include "CbPainter.h"
#include "ClassBuilderDoc.h"
#include "qt/QtRelationDiagramOnlyDialog.h"
//@END_USER2


// Static members


/*@NOTE_23094
Constructor needed for serialization, not meant to use for other purposes!
*/
RelationDiagramOnlyShape::RelationDiagramOnlyShape() //@INIT_23094
    : ConnectionShape()
{//@CODE_23094
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23094


/*@NOTE_23206
Constructor method.
*/
RelationDiagramOnlyShape::RelationDiagramOnlyShape(ClassDiagram* pClassDiagram,
                                                   ClassShape* pFromClassShape,
                                                   ClassShape* pToClassShape) //@INIT_23206
    : ConnectionShape(pClassDiagram, pFromClassShape, pToClassShape,
        pClassDiagram->GetDataModelDoc()->GetRelationDiagramOnlyPenColor(),
        pClassDiagram->GetDataModelDoc()->GetRelationDiagramOnlyTextColor())
    , _toUmlPoint()
    , _toNamePoint()
    , _fromUmlPoint()
    , _fromNamePoint()
    , _umlFrom("1")
    , _umlTo("*")
    , _fromName(pFromClassShape->GetBaseClass()->GetName())
    , _toName(pToClassShape->GetBaseClass()->GetName())
    , _multi(1)
    , _owned(1)
    , _static(0)
{//@CODE_23206
    ConstructorInclude();

    // Put in your own code
    MakeNewRouting();
    RecalculateRect();
}//@CODE_23206


/*@NOTE_23210
Constructor method.
*/
RelationDiagramOnlyShape::RelationDiagramOnlyShape(RelationDiagramOnlyShape* pRelationDiagramOnlyShape) //@INIT_23210
    : ConnectionShape(pRelationDiagramOnlyShape)
    , _toUmlPoint(pRelationDiagramOnlyShape->_toUmlPoint)
    , _toNamePoint(pRelationDiagramOnlyShape->_toNamePoint)
    , _fromUmlPoint(pRelationDiagramOnlyShape->_fromUmlPoint)
    , _fromNamePoint(pRelationDiagramOnlyShape->_fromNamePoint)
    , _umlFrom(pRelationDiagramOnlyShape->_umlFrom)
    , _umlTo(pRelationDiagramOnlyShape->_umlTo)
    , _fromName(pRelationDiagramOnlyShape->_fromName)
    , _toName(pRelationDiagramOnlyShape->_toName)
    , _multi(pRelationDiagramOnlyShape->_multi)
    , _owned(pRelationDiagramOnlyShape->_owned)
    , _static(pRelationDiagramOnlyShape->_static)
{//@CODE_23210
    ConstructorInclude();

    // Put in your own code
    ConvertRouting();
}//@CODE_23210


/*@NOTE_35310
Constructor method.
*/
RelationDiagramOnlyShape::RelationDiagramOnlyShape(ClassDiagram* pClassDiagram,
                                                   RelationDiagramOnlyShape* pRelationDiagramOnlyShape) //@INIT_35310
    : ConnectionShape(pClassDiagram, pRelationDiagramOnlyShape)
    , _toUmlPoint(pRelationDiagramOnlyShape->_toUmlPoint)
    , _toNamePoint(pRelationDiagramOnlyShape->_toNamePoint)
    , _fromUmlPoint(pRelationDiagramOnlyShape->_fromUmlPoint)
    , _fromNamePoint(pRelationDiagramOnlyShape->_fromNamePoint)
    , _umlFrom(pRelationDiagramOnlyShape->_umlFrom)
    , _umlTo(pRelationDiagramOnlyShape->_umlTo)
    , _fromName(pRelationDiagramOnlyShape->_fromName)
    , _toName(pRelationDiagramOnlyShape->_toName)
    , _multi(pRelationDiagramOnlyShape->_multi)
    , _owned(pRelationDiagramOnlyShape->_owned)
    , _static(pRelationDiagramOnlyShape->_static)
{//@CODE_35310
    ConstructorInclude();

    // Put in your own code
    ConvertRouting();
}//@CODE_35310


/*@NOTE_23092
Destructor method.
*/
RelationDiagramOnlyShape::~RelationDiagramOnlyShape()
{//@CODE_23092
    DestructorInclude();

    // Put in your own code
}//@CODE_23092


/*@NOTE_23120
Convert segments, to draw correct start and end styles.
*/
void RelationDiagramOnlyShape::ConvertRouting()
{//@CODE_23120
    // Replace first segment with appropiate segment
    if (GetOwned())
    {
        ConnectionSegment* pFirst = GetFirstConnectionSegment();
        if (pFirst && !pFirst->IsReplaced())
            (void)new RelationAggregationStartSegment(GetFirstConnectionSegment());
    }
    else
    {
        ConnectionSegment* pFirst = GetFirstConnectionSegment();
        if (pFirst && !pFirst->IsReplaced())
            (void)new RelationAssociationStartSegment(GetFirstConnectionSegment());
    }
    
    // Replace last segment with appropiate segment
    if (GetMulti())
    {
        ConnectionSegment* pLast = GetLastConnectionSegment();
        if (pLast && !pLast->IsReplaced())
            (void)new RelationMultiEndSegment(GetLastConnectionSegment());
    }
    else
    {
        ConnectionSegment* pLast = GetLastConnectionSegment();
        if (pLast && !pLast->IsReplaced())
            (void)new RelationSingleEndSegment(GetLastConnectionSegment());
    }
}//@CODE_23120


void RelationDiagramOnlyShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35136
    RelationDiagramOnlyShape* pRelationDiagramOnlyShape = 
        new RelationDiagramOnlyShape(pClassDiagram, this);
    pRelationDiagramOnlyShape->CopyState(this);
    _ptrIndex = intptr_t(pRelationDiagramOnlyShape);
}//@CODE_35136


void RelationDiagramOnlyShape::Draw(CbPainter& painter,
                                    ClassDiagramViewModel* pClassDiagramViewModel,
                                    bool selected)
{//@CODE_40383
    if (!GetHidden())
    {
        painter.Save();

        int width = 1;
        if (GetStatic())
            width = 4;
        if (selected && width < 3)
            width = 3;
        CbColorRef penColor = selected ? CbPainter::GetSelectColor() : GetPenColor();
        painter.SetPen(PS_SOLID, width, penColor);
        painter.SetSolidBrush(penColor);

        ConnectionShape::Draw(painter, pClassDiagramViewModel, selected);

        painter.SetFont(CBF_RELATION);
        if (selected)
            painter.SetBold(true);
        painter.SetTextAlign(TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);
        painter.SetTextColor(selected ? CbPainter::GetSelectColor() : GetTextColor());

        CbRect rect = GetRect();

        if (!GetFromName().IsEmpty())
        {
            CbRect fromNameRect(_fromNamePoint, painter.GetTextExtent(GetFromName()));
            rect *= fromNameRect;
            painter.TextOut(_fromNamePoint.x, _fromNamePoint.y, GetFromName());
        }

        if (!GetToName().IsEmpty())
        {
            CbRect toNameRect(_toNamePoint, painter.GetTextExtent(GetToName()));
            rect *= toNameRect;
            painter.TextOut(_toNamePoint.x, _toNamePoint.y, GetToName());
        }

        if (!GetUmlFrom().IsEmpty())
        {
            CbRect fromUmlRect(_fromUmlPoint, painter.GetTextExtent(GetUmlFrom()));
            rect *= fromUmlRect;
            painter.TextOut(_fromUmlPoint.x, _fromUmlPoint.y, GetUmlFrom());
        }

        if (!GetUmlTo().IsEmpty())
        {
            CbRect toUmlRect(_toUmlPoint, painter.GetTextExtent(GetUmlTo()));
            rect *= toUmlRect;
            painter.TextOut(_toUmlPoint.x, _toUmlPoint.y, GetUmlTo());
        }

        if (rect != GetRect())
            SetRect(rect);

        painter.Restore();
    }
}//@CODE_40383


RelationDiagramOnlyShape* RelationDiagramOnlyShape::GetRelationDiagramOnlyShape()
{//@CODE_23215
    return this;
}//@CODE_23215


/*@NOTE_23124
Make a brand new routing, delete old routing first.
*/
void RelationDiagramOnlyShape::MakeNewRouting()
{//@CODE_23124
    ConnectionShape::MakeNewRouting();
    ConvertRouting();

    if (GetInitial())
    {
        _fromNamePoint = GetStartPoint() + GetMinStartSize() + CbSize(10, -34);
        Shape::Round(_fromNamePoint);
        _toNamePoint = GetEndPoint() + GetMinEndSize() + CbSize(10, -16);
        Shape::Round(_toNamePoint);
        _fromUmlPoint = GetStartPoint() + GetMinStartSize() + CbSize(10, 0);
        Shape::Round(_fromUmlPoint);
        _toUmlPoint = GetEndPoint() + GetMinEndSize() + CbSize(10, -50);
        Shape::Round(_toUmlPoint);
    }
}//@CODE_23124


int RelationDiagramOnlyShape::OnDelete(bool checkOnly)
{//@CODE_23125
    if (!checkOnly)
    {
        Delete();
    }
    
    return 1;
}//@CODE_23125


int RelationDiagramOnlyShape::OnEditAttributes(bool checkOnly)
{//@CODE_23127
	if (checkOnly)
		return 1;

	void* ownerHwnd = Cb_OwnerHwnd();
	bool modelChanged = false;
	if (Qt_ShowRelationDiagramOnlyDialog(this, modelChanged, ownerHwnd))
	{
		if (modelChanged)
		{
            // A diagram-only relation has no model/tree backing, so a CD-canvas
            // refresh suffices -- NotifyStructureChanged would needlessly rebuild
            // the whole tree.
            GetDataModelDoc()->NotifyCdViews();
		}

		return 1;
	}

    return 0;
}//@CODE_23127


bool RelationDiagramOnlyShape::PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                            CbPoint pointLP)
{//@CODE_40853
    return ConnectionShape::PointInShape(pClassDiagramViewModel, pointLP);
}//@CODE_40853


/*@NOTE_23173
Returns the value of member '_owned'.
*/
int RelationDiagramOnlyShape::GetOwned() const
{//@CODE_23173
    return _owned;
}//@CODE_23173


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_23101
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RelationDiagramOnlyShape::CleanupReferences()
{
    ConnectionShape::CleanupReferences();
}


/*@NOTE_23091
Method which must be called first in a constructor.
*/
void RelationDiagramOnlyShape::ConstructorInclude()
{
}


/*@NOTE_23093
Method which must be called first in a destructor.
*/
void RelationDiagramOnlyShape::DestructorInclude()
{
}


/*@NOTE_23102
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RelationDiagramOnlyShape::RemoveReferences()
{
    ConnectionShape::RemoveReferences();
}


/*@NOTE_23103
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RelationDiagramOnlyShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_23105
Save the state of the current object relations to pDataModelDocObject.
*/
void RelationDiagramOnlyShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape::SaveReferences(pDataModelDocObject);
}


/*@NOTE_23096
Serialize the members only to a CbObject object.
*/
void RelationDiagramOnlyShape::Serialize(CbArchive& archive)
{
    ConnectionShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _toUmlPoint;
        archive << _toNamePoint;
        archive << _fromUmlPoint;
        archive << _fromNamePoint;
        archive << _umlFrom;
        archive << _umlTo;
        archive << _fromName;
        archive << _toName;
        archive << _multi;
        archive << _owned;
        archive << _static;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _toUmlPoint;
            archive >> _toNamePoint;
            archive >> _fromUmlPoint;
            archive >> _fromNamePoint;
            archive >> _umlFrom;
            archive >> _umlTo;
            archive >> _fromName;
            archive >> _toName;
            archive >> _multi;
            archive >> _owned;
            archive >> _static;
        }
    }
}


/*@NOTE_23095
Method which must be called first in a serialize constructor.
*/
void RelationDiagramOnlyShape::SerializeConstructorInclude()
{
}


/*@NOTE_23098
Serialize the relations to a CbObject object.
*/
void RelationDiagramOnlyShape::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(RelationDiagramOnlyShape)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
