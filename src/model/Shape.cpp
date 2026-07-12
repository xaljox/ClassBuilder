/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Shape.cpp
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Method implementations of class 'Shape'
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
#include <math.h>
#include "CbPainter.h"
//@END_USER2


// Static members


Shape::Shape(DataModelDoc* pDataModelDoc, const CbPoint& point,
             CbColorRef penColor, CbColorRef textColor) //@INIT_3815
    : DataModelDocObject(pDataModelDoc)
    , _rect(point, point)
    , _penColor(penColor)
    , _textColor(textColor)
{//@CODE_3815
    ConstructorInclude();

    // Put in your own code
}//@CODE_3815


Shape::Shape(DataModelDoc* pDataModelDoc, CbColorRef penColor,
             CbColorRef textColor) //@INIT_3910
    : DataModelDocObject(pDataModelDoc)
    , _rect(0, 0, 0, 0)
    , _penColor(penColor)
    , _textColor(textColor)
{//@CODE_3910
    ConstructorInclude();

    // Put in your own code
}//@CODE_3910


/*@NOTE_4487
Constructor needed for putting a new object in the old one's context.
*/
Shape::Shape(Shape* pOld) //@INIT_4487
    : DataModelDocObject(pOld)
{//@CODE_4487
    ReplaceConstructorInclude(pOld);

    _rect = pOld->_rect;
    _penColor = pOld->_penColor;
    _textColor = pOld->_textColor;

    // Put in your own code
}//@CODE_4487


/*@NOTE_3397
Constructor needed for serialization, not meant to use for other purposes!
*/
Shape::Shape() //@INIT_3397
    : DataModelDocObject()
    , _rect(0, 0, 0, 0)
    , _penColor(Cb_RGB(0,0,0))
    , _textColor(Cb_RGB(0,0,0))
{//@CODE_3397
    SerializeConstructorInclude();

    // Put in your own code
}//@CODE_3397


/*@NOTE_3395
Destructor method.
*/
Shape::~Shape()
{//@CODE_3395
    DestructorInclude();

    // Put in your own code
}//@CODE_3395


/*@NOTE_4389
Determine cross point of two lines defined by p1, p2 and p3, p4. if cross point is within
linesegment p1, p2 and line segment p3, p4, then true is returned, otherwise false
is returned. The calculated crosspoint is put in the reference variable p.
*/
bool Shape::CrossPoint(const CbPoint& p1, const CbPoint& p2, const CbPoint& p3,
                       const CbPoint& p4, CbPoint& p)
{//@CODE_4389
    bool result = false;
    long divisor = (p2.y-p1.y)*(p4.x-p3.x) - (p4.y-p3.y)*(p2.x-p1.x);

    if (divisor)
    {
        double x = double(p3.y*p4.x-p4.y*p3.x)*double(p2.x-p1.x) - 
                   double(p1.y*p2.x-p2.y*p1.x)*double(p4.x-p3.x);
        double y = double(p3.y*p4.x-p4.y*p3.x)*double(p2.y-p1.y) - 
                   double(p1.y*p2.x-p2.y*p1.x)*double(p4.y-p3.y);

        p.x = long(x/double(divisor));
        p.y = long(y/double(divisor));

        CbRect rect12(p1, p2);
        rect12.NormalizeRect();
        rect12.InflateRect(0, 0, 1, 1);
        CbRect rect34(p3, p4);
        rect34.NormalizeRect();
        rect34.InflateRect(0, 0, 1, 1);

        if (rect12.PtInRect(p) && rect34.PtInRect(p))
        {
            result = true;
        }
    }

    return result;
}//@CODE_4389


/*@NOTE_4497
Determine cross point of two lines defined by p1, p2 and p3, p4. if cross point is within
linesegment p1, p2 and line segment p3, p4, then true is returned, otherwise false
is returned. The calculated crosspoint is put in the reference variable p.
*/
bool Shape::CrossPoint(const CbPoint& p1, const CbPoint& p2, CbRect rect,
                       CbPoint& point)
{//@CODE_4497
    CbPoint p3;
    CbPoint p4;

    rect.NormalizeRect();
    
    // Check against top of rectangle
    p3.y = p4.y = rect.top;
    p3.x = rect.left;
    p4.x = rect.right;
    if (CrossPoint(p1, p2, p3, p4, point))
        return true;

    // Check against bottom of rectangle
    p3.y = p4.y = rect.bottom;
    p3.x = rect.left;
    p4.x = rect.right;
    if (CrossPoint(p1, p2, p3, p4, point))
        return true;

    // Check against left of rectangle
    p3.x = p4.x = rect.left;
    p3.y = rect.top;
    p4.y = rect.bottom;
    if (CrossPoint(p1, p2, p3, p4, point))
        return true;

    // Check against right of rectangle
    p3.x = p4.x = rect.right;
    p3.y = rect.top;
    p4.y = rect.bottom;
    if (CrossPoint(p1, p2, p3, p4, point))
        return true;

    return false;
}//@CODE_4497


/*@NOTE_4584
Determine nearest crossing on rectangle.
*/
CbPoint Shape::CrossPoint(CbRect rect, CbPoint p1)
{//@CODE_4584
    rect.NormalizeRect();
    CbPoint p2 = rect.CenterPoint();
    CbPoint p3;
    CbPoint p4;
    CbPoint newResult;
    CbPoint result = rect.TopLeft();
    double distance = 1.7E308;
    double newDistance;


    if (p1 != p2)
    {
        // Check against top of rectangle
        p3.y = p4.y = rect.top;
        p3.x = rect.left;
        p4.x = rect.right;
        CrossPoint(p1, p2, p3, p4, newResult);
	    {
		    CbRect rect34(p3, p4);
		    rect34.NormalizeRect();
		    rect34.InflateRect(0, 0, 1, 1);
		    if (rect34.PtInRect(newResult))
		    {
			    CbSize size = newResult - p1;
			    newDistance = double(size.cx)*double(size.cx) + 
						      double(size.cy)*double(size.cy);
			    if (newDistance < distance)
			    {
				    distance = newDistance;
                    newResult.x = ((newResult.x + 5)/10) * 10;
				    result = newResult;
			    }
		    }
	    }

        // Check against bottom of rectangle
        p3.y = p4.y = rect.bottom;
        p3.x = rect.left;
        p4.x = rect.right;
        CrossPoint(p1, p2, p3, p4, newResult);
	    {
		    CbRect rect34(p3, p4);
		    rect34.NormalizeRect();
		    rect34.InflateRect(0, 0, 1, 1);
		    if (rect34.PtInRect(newResult))
		    {
			    CbSize size = newResult - p1;
			    newDistance = double(size.cx)*double(size.cx) + 
						      double(size.cy)*double(size.cy);
			    if (newDistance < distance)
			    {
				    distance = newDistance;
                    newResult.x = ((newResult.x + 5)/10) * 10;
				    result = newResult;
			    }
		    }
	    }

        // Check against left of rectangle
        p3.x = p4.x = rect.left;
        p3.y = rect.top;
        p4.y = rect.bottom;
        CrossPoint(p1, p2, p3, p4, newResult);
	    {
		    CbRect rect34(p3, p4);
		    rect34.NormalizeRect();
		    rect34.InflateRect(0, 0, 1, 1);
		    if (rect34.PtInRect(newResult))
		    {
			    CbSize size = newResult - p1;
			    newDistance = double(size.cx)*double(size.cx) + 
						      double(size.cy)*double(size.cy);
			    if (newDistance < distance)
			    {
				    distance = newDistance;
                    newResult.y = ((newResult.y - 5)/10) * 10;
		            if (!rect34.PtInRect(newResult))
                    {
                        // We are outside rect, round to other side.
                        newResult.y += 10;
                    }
				    result = newResult;
			    }
		    }
	    }

        // Check against right of rectangle
        p3.x = p4.x = rect.right;
        p3.y = rect.top;
        p4.y = rect.bottom;
        CrossPoint(p1, p2, p3, p4, newResult);
	    {
		    CbRect rect34(p3, p4);
		    rect34.NormalizeRect();
		    rect34.InflateRect(0, 0, 1, 1);
		    if (rect34.PtInRect(newResult))
		    {
			    CbSize size = newResult - p1;
			    newDistance = double(size.cx)*double(size.cx) + 
						      double(size.cy)*double(size.cy);
			    if (newDistance < distance)
			    {
				    distance = newDistance;
                    newResult.y = ((newResult.y - 5)/10) * 10;
		            if (!rect34.PtInRect(newResult))
                    {
                        // We are outside rect, round to other side.
                        newResult.y += 10;
                    }
				    result = newResult;
			    }
		    }
	    }
    }

    return result;
}//@CODE_4584


CbRect Shape::NoteCalcRect(CbPainter& painter, const CbRect& rect,
                           CbString noteText, int fontHeight)
{//@CODE_41354
    const int size = fontHeight + 2;
    CbRect result = rect;
    result.top = result.bottom;
    int y = result.bottom - 2;
    int width = result.Width() - size * 2;
    const int yStart = y;

    int save = painter.Save();
    painter.SetFontPx(fontHeight);

    CbString remaining = noteText;
    bool moreSegments = true;
    while (moreSegments)
    {
        int nlIndex = remaining.Find(NL);
        CbString note;
        if (nlIndex == -1)
        {
            note = remaining;
            moreSegments = false;
        }
        else
        {
            note = remaining.Left(nlIndex);
            remaining = remaining.Mid(nlIndex + 2);
        }

        int index = note.Find("  ");
        while (index != -1)
        {
            note = note.Left(index) + note.Mid(index + 1);
            index = note.Find("  ");
        }

        note.TrimLeft();
        note.TrimRight();
        const bool segmentWasEmpty = note.IsEmpty();
        if (painter.GetTextExtent(note).cx > width)
        {
            CbString line;
            CbString lineOk;

            index = note.FindOneOf(" \t");
            while (index != -1)
            {
                line += note.Left(index + 1);

                if (painter.GetTextExtent(line).cx <= width || lineOk.IsEmpty())
                {
                    lineOk = line;
                    note = note.Mid(index + 1);
                }
                else
                {
                    note.TrimLeft();
                    y -= size;
                    line.Empty();
                    lineOk.Empty();
                    width = result.Width() - fontHeight / 2;
                }

                index = note.FindOneOf(" \t");
            }

            if (!lineOk.IsEmpty())
            {
                CbString rest = lineOk + note;
                if (painter.GetTextExtent(rest).cx <= width)
                {
                    y -= size;
                    note.Empty();
                }
                else
                {
                    note.TrimLeft();
                    y -= size;
                    width = result.Width() - fontHeight / 2;
                }
            }
        }

        if (!note.IsEmpty())
        {
            y -= size;
        }
        else if (segmentWasEmpty && y != yStart)
        {
            y -= size;
        }
    }

    painter.Restore(save);

    result.top = y;
    if (result.Height() < size * 2)
    {
        result.top -= size * 2 - result.Height();
    }
    return result;
}//@CODE_41354


void Shape::NoteDraw(CbPainter& painter, CbRect rect, CbString noteText,
                     int fontHeight, bool selected, CbColorRef penColor,
                     CbColorRef textColor)
{//@CODE_41359
    const int size = fontHeight + 2;

    const CbRect fillRect = rect;
    rect.top = rect.bottom;
    int y = rect.bottom - 2;
    int x = (rect.left + rect.right) / 2;

    painter.SetFontPx(fontHeight);
    painter.SetTextAlign(TA_CENTER | TA_TOP | TA_NOUPDATECP);
    painter.SetTextColor(textColor);

    if (painter.IsScreen() && selected)
    {
        painter.FillSolidRect(fillRect, CbPainter::GetSelectFillColor());
        painter.SetBkColor(CbPainter::GetSelectFillColor());
    }

    CbString remaining = noteText;
    int width = rect.Width() - size * 2;
    const int yStart = y;
    bool moreSegments = true;
    while (moreSegments)
    {
        int nlIndex = remaining.Find(NL);
        CbString note;
        if (nlIndex == -1)
        {
            note = remaining;
            moreSegments = false;
        }
        else
        {
            note = remaining.Left(nlIndex);
            remaining = remaining.Mid(nlIndex + 2);
        }

        int index = note.Find("  ");
        while (index != -1)
        {
            note = note.Left(index) + note.Mid(index + 1);
            index = note.Find("  ");
        }

        note.TrimLeft();
        note.TrimRight();
        const bool segmentWasEmpty = note.IsEmpty();
        if (painter.GetTextExtent(note).cx > width)
        {
            CbString line;
            CbString lineOk;

            index = note.FindOneOf(" \t");
            while (index != -1)
            {
                line += note.Left(index + 1);

                if (painter.GetTextExtent(line).cx <= width || lineOk.IsEmpty())
                {
                    lineOk = line;
                    note = note.Mid(index + 1);
                }
                else
                {
                    CbRect clipRect(rect.left, y - size, rect.right, y);
                    note.TrimLeft();
                    lineOk.TrimRight();
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, lineOk);
                    y -= size;
                    line.Empty();
                    lineOk.Empty();
                    width = rect.Width() - fontHeight / 2;
                }

                index = note.FindOneOf(" \t");
            }

            if (!lineOk.IsEmpty())
            {
                CbString rest = lineOk + note;
                if (painter.GetTextExtent(rest).cx <= width)
                {
                    CbRect clipRect(rect.left, y - size, rect.right, y);
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, rest);
                    y -= size;
                    note.Empty();
                }
                else
                {
                    CbRect clipRect(rect.left, y - size, rect.right, y);
                    note.TrimLeft();
                    lineOk.TrimRight();
                    painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, lineOk);
                    y -= size;
                    width = rect.Width() - fontHeight / 2;
                }
            }
        }

        if (!note.IsEmpty())
        {
            CbRect clipRect(rect.left, y - size, rect.right, y);
            painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, note);
            y -= size;
        }
        else if (segmentWasEmpty && y != yStart)
        {
            CbRect clipRect(rect.left, y - size, rect.right, y);
            painter.ExtTextOut(x, y, ETO_CLIPPED | ETO_OPAQUE, clipRect, CbString(""));
            y -= size;
        }
    }
    rect.top = y;
    if (rect.Height() < size * 2)
    {
        rect.top -= size * 2 - rect.Height();
    }

    painter.SetPen(PS_SOLID, 1, penColor);
    painter.DrawLine(CbPoint(rect.left, rect.top),                   CbPoint(rect.right, rect.top));
    painter.DrawLine(CbPoint(rect.right, rect.top),                  CbPoint(rect.right, rect.bottom - size));
    painter.DrawLine(CbPoint(rect.right, rect.bottom - size),        CbPoint(rect.right - size, rect.bottom));
    painter.DrawLine(CbPoint(rect.right - size, rect.bottom),        CbPoint(rect.right - size, rect.bottom - size));
    painter.DrawLine(CbPoint(rect.right - size, rect.bottom - size), CbPoint(rect.right, rect.bottom - size));
    painter.DrawLine(CbPoint(rect.right - size, rect.bottom),        CbPoint(rect.left, rect.bottom));
    painter.DrawLine(CbPoint(rect.left, rect.bottom),                CbPoint(rect.left, rect.top));
}//@CODE_41359


bool Shape::PointInShape(CbPoint pointLP)
{//@CODE_27535
    return GetRect().PtInRect(pointLP);
}//@CODE_27535


/*@NOTE_30432
Round CbSize. CbPoint, or CbRect to valid grid points.
*/
void Shape::Round(CbSize& size)
{//@CODE_30432
    size.cx = ((size.cx + 5)/10)*10;
    size.cy = ((size.cy - 5)/10)*10;
}//@CODE_30432


/*@NOTE_30434
Round CbSize. CbPoint, or CbRect to valid grid points.
*/
void Shape::Round(CbRect& rect)
{//@CODE_30434
    rect.left = ((rect.left + 5)/10)*10;
    //rect.top = ((rect.top - 5)/10)*10;
    rect.right = ((rect.right + 5)/10)*10;
    rect.bottom = ((rect.bottom - 5)/10)*10;
}//@CODE_30434


/*@NOTE_30436
Round CbSize. CbPoint, or CbRect to valid grid points.
*/
void Shape::Round(CbPoint& point)
{//@CODE_30436
    point.x = ((point.x + 5)/10)*10;
    point.y = ((point.y - 5)/10)*10;
}//@CODE_30436


/*@NOTE_7526
Determine nearest crossing on rectangle.
*/
CbPoint Shape::TrackCrossPoint(CbRect rect, CbPoint p1)
{//@CODE_7526
    if (p1.x < rect.left)
        p1.x = rect.left;
    else if (p1.x > rect.right)
        p1.x = rect.right;

    if (p1.y < rect.top)
        p1.y = rect.top;
    else if (p1.y > rect.bottom)
        p1.y = rect.bottom;

    return CrossPoint(rect, p1);
}//@CODE_7526


int Shape::UsesPenColor() const
{//@CODE_19890
    return 0;
}//@CODE_19890


int Shape::UsesTextColor() const
{//@CODE_19891
    return 0;
}//@CODE_19891


/*@NOTE_19881
Set the value of member '_textColor' to 'penColor'.
*/
void Shape::SetPenColor(CbColorRef penColor)
{//@CODE_19881
    _penColor = penColor;
}//@CODE_19881


/*@NOTE_3751
Returns the value of member '_rect'.
*/
CbRect Shape::GetRect()
{//@CODE_3751
    CbRect rect(_rect);
    rect.NormalizeRect();
    
    return rect;
}//@CODE_3751


/*@NOTE_3752
Set the value of member '_rect' to 'rRect'.
*/
void Shape::SetRect(const CbRect& rRect)
{//@CODE_3752
    _rect = rRect;
}//@CODE_3752


/*@NOTE_19885
Set the value of member '_textColor' to 'textColor'.
*/
void Shape::SetTextColor(CbColorRef textColor)
{//@CODE_19885
    _textColor = textColor;
}//@CODE_19885


//{{AFX DO NOT EDIT CODE BELOW THIS LINE !!!

/*@NOTE_5720
Pre condition: The current object isn't part of the active data structure, but
is on either the undo or the redo stack.

The current object isn't needed any longer and is scheduled to be deleted, a
direct normal delete will fail, since the current object can contain
references to the active part of the data structure. It is the task of this
routine to cleanup those references, so the object can be safely removed.
*/
void Shape::CleanupReferences()
{
    DataModelDocObject::CleanupReferences();
}


/*@NOTE_3394
Method which must be called first in a constructor.
*/
void Shape::ConstructorInclude()
{
}


/*@NOTE_3396
Method which must be called first in a destructor.
*/
void Shape::DestructorInclude()
{
}


/*@NOTE_5721
Remove all references to the current object, but keep the references from this
object, so the state can be restored.
*/
void Shape::RemoveReferences()
{
    DataModelDocObject::RemoveReferences();
}


/*@NOTE_4489
Method which must be called first in a replace constructor.
*/
void Shape::ReplaceConstructorInclude(Shape* pOld)
{
}


/*@NOTE_5722
Bring the current object relations into the same state as pDataModelDocObject.
*/
void Shape::RestoreReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::RestoreReferences(pDataModelDocObject);
}


/*@NOTE_5724
Save the state of the current object relations to pDataModelDocObject.
*/
void Shape::SaveReferences(DataModelDocObject* pDataModelDocObject)
{
    DataModelDocObject::SaveReferences(pDataModelDocObject);
}


/*@NOTE_3399
Serialize the members only to a CbObject object.
*/
void Shape::Serialize(CbArchive& archive)
{
    DataModelDocObject::Serialize(archive);
    if (archive.IsStoring())
    {
        archive << _rect;
        archive << _penColor;
        archive << _textColor;
    }
    else
    {
        if (1 <= _objectVersion)
        {
            archive >> _rect;
            archive >> _penColor;
            archive >> _textColor;
        }
    }
}


/*@NOTE_3398
Method which must be called first in a serialize constructor.
*/
void Shape::SerializeConstructorInclude()
{
}


/*@NOTE_3401
Serialize the relations to a CbObject object.
*/
void Shape::SerializeRelations(CbArchive& archive,
                               DataModelDocObject* pointerArray[])
{
    DataModelDocObject::SerializeRelations(archive, pointerArray);
    if (archive.IsStoring())
    {
    }
    else
    {
    }
}


// ClassBuilder macro to support serialization for this class
CB_IMPLEMENT_SERIAL(Shape)


// Methods for the relation(s) of the class

//}}AFX DO NOT EDIT CODE ABOVE THIS LINE !!!

//@START_USER3
