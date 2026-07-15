/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          RelationShape.cpp
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'RelationShape'
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


RelationShape::RelationShape(ClassDiagram* pClassDiagram, Relation* pRelation,
                             ClassShape* pFromClassShape,
                             ClassShape* pToClassShape) //@INIT_3830
    : ConnectionShape(pClassDiagram, pFromClassShape, pToClassShape,
        pClassDiagram->GetDataModelDoc()->GetRelationPenColor(),
        pClassDiagram->GetDataModelDoc()->GetRelationTextColor())
    , _verbosity(1)
    , _criticalPenColor(pClassDiagram->GetDataModelDoc()->GetCriticalRelationPenColor())
{//@CODE_3830
    ConstructorInclude(pRelation);

    // Relation names are the same as class names, so suppress the drawing of
    // the names
    if (pRelation->GetFromName() == pRelation->GetFromClass()->GetBaseName() &&
        pRelation->GetToName() == pRelation->GetToClass()->GetBaseName())
    {
        _verbosity = 0;
    }
    
    // Put in your own code
    MakeNewRouting();
    RecalculateRect();
}//@CODE_3830


/*@NOTE_4580
Copy constructor, needed for temporary copy, while modifying shape.
*/
RelationShape::RelationShape(RelationShape* pRelationShape) //@INIT_4580
    : ConnectionShape(pRelationShape)
    , _fromNamePoint(pRelationShape->_fromNamePoint)
    , _toNamePoint(pRelationShape->_toNamePoint)
    , _fromUmlPoint(pRelationShape->_fromUmlPoint)
    , _toUmlPoint(pRelationShape->_toUmlPoint)
    , _verbosity(pRelationShape->_verbosity)
    , _criticalPenColor(pRelationShape->_criticalPenColor)
{//@CODE_4580
    ConstructorInclude(pRelationShape->GetRelation());

    // Put in your own code
    ConvertRouting();
}//@CODE_4580


/*@NOTE_3842
Constructor needed for serialization, not meant to use for other purposes!
*/
RelationShape::RelationShape() //@INIT_3842
    : ConnectionShape()
    , _fromNamePoint(100, -100)
    , _toNamePoint(100, -200)
    , _fromUmlPoint(200, -100)
    , _toUmlPoint(200, -200)
    , _verbosity(1)
    , _criticalPenColor(Cb_RGB(255, 0, 0))
{//@CODE_3842
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3842


/*@NOTE_35294
Constructor method.
*/
RelationShape::RelationShape(ClassDiagram* pClassDiagram,
                             RelationShape* pRelationShape) //@INIT_35294
    : ConnectionShape(pClassDiagram, pRelationShape)
    , _fromNamePoint(pRelationShape->_fromNamePoint)
    , _toNamePoint(pRelationShape->_toNamePoint)
    , _fromUmlPoint(pRelationShape->_fromUmlPoint)
    , _toUmlPoint(pRelationShape->_toUmlPoint)
    , _verbosity(pRelationShape->_verbosity)
    , _criticalPenColor(pRelationShape->_criticalPenColor)
{//@CODE_35294
    ConstructorInclude(pRelationShape->GetRelation());

    // Put in your own code
    ConvertRouting();
}//@CODE_35294


/*@NOTE_3375
Destructor method.
*/
RelationShape::~RelationShape()
{//@CODE_3375
    DestructorInclude();

    // Put in your own code
}//@CODE_3375


/*@NOTE_4503
Convert segments, to draw correct start and end styles.
*/
void RelationShape::ConvertRouting()
{//@CODE_4503
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
    if (GetRelation()->GetMulti())
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
}//@CODE_4503


void RelationShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35132
    RelationShape* pRelationShape = new RelationShape(pClassDiagram, this);
    pRelationShape->CopyState(this);
    _ptrIndex = intptr_t(pRelationShape);
}//@CODE_35132


void RelationShape::Draw(CbPainter& painter,
                         ClassDiagramViewModel* pClassDiagramViewModel,
                         bool selected)
{//@CODE_40375
    if (!GetHidden())
    {
        painter.Save();

        int width = 1;
        if (GetRelation()->GetStatic())
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

        if (GetVerbosity())
        {
            CbRect fromNameRect(_fromNamePoint, painter.GetTextExtent(GetRelation()->GetFromName()));
            rect *= fromNameRect;
            CbRect toNameRect(_toNamePoint, painter.GetTextExtent(GetRelation()->GetToName()));
            rect *= toNameRect;
            painter.TextOut(_fromNamePoint.x, _fromNamePoint.y, GetRelation()->GetFromName());
            painter.TextOut(_toNamePoint.x, _toNamePoint.y, GetRelation()->GetToName());
        }

        CbRect fromUmlRect(_fromUmlPoint, painter.GetTextExtent(GetUmlFrom()));
        rect *= fromUmlRect;
        CbRect toUmlRect(_toUmlPoint, painter.GetTextExtent(GetUmlTo()));
        rect *= toUmlRect;
        painter.TextOut(_fromUmlPoint.x, _fromUmlPoint.y, GetUmlFrom());
        painter.TextOut(_toUmlPoint.x, _toUmlPoint.y, GetUmlTo());

        if (rect != GetRect())
            SetRect(rect);

        painter.Restore();
    }
}//@CODE_40375


int RelationShape::GetOwned() const
{//@CODE_35784
    return (GetRelation()->GetOwned() ? 1: 0);
}//@CODE_35784


RelationShape* RelationShape::GetRelationShape()
{//@CODE_4619
    return this;
}//@CODE_4619


CbString RelationShape::GetUmlFrom()
{//@CODE_5071
    CbString value;

    if (GetRelation()->GetOwned())
    {
        value = "1";
    }
    else
    {
        value = "0..1";
    }
    
    return value;
}//@CODE_5071


CbString RelationShape::GetUmlTo()
{//@CODE_5072
    CbString value;
    
    if (GetRelation()->GetMulti())
    {
        value = "*";
    }
    else
    {
        value = "0..1";
    }
    
    return value;
}//@CODE_5072


/*@NOTE_4505
Make a brand new routing, delete old routing first.
*/
void RelationShape::MakeNewRouting()
{//@CODE_4505
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
}//@CODE_4505


int RelationShape::OnDelete(bool checkOnly)
{//@CODE_4547
    return GetRelation()->GetFromRelation()->OnDelete(checkOnly);
}//@CODE_4547


int RelationShape::OnEditAttributes(bool checkOnly)
{//@CODE_3937
    return GetRelation()->GetFromRelation()->OnEditAttributes(checkOnly);
}//@CODE_3937


bool RelationShape::PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                 CbPoint pointLP)
{//@CODE_40850
    return ConnectionShape::PointInShape(pClassDiagramViewModel, pointLP);
}//@CODE_40850


/*@NOTE_35060
Set the value of member '_textColor' to 'penColor'.
*/
void RelationShape::SetPenColor(CbColorRef penColor)
{//@CODE_35060
    if (!GetRelation()->GetCritical())
    {
        Shape::SetPenColor(penColor);
    }
    else
    {
        _criticalPenColor = penColor;
    }
    
}//@CODE_35060


/*@NOTE_4693
Returns the value of member '_fromNamePoint'.
*/
const CbPoint& RelationShape::GetFromNamePoint()
{//@CODE_4693
    return _fromNamePoint;
}//@CODE_4693


/*@NOTE_4694
Set the value of member '_fromNamePoint' to 'rFromNamePoint'.
*/
void RelationShape::SetFromNamePoint(const CbPoint& rFromNamePoint)
{//@CODE_4694
    _fromNamePoint = rFromNamePoint;
}//@CODE_4694


/*@NOTE_5082
Returns the value of member '_fromUmlPoint'.
*/
const CbPoint& RelationShape::GetFromUmlPoint()
{//@CODE_5082
    return _fromUmlPoint;
}//@CODE_5082


/*@NOTE_5083
Set the value of member '_fromUmlPoint' to 'rFromUmlPoint'.
*/
void RelationShape::SetFromUmlPoint(const CbPoint& rFromUmlPoint)
{//@CODE_5083
    _fromUmlPoint = rFromUmlPoint;
}//@CODE_5083


/*@NOTE_4696
Returns the value of member '_toNamePoint'.
*/
const CbPoint& RelationShape::GetToNamePoint()
{//@CODE_4696
    return _toNamePoint;
}//@CODE_4696


/*@NOTE_4697
Set the value of member '_toNamePoint' to 'rToNamePoint'.
*/
void RelationShape::SetToNamePoint(const CbPoint& rToNamePoint)
{//@CODE_4697
    _toNamePoint = rToNamePoint;
}//@CODE_4697


/*@NOTE_5086
Returns the value of member '_toUmlPoint'.
*/
const CbPoint& RelationShape::GetToUmlPoint()
{//@CODE_5086
    return _toUmlPoint;
}//@CODE_5086


/*@NOTE_5087
Set the value of member '_toUmlPoint' to 'rToUmlPoint'.
*/
void RelationShape::SetToUmlPoint(const CbPoint& rToUmlPoint)
{//@CODE_5087
    _toUmlPoint = rToUmlPoint;
}//@CODE_5087


/*@NOTE_4744
Returns the value of member '_verbosity'.
*/
int RelationShape::GetVerbosity()
{//@CODE_4744
    return _verbosity;
}//@CODE_4744


/*@NOTE_4745
Set the value of member '_verbosity' to 'verbosity'.
*/
void RelationShape::SetVerbosity(int verbosity)
{//@CODE_4745
    _verbosity = verbosity;
}//@CODE_4745


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5696
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void RelationShape::CleanupReferences()
{
    ConnectionShape::CleanupReferences();
    CLEANUP_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)
}


/*@NOTE_3374
Method which must be called first in a constructor.
*/
void RelationShape::ConstructorInclude(Relation* pRelation)
{
    INIT_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)
}


/*@NOTE_3376
Method which must be called first in a destructor.
*/
void RelationShape::DestructorInclude()
{
    EXIT_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)
}


/*@NOTE_5697
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void RelationShape::RemoveReferences()
{
    ConnectionShape::RemoveReferences();
    REMOVE_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)
}


/*@NOTE_5698
Bring the current object relations into the same state as pDataModelDocObject.
*/
void RelationShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    RelationShape* pRelationShape = (RelationShape*)pDataModelDocObject;
    RESTORE_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)
    ConnectionShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5700
Save the state of the current object relations to pDataModelDocObject.
*/
void RelationShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape::SaveReferences(pDataModelDocObject);
    RelationShape* pRelationShape = (RelationShape*)pDataModelDocObject;
    SAVE_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)
}


/*@NOTE_3844
Serialize the members only to a CbObject object.
*/
void RelationShape::Serialize(CbArchive& archive)
{
    ConnectionShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _fromNamePoint;
        archive << _toNamePoint;
        archive << _verbosity;
        archive << _fromUmlPoint;
        archive << _toUmlPoint;
        archive << _criticalPenColor;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _fromNamePoint;
            archive >> _toNamePoint;
            archive >> _verbosity;
            archive >> _fromUmlPoint;
            archive >> _toUmlPoint;
            archive >> _criticalPenColor;
        }
    }
}


/*@NOTE_3843
Method which must be called first in a serialize constructor.
*/
void RelationShape::SerializeConstructorInclude()
{
    INIT_MULTI_PASSIVE(Relation, Relation, RelationShape, RelationShape)
}


/*@NOTE_3846
Serialize the relations to a CbObject object.
*/
void RelationShape::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(RelationShape)


// Methods for the relation(s) of the class
METHODS_MULTI_OWNED_PASSIVE(Relation, Relation, RelationShape, RelationShape)

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
