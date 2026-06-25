// qt/ContextDeclarationDialog.h -- the Qt Context Declarations dialog.
//
// The form lives in ContextDeclarationDialog.ui (Qt Designer). Ported from the
// MFC ContextDeclarationDialog: a list of context declarations + a property
// panel for the selected one. Edits are written to the model as the selection
// changes / on OK (the caller wraps the dialog in MarkLastUndo / RollBack).
#pragma once

#include <QDialog>

class DataModel;
class ContextDeclaration;

namespace Ui { class ContextDeclarationDialog; }

class ContextDeclarationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ContextDeclarationDialog(DataModel* pDataModel,
                                      QWidget* parent = nullptr);
    ~ContextDeclarationDialog();

private:
    void flushPanel();              // panel widgets -> _current
    void loadPanel();               // _current -> panel widgets (+ enable)
    void onSelectionChanged();
    void onAdd();
    void onRemove();
    void onNameEdited(const QString& name);
    void accept() override;

    Ui::ContextDeclarationDialog* _ui;
    DataModel*          _pDataModel;
    ContextDeclaration* _current = nullptr;
};
