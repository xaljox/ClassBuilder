/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          Shape.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'Shape'
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
#ifndef _SHAPE_H
#define _SHAPE_H

//@START_USER1
//@END_USER1



class Shape
    : public DataModelDocObject
{
    CB_DECLARE_SERIAL(Shape)

//@START_USER2
//@END_USER2

// Members
private:

protected:
    CbRect _rect;
    CbColorRef _penColor;
    CbColorRef _textColor;

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void ReplaceConstructorInclude(Shape* pOld);
    void SerializeConstructorInclude();

protected:
    Shape();
    static CbRect NoteCalcRect(CbPainter& painter, const CbRect& rect,
                               CbString noteText, int fontHeight);
    static void NoteDraw(CbPainter& painter, CbRect rect, CbString noteText,
                         int fontHeight, bool selected, CbColorRef penColor,
                         CbColorRef textColor);
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    Shape(DataModelDoc* pDataModelDoc, const CbPoint& point,
          CbColorRef penColor = Cb_RGB(0, 0, 0),
          CbColorRef textColor = Cb_RGB(0, 0, 0));
    Shape(DataModelDoc* pDataModelDoc, CbColorRef penColor = Cb_RGB(0, 0, 0),
          CbColorRef textColor = Cb_RGB(0, 0, 0));
    Shape(Shape* pOld);
    virtual ~Shape();
    static bool CrossPoint(const CbPoint& p1, const CbPoint& p2,
                           const CbPoint& p3, const CbPoint& p4, CbPoint& p);
    static bool CrossPoint(const CbPoint& p1, const CbPoint& p2, CbRect rect,
                           CbPoint& point);
    static CbPoint CrossPoint(CbRect rect, CbPoint p1);
    virtual bool PointInShape(CbPoint pointLP);
    static void Round(CbSize& size);
    static void Round(CbRect& rect);
    static void Round(CbPoint& point);
    static CbPoint TrackCrossPoint(CbRect rect, CbPoint p1);
    virtual int UsesPenColor() const;
    virtual int UsesTextColor() const;
    virtual CbColorRef GetPenColor() const;
    virtual void SetPenColor(CbColorRef penColor);
    CbRect GetRect();
    virtual void SetRect(const CbRect& rRect);
    CbColorRef GetTextColor() const;
    void SetTextColor(CbColorRef textColor);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SHAPE_H_INLINES
#define _SHAPE_H_INLINES

/*@NOTE_19880
Returns the value of member '_textColor'.
*/
inline CbColorRef Shape::GetPenColor() const
{//@CODE_19880
    return _penColor;
}//@CODE_19880



/*@NOTE_19884
Returns the value of member '_textColor'.
*/
inline CbColorRef Shape::GetTextColor() const
{//@CODE_19884
    return _textColor;
}//@CODE_19884



//@START_USER3
//@END_USER3

#endif
#endif
