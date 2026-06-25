/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          SignalShape.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'SignalShape'
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
#ifndef _SIGNALSHAPE_H
#define _SIGNALSHAPE_H

//@START_USER1
//@END_USER1



class SignalShape
    : public SequenceDiagramShape
{
    CB_DECLARE_SERIAL(SignalShape)
    RELATION_SINGLE_OWNED_PASSIVE(ChildActivationShape, Receiver, SignalShape, Sender)
    RELATION_MULTI_OWNED_PASSIVE(ChildActivationShape, Sender, SignalShape, Receiver)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;
    CbString _clause;
    CbString _label;
    CbSize _nameOffset;
    CbSize _labelOffset;
    bool _scope;
    bool _arguments;
    CbString _note;
    CbColorRef _signalNoMethodPenColor;
    bool _enableReturn;
    CbString _return;
    CbRect _returnActiveAreaRect;
    bool _async;
    CbSize _returnOffset;
    CbRect _activeAreaRect;
    bool _argumentNames;
    int _duration;
    static CbPainter* _measurePainter;

protected:

public:

// Methods
private:
    void DrawArrow(CbPainter& painter, CbColorRef color, const CbPoint& start,
                   const CbPoint& end, bool returnArrow = false);
    void DrawLabel(CbPainter& painter);
    CbColorRef GetPenColor(CbPainter& painter);
    void ConstructorInclude(ChildActivationShape* pReceiver,
                            ChildActivationShape* pSender);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    SignalShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    SignalShape(ChildActivationShape* pReceiver, ChildActivationShape* pSender);
    virtual ~SignalShape();
    void AddContribution(CbPainter& painter);
    void AddContribution();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected);
    virtual ChildActivationShape* GetChildActivation();
    CbPoint GetEndPoint();
    CbPoint GetLabelPoint();
    CbRect GetLabelRect(CbPainter& painter, bool draw = false);
    CbPoint GetNamePoint();
    CbRect GetNameRect(CbPainter& painter, bool draw = false);
    virtual CbColorRef GetPenColor() const;
    CbPoint GetReferencePoint();
    int GetRequiredLifelineDistance(CbPainter& painter);
    CbPoint GetReturnEndPoint();
    CbPoint GetReturnPoint();
    CbRect GetReturnRect(CbPainter& painter, bool draw = false);
    CbPoint GetReturnReferencePoint();
    CbPoint GetReturnStartPoint();
    virtual SignalShape* GetSignal();
    CbString GetSignalName();
    CbPoint GetStartPoint();
    bool IsRecursiveActivation();
    bool IsReversed();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual bool PointInShape(SequenceDiagramViewModel* pSequenceDiagramViewModel,
                              const CbPoint& pointLP);
    void SetLabelPoint(const CbPoint& point);
    void SetNamePoint(const CbPoint& point);
    virtual void SetPenColor(CbColorRef penColor);
    void SetReturnPoint(const CbPoint& point);
    const CbRect& GetActiveAreaRect() const;
    void SetActiveAreaRect(const CbRect& rActiveAreaRect);
    bool GetArgumentNames() const;
    void SetArgumentNames(bool argumentNames);
    bool GetArguments() const;
    void SetArguments(bool arguments);
    bool GetAsync() const;
    void SetAsync(bool async);
    const CbString& GetClause() const;
    void SetClause(const CbString& rClause);
    int GetDuration() const;
    void SetDuration(int duration);
    bool GetEnableReturn() const;
    void SetEnableReturn(bool enableReturn);
    const CbString& GetLabel() const;
    void SetLabel(const CbString& rLabel);
    const CbSize& GetLabelOffset() const;
    void SetLabelOffset(const CbSize& rLabelOffset);
    static CbPainter* GetMeasurePainter();
    static void SetMeasurePainter(CbPainter* pMeasurePainter);
    const CbString& GetName();
    void SetName(const CbString& rName);
    const CbSize& GetNameOffset() const;
    void SetNameOffset(const CbSize& rNameOffset);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    const CbString& GetReturn() const;
    void SetReturn(const CbString& rReturn);
    const CbRect& GetReturnActiveAreaRect() const;
    void SetReturnActiveAreaRect(const CbRect& rReturnActiveAreaRect);
    const CbSize& GetReturnOffset() const;
    void SetReturnOffset(const CbSize& rReturnOffset);
    bool GetScope() const;
    void SetScope(bool scope);
    CbColorRef GetSignalNoMethodPenColor() const;
    void SetSignalNoMethodPenColor(CbColorRef signalNoMethodPenColor);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _SIGNALSHAPE_H_INLINES
#define _SIGNALSHAPE_H_INLINES

/*@NOTE_34797
Returns the value of member '_activeAreaRect'.
*/
inline const CbRect& SignalShape::GetActiveAreaRect() const
{//@CODE_34797
    return _activeAreaRect;
}//@CODE_34797



/*@NOTE_35066
Returns the value of member '_argumentNames'.
*/
inline bool SignalShape::GetArgumentNames() const
{//@CODE_35066
    return _argumentNames;
}//@CODE_35066



/*@NOTE_35067
Set the value of member '_argumentNames' to 'argumentNames'.
*/
inline void SignalShape::SetArgumentNames(bool argumentNames)
{//@CODE_35067
    _argumentNames = argumentNames;
}//@CODE_35067



/*@NOTE_34231
Returns the value of member '_arguments'.
*/
inline bool SignalShape::GetArguments() const
{//@CODE_34231
    return _arguments;
}//@CODE_34231



/*@NOTE_34232
Set the value of member '_arguments' to 'arguments'.
*/
inline void SignalShape::SetArguments(bool arguments)
{//@CODE_34232
    _arguments = arguments;
}//@CODE_34232



/*@NOTE_34426
Returns the value of member '_async'.
*/
inline bool SignalShape::GetAsync() const
{//@CODE_34426
    return _async;
}//@CODE_34426



/*@NOTE_34427
Set the value of member '_async' to 'async'.
*/
inline void SignalShape::SetAsync(bool async)
{//@CODE_34427
    _async = async;
}//@CODE_34427



/*@NOTE_33613
Returns the value of member '_clause'.
*/
inline const CbString& SignalShape::GetClause() const
{//@CODE_33613
    return _clause;
}//@CODE_33613



/*@NOTE_33614
Set the value of member '_clause' to 'rClause'.
*/
inline void SignalShape::SetClause(const CbString& rClause)
{//@CODE_33614
    _clause = rClause;
}//@CODE_33614



/*@NOTE_35400
Returns the value of member '_duration'.
*/
inline int SignalShape::GetDuration() const
{//@CODE_35400
    return _duration;
}//@CODE_35400



/*@NOTE_35401
Set the value of member '_duration' to 'duration'.
*/
inline void SignalShape::SetDuration(int duration)
{//@CODE_35401
    if (duration < 0)
    {
        _duration = 0;
    }
    else
    {
        _duration = duration;
    }
}//@CODE_35401



/*@NOTE_34415
Returns the value of member '_enableReturn'.
*/
inline bool SignalShape::GetEnableReturn() const
{//@CODE_34415
    return _enableReturn;
}//@CODE_34415



/*@NOTE_34416
Set the value of member '_enableReturn' to 'enableReturn'.
*/
inline void SignalShape::SetEnableReturn(bool enableReturn)
{//@CODE_34416
    _enableReturn = enableReturn;
}//@CODE_34416



/*@NOTE_34238
Returns the value of member '_label'.
*/
inline const CbString& SignalShape::GetLabel() const
{//@CODE_34238
    return _label;
}//@CODE_34238



/*@NOTE_34239
Set the value of member '_label' to 'rLabel'.
*/
inline void SignalShape::SetLabel(const CbString& rLabel)
{//@CODE_34239
    _label = rLabel;
}//@CODE_34239



/*@NOTE_38308
Returns the value of member '_labelOffset'.
*/
inline const CbSize& SignalShape::GetLabelOffset() const
{//@CODE_38308
    return _labelOffset;
}//@CODE_38308



/*@NOTE_38309
Set the value of member '_labelOffset' to 'rLabelOffset'.
*/
inline void SignalShape::SetLabelOffset(const CbSize& rLabelOffset)
{//@CODE_38309
    _labelOffset = rLabelOffset;
}//@CODE_38309



/*@NOTE_39836
Returns the value of member '_measurePainter'.
*/
inline CbPainter* SignalShape::GetMeasurePainter()
{//@CODE_39836
    return _measurePainter;
}//@CODE_39836



/*@NOTE_39837
Set the value of member '_measurePainter' to 'pMeasurePainter'.
*/
inline void SignalShape::SetMeasurePainter(CbPainter* pMeasurePainter)
{//@CODE_39837
    _measurePainter = pMeasurePainter;
}//@CODE_39837



/*@NOTE_33609
Returns the value of member '_name'.
*/
inline const CbString& SignalShape::GetName()
{//@CODE_33609
    if (GetReceiver()->GetMethod())
    {
        CbString name = GetReceiver()->GetMethod()->GetSignalShapeText(
            GetArguments(), GetArgumentNames(), GetScope());
        if (_name != name)
        {
            SaveState(1);
            _name = name;
        }
    }
    
    return _name;
}//@CODE_33609



/*@NOTE_33610
Set the value of member '_name' to 'rName'.
*/
inline void SignalShape::SetName(const CbString& rName)
{//@CODE_33610
    _name = rName;
}//@CODE_33610



/*@NOTE_38305
Returns the value of member '_nameOffset'.
*/
inline const CbSize& SignalShape::GetNameOffset() const
{//@CODE_38305
    return _nameOffset;
}//@CODE_38305



/*@NOTE_38306
Set the value of member '_nameOffset' to 'rNameOffset'.
*/
inline void SignalShape::SetNameOffset(const CbSize& rNameOffset)
{//@CODE_38306
    _nameOffset = rNameOffset;
}//@CODE_38306



/*@NOTE_34419
Returns the value of member '_return'.
*/
inline const CbString& SignalShape::GetReturn() const
{//@CODE_34419
    return _return;
}//@CODE_34419



/*@NOTE_34420
Set the value of member '_return' to 'rReturn'.
*/
inline void SignalShape::SetReturn(const CbString& rReturn)
{//@CODE_34420
    _return = rReturn;
}//@CODE_34420



/*@NOTE_34793
Returns the value of member '_returnActiveAreaRect'.
*/
inline const CbRect& SignalShape::GetReturnActiveAreaRect() const
{//@CODE_34793
    return _returnActiveAreaRect;
}//@CODE_34793



/*@NOTE_34430
Returns the value of member '_returnOffset'.
*/
inline const CbSize& SignalShape::GetReturnOffset() const
{//@CODE_34430
    return _returnOffset;
}//@CODE_34430



/*@NOTE_34431
Set the value of member '_returnOffset' to 'rReturnOffset'.
*/
inline void SignalShape::SetReturnOffset(const CbSize& rReturnOffset)
{//@CODE_34431
    _returnOffset = rReturnOffset;
}//@CODE_34431



/*@NOTE_34212
Returns the value of member '_scope'.
*/
inline bool SignalShape::GetScope() const
{//@CODE_34212
    return _scope;
}//@CODE_34212



/*@NOTE_34213
Set the value of member '_scope' to 'scope'.
*/
inline void SignalShape::SetScope(bool scope)
{//@CODE_34213
    _scope = scope;
}//@CODE_34213



/*@NOTE_34398
Returns the value of member '_signalNoMethodPenColor'.
*/
inline CbColorRef SignalShape::GetSignalNoMethodPenColor() const
{//@CODE_34398
    return _signalNoMethodPenColor;
}//@CODE_34398



/*@NOTE_34399
Set the value of member '_signalNoMethodPenColor' to 'signalNoMethodPenColor'.
*/
inline void SignalShape::SetSignalNoMethodPenColor(CbColorRef signalNoMethodPenColor)
{//@CODE_34399
    _signalNoMethodPenColor = signalNoMethodPenColor;
}//@CODE_34399



//@START_USER3
//@END_USER3

#endif
#endif
