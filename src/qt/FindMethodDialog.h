// qt/FindMethodDialog.h -- the Qt Find Method dialog.
//
// The form lives in FindMethodDialog.ui (Qt Designer). The first ported
// tree dialog: a QTreeWidget of the relation target's members / relations /
// bases feeding an argument-map list, plus the method's own attributes.
// Drives the model directly. Ported from the MFC CFindMethodDialog.
#pragma once

#include <QDialog>
#include <QString>

class FindMethod;
class Class;
class Member;
class Gti;
class QTreeWidget;
class QTreeWidgetItem;

namespace Ui { class FindMethodDialog; }

class FindMethodDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FindMethodDialog(FindMethod* pFindMethod,
                              QWidget* parent = nullptr);
    ~FindMethodDialog();

    // Set on OK: true when an attribute actually changed (see the bridge).
    bool fieldsChanged() const { return _fieldsChanged; }

private:
    void fillTree(Class* pClass, QTreeWidgetItem* parent, int access = 3);
    QTreeWidgetItem* findTreeItem(QTreeWidgetItem* parent, const QString& text);
    void fillList();
    bool isMemberKey(Member* pMember) const;

    void onAdd();
    void onDeleteMap();
    void onTreeSelChanged();
    void accept() override;
    bool applyFieldChanges();          // the MFC CFindMethodDialog::Update

    Ui::FindMethodDialog* _ui;
    FindMethod*           _pFindMethod;
    Gti*                  _pGti       = nullptr;   // selected tree node
    Member*               _pMemberKey = nullptr;
    QString               _argumentMap;            // path of the selected node
    bool                  _argumentMapChanged = false;
    bool                  _fieldsChanged      = false;
};
