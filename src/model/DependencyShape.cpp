/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DependencyShape.cpp
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'DependencyShape'
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
#include "ClassBuilderDoc.h"
#include "qt/QtDependencyDialog.h"
//@END_USER2


// Static members


/*@NOTE_23285
Constructor needed for serialization, not meant to use for other purposes!
*/
DependencyShape::DependencyShape() //@INIT_23285
    : ConnectionShape()
{//@CODE_23285
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_23285


/*@NOTE_23375
Constructor method.
*/
DependencyShape::DependencyShape(ClassDiagram* pClassDiagram,
                                 ClassShape* pFromClassShape,
                                 ClassShape* pToClassShape) //@INIT_23375
    : ConnectionShape(pClassDiagram, pFromClassShape, pToClassShape,
        pClassDiagram->GetDataModelDoc()->GetDependencyPenColor(),
        pClassDiagram->GetDataModelDoc()->GetDependencyTextColor())
    , _stereotype()
    , _stereotypePoint()
    , _name()
    , _namePoint()
{//@CODE_23375
    ConstructorInclude();

    // Put in your own code
    MakeNewRouting();
    RecalculateRect();
}//@CODE_23375


/*@NOTE_23379
Constructor method.
*/
DependencyShape::DependencyShape(DependencyShape* pDependencyShape) //@INIT_23379
    : ConnectionShape(pDependencyShape)
    , _stereotype(pDependencyShape->_stereotype)
    , _stereotypePoint(pDependencyShape->_stereotypePoint)
    , _name(pDependencyShape->_name)
    , _namePoint(pDependencyShape->_namePoint)
{//@CODE_23379
    ConstructorInclude();

    // Put in your own code
    ConvertRouting();
}//@CODE_23379


/*@NOTE_35317
Constructor method.
*/
DependencyShape::DependencyShape(ClassDiagram* pClassDiagram,
                                 DependencyShape* pDependencyShape) //@INIT_35317
    : ConnectionShape(pClassDiagram, pDependencyShape)
    , _stereotype(pDependencyShape->_stereotype)
    , _stereotypePoint(pDependencyShape->_stereotypePoint)
    , _name(pDependencyShape->_name)
    , _namePoint(pDependencyShape->_namePoint)
{//@CODE_35317
    ConstructorInclude();

    // Put in your own code
    ConvertRouting();
}//@CODE_35317


/*@NOTE_23283
Destructor method.
*/
DependencyShape::~DependencyShape()
{//@CODE_23283
    DestructorInclude();

    // Put in your own code
}//@CODE_23283


/*@NOTE_23426
Convert segments, to draw correct start and end styles.
*/
void DependencyShape::ConvertRouting()
{//@CODE_23426
    // Replace first and last segment with appropiate segments
    ConnectionSegment* pFirst = GetFirstConnectionSegment();
    if (pFirst && !pFirst->IsReplaced())
        (void)new DependencyStartSegment(GetFirstConnectionSegment());
    
    ConnectionSegment* pLast = GetLastConnectionSegment();
    if (pLast && !pLast->IsReplaced())
        (void)new DependencyEndSegment(GetLastConnectionSegment());
}//@CODE_23426


void DependencyShape::CopyShape(ClassDiagram* pClassDiagram)
{//@CODE_35138
    DependencyShape* pDependencyShape = new DependencyShape(pClassDiagram, this);
    pDependencyShape->CopyState(this);
    _ptrIndex = intptr_t(pDependencyShape);
}//@CODE_35138


void DependencyShape::Draw(CbPainter& painter,
                           ClassDiagramViewModel* pClassDiagramViewModel,
                           bool selected)
{//@CODE_40371
    if (!GetHidden())
    {
        painter.Save();

        CbColorRef penColor = selected ? CbPainter::GetSelectColor() : GetPenColor();
        painter.SetPen(PS_DASH, selected ? 3 : 1, penColor);
        painter.SetSolidBrush(penColor);

        ConnectionShape::Draw(painter, pClassDiagramViewModel, selected);

        painter.SetFont(CBF_RELATION);
        if (selected)
            painter.SetBold(true);
        painter.SetTextAlign(TA_LEFT|TA_BOTTOM|TA_NOUPDATECP);
        painter.SetTextColor(selected ? CbPainter::GetSelectColor() : GetTextColor());

        CbRect rect = GetRect();

        if (!GetStereotypeString().IsEmpty())
        {
            CbRect stereotypeRect(_stereotypePoint, painter.GetTextExtent(GetStereotypeString()));
            rect *= stereotypeRect;
            painter.TextOut(_stereotypePoint.x, _stereotypePoint.y, GetStereotypeString());
        }

        if (!_name.IsEmpty())
        {
            CbRect nameRect(_namePoint, painter.GetTextExtent(_name));
            rect *= nameRect;
            painter.TextOut(_namePoint.x, _namePoint.y, _name);
        }

        if (rect != GetRect())
            SetRect(rect);

        painter.Restore();
    }
}//@CODE_40371


DependencyShape* DependencyShape::GetDependencyShape()
{//@CODE_23411
    return this;
}//@CODE_23411


/*@NOTE_23427
Returns the stereotype member, surrounded with '<<' and '>>'.
*/
CbString DependencyShape::GetStereotypeString()
{//@CODE_23427
    if (_stereotype.IsEmpty())
    {
        return _stereotype;
    }
    else
    {
        return "<<" + _stereotype + ">>";
    }
}//@CODE_23427


/*@NOTE_23412
Make a brand new routing, delete old routing first.
*/
void DependencyShape::MakeNewRouting()
{//@CODE_23412
    ConnectionShape::MakeNewRouting();
    ConvertRouting();

    if (GetInitial())
    {
        _stereotypePoint = GetStartPoint() + GetMinStartSize() + CbSize(10, -34);
        Shape::Round(_stereotypePoint);
        _namePoint = GetEndPoint() + GetMinEndSize() + CbSize(10, -16);
        Shape::Round(_namePoint);
    }
}//@CODE_23412


int DependencyShape::OnDelete(bool checkOnly)
{//@CODE_23436
    if (!checkOnly)
    {
        Delete();
    }
    
    return 1;
}//@CODE_23436


int DependencyShape::OnEditAttributes(bool checkOnly)
{//@CODE_23413
    if (checkOnly)
        return 1;
    
    void* ownerHwnd = Cb_OwnerHwnd();
    bool modelChanged = false;
    if (Qt_ShowDependencyDialog(this, modelChanged, ownerHwnd))
    {
        if (modelChanged)
        {
            // CD-only edit (same as RelationDiagramOnlyShape): the dependency's
            // attributes are diagram-side, so a CD-canvas refresh suffices --
            // NotifyStructureChanged would needlessly rebuild the whole tree.
            GetDataModelDoc()->NotifyCdViews();
        }

        return 1;
    }

    return 0;
}//@CODE_23413


bool DependencyShape::PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                   CbPoint pointLP)
{//@CODE_40847
    return ConnectionShape::PointInShape(pClassDiagramViewModel, pointLP);
}//@CODE_40847


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_23292
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void DependencyShape::CleanupReferences()
{
    ConnectionShape::CleanupReferences();
}


/*@NOTE_23282
Method which must be called first in a constructor.
*/
void DependencyShape::ConstructorInclude()
{
}


/*@NOTE_23284
Method which must be called first in a destructor.
*/
void DependencyShape::DestructorInclude()
{
}


/*@NOTE_23293
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void DependencyShape::RemoveReferences()
{
    ConnectionShape::RemoveReferences();
}


/*@NOTE_23294
Bring the current object relations into the same state as pDataModelDocObject.
*/
void DependencyShape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_23296
Save the state of the current object relations to pDataModelDocObject.
*/
void DependencyShape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    ConnectionShape::SaveReferences(pDataModelDocObject);
}


/*@NOTE_23287
Serialize the members only to a CbObject object.
*/
void DependencyShape::Serialize(CbArchive& archive)
{
    ConnectionShape::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _stereotype;
        archive << _stereotypePoint;
        archive << _name;
        archive << _namePoint;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _stereotype;
            archive >> _stereotypePoint;
            archive >> _name;
            archive >> _namePoint;
        }
    }
}


/*@NOTE_23286
Method which must be called first in a serialize constructor.
*/
void DependencyShape::SerializeConstructorInclude()
{
}


/*@NOTE_23289
Serialize the relations to a CbObject object.
*/
void DependencyShape::SerializeRelations(CbArchive& archive,
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
CB_IMPLEMENT_SERIAL(DependencyShape)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
