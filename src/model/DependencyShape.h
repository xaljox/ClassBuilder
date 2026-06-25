/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DependencyShape.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DependencyShape'
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
#ifndef _DEPENDENCYSHAPE_H
#define _DEPENDENCYSHAPE_H

//@START_USER1
//@END_USER1



class DependencyShape
    : public ConnectionShape
{
    CB_DECLARE_SERIAL(DependencyShape)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _stereotype;
    CbPoint _stereotypePoint;
    CbString _name;
    CbPoint _namePoint;

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    DependencyShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DependencyShape(ClassDiagram* pClassDiagram, ClassShape* pFromClassShape,
                    ClassShape* pToClassShape);
    DependencyShape(DependencyShape* pDependencyShape);
    DependencyShape(ClassDiagram* pClassDiagram,
                    DependencyShape* pDependencyShape);
    virtual ~DependencyShape();
    void ConvertRouting();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual DependencyShape* GetDependencyShape();
    CbString GetStereotypeString();
    virtual void MakeNewRouting();
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual bool PointInShape(ClassDiagramViewModel* pClassDiagramViewModel,
                              CbPoint pointLP);
    const CbString& GetName() const;
    void SetName(const CbString& rName);
    const CbPoint& GetNamePoint() const;
    void SetNamePoint(const CbPoint& rNamePoint);
    const CbString& GetStereotype() const;
    void SetStereotype(const CbString& rStereotype);
    const CbPoint& GetStereotypePoint() const;
    void SetStereotypePoint(const CbPoint& rStereotypePoint);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _DEPENDENCYSHAPE_H_INLINES
#define _DEPENDENCYSHAPE_H_INLINES

/*@NOTE_23393
Returns the value of member '_name'.
*/
inline const CbString& DependencyShape::GetName() const
{//@CODE_23393
    return _name;
}//@CODE_23393



/*@NOTE_23394
Set the value of member '_name' to 'rName'.
*/
inline void DependencyShape::SetName(const CbString& rName)
{//@CODE_23394
    _name = rName;
}//@CODE_23394



/*@NOTE_23396
Returns the value of member '_namePoint'.
*/
inline const CbPoint& DependencyShape::GetNamePoint() const
{//@CODE_23396
    return _namePoint;
}//@CODE_23396



/*@NOTE_23397
Set the value of member '_namePoint' to 'rNamePoint'.
*/
inline void DependencyShape::SetNamePoint(const CbPoint& rNamePoint)
{//@CODE_23397
    _namePoint = rNamePoint;
}//@CODE_23397



/*@NOTE_23387
Returns the value of member '_stereotype'.
*/
inline const CbString& DependencyShape::GetStereotype() const
{//@CODE_23387
    return _stereotype;
}//@CODE_23387



/*@NOTE_23388
Set the value of member '_stereotype' to 'rStereotype'.
*/
inline void DependencyShape::SetStereotype(const CbString& rStereotype)
{//@CODE_23388
    _stereotype = rStereotype;
}//@CODE_23388



/*@NOTE_23390
Returns the value of member '_stereotypePoint'.
*/
inline const CbPoint& DependencyShape::GetStereotypePoint() const
{//@CODE_23390
    return _stereotypePoint;
}//@CODE_23390



/*@NOTE_23391
Set the value of member '_stereotypePoint' to 'rStereotypePoint'.
*/
inline void DependencyShape::SetStereotypePoint(const CbPoint& rStereotypePoint)
{//@CODE_23391
    _stereotypePoint = rStereotypePoint;
}//@CODE_23391



//@START_USER3
//@END_USER3

#endif
#endif
