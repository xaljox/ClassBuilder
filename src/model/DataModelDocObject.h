/******************************************************************************\
*
* Project:       ClassBuilder v2.3
* File:          DataModelDocObject.h
* Creation date: July 15, 2026 21:08
* Author:        Jimmy Venema
* Purpose:       Declaration of class 'DataModelDocObject'
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
#ifndef _DATAMODELDOCOBJECT_H
#define _DATAMODELDOCOBJECT_H

//@START_USER1
//@END_USER1



class DataModelDocObject
    : public CbObject
{
    CB_DECLARE_SERIAL(DataModelDocObject)
    RELATION_NOFILTER_AVLTREE_OWNED_ACTIVE(DataModelDocObject, DataModelDocObject, Property, Property)
    RELATION_MULTI_OWNED_PASSIVE(DataModelDoc, DataModelDoc, DataModelDocObject, DataModelDocObject)

//@START_USER2
//@END_USER2

    friend class DataModelDoc;

// Members
private:
    unsigned int _id;

protected:
    static int _objectVersion;

public:
    union {
        int      _index;
        intptr_t _ptrIndex;
    };

// Methods
private:
    void ConstructorInclude(DataModelDoc* pDataModelDoc);
    void DestructorInclude();
    void ReplaceConstructorInclude(DataModelDocObject* pOld);
    void SerializeConstructorInclude();

protected:
    DataModelDocObject();
    int FindStringInStr(CbString& str, const CbString& string, int offset = 0);
    virtual void SerializeRelations(CbArchive& archive,
                                    DataModelDocObject* pointerArray[]);

public:
    DataModelDocObject(DataModelDoc* pDataModelDoc);
    DataModelDocObject(DataModelDocObject* pOld);
    DataModelDocObject(const DataModelDocObject& rDataModelDocObject) = delete;
    virtual ~DataModelDocObject();
    void CopyState(DataModelDocObject* pDataModelDocObject);
    virtual void Delete();
    void Delete(DataModelDoc* pDataModelDoc);
    virtual const CbString& GetCaption();
    virtual CbString GetContextList();
    virtual DataModelDocObject* GetDataModelDocObject() const;
    CbString GetHtmlNote();
    CbString GetIdAsString();
    CbString GetIndent(int indentLevel = 1);
    virtual const CbString& GetNote();
    virtual void OnUndoRedoAdded();
    virtual void OnUndoRedoChanged(DataModelDocObject* pOldState);
    virtual void OnUndoRedoChanging(DataModelDocObject* pNewState);
    virtual void OnUndoRedoRemoved();
    virtual void OnUndoRedoRemoving();
    DataModelDocObject& operator =(const DataModelDocObject& rDataModelDocObject) = delete;
    int ReplaceInStr(CbString& str, const CbString& oldString,
                     const CbString& newString, bool saveState = true);
    virtual void ReplaceInX(const CbString& oldString,
                            const CbString& newString);
    virtual void ReplaceReference(ConnectionSegment* pOld,
                                  ConnectionSegment* pNew);
    void SaveState(int always = 0);
    virtual void SetCaption(const CbString& rCaption);
    virtual void SetNote(const CbString& rNote);
    bool TouchesCd() const;
    bool TouchesSd() const;
    bool TouchesTree() const;
    unsigned int GetId();
    virtual void CleanupReferences();
    Property* FindEqualOrBiggerProperty(const CbString& name);
    Property* FindEqualOrSmallerProperty(const CbString& name);
    Property* FindProperty(const CbString& name);
    Property* FindReverseProperty(const CbString& name);
    bool IsProperty() const;
    bool IsPropertyInteger() const;
    bool IsPropertyReal() const;
    bool IsPropertyString() const;
    virtual void RemoveReferences();
    virtual void RestoreReferences(DataModelDocObject* pDataModelDocObject);
    virtual void SaveReferences(DataModelDocObject* pDataModelDocObject);
    virtual void Serialize(CbArchive& archive);
};

#endif


#ifdef CB_INLINES
#ifndef _DATAMODELDOCOBJECT_H_INLINES
#define _DATAMODELDOCOBJECT_H_INLINES

//@START_USER3
//@END_USER3

#endif
#endif
