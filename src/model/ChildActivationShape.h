/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ChildActivationShape.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ChildActivationShape'
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
#ifndef _CHILDACTIVATIONSHAPE_H
#define _CHILDACTIVATIONSHAPE_H

//@START_USER1
//@END_USER1



class ChildActivationShape
    : public ParentActivationShape
{
    CB_DECLARE_SERIAL(ChildActivationShape)
    RELATION_SINGLE_OWNED_ACTIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    RELATION_MULTI_OWNED_ACTIVE(ChildActivationShape, Sender, SignalShape, Receiver)
    RELATION_MULTI_OWNED_PASSIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    RELATION_MULTI_OWNED_PASSIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)
    RELATION_MULTI_PASSIVE(Method, Method, ChildActivationShape, ChildActivationShape)

//@START_USER2
//@END_USER2

// Members
private:
    bool _creation;
    bool _destruction;
    int _sequenceNumber;
    int _sequenceSubNumber;
    int _offset;
    CbColorRef _activationNoMethodPenColor;
    CbColorRef _activationUser3PenColor;
    bool _merge;
    CbColorRef _activationInitialPenColor;

protected:

public:

// Methods
private:
    CbColorRef GetPenColor(CbPainter& painter);
    Class* HasUser3Method(int& start, int& end, Class* pClass = 0) const;
    bool HasUser3Method() const;
    void ConstructorInclude(LifeLineShape* pLifeLineShape,
                            ParentActivationShape* pParentActivationShape);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ChildActivationShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ChildActivationShape(LifeLineShape* pLifeLineShape);
    ChildActivationShape(LifeLineShape* pLifeLineShape,
                         ChildActivationShape* pChildActivationShape);
    virtual ~ChildActivationShape();
    bool CanMerge();
    static int Compare(ChildActivationShape* pChildActivationShape1,
                       ChildActivationShape* pChildActivationShape2);
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected);
    virtual ChildActivationShape* GetChildActivation();
    virtual int GetMaxUpOffset();
    CbRect GetMergeRect();
    virtual CbString GetNumbering();
    virtual CbColorRef GetPenColor() const;
    virtual bool IsRecursiveActivation();
    virtual bool NeedExtraSpaceBefore();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual bool PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              const CbPoint& pointLP);
    virtual void SetPenColor(CbColorRef penColor);
    virtual void SetRect(const CbRect& rRect);
    virtual int UsesTextColor() const;
    bool WrongCreationOrDestruction();
    CbColorRef GetActivationInitialPenColor() const;
    void SetActivationInitialPenColor(CbColorRef activationInitialPenColor);
    CbColorRef GetActivationNoMethodPenColor() const;
    void SetActivationNoMethodPenColor(CbColorRef activationNoMethodPenColor);
    CbColorRef GetActivationUser3PenColor() const;
    void SetActivationUser3PenColor(CbColorRef activationUser3PenColor);
    bool GetCreation() const;
    void SetCreation(bool creation);
    bool GetDestruction() const;
    void SetDestruction(bool destruction);
    bool GetMerge() const;
    void SetMerge(bool merge);
    virtual int GetOffset() const;
    void SetOffset(int offset);
    int GetSequenceNumber() const;
    void SetSequenceNumber(int sequenceNumber);
    int GetSequenceSubNumber() const;
    void SetSequenceSubNumber(int sequenceSubNumber);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _CHILDACTIVATIONSHAPE_H_INLINES
#define _CHILDACTIVATIONSHAPE_H_INLINES

/*@NOTE_35979
Returns the value of member '_activationInitialPenColor'.
*/
inline CbColorRef ChildActivationShape::GetActivationInitialPenColor() const
{//@CODE_35979
    return _activationInitialPenColor;
}//@CODE_35979



/*@NOTE_35980
Set the value of member '_activationInitialPenColor' to 'activationInitialPenColor'.
*/
inline void ChildActivationShape::SetActivationInitialPenColor(CbColorRef activationInitialPenColor)
{//@CODE_35980
    _activationInitialPenColor = activationInitialPenColor;
}//@CODE_35980



/*@NOTE_34368
Returns the value of member '_activationNoMethodPenColor'.
*/
inline CbColorRef ChildActivationShape::GetActivationNoMethodPenColor() const
{//@CODE_34368
    return _activationNoMethodPenColor;
}//@CODE_34368



/*@NOTE_34369
Set the value of member 'activationNoMethodPenColor' to 'activationNoMethodPenColor'.
*/
inline void ChildActivationShape::SetActivationNoMethodPenColor(CbColorRef activationNoMethodPenColor)
{//@CODE_34369
    _activationNoMethodPenColor = activationNoMethodPenColor;
}//@CODE_34369



/*@NOTE_35358
Returns the value of member '_activationUser3PenColor'.
*/
inline CbColorRef ChildActivationShape::GetActivationUser3PenColor() const
{//@CODE_35358
    return _activationUser3PenColor;
}//@CODE_35358



/*@NOTE_35359
Set the value of member '_activationUser3PenColor' to 'activationUser3PenColor'.
*/
inline void ChildActivationShape::SetActivationUser3PenColor(CbColorRef activationUser3PenColor)
{//@CODE_35359
    _activationUser3PenColor = activationUser3PenColor;
}//@CODE_35359



/*@NOTE_34186
Returns the value of member '_creation'.
*/
inline bool ChildActivationShape::GetCreation() const
{//@CODE_34186
    return _creation;
}//@CODE_34186



/*@NOTE_34187
Set the value of member '_creation' to 'creation'.
*/
inline void ChildActivationShape::SetCreation(bool creation)
{//@CODE_34187
    _creation = creation;
}//@CODE_34187



/*@NOTE_34190
Returns the value of member '_destruction'.
*/
inline bool ChildActivationShape::GetDestruction() const
{//@CODE_34190
    return _destruction;
}//@CODE_34190



/*@NOTE_34191
Set the value of member '_destruction' to 'destruction'.
*/
inline void ChildActivationShape::SetDestruction(bool destruction)
{//@CODE_34191
    _destruction = destruction;
}//@CODE_34191



/*@NOTE_35390
Returns the value of member '_merge'.
*/
inline bool ChildActivationShape::GetMerge() const
{//@CODE_35390
    return _merge;
}//@CODE_35390



/*@NOTE_35391
Set the value of member '_merge' to 'merge'.
*/
inline void ChildActivationShape::SetMerge(bool merge)
{//@CODE_35391
    _merge = merge;
}//@CODE_35391



/*@NOTE_34257
Set the value of member '_offset' to 'offset'.
*/
inline void ChildActivationShape::SetOffset(int offset)
{//@CODE_34257
    _offset = offset;
}//@CODE_34257



/*@NOTE_34194
Returns the value of member '_sequenceSubNumber'.
*/
inline int ChildActivationShape::GetSequenceNumber() const
{//@CODE_34194
    return _sequenceNumber;
}//@CODE_34194



/*@NOTE_34195
Set the value of member '_sequenceSubNumber' to 'sequenceNumber'.
*/
inline void ChildActivationShape::SetSequenceNumber(int sequenceNumber)
{//@CODE_34195
    _sequenceNumber = sequenceNumber;
}//@CODE_34195



/*@NOTE_34198
Returns the value of member '_sequenceSubNumber'.
*/
inline int ChildActivationShape::GetSequenceSubNumber() const
{//@CODE_34198
    return _sequenceSubNumber;
}//@CODE_34198



/*@NOTE_34199
Set the value of member '_sequenceSubNumber' to 'sequenceSubNumber'.
*/
inline void ChildActivationShape::SetSequenceSubNumber(int sequenceSubNumber)
{//@CODE_34199
    _sequenceSubNumber = sequenceSubNumber;
}//@CODE_34199



//@START_USER3
//@END_USER3

#endif
#endif
