/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          NoteShape.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'NoteShape'
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
#ifndef _NOTESHAPE_H
#define _NOTESHAPE_H

//@START_USER1
//@END_USER1



class NoteShape
    : public ClassDiagramShape
{
    CB_DECLARE_SERIAL(NoteShape)
    RELATION_MULTI_OWNED_ACTIVE(NoteShape, NoteShape, NoteShapePoint, NoteShapePoint)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _note;
    static bool _tracking;
    int _fontHeight;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    NoteShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    NoteShape(ClassDiagram* pClassDiagram, const CbPoint& point);
    virtual ~NoteShape();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    void DrawSelectedRect(CbPainter& painter, CbColorRef color);
    CbPoint GetLeftSelectedPoint();
    virtual NoteShape* GetNoteShape();
    CbPoint GetRightSelectedPoint();
    virtual int IsAlignShape() const;
    void MoveNoteShapePoints(const CbRect& rect, const CbSize& offset);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual bool PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                              CbPoint pointLP);
    void RecalcHeight(CbPainter& painter);
    virtual void SetRect(const CbRect& rRect);
    int GetFontHeight() const;
    void SetFontHeight(int fontHeight);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    static bool GetTracking();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _NOTESHAPE_H_INLINES
#define _NOTESHAPE_H_INLINES

/*@NOTE_7472
Returns the value of member '_fontHeight'.
*/
inline int NoteShape::GetFontHeight() const
{//@CODE_7472
    return _fontHeight;
}//@CODE_7472



/*@NOTE_7473
Set the value of member '_fontHeight' to 'fontHeight'.
*/
inline void NoteShape::SetFontHeight(int fontHeight)
{//@CODE_7473
    _fontHeight = fontHeight;
}//@CODE_7473



//@START_USER3
//@END_USER3

#endif
#endif
