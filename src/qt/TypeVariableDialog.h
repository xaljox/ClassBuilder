// qt/TypeVariableDialog.h -- the Qt "Type Variable Wizard" dialog.
//
// The form lives in TypeVariableDialog.ui (Qt Designer). Builds a variable-
// declaration snippet (the model's types feed the combo); the caller inserts
// the result into the code being edited.
#pragma once

#include <QDialog>
#include <QString>

class DataModelDoc;

namespace Ui { class TypeVariableDialog; }

class TypeVariableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TypeVariableDialog(DataModelDoc* pDataModelDoc,
                                QWidget* parent = nullptr);
    ~TypeVariableDialog();

    // Valid after exec() returns Accepted: the declaration snippet to insert.
    QString insertCode() const { return _insertCode; }

private slots:
    void autoFillName();             // suggest a name from type + ptr/ref
    void onArrayToggled();           // Array gates the array-size field

private:
    void accept() override;          // validate, build the snippet

    Ui::TypeVariableDialog* _ui;
    DataModelDoc*           _pDataModelDoc;
    QString                 _insertCode;
};
