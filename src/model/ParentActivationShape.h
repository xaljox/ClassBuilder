/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          ParentActivationShape.h
* Creation date: August 14, 2026 17:48
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ParentActivationShape'
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
#ifndef _PARENTACTIVATIONSHAPE_H
#define _PARENTACTIVATIONSHAPE_H

//@START_USER1
#include <vector>
#include <utility>
//@END_USER1



class ParentActivationShape
    : public SequenceDiagramShape
{
    RELATION_NOFILTER_MULTI_OWNED_ACTIVE(ParentActivationShape, ParentActivationShape, ChildActivationShape, ChildActivationShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude();
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ParentActivationShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ParentActivationShape(SequenceDiagram* pSequenceDiagram,
                          CbColorRef penColor = Cb_RGB(0, 0, 0),
                          CbColorRef textColor = Cb_RGB(0, 0, 0));
    virtual ~ParentActivationShape();
    virtual void CopyShape(SequenceDiagram* pSequenceDiagram);
    unsigned short GetHeight();
    virtual LifeLineShape* GetLifeLineShape() const;
    virtual CbString GetNumbering();
    virtual int GetOffset() const;
    virtual ParentActivationShape* GetParentActivationShape() const;
    bool IsDirectOrIndirectChild(ChildActivationShape* pChildActivationShape);
    virtual bool IsRecursiveActivation();
    virtual bool NeedExtraSpaceAfter();
    virtual bool NeedExtraSpaceBefore();
    virtual int OnEditAttributes(bool checkOnly = false) = 0;
    virtual int OnOpen(bool checkOnly = false) = 0;
    int RecalculateRect(int start, int& sequenceNumber,
                        std::vector<std::pair<CbRect,CbRect>>* moved = nullptr);
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _PARENTACTIVATIONSHAPE_H_INLINES
#define _PARENTACTIVATIONSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
