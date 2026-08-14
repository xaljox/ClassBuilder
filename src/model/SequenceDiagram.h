/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SequenceDiagram.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SequenceDiagram'
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
#ifndef _SEQUENCEDIAGRAM_H
#define _SEQUENCEDIAGRAM_H

//@START_USER1
#include <vector>
#include <utility>
//@END_USER1



class SequenceDiagram
    : public Gti
{
    CB_DECLARE_SERIAL(SequenceDiagram)
    RELATION_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramShape, SequenceDiagramShape)
    RELATION_SINGLE_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, RootActivationShape, RootActivationShape)
    RELATION_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)
    RELATION_NOFILTER_MULTI_OWNED_ACTIVE(SequenceDiagram, SequenceDiagram, SequenceDiagramViewModel, SequenceDiagramViewModel)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, SequenceDiagram, SequenceDiagram)

//@START_USER2
friend DataModelDoc;
public:
//@END_USER2

// Members
private:
    unsigned short _height;
    int _multiPage;
    CbString _name;
    CbString _note;
    unsigned short _width;
    unsigned short _scale;
    static unsigned short _activationWidth;
    static unsigned short _activationOffsetRecursive;
    static unsigned short _activationMinimalHeight;
    static unsigned short _activationSpaceRecursive;
    static unsigned short _activationSpaceBefore;
    static unsigned short _activationSpaceAfter;
    static unsigned short _classLifeLineHeight;
    static unsigned short _classLifeLineOffset;
    static unsigned short _signalLengthRecursive;
    SeqType _numbering;
    bool _arguments;
    bool _scope;
    bool _argumentNames;
    CbString _caption;
    bool _moveOnce;

protected:

public:

// Methods
private:
    void ResetDriftedSignalTextOffsets();
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    SequenceDiagram();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SequenceDiagram(Gti* pGti);
    SequenceDiagram(Gti* pGti, SequenceDiagram* pSequenceDiagram);
    virtual ~SequenceDiagram();
    virtual void Add();
    void CheckAndUpdatePhase();
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    void Draw(CbPainter& painter,
              SequenceDiagramViewModel* pSequenceDiagramViewModel);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    CbRect GetBoundingRect();
    SequenceDiagramShape* GetHitShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                                      CbPoint pointLP, bool nested);
    unsigned short GetLifeLineHeight();
    int GetNumberOfPages();
    void MoveNoteShapePoints(const CbRect& rect, const CbSize& offset);
    void MoveNoteShapePoints(const CbRect& oldRect, const CbRect& newRect);
    virtual int OnAddClassDiagram(bool checkOnly = false);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    void OptimizePlacement();
    void RecalculateAfterEdit(SequenceDiagramShape* pSDS = nullptr,
                              const CbSize& offset = CbSize(0, 0),
                              const CbPoint& noteEndPoint = CbPoint(INT_MIN,
                              INT_MIN), const CbRect& width = CbRect(0, 0, 0,
                              0));
    void RecalculateDiagram(std::vector<std::pair<CbRect,CbRect>>* moved = nullptr);
    void ResetActivationOffsets();
    void ResolveNoteFollows(const std::vector<std::pair<CbRect,CbRect>>& moved);
    virtual void SetPhaseDownAndUpwards(Phase phase);
    void SpaceLifeLines();
    virtual void Update();
    void UpdateSequenceDiagramViews();
    static unsigned short GetActivationMinimalHeight();
    static unsigned short GetActivationOffsetRecursive();
    static unsigned short GetActivationSpaceAfter();
    static unsigned short GetActivationSpaceBefore();
    static unsigned short GetActivationSpaceRecursive();
    static unsigned short GetActivationWidth();
    bool GetArgumentNames() const;
    void SetArgumentNames(bool argumentNames);
    bool GetArguments() const;
    void SetArguments(bool arguments);
    const CbString& GetCaption();
    void SetCaption(const CbString& rCaption);
    static unsigned short GetClassLifeLineHeight();
    static unsigned short GetClassLifeLineOffset();
    unsigned short GetHeight() const;
    void SetHeight(unsigned short height);
    bool GetMoveOnce() const;
    void SetMoveOnce(bool moveOnce);
    int GetMultiPage() const;
    void SetMultiPage(int multiPage);
    const CbString& GetName() const;
    void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    SeqType GetNumbering() const;
    void SetNumbering(SeqType numbering);
    unsigned short GetScale() const;
    void SetScale(unsigned short scale);
    bool GetScope() const;
    void SetScope(bool scope);
    static unsigned short GetSignalLengthRecursive();
    unsigned short GetWidth() const;
    void SetWidth(unsigned short width);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SEQUENCEDIAGRAM_H_INLINES
#define _SEQUENCEDIAGRAM_H_INLINES

inline int SequenceDiagram::GetNumberOfPages()
{//@CODE_30076
    return 1<<_multiPage;
}//@CODE_30076



/*@NOTE_33561
Returns the value of member '_activationMinimalHeight'.
*/
inline unsigned short SequenceDiagram::GetActivationMinimalHeight()
{//@CODE_33561
    return _activationMinimalHeight;
}//@CODE_33561



/*@NOTE_33559
Returns the value of member '_activationOffsetRecursive'.
*/
inline unsigned short SequenceDiagram::GetActivationOffsetRecursive()
{//@CODE_33559
    return _activationOffsetRecursive;
}//@CODE_33559



/*@NOTE_33567
Returns the value of member '_activationSpaceAfter'.
*/
inline unsigned short SequenceDiagram::GetActivationSpaceAfter()
{//@CODE_33567
    return _activationSpaceAfter;
}//@CODE_33567



/*@NOTE_33565
Returns the value of member '_activationSpaceAfter'.
*/
inline unsigned short SequenceDiagram::GetActivationSpaceBefore()
{//@CODE_33565
    return _activationSpaceBefore;
}//@CODE_33565



/*@NOTE_33563
Returns the value of member '_signalLengthRecursive'.
*/
inline unsigned short SequenceDiagram::GetActivationSpaceRecursive()
{//@CODE_33563
    return _activationSpaceRecursive;
}//@CODE_33563



/*@NOTE_33557
Returns the value of member '_activationOffsetRecursive'.
*/
inline unsigned short SequenceDiagram::GetActivationWidth()
{//@CODE_33557
    return _activationWidth;
}//@CODE_33557



/*@NOTE_35071
Returns the value of member '_argumentNames'.
*/
inline bool SequenceDiagram::GetArgumentNames() const
{//@CODE_35071
    return _argumentNames;
}//@CODE_35071



/*@NOTE_34217
Returns the value of member '_arguments'.
*/
inline bool SequenceDiagram::GetArguments() const
{//@CODE_34217
    return _arguments;
}//@CODE_34217



/*@NOTE_33575
Returns the value of member '_classLifeLineHeight'.
*/
inline unsigned short SequenceDiagram::GetClassLifeLineHeight()
{//@CODE_33575
    return _classLifeLineHeight;
}//@CODE_33575



/*@NOTE_33577
Returns the value of member '_classLifeLineOffset'.
*/
inline unsigned short SequenceDiagram::GetClassLifeLineOffset()
{//@CODE_33577
    return _classLifeLineOffset;
}//@CODE_33577



/*@NOTE_29664
Returns the value of member '_height'.
*/
inline unsigned short SequenceDiagram::GetHeight() const
{//@CODE_29664
    return _height;
}//@CODE_29664



/*@NOTE_29665
Set the value of member '_height' to 'height'.
*/
inline void SequenceDiagram::SetHeight(unsigned short height)
{//@CODE_29665
    _height = height;
}//@CODE_29665



/*@NOTE_35412
Returns the value of member '_moveOnce'.
*/
inline bool SequenceDiagram::GetMoveOnce() const
{//@CODE_35412
    return _moveOnce;
}//@CODE_35412



/*@NOTE_35413
Set the value of member '_moveOnce' to 'moveOnce'.
*/
inline void SequenceDiagram::SetMoveOnce(bool moveOnce)
{//@CODE_35413
    _moveOnce = moveOnce;
}//@CODE_35413



/*@NOTE_29668
Returns the value of member '_multiPage'.
*/
inline int SequenceDiagram::GetMultiPage() const
{//@CODE_29668
    return _multiPage;
}//@CODE_29668



/*@NOTE_29669
Set the value of member '_multiPage' to 'multiPage'.
*/
inline void SequenceDiagram::SetMultiPage(int multiPage)
{//@CODE_29669
    _multiPage = multiPage;
}//@CODE_29669



/*@NOTE_29672
Returns the value of member '_name'.
*/
inline const CbString& SequenceDiagram::GetName() const
{//@CODE_29672
    return _name;
}//@CODE_29672



/*@NOTE_29673
Set the value of member '_name' to 'rName'.
*/
inline void SequenceDiagram::SetName(const CbString& rName)
{//@CODE_29673
    _name = rName;
}//@CODE_29673



/*@NOTE_34203
Returns the value of member '_numbering'.
*/
inline SeqType SequenceDiagram::GetNumbering() const
{//@CODE_34203
    return _numbering;
}//@CODE_34203



/*@NOTE_34204
Set the value of member '_numbering' to 'numbering'.
*/
inline void SequenceDiagram::SetNumbering(SeqType numbering)
{//@CODE_34204
    _numbering = numbering;
}//@CODE_34204



/*@NOTE_32518
Returns the value of member '_scale'.
*/
inline unsigned short SequenceDiagram::GetScale() const
{//@CODE_32518
    return _scale;
}//@CODE_32518



/*@NOTE_32519
Set the value of member '_scale' to 'scale'.
*/
inline void SequenceDiagram::SetScale(unsigned short scale)
{//@CODE_32519
    _scale = scale;
}//@CODE_32519



/*@NOTE_34411
Returns the value of member '_scope'.
*/
inline bool SequenceDiagram::GetScope() const
{//@CODE_34411
    return _scope;
}//@CODE_34411



/*@NOTE_33580
Returns the value of member '_signalLengthRecursive'.
*/
inline unsigned short SequenceDiagram::GetSignalLengthRecursive()
{//@CODE_33580
    return _signalLengthRecursive;
}//@CODE_33580



/*@NOTE_29680
Returns the value of member '_width'.
*/
inline unsigned short SequenceDiagram::GetWidth() const
{//@CODE_29680
    return _width;
}//@CODE_29680



/*@NOTE_29681
Set the value of member '_width' to 'width'.
*/
inline void SequenceDiagram::SetWidth(unsigned short width)
{//@CODE_29681
    _width = width;
}//@CODE_29681



//@START_USER3
//@END_USER3

#endif
#endif
