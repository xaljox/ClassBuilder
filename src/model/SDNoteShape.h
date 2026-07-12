/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SDNoteShape.h
* Creation date: July 12, 2026 21:59
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SDNoteShape'
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
#ifndef _SDNOTESHAPE_H
#define _SDNOTESHAPE_H

//@START_USER1
#include <vector>
#include <utility>
//@END_USER1



class SDNoteShape
    : public SequenceDiagramShape
{
    CB_DECLARE_SERIAL(SDNoteShape)
    RELATION_MULTI_OWNED_ACTIVE(SDNoteShape, SDNoteShape, SDNoteShapePoint, SDNoteShapePoint)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _note;
    int _fontHeight;
    static bool _tracking;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    SDNoteShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SDNoteShape(SequenceDiagram* pSequenceDiagram, const CbPoint& point);
    virtual ~SDNoteShape();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected);
    void DrawSelectedRect(CbPainter& painter, CbColorRef color);
    CbPoint GetLeftSelectedPoint();
    virtual SDNoteShape* GetNoteShape();
    CbPoint GetRightSelectedPoint();
    void MoveNoteShapePoints(const CbRect& rect, const CbSize& offset);
    void MoveNoteShapePoints(const CbRect& oldRect, const CbRect& newRect);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    int OnOpen(bool checkOnly = false);
    virtual bool PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              const CbPoint& pointLP);
    void RecalcHeight(CbPainter& painter);
    void ResolveNoteFollows(const std::vector<std::pair<CbRect,CbRect>>& moved);
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
#ifndef _SDNOTESHAPE_H_INLINES
#define _SDNOTESHAPE_H_INLINES

/*@NOTE_34670
Returns the value of member '_fontHeight'.
*/
inline int SDNoteShape::GetFontHeight() const
{//@CODE_34670
    return _fontHeight;
}//@CODE_34670



/*@NOTE_34671
Set the value of member '_fontHeight' to 'fontHeight'.
*/
inline void SDNoteShape::SetFontHeight(int fontHeight)
{//@CODE_34671
    _fontHeight = fontHeight;
}//@CODE_34671



/*@NOTE_34674
Returns the value of member '_tracking'.
*/
inline bool SDNoteShape::GetTracking()
{//@CODE_34674
    return _tracking;
}//@CODE_34674



//@START_USER3
//@END_USER3

#endif
#endif
