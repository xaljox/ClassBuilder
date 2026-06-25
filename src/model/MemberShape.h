/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          MemberShape.h
* Creation date: June 25, 2026 12:36
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'MemberShape'
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
#ifndef _MEMBERSHAPE_H
#define _MEMBERSHAPE_H

//@START_USER1
//@END_USER1



class MemberShape
    : public ClassDiagramShape
{
    CB_DECLARE_SERIAL(MemberShape)
    RELATION_MULTI_OWNED_PASSIVE(Member, Member, MemberShape, MemberShape)
    RELATION_MULTI_OWNED_PASSIVE(ClassShape, ClassShape, MemberShape, MemberShape)

//@START_USER2
//@END_USER2

// Members
private:

protected:

public:

// Methods
private:
    void ConstructorInclude(Member* pMember, ClassShape* pClassShape);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    MemberShape();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    MemberShape(ClassShape* pClassShape, Member* pMember);
    virtual ~MemberShape();
    virtual void CopyShape(ClassDiagram* pClassDiagram);
    virtual void Draw(CbPainter& painter,
                      ClassDiagramViewModel* pClassDiagramViewModel,
                      bool selected);
    virtual Gti* GetGti();
    virtual MemberShape* GetMemberShape();
    virtual ClassDiagramShape* GetOuterClassDiagramShape();
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int UsesPenColor() const;
    virtual void CleanupReferences();
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _MEMBERSHAPE_H_INLINES
#define _MEMBERSHAPE_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
