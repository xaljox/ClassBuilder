/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          LifeLineShape.h
* Creation date: August 14, 2026 20:37
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'LifeLineShape'
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
#ifndef _LIFELINESHAPE_H
#define _LIFELINESHAPE_H

//@START_USER1
//@END_USER1



class LifeLineShape
    : public SequenceDiagramShape
{
    RELATION_MULTI_OWNED_ACTIVE(LifeLineShape, LifeLineShape, ChildActivationShape, ChildActivationShape)
    RELATION_MULTI_OWNED_PASSIVE(SequenceDiagram, SequenceDiagram, LifeLineShape, LifeLineShape)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;
    CbString _note;
    static bool _tracking;
    bool _showActivations;
    int _order;
    float _orderWeight;

protected:

public:

// Methods
private:
    void ConstructorInclude(SequenceDiagram* pSequenceDiagram);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    LifeLineShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    LifeLineShape(SequenceDiagram* pSequenceDiagram, const CbPoint& point);
    virtual ~LifeLineShape();
    static int Compare(LifeLineShape* pLifeLineShape1,
                       LifeLineShape* pLifeLineShape2);
    static int CompareOrderWeight(LifeLineShape* pLifeLineShape1,
                                  LifeLineShape* pLifeLineShape2);
    virtual void Draw(CbPainter& painter,
                      SequenceDiagramViewModel* pSequenceDiagramViewModel,
                      bool selected) = 0;
    virtual BaseClass* GetBaseClass() const;
    ChildActivationShape* GetBottomChildActivationShape();
    Class* GetClass() const;
    ChildActivationShape* GetDestructionChildActivationShape();
    CbPoint GetEndPoint();
    virtual int GetLeftActivation();
    virtual LifeLineShape* GetLifeLine();
    virtual int GetLifeLineLength();
    CbRect GetLifeLineRect();
    CbPoint GetStartPoint();
    virtual CbString GetTypeName() = 0;
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void SetRect(const CbRect& rRect);
    void SetRectNoSort(const CbRect& rRect);
    const CbString& GetName() const;
    void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    int GetOrder() const;
    void SetOrder(int order);
    float GetOrderWeight() const;
    void SetOrderWeight(float orderWeight);
    bool GetShowActivations() const;
    void SetShowActivations(bool showActivations);
    static bool GetTracking();
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _LIFELINESHAPE_H_INLINES
#define _LIFELINESHAPE_H_INLINES

/*@NOTE_33528
Returns the value of member '_name'.
*/
inline const CbString& LifeLineShape::GetName() const
{//@CODE_33528
    return _name;
}//@CODE_33528



/*@NOTE_33529
Set the value of member '_name' to 'rName'.
*/
inline void LifeLineShape::SetName(const CbString& rName)
{//@CODE_33529
    _name = rName;
}//@CODE_33529



/*@NOTE_38571
Returns the value of member '_order'.
*/
inline int LifeLineShape::GetOrder() const
{//@CODE_38571
    return _order;
}//@CODE_38571



/*@NOTE_38572
Set the value of member '_order' to 'order'.
*/
inline void LifeLineShape::SetOrder(int order)
{//@CODE_38572
    _order = order;
}//@CODE_38572



/*@NOTE_38578
Returns the value of member '_orderWeight'.
*/
inline float LifeLineShape::GetOrderWeight() const
{//@CODE_38578
    return _orderWeight;
}//@CODE_38578



/*@NOTE_38579
Set the value of member '_orderWeight' to 'orderWeight'.
*/
inline void LifeLineShape::SetOrderWeight(float orderWeight)
{//@CODE_38579
    _orderWeight = orderWeight;
}//@CODE_38579



/*@NOTE_35396
Returns the value of member '_showActivations'.
*/
inline bool LifeLineShape::GetShowActivations() const
{//@CODE_35396
    return _showActivations;
}//@CODE_35396



/*@NOTE_35397
Set the value of member '_showActivations' to 'showActivations'.
*/
inline void LifeLineShape::SetShowActivations(bool showActivations)
{//@CODE_35397
    _showActivations = showActivations;
}//@CODE_35397



/*@NOTE_34141
Returns the value of member '_tracking'.
*/
inline bool LifeLineShape::GetTracking()
{//@CODE_34141
    return _tracking;
}//@CODE_34141



//@START_USER3
//@END_USER3

#endif
#endif
