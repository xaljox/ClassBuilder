// qt/ContextDialog.h -- the Qt "Assign Context" dialog.
//
// The form lives in ContextDialog.ui (Qt Designer). A two-list transfer
// dialog: assigned contexts on the left, unassigned context declarations on
// the right, Add/Remove buttons move items between them. Drives the model
// directly -- each Add/Remove mutates the model live (the caller's
// MarkLastUndo / RollBack handles Cancel).
#pragma once

#include <QDialog>

class Gti;

namespace Ui { class ContextDialog; }

class ContextDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ContextDialog(Gti* pGti, QWidget* parent = nullptr);
    ~ContextDialog();

private slots:
    void onAdd();      // unassigned -> assigned (CreateContext)
    void onRemove();   // assigned -> unassigned (Context::Delete)

private:
    Ui::ContextDialog* _ui;
    Gti*               _pGti;
};
