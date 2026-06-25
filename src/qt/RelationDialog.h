// qt/RelationDialog.h -- the Qt Relation attributes dialog.
//
// The form lives in RelationDialog.ui (Qt Designer). Ported from the MFC
// CRelationDialog: from/to class+name, association type (single/multi/
// static-multi), aggregation/critical/filter, the container implementation
// (standard / value-tree / avl-tree) and a note. Non-live -- read on OK
// (applyFieldChanges, the MFC ::Update). Drives the model directly.
#pragma once

#include <QDialog>

class Relation;
class BaseClass;
class Member;

namespace Ui { class RelationDialog; }

class RelationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RelationDialog(Relation* pRelation, QWidget* parent = nullptr);
    ~RelationDialog();

    bool fieldsChanged() const { return _changed; }

private:
    void fillMemberList(BaseClass* pBaseClass, bool first = true);
    void onSelchangeFromClass();
    void onSelchangeToClass();
    void onCheckTemplate();
    void onMulti();
    void onImplementation();
    void onSelchangeMember();
    int  associationType() const;   // 0 single, 1 multi, 2 static-multi
    int  implementation() const;    // 0 standard, 1 value-tree, 2 avl-tree
    void accept() override;
    bool applyFieldChanges();       // the MFC CRelationDialog::Update

    Ui::RelationDialog* _ui;
    Relation* _pRelation;
    Member*   _pMemberOrg = nullptr;
    Member*   _pMember = nullptr;
    bool      _uniqueValueTreeOrg = false;
    bool      _changed = false;
};
