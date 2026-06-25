// qt/TypeDialog.h -- the Qt Type (OtherType) properties dialog.
//
// The form lives in TypeDialog.ui (Qt Designer). Drives the model directly:
// handed the live OtherType*, reads it into the controls, writes back on OK.
#pragma once

#include <QDialog>

class OtherType;

namespace Ui { class TypeDialog; }

class TypeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TypeDialog(OtherType* pOtherType, QWidget* parent = nullptr);
    ~TypeDialog();

    // Valid after exec() returns Accepted.
    bool modelChanged() const { return _modelChanged; }

private:
    void accept() override;          // validate, then write the model

    Ui::TypeDialog* _ui;
    OtherType*      _pOtherType;
    bool            _modelChanged = false;
};
