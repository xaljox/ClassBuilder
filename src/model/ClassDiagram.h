/******************************************************************************\
*
* Project:       ClassBuilder v3.0
* File:          ClassDiagram.h
* Creation date: August 24, 2026 14:03
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'ClassDiagram'
*
* Modifications: @INSERT_MODIFICATIONS(* )
*
* Copyright 2026, Jimmy Venema
* Licensed under the MIT License (see the LICENSE file).
*
\******************************************************************************/
#ifndef _CLASSDIAGRAM_H
#define _CLASSDIAGRAM_H

//@START_USER1
//@END_USER1



class ClassDiagram
    : public Gti
{
    CB_DECLARE_SERIAL(ClassDiagram)
    RELATION_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramShape, ClassDiagramShape)
    RELATION_MULTI_ACTIVE(ClassDiagram, Hidden, ConnectionShape, Hidden)
    RELATION_NOFILTER_MULTI_OWNED_ACTIVE(ClassDiagram, ClassDiagram, ClassDiagramViewModel, ClassDiagramViewModel)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, ClassDiagram, ClassDiagram)

//@START_USER2
//@END_USER2

// Members
private:
    CbString _name;
    CbString _note;
    bool __notUsed_uml;
    unsigned short _width;
    unsigned short _height;
    int _multiPage;
    unsigned short _scale;
    CbString _caption;
    bool _privateMembers;
    bool _privateMethods;
    bool _protectedMembers;
    bool _protectedMethods;
    bool _publicMembers;
    bool _publicMethods;
    bool _getSetMethods;

protected:

public:

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void SerializeConstructorInclude();

protected:
    ClassDiagram();
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    ClassDiagram(Gti* pGti);
    ClassDiagram(Gti* pGti, ClassDiagram* pClassDiagram);
    virtual ~ClassDiagram();
    virtual void Add();
    static void AddInherit(Inherit* pInherit);
    static void AddRelation(Relation* pRelation);
    virtual bool Drag(bool ctrlKeyDown, Gti*& pGtiDropDefault);
    void Draw(CbPainter& painter, ClassDiagramViewModel* pClassDiagramViewModel);
    virtual void Drop(bool ctrlKeyDown, Gti* pGtiDrop);
    virtual bool DropTarget(bool ctrlKeyDown, Gti* pGtiDrop);
    CbPoint FindFreeShapePlacement();
    CbRect GetBoundingRect();
    ClassDiagramShape* GetHitShape(ClassDiagramViewModel* pClassDiagramViewModel,
                                   CbPoint pointLP, bool nested);
    int GetNumberOfPages();
    void MoveNoteShapePoints(const CbRect& rect, const CbSize& offset);
    void MoveSelectedShapes(ClassDiagramViewModel* pClassDiagramViewModel,
                            const CbSize& offset);
    virtual int OnDelete(bool checkOnly = false);
    virtual int OnEditAttributes(bool checkOnly = false);
    virtual int OnOpen(bool checkOnly = false);
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    void RecalculateDiagram();
    static void RemoveInherit(Inherit* pInherit);
    static void RemoveRelation(Relation* pRelation);
    virtual void Update();
    void UpdateClassDiagramViews();
    const CbString& GetCaption();
    void SetCaption(const CbString& rCaption);
    bool GetGetSetMethods() const;
    void SetGetSetMethods(bool getSetMethods);
    unsigned short GetHeight() const;
    void SetHeight(unsigned short height);
    int GetMultiPage() const;
    void SetMultiPage(int multiPage);
    const CbString& GetName();
    void SetName(const CbString& rName);
    const CbString& GetNote();
    void SetNote(const CbString& rNote);
    bool GetPrivateMembers() const;
    void SetPrivateMembers(bool privateMembers);
    bool GetPrivateMethods() const;
    void SetPrivateMethods(bool privateMethods);
    bool GetProtectedMembers() const;
    void SetProtectedMembers(bool protectedMembers);
    bool GetProtectedMethods() const;
    void SetProtectedMethods(bool protectedMethods);
    bool GetPublicMembers() const;
    void SetPublicMembers(bool publicMembers);
    bool GetPublicMethods() const;
    void SetPublicMethods(bool publicMethods);
    unsigned short GetScale() const;
    void SetScale(unsigned short scale);
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
#ifndef _CLASSDIAGRAM_H_INLINES
#define _CLASSDIAGRAM_H_INLINES

inline int ClassDiagram::GetNumberOfPages()
{//@CODE_11051
    return 1<<_multiPage;
}//@CODE_11051



/*@NOTE_35803
Returns the value of member '_getSetMethods'.
*/
inline bool ClassDiagram::GetGetSetMethods() const
{//@CODE_35803
    return _getSetMethods;
}//@CODE_35803



/*@NOTE_35804
Set the value of member '_getSetMethods' to 'getSetMethods'.
*/
inline void ClassDiagram::SetGetSetMethods(bool getSetMethods)
{//@CODE_35804
    _getSetMethods = getSetMethods;
}//@CODE_35804



/*@NOTE_7983
Returns the value of member '_height'.
*/
inline unsigned short ClassDiagram::GetHeight() const
{//@CODE_7983
    return _height;
}//@CODE_7983



/*@NOTE_7984
Set the value of member '_height' to 'height'.
*/
inline void ClassDiagram::SetHeight(unsigned short height)
{//@CODE_7984
    _height = height;
}//@CODE_7984



/*@NOTE_11048
Returns the value of member '_multiPage'.
*/
inline int ClassDiagram::GetMultiPage() const
{//@CODE_11048
    return _multiPage;
}//@CODE_11048



/*@NOTE_11049
Set the value of member '_multiPage' to 'multiPage'.
*/
inline void ClassDiagram::SetMultiPage(int multiPage)
{//@CODE_11049
    _multiPage = multiPage;
}//@CODE_11049



/*@NOTE_35806
Returns the value of member '_privateMembers'.
*/
inline bool ClassDiagram::GetPrivateMembers() const
{//@CODE_35806
    return _privateMembers;
}//@CODE_35806



/*@NOTE_35807
Set the value of member '_privateMembers' to 'privateMembers'.
*/
inline void ClassDiagram::SetPrivateMembers(bool privateMembers)
{//@CODE_35807
    _privateMembers = privateMembers;
}//@CODE_35807



/*@NOTE_35809
Returns the value of member '_privateMethods'.
*/
inline bool ClassDiagram::GetPrivateMethods() const
{//@CODE_35809
    return _privateMethods;
}//@CODE_35809



/*@NOTE_35810
Set the value of member '_privateMethods' to 'privateMethods'.
*/
inline void ClassDiagram::SetPrivateMethods(bool privateMethods)
{//@CODE_35810
    _privateMethods = privateMethods;
}//@CODE_35810



/*@NOTE_35812
Returns the value of member '_protectedMembers'.
*/
inline bool ClassDiagram::GetProtectedMembers() const
{//@CODE_35812
    return _protectedMembers;
}//@CODE_35812



/*@NOTE_35813
Set the value of member '_protectedMembers' to 'protectedMembers'.
*/
inline void ClassDiagram::SetProtectedMembers(bool protectedMembers)
{//@CODE_35813
    _protectedMembers = protectedMembers;
}//@CODE_35813



/*@NOTE_35815
Returns the value of member '_protectedMethods'.
*/
inline bool ClassDiagram::GetProtectedMethods() const
{//@CODE_35815
    return _protectedMethods;
}//@CODE_35815



/*@NOTE_35816
Set the value of member '_protectedMethods' to 'protectedMethods'.
*/
inline void ClassDiagram::SetProtectedMethods(bool protectedMethods)
{//@CODE_35816
    _protectedMethods = protectedMethods;
}//@CODE_35816



/*@NOTE_35818
Returns the value of member '_publicMembers'.
*/
inline bool ClassDiagram::GetPublicMembers() const
{//@CODE_35818
    return _publicMembers;
}//@CODE_35818



/*@NOTE_35819
Set the value of member '_publicMembers' to 'publicMembers'.
*/
inline void ClassDiagram::SetPublicMembers(bool publicMembers)
{//@CODE_35819
    _publicMembers = publicMembers;
}//@CODE_35819



/*@NOTE_35821
Returns the value of member '_publicMethods'.
*/
inline bool ClassDiagram::GetPublicMethods() const
{//@CODE_35821
    return _publicMethods;
}//@CODE_35821



/*@NOTE_35822
Set the value of member '_publicMethods' to 'publicMethods'.
*/
inline void ClassDiagram::SetPublicMethods(bool publicMethods)
{//@CODE_35822
    _publicMethods = publicMethods;
}//@CODE_35822



/*@NOTE_32514
Returns the value of member '_scale'.
*/
inline unsigned short ClassDiagram::GetScale() const
{//@CODE_32514
    return _scale;
}//@CODE_32514



/*@NOTE_32515
Set the value of member '_scale' to 'scale'.
*/
inline void ClassDiagram::SetScale(unsigned short scale)
{//@CODE_32515
    _scale = scale;
}//@CODE_32515



/*@NOTE_7979
Returns the value of member '_width'.
*/
inline unsigned short ClassDiagram::GetWidth() const
{//@CODE_7979
    return _width;
}//@CODE_7979



/*@NOTE_7980
Set the value of member '_width' to 'width'.
*/
inline void ClassDiagram::SetWidth(unsigned short width)
{//@CODE_7980
    _width = width;
}//@CODE_7980



//@START_USER3
//@END_USER3

#endif
#endif
