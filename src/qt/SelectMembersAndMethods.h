// qt/SelectMembersAndMethods.h -- the Qt "select members/methods" dialog.
//
// The form lives in SelectMembersAndMethods.ui (Qt Designer). Ported from the
// MFC SelectMembersAndMethods: a checkbox tree -- each class shape in a class
// diagram with its members and methods beneath -- controlling which member/
// method shapes appear on the class shape. Nine filter checkboxes narrow what
// the tree shows. Drives the model directly; Cancel rolls back to the undo
// point captured at construction.
#pragma once

#include <QDialog>

#include "QtSelectMembersAndMethods.h"   // SelectMembersApplyCallback

class ClassDiagram;
class ClassShape;
class UndoBase;
class QTreeWidgetItem;

namespace Ui { class SelectMembersAndMethods; }

class SelectMembersAndMethods : public QDialog
{
    Q_OBJECT
public:
    explicit SelectMembersAndMethods(ClassDiagram* pClassDiagram,
                                     SelectMembersApplyCallback onApply,
                                     void* applyContext,
                                     QWidget* parent = nullptr);
    ~SelectMembersAndMethods();

private:
    void fillTree();
    void applyChecks();
    void checkItem(QTreeWidgetItem* item, ClassShape* pClassShape);
    void onFilterChanged();
    void accept() override;
    void reject() override;

    Ui::SelectMembersAndMethods* _ui;
    ClassDiagram* _pClassDiagram;
    UndoBase*     _pUndoBase;
    SelectMembersApplyCallback _onApply;
    void*                      _applyContext;

    // Filter flags -- persist across invocations (the MFC statics).
    static bool _privateMembers;
    static bool _protectedMembers;
    static bool _publicMembers;
    static bool _privateMethods;
    static bool _protectedMethods;
    static bool _publicMethods;
    static bool _classBuilderMethods;
    static bool _relationMethods;
    static bool _getSetMethods;
};
